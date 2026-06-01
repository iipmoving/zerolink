/**
 * MODBUS RTU Simulator — C version
 *
 * Standalone C99 program. Compiles with any C compiler.
 *   gcc -std=c99 -Wall -o modbus_sim modbus_sim.c && ./modbus_sim
 *
 * Architecture matches firmware Modbus_Analysis_Lib.c:
 *   - CRC-16 lookup table (polynomial 0x8005)
 *   - FC03 read holding registers (16-bit big-endian)
 *   - FC10 write multiple registers (16-bit big-endian)
 *   - 4 independent slaves (0x05=CH1, 0x0A=CH2, 0x0F=CH3, 0x14=CH4)
 *   - Register areas: 0x1000 (RO status), 0x2000 (RW config), 0x3000 (RW sys)
 *   - I2C init simulation with tick-based delay
 *   - NULL-guard callback behavior (bug fix verification)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ================================================================
 *  CRC-16 (MODBUS polynomial, matching firmware CRC16_MODBUS)
 * ================================================================ */

static const uint8_t auchCRCHi[256] = {
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40
};

static const uint8_t auchCRCLo[256] = {
    0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,0x07,0xC7,0x05,0xC5,0xC4,0x04,
    0xCC,0x0C,0x0D,0xCD,0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,0x08,0xC8,
    0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,
    0x14,0xD4,0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,0x11,0xD1,0xD0,0x10,
    0xF0,0x30,0x31,0xF1,0x33,0xF3,0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
    0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,0x3B,0xFB,0x39,0xF9,0xF8,0x38,
    0x28,0xE8,0xE9,0x29,0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,0xEC,0x2C,
    0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,
    0xA0,0x60,0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,0xA5,0x65,0x64,0xA4,
    0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
    0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,
    0xB4,0x74,0x75,0xB5,0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,0x70,0xB0,
    0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
    0x9C,0x5C,0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,0x99,0x59,0x58,0x98,
    0x88,0x48,0x49,0x89,0x4B,0x8B,0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
    0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
};

static uint16_t CRC16_MODBUS(const uint8_t *pbuf, uint8_t uslen) {
    uint8_t ucCRCHi = 0xFF;
    uint8_t ucCRCLo = 0xFF;
    uint8_t usIndex;
    while (uslen--) {
        usIndex = ucCRCHi ^ *pbuf++;
        ucCRCHi = ucCRCLo ^ auchCRCHi[usIndex];
        ucCRCLo = auchCRCLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8) | ucCRCLo;
}

static uint16_t crc16_buf(const uint8_t *buf, int len) {
    return CRC16_MODBUS(buf, (uint8_t)len);
}

/* ================================================================
 *  MODBUS constants
 * ================================================================ */

#define FUNC_READ        0x03
#define FUNC_WRITE_SINGLE 0x06
#define FUNC_WRITE_MULTI  0x10

#define EX_NONE      0x00
#define EX_ILLEGAL_FUNC  0x01
#define EX_ILLEGAL_ADDR  0x02
#define EX_ILLEGAL_VALUE 0x03
#define EX_CRC_ERROR     0x08

/* Register area sizes (matching firmware) */
#define AREA_1000_START  0x1000
#define AREA_1000_COUNT  21    /* 0x1000-0x1014 */
#define AREA_2000_START  0x2000
#define AREA_2000_COUNT  21    /* 0x2000-0x2014 */
#define AREA_3000_START  0x3000
#define AREA_3000_COUNT  4     /* 0x3000-0x3003 */

/* 4 slave addresses */
static const uint8_t SLAVE_ADDRS[4] = {0x05, 0x0A, 0x0F, 0x14};

/*
 * Default init values (from m4_init_config.json).
 *
 * FIRMWARE NOTE: The real firmware initializes ALL registers to ZERO
 * (init_SR_RW_DATA uses memset(..., 0x00, ...)), then calls
 * Get_IHPower_Main_Init_DATA() which tries to get defaults from the
 * I2C heating MCU via __weak API_UART_TxInitCallback(). The __weak
 * default returns NULL, so registers stay zero until a real I2C
 * callback provides values.
 *
 * This simulator pre-loads typical defaults so tests don't need a
 * separate I2C init step.
 *
 * FIRMWARE NOTE: I2C field cross-wiring (Get_IHPower_Main_Init_DATA line 974-975):
 *   _0x2000->Power_MAX = IH_initSta.potPowerM;  // "MAX" register gets "potPowerM" field
 *   _0x2000->Power_MIX = IH_initSta.maxPowerM;  // "MIX" register gets "maxPowerM" field
 * And in Modbus_I2c_Data_Main (line 898-899), the reverse:
 *   IH_Work_Init[Idx].potPowerM = _0x2000->Power_MIX;
 *   IH_Work_Init[Idx].maxPowerM = _0x2000->Power_MAX;
 * Net effect: Power_MIX and Power_MAX are swapped between MODBUS registers
 * and I2C fields. This simulator stores values directly without swapping.
 */
static const uint16_t DEFAULT_INIT_2000[15] = {
    24,   /* 0x2000 Check_Pan_LV */
    144,  /* 0x2001 PPG_Max       */
    24,   /* 0x2002 Pan_Power     */
    96,   /* 0x2003 HVol_Limited  */
    32,   /* 0x2004 Load_Current  */
    16,   /* 0x2005 Current_cal   */
    40,   /* 0x2006 Power_MIX     (I2C: maps to potPowerM — see cross-wiring note) */
    88,   /* 0x2007 Power_MAX     (I2C: maps to maxPowerM — see cross-wiring note) */
    0,    /* 0x2008 wrong_Pan     */
    0,    /* 0x2009 syntony_Cur   */
    16,   /* 0x200A phase_Pan     */
    143,  /* 0x200B phase_Mix     */
    0,    /* 0x200C steel_cal     */
    0,    /* 0x200D N_Pan_syntony */
    0     /* 0x200E Work_STA      */
    /* 0x200F-0x2014 default to 0 */
};

