/**
 * MODBUS Slave Simulator
 *
 * Simulates a heating MCU MODBUS slave with register file.
 * Matches firmware behavior from Modbus_Lib_Init_An_Analysis.c:
 *   - 0x1000 area: read-only status registers
 *   - 0x2000 area: read-write config/control registers
 *   - 0x3000 area: read-write system settings
 *
 * Also simulates the I2C backend (heating MCU):
 *   - When work_sta (0x200E) is written with bit4 set, triggers init sequence
 *   - After init, status registers get populated with "I2C" data
 */

var MB = (typeof require !== 'undefined') ? require('./modbus_rtu.js') : window.MB;

// --- Register area definitions (matching firmware) ---

var AREA_1000_START  = 0x1000;
var AREA_1000_SIZE   = 21;   // 0x1000-0x1014
var AREA_2000_START  = 0x2000;
var AREA_2000_SIZE   = 21;   // 0x2000-0x2014
var AREA_3000_START  = 0x3000;
var AREA_3000_SIZE   = 4;    // 0x3000-0x3003

// --- Init configuration defaults (from m4_init_config.json) ---

var DEFAULT_INIT = {
    0x2000: 24,   // Check_Pan_LV (0x18)
    0x2001: 144,  // PPG_Max (0x90)
    0x2002: 24,   // Pan_Power (600/25)
    0x2003: 96,   // HVol_Limited (0x60)
    0x2004: 32,   // Load_Current (0x20)
    0x2005: 16,   // Current_calibration (0x10)
    0x2006: 40,   // Power_MIX (1000/25)
    0x2007: 88,   // Power_MAX (2200/25)
    0x2008: 0,
    0x2009: 0,
    0x200A: 16,   // phase_Pan
    0x200B: 143,  // phase_Mix
    0x200C: 0,
    0x200D: 0,
    0x200E: 0,    // Work_STA
    0x200F: 0,    // FAN_Speed
    0x2010: 0,    // target_Power
    0x2011: 0,    // intermittent_Heat
    0x2012: 0,    // jitter_frequency (powerSwitch)
    0x2013: 0,    // BuzzCof
    0x2014: 0     // syntony_Current_Short
};

/**
 * MODBUS slave simulator. Each instance = one heating channel (one slave address).
 */
function ModbusSlave(addr, name) {
    this.addr = addr;
    this.name = name || ('Slave_' + addr.toString(16));

    // Register files
    this.reg_1000 = new Uint16Array(AREA_1000_SIZE);  // status (read-only)
    this.reg_2000 = new Uint16Array(AREA_2000_SIZE);  // config/control
    this.reg_3000 = new Uint16Array(AREA_3000_SIZE);  // system settings

    // I2C simulation state
    this._i2c_init_received = false;   // set true when I2C init data arrives
    this._i2c_status_valid  = false;   // set true when I2C status data is flowing
    this._heating_on = false;
    this._actual_power_w = 0;
    this._tick_count = 0;

    // Copy default init values to 0x2000 area
    this._load_defaults();

    // Init 0x1000 status with power-on defaults
    this.reg_1000[0x0F] = 0x0100;  // Version_Number
    this.reg_1000[0x00] = 0x0000;  // SYS_Sta (bit7=0 = not initialized)

    // Init 0x3000 defaults
    this.reg_3000[0] = 64;   // Power_Calibration
    this.reg_3000[1] = addr; // Slave_Addr
    this.reg_3000[2] = 5;    // Baud_rate_SET (57600)
    this.reg_3000[3] = 0;    // Save_order
}

ModbusSlave.prototype._load_defaults = function () {
    for (var off = 0; off < AREA_2000_SIZE; off++) {
        var addr = AREA_2000_START + off;
        if (DEFAULT_INIT.hasOwnProperty(addr.toString())) {
            this.reg_2000[off] = DEFAULT_INIT[addr.toString()];
        } else {
            this.reg_2000[off] = 0;
        }
    }
};

// --- I2C simulation: called when FC10 writes to 0x200E ---

