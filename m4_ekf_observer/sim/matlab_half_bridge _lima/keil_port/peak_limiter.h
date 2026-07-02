/**
 * @brief 峰值限幅器: 最后一步增量不超过前两增量平均
 *
 * 原理:
 *   正常峰值附近最后一步(d1)不应显著大于前两步(d2,d3)的平均
 *   若 d1 > (d2+d3)/2, 将峰值修正为 I[pk-1] + (d2+d3)/2
 *   真的大干扰会自动限幅, 正常信号影响极小
 *
 * 使用条件:
 *   - 仅在窗口 [v_up, CMP_UOFF) 内找到的峰值上调用
 *   - 仅当 I_peak > 5A 时启用 (小信号时采样稀疏, 不适用)
 *
 * @param I      电流数组 (float, A)
 * @param idx    候选峰值索引
 * @param n      数组长度
 * @param corr   [out] 修正后的峰值
 * @return       true=被限幅, false=未修正
 */
static bool peak_limiter(const float* I, uint16_t idx, uint16_t n, float* corr)
{
    if (idx < 3 || idx >= n)
    {
        *corr = I[idx];
        return false;
    }

    float d3 = I[idx - 2] - I[idx - 3];
    float d2 = I[idx - 1] - I[idx - 2];
    float d1 = I[idx]     - I[idx - 1];

    float avg_d2d3 = (d2 + d3) * 0.5f;

    /* 仅当最后一步大于前两步平均时才限幅 */
    if (d1 > avg_d2d3 && d1 > 0.0f)
    {
        *corr = I[idx - 1] + avg_d2d3;
        return true;
    }

    *corr = I[idx];
    return false;
}