/* ================================================================
 *  Frame builder (Master → Slave)
 * ================================================================ */

static int build_fc03(uint8_t *frame, uint8_t addr, uint16_t start_reg, uint16_t count) {
    frame[0] = addr;
    frame[1] = FUNC_READ;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    uint16_t crc = crc16_buf(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;
    return 8;
}

static int build_fc10(uint8_t *frame, uint8_t addr, uint16_t start_reg,
                       const uint16_t *regs, uint16_t count) {
    int byte_cnt = count * 2;
    frame[0] = addr;
    frame[1] = FUNC_WRITE_MULTI;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    frame[6] = (uint8_t)(byte_cnt);
    int i;
    for (i = 0; i < (int)count; i++) {
        frame[7 + i*2]     = (regs[i] >> 8) & 0xFF;
        frame[7 + i*2 + 1] = regs[i] & 0xFF;
    }
    int total = 7 + byte_cnt;
    uint16_t crc = crc16_buf(frame, total);
    frame[total] = crc & 0xFF;
    frame[total+1] = (crc >> 8) & 0xFF;
    return total + 2;
}

/* ================================================================
 *  Response builder (Slave → Master)
 * ================================================================ */

static int build_fc03_resp(uint8_t *frame, uint8_t addr, const uint16_t *regs, uint16_t count) {
    int byte_cnt = count * 2;
    frame[0] = addr;
    frame[1] = FUNC_READ;
    frame[2] = (uint8_t)(byte_cnt);
    int i;
    for (i = 0; i < (int)count; i++) {
        frame[3 + i*2]     = (regs[i] >> 8) & 0xFF;
        frame[3 + i*2 + 1] = regs[i] & 0xFF;
    }
    int total = 3 + byte_cnt;
    uint16_t crc = crc16_buf(frame, total);
    frame[total] = crc & 0xFF;
    frame[total+1] = (crc >> 8) & 0xFF;
    return total + 2;
}

static int build_fc10_resp(uint8_t *frame, uint8_t addr, uint16_t start_reg, uint16_t count) {
    frame[0] = addr;
    frame[1] = FUNC_WRITE_MULTI;
    frame[2] = (start_reg >> 8) & 0xFF;
    frame[3] = start_reg & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    uint16_t crc = crc16_buf(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;
    return 8;
}

static int build_exception_resp(uint8_t *frame, uint8_t addr, uint8_t func, uint8_t ex_code) {
    frame[0] = addr;
    frame[1] = func | 0x80;
    frame[2] = ex_code;
    uint16_t crc = crc16_buf(frame, 3);
    frame[3] = crc & 0xFF;
    frame[4] = (crc >> 8) & 0xFF;
    return 5;
}

/* ================================================================
 *  Slave: register file + handlers
 * ================================================================ */

/* Forward declaration for Check_Write_Data callback type */
typedef struct ModbusSlave ModbusSlave;
typedef int (*CheckWriteDataFn)(ModbusSlave *s);

typedef struct ModbusSlave {
    uint8_t  addr;
    char     name[4];

    /* Register areas (16-bit, RAM working copies matched to firmware) */
    uint16_t reg_1000[AREA_1000_COUNT];  /* RO status, firmware: _0x1000 */
    uint16_t reg_2000[AREA_2000_COUNT];  /* RW config, firmware: _0x2000 (RAM) */
    uint16_t reg_3000[AREA_3000_COUNT];  /* RW system, firmware: _0x3000 (RAM) */

    /* EEPROM backup copies (firmware: _0x2000_EEP, _0x3000_EEP).
     * On FC10/FC06 write: RAM written first, then Check_Write_Data().
     * If check passes → EEPROM also updated. If check fails → RAM modified
     * but EEPROM preserved, MB_RSP_10H/MB_RSP_06H sets TxCount=0. */
    uint16_t reg_2000_eep[AREA_2000_COUNT];
    uint16_t reg_3000_eep[AREA_3000_COUNT];

    /* Check_Write_Data callbacks (per-area, matching firmware behavior).
     * 0x2000: all slaves return 1 (allow all).
     * 0x3000: S1/S4 check Power_Calibration range 36-96; S2/S3 allow all. */
    CheckWriteDataFn check_0x2000;
    CheckWriteDataFn check_0x3000;

    /* I2C simulation */
    int      i2c_init_received;
    int      i2c_status_valid;
    int      heating_on;
    int      init_pending_ticks;
    int      tick_count;
    uint16_t actual_power_w;
} ModbusSlave;

static void slave_resolve(const ModbusSlave *s, uint16_t addr,
                           const uint16_t **area, int *offset, int *writable) {
    if (addr >= AREA_1000_START && addr < AREA_1000_START + AREA_1000_COUNT) {
        *area = s->reg_1000; *offset = addr - AREA_1000_START; *writable = 0;
    } else if (addr >= AREA_2000_START && addr < AREA_2000_START + AREA_2000_COUNT) {
        *area = s->reg_2000; *offset = addr - AREA_2000_START; *writable = 1;
    } else if (addr >= AREA_3000_START && addr < AREA_3000_START + AREA_3000_COUNT) {
        *area = s->reg_3000; *offset = addr - AREA_3000_START; *writable = 1;
    } else {
        *area = NULL; *offset = 0; *writable = 0;
    }
}

/*
 * I2C init completion — simulates heating MCU responding after MODBUS write.
 *
 * Firmware reference: Modbus_I2c_Data_Main() reads from _0x2000_EEP copies
 * and calls API_UART_RxInitCallback/API_UART_RxControlCallback to push data
 * to the I2C heating MCU. The MCU then responds with status data that
 * Update_Static_Register_DATA() writes into _0x1000.
 *
 * The I2C bridge also has field cross-wiring (see DEFAULT_INIT_2000 comment).
 * For the simulator we read from EEPROM to match firmware behavior.
 */
static void slave_complete_init(ModbusSlave *s) {
    s->i2c_init_received = 1;
    s->i2c_status_valid  = 1;
    /* Set SYS_STA bit7 (B_INIT_SUC_FLAG), bit6 (power stable) */
    s->reg_1000[0x00] |= 0x00C0;
    /*
     * NOTE: Firmware I2C field cross-wiring (Modbus_I2c_Data_Main line 892-906):
     *   IH_Work_Init[Idx].potPowerM  = _0x2000_EEP->Power_MIX;
     *   IH_Work_Init[Idx].maxPowerM  = _0x2000_EEP->Power_MAX;
     * The heating MCU sees potPowerM/MaxPowerM which are swapped vs MODBUS
     * register names. For the simulator, power values flow directly.
     */
    /* Populate status from EEPROM control registers (firmware uses _EEP copies) */
    uint16_t power_w = (s->reg_2000_eep[0x10] & 0xFF) * 25;
    s->reg_1000[0x01] = 220;       /* Vol_AD */
    s->reg_1000[0x02] = power_w > 0 ? (uint16_t)(power_w * 10 / 220) : 0;
    s->reg_1000[0x03] = 300;       /* IGBT_AD */
    s->reg_1000[0x04] = 310;       /* Bot_AD */
    s->reg_1000[0x06] = power_w;
    s->reg_1000[0x07] = power_w;
    s->reg_1000[0x08] = power_w > 0 ? 50 : 0;
    s->actual_power_w = power_w;
    /* Sync RAM 0x2010 from EEPROM (in firmware, this is done by
     * Update_Static_Register_DATA reading back from I2C) */
    s->reg_2000[0x10] = s->reg_2000_eep[0x10];
}

/*
 * Called after FC10 writes to 0x200E control block.
 * Firmware: after MODBUS write, Modbus_I2c_Data_Main() reads from _EEP
 * copies and pushes to I2C heating MCU. The init acknowledgment comes
 * back asynchronously via I2C.
 */
static void slave_on_control_write(ModbusSlave *s) {
    /* Firmware reads from EEP copies in Modbus_I2c_Data_Main */
    uint16_t work_sta = s->reg_2000_eep[0x0E];
    int bit4 = (work_sta & 0x0010) != 0;

    /* If I2C init never done, ANY write to 0x200E triggers init
     * (firmware: check_and_init writes zeros to kick off I2C init) */
    if (!s->i2c_init_received) {
        s->heating_on = (bit4 || work_sta == 0);
        s->init_pending_ticks = 5;
        return;
    }

    if (bit4 && !s->heating_on) {
        s->heating_on = 1;
        s->init_pending_ticks = 5;
    }
    if (!bit4 && s->heating_on) {
        s->heating_on = 0;
        s->i2c_status_valid = 0;
        s->actual_power_w = 0;
        s->init_pending_ticks = 0;
    }
}

/*
 * Periodic tick — simulates the firmware's 10ms polling cycle.
 * Firmware: Update_Static_Register_DATA() reads from I2C heating MCU
 * (via API_UART_TxStatusCallback) which has its own copy of the data
 * pushed by Modbus_I2c_Data_Main(). Here we read from EEPROM directly.
 */
static void slave_tick(ModbusSlave *s) {
    s->tick_count++;
    if (s->init_pending_ticks > 0) {
        s->init_pending_ticks--;
        if (s->init_pending_ticks == 0)
            slave_complete_init(s);
    }
    /* Every 10 ticks (~100ms in firmware), update power from EEPROM */
    if (s->tick_count % 10 == 0)
        s->reg_1000[0x06] = (s->reg_2000_eep[0x10] & 0xFF) * 25;
}

/**
 * Handle one MODBUS request frame. Returns response length (0 = silent).
 */
static int slave_handle(ModbusSlave *s, const uint8_t *req, int req_len,
                         uint8_t *resp) {
    if (req_len < 4) return 0;

    /* CRC check */
    uint16_t calc_crc = crc16_buf(req, req_len - 2);
    uint16_t rx_crc = req[req_len - 2] | ((uint16_t)req[req_len - 1] << 8);
    if (calc_crc != rx_crc)
        return build_exception_resp(resp, req[0], req[1], EX_CRC_ERROR);

    /* Address check */
    if (req[0] != s->addr) return 0;  /* silent */

    uint8_t func = req[1];
    uint16_t start_reg = ((uint16_t)req[2] << 8) | req[3];
    uint16_t count     = ((uint16_t)req[4] << 8) | req[5];

    switch (func) {
    case FUNC_READ: {  /* FC03 */
        if (req_len != 8)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_VALUE);

        /* Range check */
        const uint16_t *a1, *a2;
        int o1, o2, w;
        slave_resolve(s, start_reg, &a1, &o1, &w);
        slave_resolve(s, start_reg + count - 1, &a2, &o2, &w);
        if (!a1 || !a2)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_ADDR);

        /* Read registers */
        uint16_t regs[64];
        int i;
        for (i = 0; i < (int)count && i < 64; i++) {
            const uint16_t *area;
            int off;
            slave_resolve(s, start_reg + i, &area, &off, &w);
            regs[i] = area[off];
        }
        return build_fc03_resp(resp, s->addr, regs, count);
    }

    case FUNC_WRITE_MULTI: {  /* FC10 */
        uint16_t reg_count = count;
        uint8_t  byte_cnt  = req[6];
        if (byte_cnt != reg_count * 2 || req_len != 9 + byte_cnt)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_VALUE);

        /* Range check */
        const uint16_t *a1, *a2;
        int o1, o2, w;
        slave_resolve(s, start_reg, &a1, &o1, &w);
        slave_resolve(s, start_reg + reg_count - 1, &a2, &o2, &w);
        if (!a1 || !a2)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_ADDR);

        /* Determine which area this write targets (for EEPROM + validation) */
        int target_is_0x2000 = (start_reg >= AREA_2000_START &&
                                start_reg < AREA_2000_START + AREA_2000_COUNT);
        int target_is_0x3000 = (start_reg >= AREA_3000_START &&
                                start_reg < AREA_3000_START + AREA_3000_COUNT);

        /* Step 1: Write to RAM (firmware MB_RSP_10H line 1006-1012).
         * MODBUS register is 16-bit, firmware uses low byte for 8-bit data. */
        int i;
        for (i = 0; i < (int)reg_count; i++) {
            const uint16_t *area;
            int off, writable;
            slave_resolve(s, start_reg + i, &area, &off, &writable);
            if (!writable)
                return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_ADDR);
            uint16_t val = ((uint16_t)req[7+i*2] << 8) | req[7+i*2+1];
            ((uint16_t *)area)[off] = val;
        }

        /* Step 2: Validate write via Check_Write_Data (firmware line 1015).
         * Firmware: if check returns 0, TxCount set to 0 → MB_RSP sends
         * exception 03H. RAM already modified, EEPROM NOT updated. */
        int write_ok = 1;
        if (target_is_0x2000) write_ok = s->check_0x2000(s);
        if (target_is_0x3000 && write_ok) write_ok = s->check_0x3000(s);

        if (!write_ok)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_VALUE);

        /* Step 3: Write to EEPROM backup (firmware line 1017-1026).
         * Only executed after Check_Write_Data returns 1. */
        if (target_is_0x2000) {
            int off = start_reg - AREA_2000_START;
            for (i = 0; i < (int)reg_count && (off+i) < AREA_2000_COUNT; i++)
                s->reg_2000_eep[off + i] = s->reg_2000[off + i];
        }
        if (target_is_0x3000) {
            int off = start_reg - AREA_3000_START;
            for (i = 0; i < (int)reg_count && (off+i) < AREA_3000_COUNT; i++)
                s->reg_3000_eep[off + i] = s->reg_3000[off + i];
        }

        /* If control block was written, trigger I2C init.
         * Firmware Modbus_I2c_Data_Main reads from _EEP copies. */
        if (start_reg <= 0x200E && (start_reg + reg_count) > 0x200E)
            slave_on_control_write(s);

        return build_fc10_resp(resp, s->addr, start_reg, reg_count);
    }

    case FUNC_WRITE_SINGLE: {  /* FC06 */
        if (req_len != 8)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_VALUE);

        /* FC06 blocked for 0x200E-0x2012 (I2C block requires FC10).
         * Firmware: MB_RSP_06H rejects these addresses silently (TxCount=0). */
        if (start_reg >= 0x200E && start_reg <= 0x2012)
            return 0;  /* silent */

        const uint16_t *area;
        int off, writable;
        slave_resolve(s, start_reg, &area, &off, &writable);
        if (!area || !writable)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_ADDR);

        uint16_t val = ((uint16_t)req[4] << 8) | req[5];

        /* Step 1: Write to RAM (firmware MB_RSP_06H line 893) */
        ((uint16_t *)area)[off] = val;

        /* Step 2: Validate via Check_Write_Data (firmware line 897).
         * If check fails → EEPROM NOT updated, TxCount→0, exception 03H. */
        int target_is_0x2000 = (start_reg >= AREA_2000_START &&
                                start_reg < AREA_2000_START + AREA_2000_COUNT);
        int target_is_0x3000 = (start_reg >= AREA_3000_START &&
                                start_reg < AREA_3000_START + AREA_3000_COUNT);
        int write_ok = 1;
        if (target_is_0x2000) write_ok = s->check_0x2000(s);
        if (target_is_0x3000 && write_ok) write_ok = s->check_0x3000(s);

        if (!write_ok)
            return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_VALUE);

        /* Step 3: Write to EEPROM backup (firmware line 898-903) */
        if (target_is_0x2000) s->reg_2000_eep[off] = val;
        if (target_is_0x3000) s->reg_3000_eep[off] = val;

        /* Echo response (matching firmware MB_RSP_06H line 908-913) */
        int flen = 0;
        resp[flen++] = s->addr;
        resp[flen++] = FUNC_WRITE_SINGLE;
        resp[flen++] = req[2]; resp[flen++] = req[3];
        resp[flen++] = req[4]; resp[flen++] = req[5];
        uint16_t crc = crc16_buf(resp, flen);
        resp[flen++] = crc & 0xFF;
        resp[flen++] = (crc >> 8) & 0xFF;
        return flen;
    }

    default:
        return build_exception_resp(resp, s->addr, func, EX_ILLEGAL_FUNC);
    }
}

