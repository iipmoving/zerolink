/**
 * MODBUS Master Client
 *
 * Simulates the behavior of m4_modbus_tool.py in pure JS.
 * Three operations: init, send_power, read_back.
 * Matches the byte-level protocol verified against Python tool output.
 */

var MB = (typeof require !== 'undefined') ? require('./modbus_rtu.js') : window.MB;

/**
 * MODBUS master for one slave channel.
 */
function ModbusMaster(slave_addr) {
    this.addr = slave_addr;
    this.log = [];  // transaction log
}

ModbusMaster.prototype._tx = function (frame, label) {
    var entry = {
        label: label,
        tx: MB.hexDump(frame),
        tx_bytes: Array.from(frame),
        rx: null,
        rx_bytes: null,
        ok: false,
        err: null
    };
    this.log.push(entry);
    return entry;
};

ModbusMaster.prototype._rx = function (entry, rx_frame) {
    if (rx_frame) {
        entry.rx = MB.hexDump(rx_frame);
        entry.rx_bytes = Array.from(rx_frame);
    }
    return entry;
};

// --- Operation 1: INIT ---
//
// Step 1: FC10 write 5 zero registers to 0x200E (triggers I2C init)
// Step 2: FC03 read 0x1000 count=1, poll until bit7 set (init complete)
// Returns: { ok, steps, sys_sta }

ModbusMaster.prototype.init = function (slave, opts) {
    opts = opts || {};
    var max_retries = opts.max_retries || 20;
    var poll_delay   = opts.poll_delay || 10;  // ms (simulated)

    var result = { ok: false, steps: [], sys_sta: 0 };
    var self = this;

    // Step 1: Send FC10 with 5 zero registers at 0x200E
    var zero_data = new Uint8Array(10);  // 5 registers × 2 bytes = 10 zeros
    var fc10_frame = MB.build_FC10(this.addr, 0x200E, zero_data);
    var entry1 = this._tx(fc10_frame, 'INIT: FC10 write zeros to 0x200E');
    result.steps.push(entry1);

    var resp1 = slave.handle_request(fc10_frame);
    this._rx(entry1, resp1);
    if (!resp1) { result.err = 'no_response'; return result; }

    var parsed1 = MB.parse_response(resp1);
    if (!parsed1.ok || parsed1.func !== 0x10) {
        entry1.ok = false;
        entry1.err = 'bad_response';
        result.err = 'init_fc10_failed';
        return result;
    }
    entry1.ok = true;

    // Step 2: Poll SYS_STA (0x1000) until bit7 = 1
    for (var i = 0; i < max_retries; i++) {
        // Advance slave time
        slave.tick();

        var fc03_frame = MB.build_FC03(this.addr, 0x1000, 1);
        var label = 'INIT: poll SYS_STA #' + (i + 1);
        var entry = this._tx(fc03_frame, label);
        result.steps.push(entry);

        var resp = slave.handle_request(fc03_frame);
        this._rx(entry, resp);
        if (!resp) { entry.err = 'no_response'; continue; }

        var parsed = MB.parse_response(resp);
        if (!parsed.ok || parsed.func !== 3) { entry.err = 'bad_response'; continue; }

        var sys_sta = (parsed.data[0] << 8) | parsed.data[1];
        entry.sys_sta = '0x' + sys_sta.toString(16).toUpperCase();
        entry.ok = true;

        if (sys_sta & 0x0080) {
            result.ok = true;
            result.sys_sta = sys_sta;
            result.poll_count = i + 1;
            return result;
        }
    }

    result.err = 'init_timeout';
    return result;
};

// --- Operation 2: SEND POWER ---
//
// FC10 write 5 registers to 0x200E:
//   [work_sta, fan_speed, target_power, intermittent_heat, jitter]
// Returns: { ok, work_sta, fan, power_w, jitter }

