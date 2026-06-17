/**
 * @file    ekf_lkf.c
 * @brief   EKF_LKF — 扩展卡尔曼滤波观测器 (v2.3 MODULE_SKELETON)
 * @layer   base_class
 *
 * ================================================================
 * 设计说明
 * ================================================================
 *
 * 模块职责:
 *   从 ElecParams 输出 LINK 读取观测值 (I_peak, Vdc, phi, f_sw, P_W, L_uH)，
 *   运行 EKF 估计负载参数 [R, L, f_res]，输出滤波后的稳定值。
 *
 * 调度:
 *   Switcher Slot 3, 在 ElecParams (Slot 2) 之后, APP_Power (Slot 4) 之前。
 *   仅在 ElecParams 产出新数据后执行 (20ms 一次，非每 1ms slot)。
 *
 * Kalman 状态:
 *   状态向量 x = [R, L, f_res]^T
 *   测量向量 z = [I_peak, Vdc, phi, f_sw, P_W]^T  (来自 ElecParams 输出)
 *
 *   预测: x_k|k-1 = x_k-1  (随机游走，参数慢变)
 *   P_k|k-1 = P_k-1 + Q    (协方差预测)
 *
 *   更新: K = P * H^T * (H * P * H^T + R_meas)^-1
 *        x_k = x_k|k-1 + K * (z - h(x_k|k-1))
 *        P_k = (I - K * H) * P_k|k-1
 *
 *   测量模型 h(x):
 *     h[0] = Vdc / (2 * R)                         — I_peak 近似
 *     h[1] = Vdc                                    — Vdc 直接测量
 *     h[2] = atan(Q_factor)                         — φ 通过 Q 值
 *     h[3] = f_sw                                    — f_sw 直接测量
 *     h[4] = (I_rms^2) * R                          — P 通过 I_rms
 *
 *   当 PC 端 EKF 整定完成后，替换此处桩代码为完整 EKF 数学。
 *
 * ================================================================
 * 桩代码行为 (当前):
 *   - 从 ElecParams 读取观测值并存储到内部状态
 *   - 输出 = 观测值直通 (Identity pass-through, 无滤波)
 *   - 状态向量初始化为首次观测值
 *   - 协方差矩阵预分配但未迭代更新
 *   - EKF 数学标记为 TODO，待 PC 工具整定后填充
 * ================================================================
 */

#include "../include/ekf_lkf_io.h"
#include <math.h>
#include <string.h>

/* ---- 模块骨架 ---- */
MODULE_SKELETON(EKF_LKF);

/* ---- 标定 ---- */
#define N_STATES    3   /* [R, L, f_res] */
#define N_MEAS      5   /* [I_peak, Vdc, phi, f_sw, P_W] */

/* ---- I/O 实例 ---- */
static MODULE_INPUT(EKF_LKF)*  s_inPara;    /* InputCallback 直穿赋值 */
static MODULE_OUTPUT(EKF_LKF)  s_outPara;   /* 输出缓冲区 */

/* ---- 单炉头 EKF 内部状态 ---- */
typedef struct {
    /* 状态向量 */
    float R;           /* 等效电阻 (Ω) */
    float L;           /* 等效电感 (H) */
    float f_res;       /* 谐振频率 (Hz) */

    /* 协方差矩阵 (3×3, row-major) */
    float P[9];

    /* 噪声矩阵 */
    float Q[9];        /* 过程噪声 (3×3) */
    float R_meas[25];  /* 测量噪声 (5×5) */

    /* 上次观测 */
    float z[5];        /* [I_peak, Vdc, phi, f_sw, P_W] */

    /* 观测有效性计数 */
    uint8_t valid_count;
    uint8_t initialized;  /* 1=首次初始化已完成 */
    uint8_t res[2];
} EKF_InternalState;

/* ---- 实例 (每炉头) ---- */
static EKF_InternalState s_ekf[EKF_POTMAX];

/* ========== 内部辅助 ==================================================== */

/**
 * @brief 将 ElecParams 输出参数转换为内部观测向量 z[]
 * @param p     ElecParams 输出参数指针
 * @param z     输出观测向量 [I_peak, Vdc, phi, f_sw, P_W]
 */