/*
 * Check_Write_Data callbacks — matching firmware behavior.
 *
 * Firmware reference (Modbus_Lib_Init_An_Analysis.c):
 *   S1_Check_Write_Data_0x2000: always returns 1 (target_Power range
 *     check commented out in firmware)
 *   S1_Check_Write_Data_0x3000: Power_Calibration range 36-96
 *   S2_Check_Write_Data_0x2000: always returns 1
 *   S2_Check_Write_Data_0x3000: always returns 1 (check commented out)
 *   S3_Check_Write_Data_0x2000: always returns 1
 *   S3_Check_Write_Data_0x3000: always returns 1 (check commented out)
 *   S4_Check_Write_Data_0x2000: always returns 1
 *   S4_Check_Write_Data_0x3000: Power_Calibration range 36-96
 *
 * In firmware, if Check_Write_Data returns 0:
 *   - RAM has already been modified by MB_RSP_10H/MB_RSP_06H
 *   - EEPROM is NOT updated
 *   - TxCount is set to 0 → MB_RSP sends exception 03H
 */
static int check_0x2000_allow_all(ModbusSlave *s) { (void)s; return 1; }

static int check_0x3000_s1(ModbusSlave *s) {
    /* S1: Power_Calibration must be 36-96 (firmware line 303-308) */
    uint16_t cal = s->reg_3000[0];  /* 0x3000 Power_Calibration */
    return (cal >= 36 && cal <= 96) ? 1 : 0;
}

