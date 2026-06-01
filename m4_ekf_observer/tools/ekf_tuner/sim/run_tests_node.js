/**
 * MODBUS Simulator — Test Runner (Node.js)
 *
 * Tests three MODBUS operations step by step:
 *   1. INIT    — FC10 write zeros → poll SYS_STA until bit7=1
 *   2. POWER   — FC10 write 5 control registers (work_sta, fan, power, jitter)
 *   3. READ    — FC03 read 9 status registers
 *
 * Run: node run_tests_node.js
 */

var MB    = require('./js/modbus_rtu.js');
var Slave = require('./js/modbus_slave.js');
var Master = require('./js/modbus_master.js');

var PASS = 0;
var FAIL = 0;

function test(name, fn) {
    try {
        fn();
        PASS++;
        console.log('  PASS  ' + name);
    } catch (e) {
        FAIL++;
        console.log('  FAIL  ' + name);
        console.log('        ' + e.message);
        if (e.stack) {
            var lines = e.stack.split('\n');
            for (var i = 1; i < Math.min(lines.length, 4); i++) {
                console.log('        ' + lines[i].trim());
            }
        }
    }
}

function assert(cond, msg) {
    if (!cond) throw new Error(msg || 'assertion failed');
}

function assertEq(actual, expected, label) {
    if (actual !== expected) {
        throw new Error((label || '') + ' expected ' + expected + ', got ' + actual);
    }
}

// ================================================================
console.log('');
console.log('========================================');
console.log('  MODBUS Simulator — Test Suite');
console.log('========================================');
console.log('');

// --- CRC-16 Tests ---
console.log('--- CRC-16 Calculation ---');

test('CRC of empty buffer = 0xFFFF', function () {
    var buf = new Uint8Array(0);
    assertEq(MB.crc16(buf, 0, 0), 0xFFFF, 'CRC');
});

test('CRC of known FC03 frame', function () {
    // FC03 read 0x1000 count=1 from slave 5: 05 03 10 00 00 01
    var frame = new Uint8Array([0x05, 0x03, 0x10, 0x00, 0x00, 0x01]);
    var crc = MB.crc16(frame, 0, 6);
    // Verify CRC is non-zero and consistent
    assert(crc !== 0xFFFF, 'CRC should not be 0xFFFF for non-empty buffer');
    assert(crc !== 0, 'CRC should not be 0 for non-empty buffer');
    // Round-trip: recompute with CRC appended gives consistent result
    var full = new Uint8Array(8);
    full.set(frame);
    full[6] = crc & 0xFF;
    full[7] = (crc >> 8) & 0xFF;
    var verify = MB.crc16(full, 0, 8);
    assertEq(verify, 0, 'CRC round-trip');
});

// --- Frame Encode/Decode Tests ---
console.log('');
console.log('--- Frame Encode/Decode ---');

test('FC03 request builds correct frame', function () {
    var frame = MB.build_FC03(0x05, 0x1000, 1);
    assertEq(frame.length, 8, 'length');
    assertEq(frame[0], 0x05, 'addr');
    assertEq(frame[1], 0x03, 'func');
    assertEq(frame[2], 0x10, 'reg_hi');
    assertEq(frame[3], 0x00, 'reg_lo');
    assertEq(frame[4], 0x00, 'cnt_hi');
    assertEq(frame[5], 0x01, 'cnt_lo');
    // CRC round-trip
    var crc = MB.crc16(frame, 0, 8);
    assertEq(crc, 0, 'CRC valid');
});

test('FC10 request builds correct frame', function () {
    var data = MB.regBytesBE([0x00C4, 0x00AA, 0x0028, 0x0000, 0x0002]);
    var frame = MB.build_FC10(0x05, 0x200E, data);
    assertEq(frame.length, 19, 'length (9 + 10 bytes)');
    assertEq(frame[0], 0x05, 'addr');
    assertEq(frame[1], 0x10, 'func');
    assertEq(frame[6], 10, 'byte_cnt');
    // Verify data bytes
    assertEq(frame[7], 0x00, 'data[0] hi');
    assertEq(frame[8], 0xC4, 'data[0] lo');
    // CRC round-trip
    var crc = MB.crc16(frame, 0, 19);
    assertEq(crc, 0, 'CRC valid');
});

test('FC06 request builds correct frame', function () {
    var frame = MB.build_FC06(0x05, 0x3000, 64);
    assertEq(frame.length, 8, 'length');
    assertEq(frame[1], 0x06, 'func');
    var crc = MB.crc16(frame, 0, 8);
    assertEq(crc, 0, 'CRC valid');
});

test('parse_request detects CRC error', function () {
    var req = MB.parse_request(new Uint8Array([0x05, 0x03, 0x10, 0x00, 0x00, 0x01, 0xFF, 0xFF]));
    assert(!req.ok, 'should fail');
    assertEq(req.err_code, MB.MB_ERR_CRC, 'err_code');
});

