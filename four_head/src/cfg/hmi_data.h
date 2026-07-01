/* hmi_data.h — 由 gen_hmi.js 自动生成, 勿手动编辑 */
#ifndef HMI_DATA_H
#define HMI_DATA_H

#include "../app/app_hmi.h"

extern const uint32_t hmi_timeouts[HMI_TO_COUNT];
extern const HmiPowerOnStep_t hmi_power_on_seq[];
#define HMI_POWER_ON_SEQ_LEN 3
extern const HmiGlobalNodeCfg_t hmi_global_nodes[HMI_NODE_COUNT];
extern const HmiZoneCfg_t hmi_zone_nodes[HMI_ZONE_COUNT];
extern const HmiProcessCfg_t hmi_process_nodes[HMI_PROC_COUNT];
extern const HmiElementCfg_t hmi_elements;
extern const HmiModeRule_t hmi_mode_rules[];
#define HMI_MODE_RULE_COUNT 4
extern const HmiDisplayPatterns_t hmi_patterns;
#define HMI_CHILD_LOCK_WL_LEN 2

extern const HmiConfig_t hmi_cfg;

#endif /* HMI_DATA_H */