static int check_0x3000_s2(ModbusSlave *s) {
    /* S2: always allow (firmware line 332-341, check commented out) */
    (void)s; return 1;
}

static int check_0x3000_s3(ModbusSlave *s) {
    /* S3: always allow (firmware line 369-378, check commented out) */
    (void)s; return 1;
}

static int check_0x3000_s4(ModbusSlave *s) {
    /* S4: Power_Calibration must be 36-96 (firmware line 403-412) */
    uint16_t cal = s->reg_3000[0];  /* 0x3000 Power_Calibration */
    return (cal >= 36 && cal <= 96) ? 1 : 0;
}

static void slave_init(ModbusSlave *s, uint8_t addr, const char *name) {
    memset(s, 0, sizeof(*s));
    s->addr = addr;
    strncpy(s->name, name, 3);
    s->name[3] = '\0';

    /* Load default init values into 0x2000 area (RAM + EEPROM).
     * Firmware actually zeros everything then gets values from I2C;
     * we pre-load defaults here for standalone testing. */
    int i;
    for (i = 0; i < 15 && i < AREA_2000_COUNT; i++)
        s->reg_2000[i] = s->reg_2000_eep[i] = DEFAULT_INIT_2000[i];
    /* 0x200F-0x2014 remain 0 in both RAM and EEPROM */

    /* 0x1000 defaults (firmware init sets Version_Number, others stay 0) */
    s->reg_1000[0x0F] = 0x0100;  /* Version_Number */

    /* 0x3000 defaults (matching firmware Modbus_Cofg_Init_SET) */
    s->reg_3000[0] = s->reg_3000_eep[0] = 64;   /* Power_Calibration */
    s->reg_3000[1] = s->reg_3000_eep[1] = addr;  /* Slave_Addr */
    s->reg_3000[2] = s->reg_3000_eep[2] = 5;     /* Baud_rate_SET (57600) */

    /* Assign Check_Write_Data callbacks matching firmware S1-S4 */
    s->check_0x2000 = check_0x2000_allow_all;
    if (addr == 0x05) {
        s->check_0x3000 = check_0x3000_s1;   /* S1: Power_Calibration 36-96 */
    } else if (addr == 0x0A) {
        s->check_0x3000 = check_0x3000_s2;   /* S2: allow all */
    } else if (addr == 0x0F) {
        s->check_0x3000 = check_0x3000_s3;   /* S3: allow all */
    } else {
        s->check_0x3000 = check_0x3000_s4;   /* S4: Power_Calibration 36-96 */
    }
}