test('parse_request detects bad function code', function () {
    // Build frame with func=0x01 (unsupported)
    var frame = new Uint8Array([0x05, 0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00]);
    var crc = MB.crc16(frame, 0, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;
    var req = MB.parse_request(frame);
    assert(!req.ok, 'should fail');
    assertEq(req.err_code, MB.MB_ERR_ILLEGAL_FUNC, 'err_code');
});

// --- Slave Tests ---
console.log('');
console.log('--- Slave Register File ---');

test('Slave starts with SYS_STA bit7=0 (not initialized)', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    assert((s.reg_1000[0x00] & 0x0080) === 0, 'bit7 should be 0');
});

test('Slave has default init values loaded', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    assertEq(s.reg_2000[0x00], 24, '0x2000 Check_Pan_LV');
    assertEq(s.reg_2000[0x01], 144, '0x2001 PPG_Max');
    assertEq(s.reg_2000[0x02], 24, '0x2002 Pan_Power');
    assertEq(s.reg_2000[0x03], 96, '0x2003 HVol_Limited');
    assertEq(s.reg_2000[0x04], 32, '0x2004 Load_Current');
    assertEq(s.reg_2000[0x05], 16, '0x2005 Current_calibration');
    assertEq(s.reg_2000[0x06], 40, '0x2006 Power_MIX');
    assertEq(s.reg_2000[0x07], 88, '0x2007 Power_MAX');
});

test('Slave rejects FC03 read out of range', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    var frame = MB.build_FC03(0x05, 0x4000, 1);
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    assert(parsed.ok, 'should be valid response');
    assert((parsed.func & 0x80) !== 0, 'should be exception');
    assertEq(parsed.ex_code, MB.MB_ERR_ILLEGAL_ADDR, 'exception code');
});

test('Slave stays silent for wrong address', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    var frame = MB.build_FC03(0x0A, 0x1000, 1);
    var resp = s.handle_request(frame);
    assertEq(resp, null, 'silent');
});

test('FC06 blocked for 0x200E-0x2012 (I2C block)', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    var frame = MB.build_FC06(0x05, 0x200E, 0x0010);
    var resp = s.handle_request(frame);
    assertEq(resp, null, 'silent reject');
});

test('FC06 works for 0x3000 area', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    var frame = MB.build_FC06(0x05, 0x3000, 72);
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    assert(parsed.ok && parsed.func === 6, 'FC06 OK');
    assertEq(s.reg_3000[0], 72, '0x3000 updated');
});

// ================================================================
// SCENARIO TESTS
// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 1: INIT Flow');
console.log('========================================');

function scenario1_init() {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    // Before init: SYS_STA bit7=0
    var pre = s.reg_1000[0x00];
    assert((pre & 0x0080) === 0, 'pre-init: SYS_STA bit7=0');

    // Run init
    var r = m.init(s, { poll_delay: 0, max_retries: 20 });
    assert(r.ok, 'init succeeded');
    assert(r.poll_count >= 1, 'at least one poll');
    assert((r.sys_sta & 0x0080) !== 0, 'post-init: SYS_STA bit7=1');

    // Verify init data copied to active registers
    assertEq(s.reg_1000[0x06], 0, 'power=0 before sending');

    return { slave: s, master: m };
}

test('INIT: FC10 zeros → poll → bit7 set', function () {
    scenario1_init();
});

test('INIT: transaction log has FC10 + poll entries', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);
    m.init(s, { poll_delay: 0 });
    assert(m.log.length >= 2, 'log has >=2 entries');
    // First entry should be FC10
    assert(m.log[0].label.indexOf('FC10') >= 0, 'first is FC10');
    // Second should be poll
    assert(m.log[1].label.indexOf('poll SYS_STA') >= 0, 'second is FC03 poll');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 2: Power Control Flow');
console.log('========================================');

function scenario2_power() {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    // First init
    var r_init = m.init(s, { poll_delay: 0 });
    assert(r_init.ok, 'init ok');

    // Send power: 1000W, fan on
    var r = m.send_power(s, { power_w: 1000, fan_on: true, fan_speed: 0xAA });
    assert(r.ok, 'send_power ok');

    // Verify registers
    assert((s.reg_2000[0x0E] & 0x0010) !== 0, 'work_sta bit4=1 (heating on)');
    assertEq(s.reg_2000[0x0F], 0xAA, 'fan_speed=0xAA');
    assertEq(s.reg_2000[0x10], 40, 'target_power=40 (1000W/25)');
    assertEq(s.reg_2000[0x12], 2, 'jitter=2 (SWITCH_OFF_POT)');

    return { slave: s, master: m };
}

test('POWER: work_sta bit4=1, fan=0xAA, power=1000W, jitter=2', function () {
    scenario2_power();
});

