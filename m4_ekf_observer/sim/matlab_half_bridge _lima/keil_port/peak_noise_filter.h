/**
 * @brief 峰值噪声滤除: 利用差值递减规律检测并修正噪声尖峰
 *
 * 原理:
 *   正常正弦波峰值附近, 相邻点差值逐级递减 (d3 > d2 > d1 ≈ 0)
 *   噪声尖峰出现时, 最后一步的差值 d1 远大于前一步 d2
 *   
 *   若 |d1| > |d2| × 1.5 → 判定为噪声
 *     用 d2 × (d2/d3) 等比外推预测修正后的峰值
 *     若外推值 < 0 则不修正 (已到顶)
 *   否则 → 峰值可信, 直接使用
 *
 * @param I      电流数组指针 (float, 已转换为安培)
 * @param idx    候选峰值索引
 * @param n      数组长度
 * @param corr   [out] 修正后的峰值
 * @return       true=检测到噪声并修正, false=峰值正常
 */
static bool peak_noise_filter(const float* I, uint16_t idx, uint16_t n, float* corr)
{
    /* 需要前3个点做差值分析 */
    if (idx < 3 || idx >= n)
    {
        *corr = I[idx];
        return false;
    }

    float d3 = I[idx - 2] - I[idx - 3];  /* pk-3 → pk-2 */
    float d2 = I[idx - 1] - I[idx - 2];  /* pk-2 → pk-1 */
    float d1 = I[idx]     - I[idx - 1];  /* pk-1 → pk (最后一步) */

    /* 防止除零 */
    if (fabsf(d2) < 1e-6f)
    {
        *corr = I[idx];
        return false;
    }

    /* 判断: 最后一步跳变是否远大于前一步 */
    if (fabsf(d1) > fabsf(d2) * 1.5f)
    {
        /* 噪声: 用等比趋势推算修正值 */
        float ratio = (fabsf(d3) > 1e-6f) ? (d2 / d3) : 0.7f;
        float predicted_d1 = d2 * ratio;

        if (predicted_d1 > 0.0f)
        {
            *corr = I[idx - 1] + predicted_d1;
        }
        else
        {
            *corr = I[idx];  /* 已到顶 */
        }
        return true;
    }
    else
    {
        *corr = I[idx];
        return false;
    }
}