/* ================================================================
 *  Master: init / power / read operations
 * ================================================================ */

typedef struct {
    uint8_t addr;
    int     ok;
    int     poll_count;
    uint16_t sys_sta;
    char    err[64];
} MasterResult;

static int master_init(ModbusSlave *s, uint8_t addr, MasterResult *r) {
    memset(r, 0, sizeof(*r));
    r->addr = addr;

    /* Step 1: FC10 write 5 zero registers to 0x200E */
    uint8_t frame[64];
    uint16_t zeros[5] = {0,0,0,0,0};
    int flen = build_fc10(frame, addr, 0x200E, zeros, 5);
    uint8_t resp[64];
    int rlen = slave_handle(s, frame, flen, resp);
    if (rlen < 5) { snprintf(r->err, 64, "FC10 init no response"); return 0; }
    if (resp[1] != FUNC_WRITE_MULTI) { snprintf(r->err, 64, "FC10 init bad resp"); return 0; }

    /* Step 2: Poll SYS_STA until bit7 set */
    int retry;
    for (retry = 0; retry < 20; retry++) {
        slave_tick(s);
        flen = build_fc03(frame, addr, 0x1000, 1);
        rlen = slave_handle(s, frame, flen, resp);
        if (rlen < 5) continue;
        if (resp[1] != FUNC_READ) continue;

        uint16_t sta = ((uint16_t)resp[3] << 8) | resp[4];
        if (sta & 0x0080) {
            r->ok = 1;
            r->poll_count = retry + 1;
            r->sys_sta = sta;
            return 1;
        }
    }
    snprintf(r->err, 64, "init timeout");
    return 0;
}

static int master_send_power(ModbusSlave *s, uint8_t addr, int on,
                              int fan_on, uint16_t power_w, MasterResult *r) {
    memset(r, 0, sizeof(*r));
    uint16_t fan_speed = fan_on ? 0xAA : 0x00;
    uint16_t power_div25 = power_w / 25;

    uint16_t work_sta;
    if (on) {
        uint8_t lo = fan_on ? 0x04 : 0x00;
        uint8_t hi = (~lo) & 0x0F;
        work_sta = ((uint16_t)hi << 4) | lo;
        work_sta |= 0x0010;  /* bit4 heating enable */
    } else {
        work_sta = 0x0000;
    }
    uint16_t jitter = on ? 0x0002 : 0x0000;

    uint16_t regs[5] = {work_sta, fan_speed, power_div25, 0x0000, jitter};
    uint8_t frame[64];
    int flen = build_fc10(frame, addr, 0x200E, regs, 5);
    uint8_t resp[64];
    int rlen = slave_handle(s, frame, flen, resp);
    if (rlen < 5) { snprintf(r->err, 64, "FC10 power no resp"); return 0; }
    if (resp[1] != FUNC_WRITE_MULTI) { snprintf(r->err, 64, "FC10 power bad resp"); return 0; }

    /* Allow I2C init to complete */
    int t;
    for (t = 0; t < 10; t++) slave_tick(s);

    r->ok = 1;
    return 1;
}

static int master_read_back(ModbusSlave *s, uint8_t addr, MasterResult *r,
                             uint16_t *data_out, int max_count) {
    memset(r, 0, sizeof(*r));
    uint8_t frame[64];
    int flen = build_fc03(frame, addr, 0x1000, 9);
    uint8_t resp[64];
    int rlen = slave_handle(s, frame, flen, resp);
    if (rlen < 5) { snprintf(r->err, 64, "FC03 read no resp"); return 0; }
    if (resp[1] != FUNC_READ) { snprintf(r->err, 64, "FC03 read bad resp"); return 0; }

    int byte_cnt = resp[2];
    int count = byte_cnt / 2;
    if (count > max_count) count = max_count;
    int i;
    for (i = 0; i < count; i++)
        data_out[i] = ((uint16_t)resp[3 + i*2] << 8) | resp[3 + i*2 + 1];

    r->ok = 1;
    return 1;
}