static void obs_from_elec(const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p, float *z)
{
    /* int32_t ×100 定标 → float */
    z[0] = (float)p->I_peak_A * 0.01f;    /* I_peak (A) */
    z[1] = (float)p->Vdc_mean * 0.01f;    /* Vdc (V) */
    z[2] = (float)p->phi_deg  * 0.01f;    /* φ (deg) */
    z[3] = (float)p->f_sw_Hz  * 0.01f;    /* f_sw (Hz) */
    z[4] = (float)p->P_W      * 0.01f;    /* P (W) */
}

/**
 * @brief 从首次观测初始化状态向量
 * @param s   EKF 内部状态指针
 * @param z   观测向量 [I_peak, Vdc, phi, f_sw, P_W]
 *
 * 初始化策略:
 *   R = P / I_rms^2   (但 I_rms 需要从 I_peak 推算, 近似 R = Vdc / (2 * I_peak))
 *   L = 从 ElecParams 的 L_uH 初始化
 *   f_res = 从 ElecParams 的 f_res_kHz 初始化
 *
 * 注意: 此初始化使用原始观测值, 后续 EKF 迭代会逐步修正。
 */
static void ekf_init(EKF_InternalState *s, const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p, const float *z)
{
    (void)z;

    /* R ≈ Vdc / (2 * I_peak) — 谐振时近似 */
    float i_peak = (float)p->I_peak_A * 0.01f;
    float vdc    = (float)p->Vdc_mean * 0.01f;
    s->R = (i_peak > 0.1f) ? (vdc / (2.0f * i_peak)) : 10.0f;

    /* L = L_uH × 1e-6 → H */
    s->L = (float)p->L_uH * 0.01f * 1e-6f;

    /* f_res = f_res_kHz (0.01kHz) * 10 → Hz */
    s->f_res = (float)p->f_res_kHz * 10.0f;

    /* 协方差初始化: 对角线 = 状态不确定度 */
    float P_init[] = {
        100.0f, 0.0f,   0.0f,
        0.0f,   1e-6f,  0.0f,
        0.0f,   0.0f,   1e6f
    };
    memcpy(s->P, P_init, sizeof(P_init));

    /* 过程噪声 (随机游走, 小量) */
    float Q_init[] = {
        0.1f,    0.0f,    0.0f,
        0.0f,    1e-9f,   0.0f,
        0.0f,    0.0f,    100.0f
    };
    memcpy(s->Q, Q_init, sizeof(Q_init));

    /* 测量噪声 (对角线, 来自传感器特性) */
    memset(s->R_meas, 0, sizeof(s->R_meas));
    s->R_meas[0]  = 0.25f;    /* I_peak: σ ≈ 0.5A */
    s->R_meas[6]  = 4.0f;     /* Vdc:    σ ≈ 2V */
    s->R_meas[12] = 1.0f;     /* phi:    σ ≈ 1° */
    s->R_meas[18] = 100.0f;   /* f_sw:   σ ≈ 10Hz */
    s->R_meas[24] = 100.0f;   /* P_W:    σ ≈ 10W */

    /* 保存观测 */
    memcpy(s->z, z, N_MEAS * sizeof(float));
    s->valid_count = 0;
    s->initialized = 1;
}

/**
 * @brief EKF 预测步 — 随机游走
 * @param s   EKF 内部状态指针
 *
 * 状态转移: x_k+1 = x_k + w  (随机游走)
 * 协方差:   P_k+1 = P_k + Q
 */
static void ekf_predict(EKF_InternalState *s)
{
    /* 协方差预测: P += Q (对角线加性, 因为 F = I) */
    for (uint8_t i = 0; i < N_STATES; i++) {
        s->P[i * N_STATES + i] += s->Q[i * N_STATES + i];
    }

    /* 对角线非负保护 */
    for (uint8_t i = 0; i < 9; i++) {
        if (s->P[i] < 0.0f) s->P[i] = 1e-12f;
    }
}