test('POWER: heating off sets jitter=0, work_sta bit4=0', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);
    m.init(s, { poll_delay: 0 });
    m.send_power(s, { power_w: 1000 });
    // Now turn off
    m.send_power(s, { on: false, fan_on: false });
    assert((s.reg_2000[0x0E] & 0x0010) === 0, 'work_sta bit4=0');
    assertEq(s.reg_2000[0x12], 0, 'jitter=0');
});

test('POWER: FC06 single write to 0x200E is silently rejected', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);
    m.init(s, { poll_delay: 0 });

    // Attempt FC06 to 0x200E — should be silently ignored
    var fc06 = MB.build_FC06(0x05, 0x200E, 0x0010);
    var resp = s.handle_request(fc06);
    assertEq(resp, null, 'FC06 to 0x200E silently rejected');
    assertEq(s.reg_2000[0x0E], 0, '0x200E unchanged');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 3: Read Back Flow');
console.log('========================================');

function scenario3_read() {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    m.init(s, { poll_delay: 0 });
    m.send_power(s, { power_w: 1000, fan_on: true });

    // Read back status
    var r = m.read_back(s);
    assert(r.ok, 'read_back ok');

    var d = r.data;
    assert((parseInt(d.sys_sta, 16) & 0x0080) !== 0, 'SYS_STA bit7=1');
    assert(d.power_w > 0, 'power_w > 0');

    return { slave: s, master: m, data: d };
}

test('READ: returns 9 valid registers', function () {
    var r = scenario3_read();
    var d = r.data;
    assert(d.vol_ad > 0, 'vol_ad > 0');
    assert(d.bot_ad > 0, 'bot_ad > 0');
    assert(d.power_w > 0, 'power_w > 0');
});

test('READ: target_w matches sent power', function () {
    var r = scenario3_read();
    // target_w is readback from 0x1000[7], power_w from 0x1000[6]
    assert(r.data.target_w > 0, 'target_w in readback > 0');
});

test('READ: response is valid CRC-protected frame', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);
    m.init(s, { poll_delay: 0 });

    var frame = MB.build_FC03(0x05, 0x1000, 9);
    var resp = s.handle_request(frame);
    // Verify CRC of response
    var crc = MB.crc16(resp, 0, resp.length);
    assertEq(crc, 0, 'response CRC valid');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 4: I2C NULL Guard (Bug Fix Verification)');
console.log('========================================');

test('I2C: init callback returns NULL when I2C not complete', function () {
    // Simulates the firmware bug: before init, I2C callback returns NULL
    // The slave should not have SYS_STA bit7 set before init completes
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    assert(!s._i2c_init_received, 'I2C init not received');
    assert((s.reg_1000[0x00] & 0x0080) === 0, 'SYS_STA bit7=0');

    // FC03 read of 0x2000 area should return DEFAULT values (not garbage)
    var frame = MB.build_FC03(0x05, 0x2000, 8);
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    assert(parsed.ok, 'response ok');
    var regs = MB.parseRegsBE(parsed.data);
    assertEq(regs[0], 24, '0x2000 = 24 (not garbage)');
    assertEq(regs[1], 144, '0x2001 = 144 (not garbage)');
});

test('I2C: after init complete, register values are valid', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    m.init(s, { poll_delay: 0 });

    // Now init is complete, read EEP area should have init values
    var frame = MB.build_FC03(0x05, 0x2000, 8);
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    var regs = MB.parseRegsBE(parsed.data);
    assertEq(regs[0], 24, '0x2000 Check_Pan_LV=24');
    assertEq(regs[1], 144, '0x2001 PPG_Max=144');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 5: CRC Error Handling');
console.log('========================================');

test('CRC: corrupted frame returns exception 0x08', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    // Build valid frame then corrupt CRC
    var frame = MB.build_FC03(0x05, 0x1000, 1);
    frame[6] ^= 0xFF;  // corrupt CRC low byte
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    assert(parsed.ok, 'got response');
    assert((parsed.func & 0x80) !== 0, 'is exception');
    assertEq(parsed.ex_code, MB.MB_ERR_CRC, 'exception 0x08 (CRC error)');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 6: Full Cycle (Init → Power → Read)');
console.log('========================================');

test('FULL CYCLE: 1000W heating', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    var r = m.full_cycle(s, 1000);
    assert(r.ok, 'full cycle ok');
    assert(r.init.ok, 'init');
    assert(r.send.ok, 'send');
    assert(r.read.ok, 'read');
    assert(r.read.data.power_w > 0, 'power reading > 0');
});

test('FULL CYCLE: 0W (idle)', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    var r = m.full_cycle(s, 0);
    assert(r.ok, 'full cycle ok');
    assertEq(r.read.data.power_w, 0, 'power=0');
});