/* ================================================================
 *  Test runner
 * ================================================================ */

static int pass_cnt = 0;
static int fail_cnt = 0;

#define TEST(name)  do { printf("  %-55s", name); } while(0)
#define PASS()      do { printf("PASS\n"); pass_cnt++; } while(0)
#define FAIL(msg)   do { printf("FAIL  %s\n", msg); fail_cnt++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, label) do { \
    if ((a) != (b)) { char _buf[128]; snprintf(_buf,128,"%s: exp %d got %d",label,(int)(b),(int)(a)); FAIL(_buf); return; } \
} while(0)

/* ---- CRC tests ---- */
static void test_crc_empty(void) {
    TEST("CRC empty buffer = 0xFFFF");
    uint8_t buf[1] = {0};
    ASSERT_EQ(CRC16_MODBUS(buf, 0), 0xFFFF, "CRC");
    PASS();
}

static void test_crc_roundtrip(void) {
    TEST("CRC match (compute vs stored, firmware pattern)");
    uint8_t frame[8];
    build_fc03(frame, 0x05, 0x1000, 1);
    /* Compute CRC of first 6 bytes (firmware: RxCount-2) */
    uint16_t calc = crc16_buf(frame, 6);
    /* Extract stored CRC from last 2 bytes (LE: low first) */
    uint16_t stored = frame[6] | ((uint16_t)frame[7] << 8);
    ASSERT_EQ(calc, stored, "CRC match");
    PASS();
}

/* ---- Frame tests ---- */
static void test_fc03_frame(void) {
    TEST("FC03 frame structure");
    uint8_t frame[8];
    build_fc03(frame, 0x05, 0x1000, 9);
    ASSERT_EQ(frame[0], 0x05, "addr");
    ASSERT_EQ(frame[1], 0x03, "func");
    ASSERT_EQ(frame[2], 0x10, "reg_hi");
    ASSERT_EQ(frame[3], 0x00, "reg_lo");
    ASSERT_EQ(frame[4], 0x00, "cnt_hi");
    ASSERT_EQ(frame[5], 0x09, "cnt_lo");
    PASS();
}

static void test_fc10_frame(void) {
    TEST("FC10 frame structure (5 regs)");
    uint16_t regs[5] = {0x00C4, 0x00AA, 0x0028, 0x0000, 0x0002};
    uint8_t frame[64];
    int len = build_fc10(frame, 0x05, 0x200E, regs, 5);
    ASSERT_EQ(len, 19, "len");
    ASSERT_EQ(frame[0], 0x05, "addr");
    ASSERT_EQ(frame[1], 0x10, "func");
    ASSERT_EQ(frame[6], 10, "byte_cnt");
    ASSERT_EQ(frame[7], 0x00, "d0_hi");
    ASSERT_EQ(frame[8], 0xC4, "d0_lo");
    PASS();
}

/* ---- Slave tests ---- */
static void test_slave_defaults(void) {
    TEST("Slave default init values");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    ASSERT_EQ(s.reg_2000[0x00], 24,  "0x2000 Check_Pan_LV");
    ASSERT_EQ(s.reg_2000[0x01], 144, "0x2001 PPG_Max");
    ASSERT_EQ(s.reg_2000[0x02], 24,  "0x2002 Pan_Power");
    ASSERT_EQ(s.reg_2000[0x06], 40,  "0x2006 Power_MIX");
    ASSERT_EQ(s.reg_2000[0x07], 88,  "0x2007 Power_MAX");
    ASSERT((s.reg_1000[0x00] & 0x0080) == 0, "SYS_STA bit7=0");
    PASS();
}

static void test_slave_silent_wrong_addr(void) {
    TEST("Slave silent for wrong address");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint8_t frame[8], resp[64];
    build_fc03(frame, 0x0A, 0x1000, 1);
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT_EQ(rlen, 0, "silent");
    PASS();
}

static void test_slave_crc_error(void) {
    TEST("Slave CRC error → exception 0x08");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint8_t frame[8];
    build_fc03(frame, 0x05, 0x1000, 1);
    frame[6] ^= 0xFF;
    uint8_t resp[64];
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT(rlen >= 5, "has response");
    ASSERT((resp[1] & 0x80) != 0, "is exception");
    ASSERT_EQ(resp[2], EX_CRC_ERROR, "ex_code=0x08");
    PASS();
}

static void test_slave_oob_read(void) {
    TEST("Slave rejects out-of-range read");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint8_t frame[8], resp[64];
    build_fc03(frame, 0x05, 0x4000, 1);
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT(rlen >= 5, "has response");
    ASSERT((resp[1] & 0x80) != 0, "is exception");
    PASS();
}

/* ---- 16-bit / 8-bit tests ---- */
static void test_16bit_register_read(void) {
    TEST("16-bit register: full 16-bit value in frame");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    s.reg_1000[0x01] = 0x00DC;  /* 220 in low byte */
    uint8_t frame[8], resp[64];
    build_fc03(frame, 0x05, 0x1001, 1);
    int rlen = slave_handle(&s, frame, 8, resp);
    (void)rlen;
    /* MODBUS frame: high byte first */
    ASSERT_EQ(resp[3], 0x00, "high byte");
    ASSERT_EQ(resp[4], 0xDC, "low byte (220)");
    PASS();
}

static void test_8bit_write_low_byte(void) {
    TEST("8-bit data: write preserves as low byte in 16-bit reg");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint16_t power_reg[5] = {0x00C4, 0x00AA, 0x0028, 0x0000, 0x0002};
    uint8_t frame[64], resp[64];
    int flen = build_fc10(frame, 0x05, 0x200E, power_reg, 5);
    slave_handle(&s, frame, flen, resp);
    /* 0x2010 target_power = 0x0028 → low byte = 0x28 = 40 */
    ASSERT_EQ(s.reg_2000[0x10], 0x0028, "reg value 0x0028");
    /* Low byte only (8-bit) = 40 → 40*25 = 1000W */
    ASSERT_EQ(s.reg_2000[0x10] & 0xFF, 40, "low byte = 40");
    PASS();
}

