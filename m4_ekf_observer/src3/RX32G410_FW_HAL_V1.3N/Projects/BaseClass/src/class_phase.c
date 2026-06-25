/**
 * @file    phase_controller.c
 * @brief  µç´ĹÂŻĎŕÎ»˝ÇŇěłŁĽě˛âÓë±Ł»¤ÄŁżéŁ¨ľ«Ľň°ćŁ©
 * @note   - Č«˛ż¶¨µăŐűĘýŁ¬ĎŕÎ»˝ÇˇÁ10
 *         - ËůÓĐ˛Ľ¶ű±ęÖľĽŻÓÚÁŞşĎĚĺ×´Ě¬ĽÄ´ćĆ÷Ł¬żÉÍ¨ąý .all ŐűĚĺÇĺÁă
 *         - ˝öąÜŔíĎŕÎ»ŇěłŁ×´Ě¬Ł¬˛»°üş¬ą¦ÂĘˇ˘PWM µČÍâ˛ż±äÁż
 *         - ˝áąąĚĺ 8 ×Ö˝ÚŁ¬32 Î»¶ÔĆë
 */
#include "phase.h"
/* ===================== ˝ÓżÚşŻĘý ===================== */

/**
 * @brief łőĘĽ»ŻżŘÖĆĆ÷Ł¬ËůÓĐ×´Ě¬ÇĺÁăŁ¬»ůĎß´ýłőĘĽ»Ż
 */
void PhaseController_Init(PhaseController *ctrl) {
    ctrl->phase_angle = 0;
    ctrl->phase_baseline = 0;
    ctrl->inhibit_counter = 0;
    ctrl->over_limit_counter = 0;
    ctrl->status.all = 0;                /* ŐűĚĺÇĺÁă */
    ctrl->padding[0] = 0;
}

/**
 * @brief Íâ˛żĘÖ¶ŻÖŘÖĂ»ůĎßŁ¨Čçą¦ÂĘµ÷Őű»ň»»ąřşóµ÷ÓĂŁ©
 * @param ctrl      żŘÖĆĆ÷Ö¸Őë
 * @param cur_phase µ±Ç°ĎŕÎ»˝Ç (ˇÁ10)Ł¬˝«×÷ÎŞĐÂµÄ»ůĎßĆđµă
 */
void PhaseController_ResetBaseline(PhaseController *ctrl, int16_t cur_phase) {
    ctrl->phase_baseline = cur_phase;
    ctrl->inhibit_counter = 0;
    ctrl->status.all = 0;
    ctrl->status.bits.baseline_initialized = 1;   /* ±ęĽÇŇŃłőĘĽ»Ż */
}

/**
 * @brief Ăż¸öżŘÖĆÖÜĆÚµ÷ÓĂŇ»´ÎŁ¬¸üĐÂĎŕÎ»ŇěłŁ×´Ě¬±ęÖľ
 * @note  µ÷ÓĂÇ°Đč˝«×îĐÂµÄ phase_angle (ˇÁ10) ĚîČë˝áąąĚĺŁ»
 *        ±ľşŻĘýÖ»¸üĐÂ status ±ęÖľşÍÄÚ˛ż±äÁżŁ¬˛»˛Ů×÷ PWMˇŁ
 */
void PhaseController_Update(PhaseController *ctrl) {
    /* 1. ŇĆąř±Ł»¤Ľě˛â (ĎŕÎ»˝Ç ˇÝ100ˇă Á¬ĐřČ·ČĎ) */
    if (ctrl->phase_angle >= PHASE_OVER_LIMIT) {
        ctrl->over_limit_counter++;
        if (ctrl->over_limit_counter >= OVER_LIMIT_CONFIRM_COUNT) {
            ctrl->status.bits.pot_removed = 1;    /* Č·ČĎŇĆąř */
        }
    } else {
        ctrl->over_limit_counter = 0;
        ctrl->status.bits.pot_removed = 0;
    }

    /* 2. »ůĎßłőĘĽ»Ż (˝öÖ´ĐĐŇ»´Î) */
    if (!ctrl->status.bits.baseline_initialized) {
        ctrl->phase_baseline = ctrl->phase_angle;
        ctrl->status.bits.baseline_initialized = 1;
    }

    /* 3. Í»ÉýĽě˛â (Ďŕ¶Ô»ůĎß > ăĐÖµ) */
    int16_t delta_from_baseline = ctrl->phase_angle - ctrl->phase_baseline;
    if (!ctrl->status.bits.baseline_locked &&
        (delta_from_baseline > PHASE_SUDDEN_RISE_THRESHOLD)) {
        ctrl->status.bits.baseline_locked = 1;
        ctrl->status.bits.sudden_rise = 1;
        ctrl->status.bits.inhibit_increase = 1;
        ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
    }

    /* »ůĎßÎ´Ëř¶¨Ę±Ł¬Ň»˝×µÍÍ¨ÂË˛¨¸ú×Ů (Q12 ¶¨µă) */
    if (!ctrl->status.bits.baseline_locked) {
        int32_t temp = (int32_t)delta_from_baseline * BASELINE_ALPHA_Q12;
        ctrl->phase_baseline += (int16_t)(temp >> 12);
    }

    /* 4. ĎŕÎ»˝ÇąýµÍĽě˛â */
    if (ctrl->phase_angle < PHASE_TOO_LOW_THRESHOLD) {
        ctrl->status.bits.too_low = 1;
        ctrl->status.bits.inhibit_increase = 1;
        ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
    } else {
        ctrl->status.bits.too_low = 0;
    }

    /* 5. ˝űÖąÔöĽÓ±ŁłÖÖÜĆÚąÜŔí */
    if (ctrl->inhibit_counter > 0) {
        ctrl->inhibit_counter--;
        if (ctrl->inhibit_counter == 0) {
            ctrl->status.bits.inhibit_increase = 0;
            if (ctrl->status.bits.baseline_locked) {
                int16_t check_delta = ctrl->phase_angle - ctrl->phase_baseline;
                if (check_delta <= PHASE_SUDDEN_RISE_THRESHOLD) {
                    /* ŇěłŁĎűĘ§Łş˝âËř»ůĎßŁ¬ÓĂµ±Ç°ÖµÖŘÖĂ */
                    ctrl->status.bits.baseline_locked = 0;
                    ctrl->status.bits.sudden_rise = 0;
                    ctrl->phase_baseline = ctrl->phase_angle;
                } else {
                    /* ŇěłŁČÔ´ćÔÚŁ¬ĐřĆÚ˝űÖą */
                    ctrl->status.bits.inhibit_increase = 1;
                    ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
                }
            }
        }
    }

    /* 6. ¸üĐÂÔ¤ľŻ±ęÖľ (ĎŕÎ»˝Ç ˇÝ100ˇă µ«Î´Č·ČĎŇĆąř) */
    if (ctrl->over_limit_counter > 0 && !ctrl->status.bits.pot_removed) {
        ctrl->status.bits.over_limit_warn = 1;
    } else {
        ctrl->status.bits.over_limit_warn = 0;
    }
}