/**
 * 段码显示工具 - 共阴极数码管段码映射
 */

// 段码映射表（段码 → 字符）
const SEG_CODE_MAP = {
    // 数字
    0x3F: '0', 0x06: '1', 0x5B: '2', 0x4F: '3',
    0x66: '4', 0x6D: '5', 0x7D: '6', 0x07: '7',
    0x7F: '8', 0x6F: '9',
    // 大写字母
    0x77: 'A', 0x7C: 'B', 0x39: 'C', 0x5E: 'd',
    0x79: 'E', 0x71: 'F', 0x3D: 'G', 0x76: 'H',
    0x1E: 'J', 0x38: 'L', 0x37: 'N', 0x73: 'P',
    0x3E: 'U',
    // 小写字母
    0x58: 'c', 0x54: 'n', 0x5C: 'o', 0x50: 'r',
    0x78: 't', 0x6E: 'y',
    // 特殊
    0x40: '-', 0x00: ' ', 0x08: '_',
};

// 字符 → 段码（反向映射）
const CHAR_TO_SEG = {};
for (const [code, char] of Object.entries(SEG_CODE_MAP)) {
    CHAR_TO_SEG[char] = parseInt(code);
}

/**
 * 段码 → 字符
 */
function segCodeToChar(code) {
    return SEG_CODE_MAP[code] || '?';
}

/**
 * 字符 → 段码
 */
function charToSegCode(char) {
    return CHAR_TO_SEG[char] || 0x00;
}

/**
 * 段码数组 → 可读字符串
 */
function segArrayToString(segArray, dpMask = 0, colonMask = 0) {
    const result = [];
    for (let i = 0; i < Math.min(4, segArray.length); i++) {
        let char = segCodeToChar(segArray[i]);
        if (dpMask & (1 << i)) {
            char += '.';
        }
        result.push(char);
    }
    return result.join('');
}

/**
 * 文本 → 段码数组
 */
function textToSegArray(text) {
    const segmentBytes = [];
    for (let i = 0; i < Math.min(4, text.length); i++) {
        const code = charToSegCode(text[i]);
        segmentBytes.push(code);
    }
    
    // 填充到4位
    while (segmentBytes.length < 4) {
        segmentBytes.push(0x00);
    }
    
    return segmentBytes;
}

/**
 * 格式化段码为十六进制字符串
 */
function formatSegHex(segArray) {
    return '[' + segArray.slice(0, 4).map(c => `0x${c.toString(16).toUpperCase().padStart(2, '0')}`).join(', ') + ']';
}

// 常用显示段码常量
const SEG_CONSTANTS = {
    SEG_8888:     [0x7F, 0x7F, 0x7F, 0x7F],
    SEG_DASH:     [0x40, 0x40, 0x40, 0x00],  // ---
    SEG_85_DEG:   [0x7F, 0x6D, 0x00, 0x00],  // 85°
    SEG_200:      [0x5B, 0x3F, 0x3F, 0x00],  // 200
    SEG_250:      [0x5B, 0x6D, 0x3F, 0x00],  // 250
    SEG_300:      [0x4F, 0x3F, 0x3F, 0x00],  // 300
    SEG_100:      [0x06, 0x3F, 0x3F, 0x00],  // 100
    SEG_LOCK:     [0x38, 0x76, 0x5C, 0x67],  // LOCk
    SEG_END:      [0x79, 0x5E, 0x5E, 0x00],  // End
    SEG_NWAT:     [0x37, 0x55, 0x54, 0x79],  // NWAt
    SEG_E01:      [0x79, 0x00, 0x06, 0x00],  // E01
};