/* ---- Scenario: INIT ---- */
static void test_scenario_init(void) {
    TEST("SCENARIO: INIT (FC10 zeros → poll → bit7=1)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    MasterResult r;
    int ok = master_init(&s, 0x05, &r);
    ASSERT(ok, r.err);
    ASSERT(r.poll_count >= 1, "at least 1 poll");
    ASSERT((s.reg_1000[0x00] & 0x0080) != 0, "SYS_STA bit7=1");
    PASS();
}

/* ---- Scenario: POWER ---- */
static void test_scenario_power(void) {
    TEST("SCENARIO: POWER (1000W, fan on, bit4=1, jitter=2)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    MasterResult r;
    master_init(&s, 0x05, &r);
    master_send_power(&s, 0x05, 1, 1, 1000, &r);
    ASSERT(r.ok, r.err);
    ASSERT((s.reg_2000[0x0E] & 0x0010) != 0, "work_sta bit4=1");
    ASSERT_EQ(s.reg_2000[0x0F], 0xAA, "fan=0xAA");
    ASSERT_EQ(s.reg_2000[0x10] & 0xFF, 40, "power=40 (1000W)");
    ASSERT_EQ(s.reg_2000[0x12], 2, "jitter=2");
    PASS();
}

static void test_scenario_power_off(void) {
    TEST("SCENARIO: POWER off (bit4=0, jitter=0)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    MasterResult r;
    master_init(&s, 0x05, &r);
    master_send_power(&s, 0x05, 1, 1, 1000, &r);
    master_send_power(&s, 0x05, 0, 0, 0, &r);
    ASSERT(r.ok, r.err);
    ASSERT((s.reg_2000[0x0E] & 0x0010) == 0, "work_sta bit4=0");
    ASSERT_EQ(s.reg_2000[0x12], 0, "jitter=0");
    PASS();
}

/* ---- Scenario: READ ---- */
static void test_scenario_read(void) {
    TEST("SCENARIO: READ (FC03 9 regs, verify data)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    MasterResult r;
    master_init(&s, 0x05, &r);
    master_send_power(&s, 0x05, 1, 1, 1000, &r);
    uint16_t data[9];
    master_read_back(&s, 0x05, &r, data, 9);
    ASSERT(r.ok, r.err);
    ASSERT((data[0] & 0x0080) != 0, "SYS_STA bit7=1");
    ASSERT(data[1] > 0, "Vol_AD > 0");
    ASSERT(data[6] > 0, "power > 0");
    PASS();
}

/* ---- Firmware: Check_Write_Data Validation ---- */
static void test_check_write_allow(void) {
    TEST("CHECK_WRITE: S1 Power_Calibration=64 → allowed (range 36-96)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint8_t frame[8], resp[64];
    frame[0] = 0x05; frame[1] = 0x06;
    frame[2] = 0x30; frame[3] = 0x00;
    frame[4] = 0x00; frame[5] = 64;
    uint16_t crc = crc16_buf(frame, 6);
    frame[6] = crc & 0xFF; frame[7] = (crc >> 8) & 0xFF;
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT(rlen >= 8, "FC06 response received");
    ASSERT((resp[1] & 0x80) == 0, "not an exception");
    ASSERT_EQ(s.reg_3000[0], 64, "RAM updated");
    ASSERT_EQ(s.reg_3000_eep[0], 64, "EEPROM synced");
    PASS();
}

static void test_check_write_reject(void) {
    TEST("CHECK_WRITE: S1 Power_Calibration=100 → rejected (outside 36-96)");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint16_t saved_eep = s.reg_3000_eep[0];
    uint8_t frame[8], resp[64];
    frame[0] = 0x05; frame[1] = 0x06;
    frame[2] = 0x30; frame[3] = 0x00;
    frame[4] = 0x00; frame[5] = 100;
    uint16_t crc = crc16_buf(frame, 6);
    frame[6] = crc & 0xFF; frame[7] = (crc >> 8) & 0xFF;
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT(rlen >= 5, "exception response received");
    ASSERT((resp[1] & 0x80) != 0, "is exception");
    ASSERT_EQ(resp[2], EX_ILLEGAL_VALUE, "ex_code=03 (ILLEGAL_VALUE)");
    /* Firmware: RAM already modified by MB_RSP_06H, EEPROM NOT updated */
    ASSERT_EQ(s.reg_3000[0], 100, "RAM modified despite rejection");
    ASSERT_EQ(s.reg_3000_eep[0], saved_eep, "EEPROM NOT updated");
    PASS();
}

static void test_check_write_s2_allow(void) {
    TEST("CHECK_WRITE: S2 Power_Calibration=100 → allowed (S2 no check)");
    ModbusSlave s;
    slave_init(&s, 0x0A, "CH2");
    uint8_t frame[8], resp[64];
    frame[0] = 0x0A; frame[1] = 0x06;
    frame[2] = 0x30; frame[3] = 0x00;
    frame[4] = 0x00; frame[5] = 100;
    uint16_t crc = crc16_buf(frame, 6);
    frame[6] = crc & 0xFF; frame[7] = (crc >> 8) & 0xFF;
    int rlen = slave_handle(&s, frame, 8, resp);
    ASSERT(rlen >= 8, "FC06 response received");
    ASSERT((resp[1] & 0x80) == 0, "not an exception (S2 allows all)");
    ASSERT_EQ(s.reg_3000_eep[0], 100, "EEPROM updated");
    PASS();
}

static void test_fc10_eeprom_sync(void) {
    TEST("FC10: EEPROM synced after successful 5-reg write");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    uint16_t regs[5] = {0x00C4, 0x00AA, 0x0028, 0x0000, 0x0002};
    uint8_t frame[64], resp[64];
    int flen = build_fc10(frame, 0x05, 0x200E, regs, 5);
    slave_handle(&s, frame, flen, resp);
    ASSERT_EQ(s.reg_2000_eep[0x0E], s.reg_2000[0x0E], "Work_STA EEP sync");
    ASSERT_EQ(s.reg_2000_eep[0x0F], s.reg_2000[0x0F], "Fan EEP sync");
    ASSERT_EQ(s.reg_2000_eep[0x10], s.reg_2000[0x10], "Target_Pwr EEP sync");
    ASSERT_EQ(s.reg_2000_eep[0x12], s.reg_2000[0x12], "Jitter EEP sync");
    PASS();
}

/* ---- Scenario: NULL Guard (Bug Fix) ---- */
static void test_null_guard_before_init(void) {
    TEST("BUG FIX: registers NOT overwritten when I2C incomplete");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    /* Before init: read 0x2000 area should have defaults, not garbage */
    ASSERT_EQ(s.reg_2000[0x00], 24,  "0x2000 intact (not garbage)");
    ASSERT_EQ(s.reg_2000[0x01], 144, "0x2001 intact (not garbage)");
    ASSERT((s.reg_1000[0x00] & 0x0080) == 0, "SYS_STA bit7=0 (I2C not ready)");
    PASS();
}

static void test_null_guard_after_init(void) {
    TEST("BUG FIX: registers valid after I2C init completes");
    ModbusSlave s;
    slave_init(&s, 0x05, "CH1");
    MasterResult r;
    master_init(&s, 0x05, &r);
    /* After init: values should be set */
    ASSERT((s.reg_1000[0x00] & 0x0080) != 0, "SYS_STA bit7=1");
    ASSERT_EQ(s.reg_2000[0x00], 24, "Check_Pan_LV=24");
    PASS();
}

/* ---- Scenario: 4-Channel ---- */
static void test_4ch_independent(void) {
    TEST("4-CH: independent operation (500/1000/1500/2000W)");
    ModbusSlave slaves[4];
    int i;
    for (i = 0; i < 4; i++) {
        char name[4];
        snprintf(name, 4, "CH%d", i+1);
        slave_init(&slaves[i], SLAVE_ADDRS[i], name);
    }

    /* Init all 4 */
    for (i = 0; i < 4; i++) {
        MasterResult r;
        int ok = master_init(&slaves[i], SLAVE_ADDRS[i], &r);
        ASSERT(ok, r.err);
    }

    /* Send different power to each */
    uint16_t powers[4] = {500, 1000, 1500, 2000};
    for (i = 0; i < 4; i++) {
        MasterResult r;
        master_send_power(&slaves[i], SLAVE_ADDRS[i], 1, 1, powers[i], &r);
        ASSERT(r.ok, r.err);
    }

    /* Verify each independently */
    for (i = 0; i < 4; i++) {
        MasterResult r;
        uint16_t data[9];
        master_read_back(&slaves[i], SLAVE_ADDRS[i], &r, data, 9);
        ASSERT(r.ok, r.err);
        uint16_t expected_div25 = powers[i] / 25;
        ASSERT_EQ(data[6], powers[i], "power reading");
        ASSERT_EQ(slaves[i].reg_2000[0x10] & 0xFF, expected_div25, "target_power register");
    }

    /* CH1 should NOT be affected by CH2 */
    ASSERT((slaves[0].reg_1000[0x00] & 0x0080) != 0, "CH1 still init");
    ASSERT_EQ(slaves[0].reg_2000[0x10] & 0xFF, 500/25, "CH1 power unchanged");
    PASS();
}

static void test_4ch_routing(void) {
    TEST("4-CH: routing by address");
    ModbusSlave s1, s2;
    slave_init(&s1, 0x05, "CH1");
    slave_init(&s2, 0x0A, "CH2");

    MasterResult r;
    master_init(&s2, 0x0A, &r);
    ASSERT(r.ok, r.err);

    /* CH2 init should NOT affect CH1 */
    ASSERT((s1.reg_1000[0x00] & 0x0080) == 0, "CH1 not affected");
    ASSERT((s2.reg_1000[0x00] & 0x0080) != 0, "CH2 init ok");
    PASS();
}

/* ================================================================ */
int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  MODBUS Simulator — C Version\n");
    printf("  CRC16 table + FC03/FC10 + 4-channel\n");
    printf("========================================\n\n");

    printf("--- CRC-16 ---\n");
    test_crc_empty();
    test_crc_roundtrip();

    printf("\n--- Frame Encode/Decode ---\n");
    test_fc03_frame();
    test_fc10_frame();

    printf("\n--- Slave ---\n");
    test_slave_defaults();
    test_slave_silent_wrong_addr();
    test_slave_crc_error();
    test_slave_oob_read();

    printf("\n--- 16-bit / 8-bit Conversion ---\n");
    test_16bit_register_read();
    test_8bit_write_low_byte();

    printf("\n--- Scenario: INIT ---\n");
    test_scenario_init();

    printf("\n--- Scenario: POWER ---\n");
    test_scenario_power();
    test_scenario_power_off();

    printf("\n--- Scenario: READ ---\n");
    test_scenario_read();

    printf("\n--- Firmware: Check_Write_Data Validation ---\n");
    test_check_write_allow();
    test_check_write_reject();
    test_check_write_s2_allow();
    test_fc10_eeprom_sync();

    printf("\n--- Scenario: NULL Guard Bug Fix ---\n");
    test_null_guard_before_init();
    test_null_guard_after_init();

    printf("\n--- Scenario: 4-Channel ---\n");
    test_4ch_routing();
    test_4ch_independent();

    printf("\n========================================\n");
    int total = pass_cnt + fail_cnt;
    printf("  Results: %d/%d passed", pass_cnt, total);
    if (fail_cnt > 0) printf(", %d FAILED", fail_cnt);
    printf("\n========================================\n\n");

    return fail_cnt > 0 ? 1 : 0;
}