/**
 * @brief EKF 更新步 — 非线性测量更新 (STUB)
 * @param s   EKF 内部状态指针 (状态/协方差被更新)
 * @param z   当前观测向量
 * @param p   ElecParams 输出参数 (提供 L_uH 等辅助量)
 *
 * TODO: 当前为桩代码, 输出 = 观测直通,
 *       待 PC 工具 EKF 整定后实现完整非线性更新:
 *
 *       1. 计算测量预测 h(x_k|k-1)
 *          h[0] = Vdc / (2*R)                 — I_peak 估计
 *          h[1] = Vdc                          — Vdc 直接
 *          h[2] = atan(ωL/R) − atan((ωL−1/ωC)/R)  — φ 估计
 *          h[3] = f_sw                          — f_sw 直接
 *          h[4] = (I_rms²) × R                 — P 估计
 *
 *       2. 计算雅可比矩阵 H (3×5)
 *          H[i][j] = ∂h[i] / ∂x[j]
 *
 *       3. 卡尔曼增益 K = P × H^T × (H × P × H^T + R_meas)^-1
 *          (使用 3×5×3 显式展开，不依赖矩阵库)
 *
 *       4. 状态更新 x += K × (z − h(x))
 *       5. 协方差更新 P = (I − K×H) × P
 */
static void ekf_update(EKF_InternalState *s, const float *z,
                        const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p)
{
    /* === STUB: 直通模式 (无滤波, 输出 = 观测) === */
    /* 待 PC 端 EKF 整定后替换下方为完整 EKF 数学 */

    /* 使用 ElecParams 的 L_uH 和 R_ohm 作为当前状态估计 */
    float L_uH = (float)p->L_uH * 0.01f;
    s->L = L_uH * 1e-6f;  /* μH → H */
    s->R = (float)p->R_ohm * 0.01f;
    s->f_res = (float)p->f_res_kHz * 10.0f;

    /* 保存观测 */
    memcpy(s->z, z, N_MEAS * sizeof(float));
    s->valid_count++;
}

/* ========== 初始化 ====================================================== */

static void Init(void)
{
    memset(s_ekf, 0, sizeof(s_ekf));
    memset(&s_outPara, 0, sizeof(s_outPara));

    s_inPara      = NULL;      /* InputCallback 直穿赋值 */
    g_input.para  = &s_inPara;  /* 绑定输入指针变量地址 */
    g_output.para = &s_outPara; /* 绑定输出缓冲区 */
}

/* ========== 主处理 ====================================================== */

static void ProcessInput(void)
{
    EKF_LKF_Input  *in  = (EKF_LKF_Input *)g_input.para;
    EKF_LKF_Output *out = (EKF_LKF_Output *)g_output.para;

    if (!in || !in->ElecParams_params) return;

    /* ====== 输入段 ====== */
    /* v2.3: 检查数据层 LINK status (ElecParams 输出 LINK 置 ST_OUT 表示新数据就绪) */
    if (!(in->ElecParams_params->status & ST_OUT)) return;
    /* 注: 不清除 ST_OUT — 此为 ElecParams 共享输出 LINK, 对各消费者只读 */

    /* ELEC_POTMAX = 4 炉头, 与 ElecParams 输出 LINK 的 params[] 一致 */
    uint8_t max_h = EKF_POTMAX;

    for (uint8_t h = 0; h < max_h; h++) {
        const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p = &in->ElecParams_params->params[h];

        if (!p->valid) {
            memset(&out->PowerBase_params.params[h], 0, sizeof(out->PowerBase_params.params[h]));
            continue;
        }

        /* 转换为内部观测向量 */
        float z[N_MEAS];
        obs_from_elec(p, z);

        /* EKF 步 */
        if (!s_ekf[h].initialized) {
            ekf_init(&s_ekf[h], p, z);
        } else {
            ekf_predict(&s_ekf[h]);
            ekf_update(&s_ekf[h], z, p);
        }

        /* 填充输出 (STUB: 直通 ElecParams) */
        out->PowerBase_params.params[h].R_ohm     = (int32_t)(s_ekf[h].R * 100.0f + 0.5f);
        out->PowerBase_params.params[h].L_uH      = (int32_t)(s_ekf[h].L / 1e-6f * 100.0f + 0.5f);
        out->PowerBase_params.params[h].f_res_kHz = (int32_t)(s_ekf[h].f_res / 10.0f + 0.5f);
        out->PowerBase_params.params[h].Q_factor  = p->Q_factor;  /* STUB: 直通 */
        out->PowerBase_params.params[h].valid     = 1;
    }

    out->PowerBase_params.status |= ST_OUT;
}

/* ========== 模块导出 ==================================================== */

MODULE_EXPORT(EKF_LKF);
