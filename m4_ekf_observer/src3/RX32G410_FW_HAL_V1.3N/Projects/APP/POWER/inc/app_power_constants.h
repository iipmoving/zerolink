/********************************************************************************
 * 文件名: app_power_constants.h
 * 描述: APP_POWER 模块使用的常量定义（从 API_HRTIM.h 剥离）
 * 说明: 这些常量主要用于 APP_POWER 的功率控制逻辑，暂时不依赖底层 API
 ********************************************************************************/

#ifndef _APP_POWER_CONSTANTS_H_
#define _APP_POWER_CONSTANTS_H_

#include <stdint.h>

/* ===================================================================
 * PPG 值结构体定义
 * =================================================================== */
typedef struct {
    uint16_t prioed;  // PPG 周期
    uint16_t duty;    // PPG 占空比
} PPGvalueDef;

/* ===================================================================
 * 频率常量定义
 * =================================================================== */

// 基础频率定义（单位：Hz）
#define FRE_20K				20000
#define FRE_22K				22000
#define FRE_25K				25000
#define FRE_27K				27000
#define FRE_28K				28000
#define FRE_29K				29000
#define FRE_30K				30000
#define FRE_35K				35000
#define FRE_38K				38000
#define FRE_40K				40000
#define FRE_42K				42000
#define FRE_45K				45000
#define FRE_50K				50000
#define FRE_55K				55000
#define FRE_500K			500000
#define FRE_1000K			1000000
#define FRE_2000K			2000000

// 负载频率定义
#define LOAD_FRE_IRON		25000		// 铁锅负载频率
#define LOAD_FRE_STEEL		28000		// 钢锅负载频率

// 频率限制
#define MIN_FRE			20000		// 最小频率
#define MAX_FRE			60000		// 最大频率
#define LARGE_FRE			40000		// 大频率
#define MID_FRE			30000		// 中等频率
#define FRE_SOFTSTART		30000		// 软启动频率
#define FRE_PAN				60000		// 检锅频率

// PWM 时钟频率（用于计算 PWM 值）
#define PWM_SF				120000000	// HRTIM 时钟频率 120MHz

/* ===================================================================
 * PWM 值常量定义（根据频率计算）
 * =================================================================== */

// 负载频率对应的 PWM 值
#define IRON_LOAD_FRE_PWM		(PWM_SF/LOAD_FRE_IRON)		// 铁锅负载 PWM 值
#define STEEL_LOAD_FRE_PWM		(PWM_SF/LOAD_FRE_STEEL)		// 钢锅负载 PWM 值

// 频率对应的 PWM 值
#define FRE_20K_PWM			(PWM_SF/FRE_20K)
#define FRE_22K_PWM			(PWM_SF/FRE_22K)
#define FRE_25K_PWM			(PWM_SF/FRE_25K)
#define FRE_27K_PWM			(PWM_SF/FRE_27K)
#define FRE_28K_PWM			(PWM_SF/FRE_28K)
#define FRE_29K_PWM			(PWM_SF/FRE_29K)
#define FRE_30K_PWM			(PWM_SF/FRE_30K)
#define FRE_35K_PWM			(PWM_SF/FRE_35K)
#define FRE_38K_PWM			(PWM_SF/FRE_38K)
#define FRE_40K_PWM			(PWM_SF/FRE_40K)
#define FRE_42K_PWM			(PWM_SF/FRE_42K)
#define FRE_45K_PWM			(PWM_SF/FRE_45K)
#define FRE_50K_PWM			(PWM_SF/FRE_50K)
#define FRE_55K_PWM			(PWM_SF/FRE_55K)
#define FRE_500K_PWM		(PWM_SF/FRE_500K)
#define FRE_1000K_PWM		(PWM_SF/FRE_1000K)
#define FRE_2000K_PWM		(PWM_SF/FRE_2000K)
#define FRE_SOFTSTART_PWM	(PWM_SF/FRE_SOFTSTART)

// 频率限制对应的 PWM 值
#define LARGE_FRE_PWM		(PWM_SF/LARGE_FRE)
#define MID_FRE_PWM		(PWM_SF/MID_FRE)
#define MIN_FRE_PWM		(PWM_SF/MIN_FRE)
#define MAX_FRE_PWM		(PWM_SF/MAX_FRE)
#define PAN_FRE_PWM		(PWM_SF/FRE_PAN)

// 特殊 PWM 值
#define OFF_FRE_PWM		0					// 关闭 PWM
#define START_FRE_PWM		FRE_40K_PWM		// 启动频率
#define POTTYPE2_FRE_PWM	FRE_38K_PWM		// 第二种炉头类型频率
#define FRE_CYCLE_PWM		MIN_FRE_PWM		// 周期 PWM

// 周期常量
#define MAX_FRE_PERIOD		MAX_FRE_PWM*2

/* ===================================================================
 * PPG 开关状态
 * =================================================================== */
#define PPG_ON				1		// PPG 开启
#define PPG_OFF				0		// PPG 关闭

/* ===================================================================
 * 通道编号定义
 * =================================================================== */
#define PotCh1				0		// 通道1
#define PotCh2				1		// 通道2
#define PotCh3				2		// 通道3
#define PotCh4				3		// 通道4

// 额外的通道定义
#define PotChTest1			0		// 测试通道1
#define PotChBase			1		// 基础通道

// 炉头数量定义（使用 CONST 后缀避免与结构体成员名冲突）
#define POT_NUM_CONST			4		// 炉头数量（常量）
#define POT_MAX_CONST			4		// 最大炉头数（常量）

/* ===================================================================
 * 检锅相关常量
 * =================================================================== */
#define C_CHECKPAN_PPG_ON	3		// PPG 起振时间点
#define C_CHECKPAN_PPG_OFF	0x2		// PPG 停止时间点


#define		DTS1					48
#define		DTS1US				DTS1
#define		DTS2US				DTS1*2
#define		DTS3US		 		DTS1*3//DTS1US*4		//0XE0+4500/DTS_PER_0xE0-32		//DT=（32+DTG[4:0]）× Tdtg，其中 Tdtg = 16 × TDTS；
#define		DTS4US		 		DTS1*4//DTS1US*4		//0XE0+4500/DTS_PER_0xE0-32		//DT=（32+DTG[4:0]）× Tdtg，其中 Tdtg = 16 × TDTS；

#define		DTS6US				0xff
#define		DTSMAX				0XFF




#endif // _APP_POWER_CONSTANTS_H_