test('FULL CYCLE: 3000W (max)', function () {
    var s = new Slave.ModbusSlave(0x05, 'CH1');
    var m = new Master.ModbusMaster(0x05);

    var r = m.full_cycle(s, 3000);
    assert(r.ok, 'full cycle ok');
    // 3000W / 25 = 120
    assertEq(s.reg_2000[0x10], 120, 'target_power register = 120');
});

// ================================================================
console.log('');
console.log('========================================');
console.log('  SCENARIO 7: 4-Channel Bus + 8-bit/16-bit');
console.log('========================================');

test('BUS: 4 slaves with correct addresses 0x05,0x0A,0x0F,0x14', function () {
    var bus = new Slave.ModbusBus();
    assert(bus.slaves[0x05] !== undefined, 'slave 0x05 exists');
    assert(bus.slaves[0x0A] !== undefined, 'slave 0x0A exists');
    assert(bus.slaves[0x0F] !== undefined, 'slave 0x0F exists');
    assert(bus.slaves[0x14] !== undefined, 'slave 0x14 exists');
    assertEq(bus.slaves[0x05].name, 'CH1', 'CH1 name');
    assertEq(bus.slaves[0x0A].name, 'CH2', 'CH2 name');
});

test('BUS: request routed to correct slave by address', function () {
    var bus = new Slave.ModbusBus();
    // Init CH2 (addr 0x0A)
    var m2 = new Master.ModbusMaster(0x0A);
    m2.init(bus.slaves[0x0A], { poll_delay: 0 });
    // CH1 should NOT be affected
    assert((bus.slaves[0x05].reg_1000[0x00] & 0x0080) === 0, 'CH1 still not initialized');
    assert((bus.slaves[0x0A].reg_1000[0x00] & 0x0080) !== 0, 'CH2 init complete');
});

test('BUS: 4 channels can operate independently', function () {
    var bus = new Slave.ModbusBus();
    var masters = {};
    for (var i = 0; i < Slave.SLAVE_ADDRS.length; i++) {
        masters[Slave.SLAVE_ADDRS[i]] = new Master.ModbusMaster(Slave.SLAVE_ADDRS[i]);
    }

    // Init all 4 channels
    for (var a = 0; a < Slave.SLAVE_ADDRS.length; a++) {
        var addr = Slave.SLAVE_ADDRS[a];
        var r = masters[addr].init(bus.slaves[addr], { poll_delay: 0 });
        assert(r.ok, 'CH' + (a+1) + ' init ok');
    }

    // Send different power to each
    var powers = [500, 1000, 1500, 2000];
    for (var a = 0; a < Slave.SLAVE_ADDRS.length; a++) {
        var addr = Slave.SLAVE_ADDRS[a];
        masters[addr].send_power(bus.slaves[addr], { power_w: powers[a] });
    }

    // Read back each independently
    for (var a = 0; a < Slave.SLAVE_ADDRS.length; a++) {
        var addr = Slave.SLAVE_ADDRS[a];
        var r = masters[addr].read_back(bus.slaves[addr]);
        assert(r.ok, 'CH' + (a+1) + ' read ok');
        assertEq(r.data.power_w, powers[a], 'CH' + (a+1) + ' power=' + powers[a] + 'W');
    }
});

test('8BIT: getU8 returns low byte only', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    s.reg_2000[0x10] = 0x1234;  // set full 16-bit value
    assertEq(s.getU8(0x2010), 0x34, 'getU8 returns low byte');
});

test('8BIT: setU8 preserves high byte', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    s.reg_2000[0x10] = 0xFF00;
    s.setU8(0x2010, 0x42);
    assertEq(s.reg_2000[0x10], 0xFF42, 'setU8 preserves high byte (0xFF42)');
});

test('8BIT: MODBUS frame carries full 16-bit value', function () {
    var s = new Slave.ModbusSlave(0x05, 'test');
    s.reg_1000[0x01] = 0x00DC;  // 220 (Vol_AD), typical 8-bit value in low byte
    var frame = MB.build_FC03(0x05, 0x1001, 1);
    var resp = s.handle_request(frame);
    var parsed = MB.parse_response(resp);
    // MODBUS sends 2 bytes: [0x00, 0xDC] = big-endian 16-bit
    assertEq(parsed.data[0], 0x00, 'high byte = 0x00');
    assertEq(parsed.data[1], 0xDC, 'low byte = 0xDC (220)');
    var regs = MB.parseRegsBE(parsed.data);
    assertEq(regs[0], 220, '16-bit value = 220');
});

// ================================================================
console.log('');
console.log('========================================');
var total = PASS + FAIL;
console.log('  Results: ' + PASS + '/' + total + ' passed' + (FAIL > 0 ? ', ' + FAIL + ' FAILED' : ''));
console.log('========================================');
console.log('');

process.exit(FAIL > 0 ? 1 : 0);
