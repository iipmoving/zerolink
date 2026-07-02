/**
 * MODBUS RTU Protocol Engine
 *
 * Pure JS implementation of MODBUS RTU frame encode/decode + CRC-16.
 * Matches firmware behavior in Modbus_Analysis_Lib.c.
 *
 * Supported function codes: 0x03 (read), 0x06 (single write), 0x10 (batch write)
 */

// --- CRC-16 (MODBUS polynomial 0x8005, init 0xFFFF) ---

const CRC16_TABLE = (function () {
    var table = new Uint16Array(256);
    for (var i = 0; i < 256; i++) {
        var crc = i;
        for (var j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
        table[i] = crc;
    }
    return table;
})();

function crc16(buf, off, len) {
    var crc = 0xFFFF;
    var end = off + len;
    for (var i = off; i < end; i++) {
        var idx = (crc ^ buf[i]) & 0xFF;
        crc = (crc >> 8) ^ CRC16_TABLE[idx];
    }
    return crc;
}

// --- Frame Builder (Master side) ---

/**
 * Build FC03 Read Holding Registers request.
 * Returns Uint8Array: [addr, 0x03, reg_hi, reg_lo, cnt_hi, cnt_lo, crc_lo, crc_hi]
 */
function build_FC03(slave_addr, start_reg, count) {
    var frame = new Uint8Array(8);
    frame[0] = slave_addr;
    frame[1] = 0x03;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    var crc = crc16(frame, 0, 6);
    frame[6] = crc & 0xFF;       // CRC low byte first (standard MODBUS RTU)
    frame[7] = (crc >> 8) & 0xFF;
    return frame;
}

/**
 * Build FC06 Write Single Register request.
 * Returns Uint8Array: [addr, 0x06, reg_hi, reg_lo, val_hi, val_lo, crc_lo, crc_hi]
 */
function build_FC06(slave_addr, reg, value) {
    var frame = new Uint8Array(8);
    frame[0] = slave_addr;
    frame[1] = 0x06;
    frame[2] = (reg >> 8) & 0xFF;
    frame[3] = reg & 0xFF;
    frame[4] = (value >> 8) & 0xFF;
    frame[5] = value & 0xFF;
    var crc = crc16(frame, 0, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;
    return frame;
}

/**
 * Build FC10 Write Multiple Registers request.
 * data is Uint8Array of register bytes (N*2 bytes, big-endian per register).
 */
function build_FC10(slave_addr, start_reg, data) {
    var reg_count = data.length >> 1;
    var byte_count = data.length;
    var frame = new Uint8Array(9 + byte_count);
    frame[0] = slave_addr;
    frame[1] = 0x10;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (reg_count >> 8) & 0xFF;
    frame[5] = reg_count & 0xFF;
    frame[6] = byte_count;
    for (var i = 0; i < byte_count; i++) {
        frame[7 + i] = data[i];
    }
    var crc = crc16(frame, 0, 7 + byte_count);
    frame[7 + byte_count] = crc & 0xFF;
    frame[8 + byte_count] = (crc >> 8) & 0xFF;
    return frame;
}

// --- Frame Parser (Slave side) ---

var MB_ERR_NONE           = 0;
var MB_ERR_ILLEGAL_FUNC   = 0x01;
var MB_ERR_ILLEGAL_ADDR   = 0x02;
var MB_ERR_ILLEGAL_VALUE  = 0x03;
var MB_ERR_CRC            = 0x08;

/**
 * Parse a MODBUS request frame. Returns:
 *   { ok: true, addr, func, start_reg, count, data (Uint8Array), byte_count }
 *   { ok: false, err_code }
 */
function parse_request(frame) {
    if (frame.length < 4) return { ok: false, err_code: MB_ERR_ILLEGAL_VALUE };

    // Verify CRC
    var calc = crc16(frame, 0, frame.length - 2);
    var rx_crc = frame[frame.length - 2] | (frame[frame.length - 1] << 8);
    if (calc !== rx_crc) return { ok: false, err_code: MB_ERR_CRC };

    var addr = frame[0];
    var func = frame[1];

    if (func === 0x03) {
        if (frame.length !== 8) return { ok: false, err_code: MB_ERR_ILLEGAL_VALUE };
        var start_reg = (frame[2] << 8) | frame[3];
        var count     = (frame[4] << 8) | frame[5];
        return { ok: true, addr: addr, func: func, start_reg: start_reg, count: count };
    }

    if (func === 0x06) {
        if (frame.length !== 8) return { ok: false, err_code: MB_ERR_ILLEGAL_VALUE };
        var reg   = (frame[2] << 8) | frame[3];
        var value = (frame[4] << 8) | frame[5];
        return { ok: true, addr: addr, func: func, start_reg: reg, count: 1, value: value };
    }

    if (func === 0x10) {
        var start_reg = (frame[2] << 8) | frame[3];
        var reg_count = (frame[4] << 8) | frame[5];
        var byte_cnt  = frame[6];
        if (byte_cnt !== reg_count * 2) return { ok: false, err_code: MB_ERR_ILLEGAL_VALUE };
        if (frame.length !== 9 + byte_cnt) return { ok: false, err_code: MB_ERR_ILLEGAL_VALUE };
        var data = frame.slice(7, 7 + byte_cnt);
        return { ok: true, addr: addr, func: func, start_reg: start_reg, count: reg_count, data: data, byte_count: byte_cnt };
    }

    return { ok: false, err_code: MB_ERR_ILLEGAL_FUNC };
}

// --- Response Builder (Slave side) ---

/**
 * Build FC03 response: [addr, 0x03, byte_cnt, data..., crc_lo, crc_hi]
 * data is Uint8Array of register bytes (N*2 bytes, big-endian per register).
 */
function build_FC03_response(addr, data) {
    var byte_cnt = data.length;
    var frame = new Uint8Array(5 + byte_cnt);
    frame[0] = addr;
    frame[1] = 0x03;
    frame[2] = byte_cnt;
    for (var i = 0; i < byte_cnt; i++) frame[3 + i] = data[i];
    var crc = crc16(frame, 0, 3 + byte_cnt);
    frame[3 + byte_cnt] = crc & 0xFF;
    frame[4 + byte_cnt] = (crc >> 8) & 0xFF;
    return frame;
}

/**
 * Build FC06 response (echo): [addr, 0x06, reg_hi, reg_lo, val_hi, val_lo, crc_lo, crc_hi]
 */
function build_FC06_response(addr, reg, value) {
    return build_FC06(addr, reg, value);  // echo is identical to request
}

/**
 * Build FC10 response: [addr, 0x10, reg_hi, reg_lo, cnt_hi, cnt_lo, crc_lo, crc_hi]
 */
function build_FC10_response(addr, start_reg, count) {
    var frame = new Uint8Array(8);
    frame[0] = addr;
    frame[1] = 0x10;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    var crc = crc16(frame, 0, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;
    return frame;
}

/**
 * Build exception response: [addr, func|0x80, ex_code, crc_lo, crc_hi]
 */
function build_exception_response(addr, func, ex_code) {
    var frame = new Uint8Array(5);
    frame[0] = addr;
    frame[1] = func | 0x80;
    frame[2] = ex_code;
    var crc = crc16(frame, 0, 3);
    frame[3] = crc & 0xFF;
    frame[4] = (crc >> 8) & 0xFF;
    return frame;
}

// --- Parse response (Master side) ---

/**
 * Parse a MODBUS response frame. Returns parsed data or error.
 * For FC03: { ok: true, func: 3, data: Uint8Array }
 * For FC06/FC10: { ok: true, func: 6|0x10, start_reg, count }
 * For exception: { ok: true, func: 0x83|0x86|0x90, ex_code }
 * For CRC fail: { ok: false, err: 'CRC' }
 */
function parse_response(frame) {
    if (frame.length < 5) return { ok: false, err: 'too_short' };

    var calc = crc16(frame, 0, frame.length - 2);
    var rx_crc = frame[frame.length - 2] | (frame[frame.length - 1] << 8);
    if (calc !== rx_crc) return { ok: false, err: 'CRC' };

    var addr = frame[0];
    var func = frame[1];

    if (func & 0x80) {
        return { ok: true, func: func, ex_code: frame[2], addr: addr };
    }

    if (func === 0x03) {
        var byte_cnt = frame[2];
        var data = frame.slice(3, 3 + byte_cnt);
        return { ok: true, func: 3, data: data, addr: addr };
    }

    if (func === 0x06) {
        var reg = (frame[2] << 8) | frame[3];
        var val = (frame[4] << 8) | frame[5];
        return { ok: true, func: 6, start_reg: reg, value: val, addr: addr };
    }

    if (func === 0x10) {
        var start_reg = (frame[2] << 8) | frame[3];
        var count = (frame[4] << 8) | frame[5];
        return { ok: true, func: 0x10, start_reg: start_reg, count: count, addr: addr };
    }

    return { ok: false, err: 'unknown_func' };
}

// --- Utilities ---

function hexDump(bytes) {
    var parts = [];
    for (var i = 0; i < bytes.length; i++) {
        parts.push((bytes[i] < 0x10 ? '0' : '') + bytes[i].toString(16).toUpperCase());
    }
    return parts.join(' ');
}

function regBytesBE(regs) {
    var buf = new Uint8Array(regs.length * 2);
    for (var i = 0; i < regs.length; i++) {
        buf[i * 2]     = (regs[i] >> 8) & 0xFF;
        buf[i * 2 + 1] = regs[i] & 0xFF;
    }
    return buf;
}

function parseRegsBE(data) {
    var regs = [];
    for (var i = 0; i < data.length; i += 2) {
        regs.push((data[i] << 8) | data[i + 1]);
    }
    return regs;
}

// Node.js / browser dual support
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        crc16: crc16,
        build_FC03: build_FC03,
        build_FC06: build_FC06,
        build_FC10: build_FC10,
        build_FC03_response: build_FC03_response,
        build_FC06_response: build_FC06_response,
        build_FC10_response: build_FC10_response,
        build_exception_response: build_exception_response,
        parse_request: parse_request,
        parse_response: parse_response,
        hexDump: hexDump,
        regBytesBE: regBytesBE,
        parseRegsBE: parseRegsBE,
        MB_ERR_NONE: MB_ERR_NONE,
        MB_ERR_ILLEGAL_FUNC: MB_ERR_ILLEGAL_FUNC,
        MB_ERR_ILLEGAL_ADDR: MB_ERR_ILLEGAL_ADDR,
        MB_ERR_ILLEGAL_VALUE: MB_ERR_ILLEGAL_VALUE,
        MB_ERR_CRC: MB_ERR_CRC
    };
}