ModbusSlave.prototype._on_control_write = function () {
    var work_sta = this.reg_2000[0x0E];  // 0x200E
    var bit4 = (work_sta & 0x0010) !== 0;  // heating enable

    // If I2C init never completed, ANY write to 0x200E triggers init
    // (matches firmware: check_and_init writes zeros to kick off I2C init)
    if (!this._i2c_init_received) {
        this._heating_on = bit4;  // 只有 bit4=1 才算开机, 全零(init)不强制开
        this._init_pending_ticks = 5;
        return;
    }

    // After init complete: normal control flow
    if (bit4 && !this._heating_on) {
        this._heating_on = true;
        this._init_pending_ticks = 5;
    }

    if (!bit4 && this._heating_on) {
        this._heating_on = false;
        this._i2c_status_valid = false;
        this._actual_power_w = 0;
        this._init_pending_ticks = 0;
    }
};

ModbusSlave.prototype._complete_init = function () {
    // Simulate I2C init data received from heating MCU
    // This sets ihStatus |= 0x80 in the firmware
    this._i2c_init_received = true;
    this._i2c_status_valid = true;

    // After init, copy init values from 0x2000 EEP area to active registers
    // (In firmware, Get_IHPower_Main_Init_DATA does this)
    // The init values are already in reg_2000 from _load_defaults()

    // Set SYS_Sta bit7 = init complete
    this.reg_1000[0x00] |= 0x0080;  // SYS_Sta bit7=1 (B_INIT_SUC_FLAG)

    // Set power-stable flag
    this.reg_1000[0x00] |= 0x0040;  // SYS_Sta bit6=1 (power stable)

    // Populate status registers with running data
    var power_w = this.reg_2000[0x10] * 25;  // 0x2010 target_power in 25W units
    this.reg_1000[0x01] = 220;    // Vol_AD (simulated 220V)
    this.reg_1000[0x02] = power_w > 0 ? (power_w * 10 / 220) : 0;  // Current_AD
    this.reg_1000[0x03] = 300;    // IGBT_AD (~25C)
    this.reg_1000[0x04] = 310;    // Bot_AD (~28C)
    this.reg_1000[0x06] = power_w;     // Practical_Power
    this.reg_1000[0x07] = power_w;     // target_Power (readback)
    this.reg_1000[0x08] = power_w > 0 ? 50 : 0;  // Practical_PPG
    this.reg_1000[0x09] = 0;      // P_limited_STA
    this.reg_1000[0x11] = 0;      // ERROR
};

ModbusSlave.prototype._update_status = function () {
    // Called periodically to simulate running status
    if (!this._heating_on || !this._i2c_status_valid) return;

    var power_w = this.reg_2000[0x10] * 25;
    this.reg_1000[0x06] = power_w;  // Practical_Power tracks target
    this.reg_1000[0x07] = power_w;  // target_Power readback

    // Simulate temperature rise
    var igbt = this.reg_1000[0x03];
    var bot  = this.reg_1000[0x04];
    if (power_w > 0) {
        this.reg_1000[0x03] = Math.min(igbt + 1, 700);  // IGBT heats up
        this.reg_1000[0x04] = Math.min(bot + 1, 500);   // pan heats up
    }

    // Fan speed visible in status
    this.reg_1000[0x10] = this.reg_2000[0x0F];  // Fan_AD
};

// --- Address resolution ---

ModbusSlave.prototype._resolve = function (reg_addr) {
    if (reg_addr >= AREA_1000_START && reg_addr < AREA_1000_START + AREA_1000_SIZE) {
        return { area: this.reg_1000, offset: reg_addr - AREA_1000_START, writable: false };
    }
    if (reg_addr >= AREA_2000_START && reg_addr < AREA_2000_START + AREA_2000_SIZE) {
        return { area: this.reg_2000, offset: reg_addr - AREA_2000_START, writable: true };
    }
    if (reg_addr >= AREA_3000_START && reg_addr < AREA_3000_START + AREA_3000_SIZE) {
        return { area: this.reg_3000, offset: reg_addr - AREA_3000_START, writable: true };
    }
    return null;
};

