/* 单个开关周期的HRTIM时序状态 */
typedef struct {
    uint16_t highOn;        /* 上管开通 CNT */
    uint16_t highOff;       /* 上管关断 CNT */
    uint16_t lowOn;         /* 下管开通 CNT */
    uint16_t lowOff;        /* 下管关断 CNT (=周期结束) */
} IH_HrtimState;

/* 单个开关周期的完整输入数据 */
typedef struct {
    uint16_t I_peak;        /* 谐振电流峰值 (ADC) */
    uint16_t Vdc;           /* 母线电压 (ADC) */
    uint16_t phi;           /* 相位角 (0.1°) */
    IH_HrtimState hrtim;    /* HRTIM四状态 */
} IH_CycleData;

/* 20ms 帧的完整输入 */
typedef struct {
    IH_CycleData cycle[20]; /* 最多20个周期 */
    uint8_t count;          /* 实际有效周期数 (≤20) */
    uint16_t duty_ratio;    /* 导通占比 (0.01%) — 暂注 */
} IH_ElecInput;