ModbusMaster.prototype.send_power = function (slave, opts) {
    opts = opts || {};
    var on        = opts.on !== false;   // default: heating on
    var fan_speed = 0xAA;               // 风机常开
    var power_w   = opts.power_w != null ? opts.power_w : 0;
    var power_div25 = Math.round(power_w / 25);

    // Build work_sta byte:
    // ON:  nibble inversion + bit4 forced
    // OFF: all zeros, power=0
    var work_sta;
    if (on) {
        var lo = 0x04;  /* fan bit */
        var hi = (~lo) & 0x0F;
        work_sta = (hi << 4) | lo;
        work_sta |= 0x0010;  // bit4 = heating enable
    } else {
        work_sta = 0x0000;
        power_div25 = 0;  // 关功率时 power=0
    }

    var jitter = on ? 0x0002 : 0x0000;  // 开=2, 关=0

    var data = MB.regBytesBE([work_sta, fan_speed, power_div25, 0x0000, jitter]);
    var fc10_frame = MB.build_FC10(this.addr, 0x200E, data);

    var entry = this._tx(fc10_frame, 'POWER: FC10 write 5regs to 0x200E (on=' + on + ' pwr=' + power_w + 'W)');
    var resp = slave.handle_request(fc10_frame);
    this._rx(entry, resp);

    if (!resp) { entry.ok = false; entry.err = 'no_response'; return { ok: false, err: 'no_response' }; }

    var parsed = MB.parse_response(resp);
    if (!parsed.ok || parsed.func !== 0x10) {
        entry.ok = false;
        entry.err = 'bad_response';
        return { ok: false, err: 'bad_response' };
    }
    entry.ok = true;

    // Advance slave time to let I2C init complete
    for (var t = 0; t < 10; t++) slave.tick();

    return {
        ok: true,
        work_sta: '0x' + work_sta.toString(16).toUpperCase(),
        fan: '0x' + fan_speed.toString(16).toUpperCase(),
        power_w: power_w,
        jitter: jitter
    };
};

// --- Operation 3: READ BACK ---
//
// FC03 read 9 registers from 0x1000 (standard polling block)
// Returns: { ok, data: {sys_sta, vol_ad, cur_ad, igbt_ad, bot_ad, top_ad,
//                        power_w, target_w, ppg} }

ModbusMaster.prototype.read_back = function (slave) {
    var fc03_frame = MB.build_FC03(this.addr, 0x1000, 9);
    var entry = this._tx(fc03_frame, 'READ: FC03 read 9regs from 0x1000');
    var resp = slave.handle_request(fc03_frame);
    this._rx(entry, resp);

    if (!resp) { entry.ok = false; entry.err = 'no_response'; return { ok: false, err: 'no_response' }; }

    var parsed = MB.parse_response(resp);
    if (!parsed.ok || parsed.func !== 3) {
        entry.ok = false;
        entry.err = 'bad_response';
        return { ok: false, err: 'bad_response' };
    }
    entry.ok = true;

    var d = parsed.data;
    return {
        ok: true,
        data: {
            sys_sta:    '0x' + ((d[0] << 8) | d[1]).toString(16).toUpperCase(),
            vol_ad:      (d[2] << 8) | d[3],
            cur_ad:      (d[4] << 8) | d[5],
            igbt_ad:     (d[6] << 8) | d[7],
            bot_ad:      (d[8] << 8) | d[9],
            top_ad:      (d[10] << 8) | d[11],
            power_w:     (d[12] << 8) | d[13],
            target_w:    (d[14] << 8) | d[15],
            ppg:         (d[16] << 8) | d[17]
        }
    };
};

// --- Convenience: full cycle ---

ModbusMaster.prototype.full_cycle = function (slave, power_w) {
    var self = this;
    var r = {};

    r.init = this.init(slave);
    if (!r.init.ok) return r;

    // Let I2C settle
    for (var t = 0; t < 5; t++) slave.tick();

    r.send = this.send_power(slave, { power_w: power_w != null ? power_w : 1000 });
    if (!r.send.ok) return r;

    // Let power ramp
    for (var t = 0; t < 10; t++) slave.tick();

    r.read = this.read_back(slave);
    r.ok = r.read.ok;
    return r;
};

// Node.js / browser dual support
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { ModbusMaster: ModbusMaster };
}