// --- Main request handler ---

/**
 * Process a MODBUS request frame, return response frame (Uint8Array or null for silent).
 */
ModbusSlave.prototype.handle_request = function (frame) {
    var req = MB.parse_request(frame);
    if (!req.ok) {
        return MB.build_exception_response(frame[0], frame[1] || 0, req.err_code);
    }

    // Address check (firmware: if addr != slave_addr, stay silent or return error)
    if (req.addr !== this.addr) {
        return null;  // silent (not our address)
    }

    switch (req.func) {
        case 0x03: return this._handle_FC03(req);
        case 0x06: return this._handle_FC06(req);
        case 0x10: return this._handle_FC10(req);
        default:
            return MB.build_exception_response(this.addr, req.func, MB.MB_ERR_ILLEGAL_FUNC);
    }
};

// --- FC03: Read Holding Registers ---

ModbusSlave.prototype._handle_FC03 = function (req) {
    var start = req.start_reg;
    var count = req.count;

    // Range check
    var res_start = this._resolve(start);
    var res_end   = this._resolve(start + count - 1);
    if (!res_start || !res_end) {
        return MB.build_exception_response(this.addr, 0x03, MB.MB_ERR_ILLEGAL_ADDR);
    }

    // Build response data
    var data = new Uint8Array(count * 2);
    for (var i = 0; i < count; i++) {
        var res = this._resolve(start + i);
        var val = res.area[res.offset];
        data[i * 2]     = (val >> 8) & 0xFF;
        data[i * 2 + 1] = val & 0xFF;
    }

    return MB.build_FC03_response(this.addr, data);
};

// --- FC06: Write Single Register ---

ModbusSlave.prototype._handle_FC06 = function (req) {
    var reg = req.start_reg;
    var val = req.value;

    var res = this._resolve(reg);
    if (!res) {
        return MB.build_exception_response(this.addr, 0x06, MB.MB_ERR_ILLEGAL_ADDR);
    }
    if (!res.writable) {
        return MB.build_exception_response(this.addr, 0x06, MB.MB_ERR_ILLEGAL_ADDR);
    }

    // Firmware restriction: FC06 blocked for 0x200E-0x2012 (I2C block requires FC10)
    if (reg >= 0x200E && reg <= 0x2012) {
        return null;  // silent reject (firmware: TxCount=0)
    }

    res.area[res.offset] = val;

    // If writing to 0x3003 (Save_order), simulate EEPROM save
    if (reg === 0x3003 && val === 1) {
        this._eeprom_save();
    }

    return MB.build_FC06_response(this.addr, reg, val);
};

// --- FC10: Write Multiple Registers ---

ModbusSlave.prototype._handle_FC10 = function (req) {
    var start = req.start_reg;
    var count = req.count;
    var data  = req.data;

    // Range check
    var res_start = this._resolve(start);
    var res_end   = this._resolve(start + count - 1);
    if (!res_start || !res_end) {
        return MB.build_exception_response(this.addr, 0x10, MB.MB_ERR_ILLEGAL_ADDR);
    }

    // Write data
    for (var i = 0; i < count; i++) {
        var res = this._resolve(start + i);
        if (!res.writable) {
            return MB.build_exception_response(this.addr, 0x10, MB.MB_ERR_ILLEGAL_ADDR);
        }
        res.area[res.offset] = (data[i * 2] << 8) | data[i * 2 + 1];
    }

    // If writing to control block (0x200E-0x2012), trigger I2C simulation
    if (start <= 0x200E && (start + count) > 0x200E) {
        this._on_control_write();
    }

    return MB.build_FC10_response(this.addr, start, count);
};

// --- EEPROM simulation ---

ModbusSlave.prototype._eeprom_save = function () {
    // Simulate saving 0x2000 and 0x3000 to EEPROM
    this._eeprom_2000 = new Uint16Array(this.reg_2000);
    this._eeprom_3000 = new Uint16Array(this.reg_3000);
};

