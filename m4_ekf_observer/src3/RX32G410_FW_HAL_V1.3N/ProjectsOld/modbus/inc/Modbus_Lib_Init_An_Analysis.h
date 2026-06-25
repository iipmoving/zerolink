#ifndef  	_Modbus_Lib_Init_An_Analysis_H
#define 	_Modbus_Lib_Init_An_Analysis_H


/**
 * @brief IH_STA_READ_WRITE 结构体成员索引枚举
 * @note 对应地址 0x2000-0x2013 的可读写参数
 */
typedef enum {
    MEMBER_Check_Pan_LV = 0,           ///< 0x2000 检锅强度设定
    MEMBER_PPG_Max,                    ///< 0x2001 最大PPG限制
    MEMBER_Pan_Power,                  ///< 0x2002 移锅功率
    MEMBER_HVol_Limited,               ///< 0x2003 反压限制
    MEMBER_Load_Current,               ///< 0x2004 负载有效电流
    MEMBER_Current_calibration,        ///< 0x2005 电流修正系数
    MEMBER_Power_MIX,                  ///< 0x2006 最小连续功率
    MEMBER_Power_MAX,                  ///< 0x2007 最大连续功率
    MEMBER_wrong_Pan,                  ///< 0x2008 恶略锅具保护功率
    MEMBER_syntony_Current,            ///< 0x2009 谐振电流保护值
    MEMBER_phase_Pan,                  ///< 0x200A 移锅相位
    MEMBER_phase_Mix,                  ///< 0x200B 最小相位
    MEMBER_steel_calibration,          ///< 0x200C 钢铁锅修正
    MEMBER_N_Pan_syntony_C,            ///< 0x200D 移锅谐振电流限制
    MEMBER_Work_STA,                   ///< 0x200E 工作状态
    MEMBER_FAN_Speed,                  ///< 0x200F 风扇转速
    MEMBER_target_Power,               ///< 0x2010 目标功率
    MEMBER_intermittent_Heat,          ///< 0x2011 间断加热
    MEMBER_jitter_frequency,           ///< 0x2012 抖频参数
    MEMBER_BuzzCof,                    ///< 0x2013 蜂鸣器控制
    MEMBER_COUNT_0x2000                ///< 0x2000区域成员总数，用于边界检查
} IH_0x2000_MEMBER_IDX;

/**
 * @brief IH_STA_READ_WRITE_SYS_SET 结构体成员索引枚举
 * @note 对应地址 0x3000-0x3003 的系统设置参数
 */
typedef enum {
    MEMBER_Power_Calibration = 0,      ///< 0x3000 功率校准值
    MEMBER_Slave_Addr,                 ///< 0x3001 从机地址设置
    MEMBER_Baud_rate_SET,              ///< 0x3002 波特率设置
    MEMBER_Save_order,                 ///< 0x3003 保存命令
    MEMBER_COUNT_0x3000                ///< 0x3000区域成员总数，用于边界检查
} IH_0x3000_MEMBER_IDX;

/**
 * @brief ARM设备索引枚举
 * @note 用于标识不同的ARM设备
 */
typedef enum {
    ARM_DEVICE_1 = 0,                  ///< 第一个ARM设备
    ARM_DEVICE_2,                      ///< 第二个ARM设备
	ARM_DEVICE_3,                      ///< 第三个ARM设备
	ARM_DEVICE_4,                      ///< 第四个ARM设备	
    ARM_DEVICE_COUNT                   ///< ARM设备总数，用于边界检查
} ARM_DEVICE_IDX;





/**
 * @brief 获取0x3000区域系统设置参数数据
 * @param member_idx 参数索引，使用 IH_0x3000_MEMBER_IDX 枚举值
 * @param arm_idx ARM设备索引，使用 ARM_DEVICE_IDX 枚举值
 * @return 参数值，如果索引无效返回 0xFFFF
 * @note 对应地址范围：0x3000-0x3003
 */
unsigned short Get_0x3000_ADDR_DATA(IH_0x3000_MEMBER_IDX member_idx, ARM_DEVICE_IDX arm_idx);
/**
 * @brief 获取0x2000区域参数数据
 * @param member_idx 参数索引，使用 IH_0x2000_MEMBER_IDX 枚举值
 * @param arm_idx ARM设备索引，使用 ARM_DEVICE_IDX 枚举值
 * @return 参数值，如果索引无效返回 0xFFFF
 * @note 对应地址范围：0x2000-0x2013
 */
unsigned short Get_0x2000_ADDR_DATA(IH_0x2000_MEMBER_IDX member_idx, ARM_DEVICE_IDX arm_idx);



//========================================================================
// 函数: 
// 描述: Modbus 协议 解析
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 主函数调用 
//========================================================================
void Modbus_Protocol_Analysis_Main(void);

//extern	unsigned char MB_Uart_Rx_Data[DF_MB_Uart_Rx_LONG]; //
//extern	unsigned char MB_Uart_Rx_Long; //

#endif 



