// --- Tick (for status updates over time) ---

ModbusSlave.prototype.tick = function () {
    this._tick_count++;

    // Process pending I2C init
    if (this._init_pending_ticks > 0) {
        this._init_pending_ticks--;
        if (this._init_pending_ticks === 0) {
            this._complete_init();
        }
    }

    if (this._tick_count % 10 === 0) {  // every 10 ticks ≈ 100ms
        this._update_status();
    }
};

// --- Debug dump ---

ModbusSlave.prototype.dump_regs = function (area_start, count) {
    var res = this._resolve(area_start);
    if (!res) return {};
    var obj = {};
    for (var i = 0; i < count; i++) {
        var addr = area_start + i;
        var r = this._resolve(addr);
        if (r) {
            obj['0x' + addr.toString(16).toUpperCase()] = r.area[r.offset];
        }
    }
    return obj;
};

// --- 8-bit access helpers ---
// MODBUS registers are 16-bit, but firmware data is uint8_t.
// These helpers mask/isolate the low byte to match firmware behavior.

ModbusSlave.prototype.getU8 = function (reg_addr) {
    var res = this._resolve(reg_addr);
    if (!res) return 0;
    return res.area[res.offset] & 0xFF;
};

ModbusSlave.prototype.setU8 = function (reg_addr, val) {
    var res = this._resolve(reg_addr);
    if (!res || !res.writable) return false;
    // Preserve high byte, update low byte (matches little-endian uint8_t→uint16_t)
    res.area[res.offset] = (res.area[res.offset] & 0xFF00) | (val & 0xFF);
    return true;
};

// --- 4-Channel Bus ---

var SLAVE_ADDRS = [0x05, 0x0A, 0x0F, 0x14];  // MODBUS slave addresses for CH1-4

/**
 * 4-channel MODBUS bus. Routes requests to the correct slave by address.
 */
function ModbusBus() {
    this.slaves = {};
    for (var i = 0; i < SLAVE_ADDRS.length; i++) {
        var addr = SLAVE_ADDRS[i];
        this.slaves[addr] = new ModbusSlave(addr, 'CH' + (i + 1));
    }
}

ModbusBus.prototype.handle_request = function (frame) {
    var addr = frame[0];
    var slave = this.slaves[addr];
    if (!slave) return null;  // unknown address, silent
    return slave.handle_request(frame);
};

ModbusBus.prototype.tick = function () {
    for (var i = 0; i < SLAVE_ADDRS.length; i++) {
        this.slaves[SLAVE_ADDRS[i]].tick();
    }
};

ModbusBus.prototype.get = function (ch) {
    var addr = SLAVE_ADDRS[ch - 1];
    return this.slaves[addr];
};

ModbusSlave.prototype.status_summary = function () {
    return {
        name: this.name,
        addr: '0x' + this.addr.toString(16).toUpperCase(),
        heating: this._heating_on,
        init_done: this._i2c_init_received,
        status_valid: this._i2c_status_valid,
        sys_sta: '0x' + this.reg_1000[0x00].toString(16).toUpperCase(),
        power_w: this.reg_1000[0x06],
        target_w: this.reg_2000[0x10] * 25,
        fan: '0x' + this.reg_2000[0x0F].toString(16).toUpperCase(),
        jitter: this.reg_2000[0x12],
        work_sta: '0x' + this.reg_2000[0x0E].toString(16).toUpperCase()
    };
};

// Node.js / browser dual support
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        ModbusSlave: ModbusSlave,
        ModbusBus: ModbusBus,
        SLAVE_ADDRS: SLAVE_ADDRS,
        DEFAULT_INIT: DEFAULT_INIT,
        AREA_1000_START: AREA_1000_START,
        AREA_1000_SIZE: AREA_1000_SIZE,
        AREA_2000_START: AREA_2000_START,
        AREA_2000_SIZE: AREA_2000_SIZE,
        AREA_3000_START: AREA_3000_START,
        AREA_3000_SIZE: AREA_3000_SIZE
    };
}
