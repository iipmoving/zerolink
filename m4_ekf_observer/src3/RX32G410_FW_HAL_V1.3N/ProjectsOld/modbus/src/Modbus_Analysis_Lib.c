/**
  ******************************************************************************
  * 文件名程: Modbus.c 
  * 作    者: Keelycenc
  * 版    本: V1.0  
  * 编写日期: 2022-1-13
  * 功    能: Modbus RTU协议 从机底层驱动程序
  ******************************************************************************
  **/

/* 包含头文件 ----------------------------------------------------------------*/
#include "Modbus_Analysis_Lib.h"
#include 	<string.h>

//#define DF_Dbug_Modbus_UR    //使用 串口调试
#define code                const
#define DF_isARM_MDK         1		//使用的是 M0内核 编译器 手动调换顺序       	


#ifdef DF_Dbug_Modbus_UR  
#include "Uart_Dbug_Dispose.h"  
#endif

typedef struct
{
  unsigned short		 IN1;
	/* 01H 05H 读写单个强制线圈 */
	unsigned short	 D01;
	unsigned short	 D02;
	unsigned short	 D03;
	unsigned short	 D04;
}REG_VALUE;

REG_VALUE 		R_value; 


PDUData_TypeDef 	*PduData_Cofg = NULL;  // 当前配置数组
unsigned char     		 PduData_count   = 0;     // 实例总数
unsigned char     		 Now_Idx   = 0;     //当前执行的 Idx

unsigned char  			*Rx_Buf;		//接收缓存（通过解析函数传递进来）
unsigned short 			RxCount;	//接收到的 数据长度（通过解析函数传递进来）


unsigned short 			TxCount;	//发送的 数据长度 

unsigned char 			is_Modbus_Lib_Init_OK=0;	//初始化完成 标识

#if	DF_isARM_MDK
//========================================================================
// 函数: 
// 描述: mdbus 专用内存 复制，解决大小端问题
// 参数: 
// 返回:  
// 版本: VER1.0
// 日期: 2025年12月15日
// 备注: 	
//========================================================================

// ==================== 发送时：小端转大端 ====================
// MCU数据 -> Modbus网络
static void modbus_memcpy(void* dest, const void* src, unsigned int n)
{
    unsigned char* p_dest = (unsigned char*)dest;
    const unsigned short* p_src = (const unsigned short*)src;
    unsigned int reg_count = n / 2;
    unsigned int i;
    
    for(i = 0; i < reg_count; i++)
    {
        // 小端 -> 大端
        p_dest[i * 2] = (p_src[i] >> 8) & 0xFF;     // 高字节
        p_dest[i * 2 + 1] = p_src[i] & 0xFF;        // 低字节
    }
}

// ==================== 接收时：大端转小端 ====================
// Modbus网络 -> MCU数据
static void modbus_receive_memcpy(void* dest, const void* src, unsigned int n)
{
    unsigned short* p_dest = (unsigned short*)dest;
    const unsigned char* p_src = (const unsigned char*)src;
    unsigned int reg_count = n / 2;
    unsigned int i;
    
    for(i = 0; i < reg_count; i++)
    {
        // 大端 -> 小端
        // p_src[0]是高字节，p_src[1]是低字节
        p_dest[i] = (p_src[i * 2] << 8) | p_src[i * 2 + 1];
    }
}
#endif

//========================================================================
// 函数: 
// 描述: 对库的 参数进行 初始化
// 参数: 
// 返回:  
// 版本: VER1.0
// 日期: 2025年11月28日
// 备注: 	
//========================================================================
unsigned char Modbus_Init_Lib(PDUData_TypeDef* pdu_data_struct, unsigned char num)
{
	PDUData_TypeDef* current_slave; //临时变量 
	Register_Area_t* current_area;
	unsigned char slave_idx,area_idx; //循环 检索
    // 1. 参数合法性检查 (主配置)
    if (pdu_data_struct == NULL) {
        return MODBUS_INIT_ERR_NULL_POINTER; // 主配置结构体数组指针为空
    }
    if (num == 0) {
        return MODBUS_INIT_ERR_INVALID_COUNT; // 从机数量为 0
    }

    // 2. 遍历每个从机配置
    for (slave_idx = 0; slave_idx < num; slave_idx++) 
	{
         current_slave = &pdu_data_struct[slave_idx];

        // 3. 检查从机内部配置的有效性
        if (current_slave->ARM_Count == 0 || current_slave->Us_Cof_ARM_Num == NULL || current_slave->Tx_Buf == NULL ) {
             // 可以选择返回错误或跳过此从机，这里选择返回错误
            return MODBUS_INIT_ERR_INVALID_COUNT; // 或定义一个新的错误码，如 MODBUS_INIT_ERR_INVALID_ARM_CONFIG
        }

        // 4. 遍历该从机的每个内存区域配置
        for (area_idx = 0; area_idx < current_slave->ARM_Count; area_idx++) 
		{
            current_area = &current_slave->Us_Cof_ARM_Num[area_idx];

            // 5. 检查 Data_Size (最大只允许 4 字节)
            if (current_area->Data_Size != 2 && current_area->Data_Size != 4) {
                return MODBUS_INIT_ERR_INVALID_DATASIZE; // 数据大小非法
            }

            // 6. 检查 Data_ptr (无论读写都不能为空)
            if (current_area->Data_ptr == NULL) {
                return MODBUS_INIT_ERR_NULL_DATAPTR; // 数据指针为空
            }

            // 7. 检查 Data_Pyte 有效性
            if (current_area->Data_Pyte != 0 && current_area->Data_Pyte != 1) {
                return MODBUS_INIT_ERR_INVALID_DATATYPE; // 数据类型非法
            }

            // 8. 检查 Data_ptr_EEPROM (仅对可读可写区域)
            if (current_area->Data_Pyte == 1) 
			{ // 可读可写
                if (current_area->Data_ptr_EEPROM == NULL) {
                    return MODBUS_INIT_ERR_NULL_EEPROM_PTR; // 可读可写区域的 EEPROM 指针为空
                }
                if (current_area->Check_Write_Data == NULL) {
                    return MODBUS_INIT_ERR_Write_unll; // 可读可写区域的 EEPROM 指针为空
                }
            }
            // 如果是只读 (Data_Pyte == 0)，Data_ptr_EEPROM 可以为 NULL (根据备注要求)，这里不做检查
        }
    }

    // 9. 所有检查通过，设置全局实例指针和数量
    PduData_Cofg  = pdu_data_struct; // 设置全局实例指针
    PduData_count = num;             // 设置全局实例数量
	is_Modbus_Lib_Init_OK=1;	//初始化完成 标识
    return MODBUS_Lib_N_Error; // 初始化成功
}

    
// ========================================
// 内部函数：安全获取句柄
// ========================================

/**
 * @brief 根据索引获取有效的串口句柄指针
 * @param idx: 用户传入的串口逻辑索引
 * @return 若有效返回句柄指针，否则返回 NULL
 */
static PDUData_TypeDef* Modbus_internal_get_handle(unsigned char idx) 
{
    PDUData_TypeDef *huart;  // 局部句柄指针

	 if ( is_Modbus_Lib_Init_OK == 0) return NULL;	//初始化完成 标识
//    // 检查全局句柄指针是否为空（用户未绑定内存）
//    if (PduData_Cofg == NULL) return NULL;

//    // 检查启用的Modbus 从机数量
//    if (PduData_count == 0) return NULL;

//    // 检查索引是否越界
//    if (idx >= PduData_count) return NULL;

    // 获取对应索引的句柄
    huart = &PduData_Cofg[idx];

  
    // 所有检查通过，返回有效句柄
    return huart;
}


/* 宏定义 --------------------------------------------------------------------*/
#define MB_SLAVEADDR            0x01//从机地址
#define MB_ALLSLAVEADDR         0x00FF

#define FUN_CODE_01H            0x01  //功能码01H
#define FUN_CODE_02H            0x02  //功能码02H
#define FUN_CODE_03H            0x03  //功能码03H
#define FUN_CODE_04H            0x04  //功能码04H
#define FUN_CODE_05H            0x05  //功能码05H
#define FUN_CODE_06H            0x06  //功能码06H
#define FUN_CODE_10H            0x10  //功能码10H

/*
#define IS_NOT_FUNCODE(code)  (!((code == FUN_CODE_01H)||\
                                 (code == FUN_CODE_02H)||\
                                 (code == FUN_CODE_03H)||\
                                 (code == FUN_CODE_04H)||\
                                 (code == FUN_CODE_05H)||\
                                 (code == FUN_CODE_06H)||\
                                 (code == FUN_CODE_10H)))
*/

#define IS_NOT_FUNCODE(code)  (!( (code == FUN_CODE_03H)||\
                                  (code == FUN_CODE_06H)||\
                                  (code == FUN_CODE_10H)))
                               










#define EX_CODE_NONE	0x00  //无 异常码 

#define EX_CODE_01H 	0x01 //不合法功能代码(从机接收的是一种不能执行功能代码。发出查询命令后，该代码指示无程序功能。
   
#define EX_CODE_02H 	0x02 //不合法数据地址(接收的数据地址，是从机不允许的地址。
   
#define EX_CODE_03H 	0x03 //不合法数据(查询数据区的值是从机不允许的值。
   
#define EX_CODE_04H 	0x04 //从机设备故障(从机执行主机请求的动作时出现不可恢复的错误。
   
#define EX_CODE_05H 	0x05 //确认(从机已接收请求处理数据，但需要较长的处理时间，为避免主机出现超时错误而发送该确认响应。主机以此再发送一个“查询程序完成”未决定从机是否已完成处理。
   
#define EX_CODE_06H 	0x06 //从机设备忙碌(从机正忙于处理一个长时程序命令，请求主机在从机空闲时发送信息。
   
#define EX_CODE_07H 	0x07 //否定(从机不能执行查询要求的程序功能时，该代码使用十进制13或14代码，向主机返回一个“不成功的编程请求”信息。主机应请求诊断从机的错误信息。
   
#define EX_CODE_08H 	0x08 //内存奇偶校验错误(从机读扩展内存中的数据时，发现有奇偶校验错误，主机按从机的要求重新发送数据请求。




 



#define COIL_D01		0x01
#define COIL_D02		0x02
#define COIL_D03		0x03
#define COIL_D04		0x04

#define REG_IN1		  0x0020
/* 03H 读保持寄存器 */
/* 06H 写保持寄存器 */
/* 10H 写多个寄存器 */
#define HOLD_REG_01		0x0010
#define HOLD_REG_02		0x0011
#define HOLD_REG_03		0x0012
 
 
 

// CRC 高位字节值表
 code unsigned char auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低位字节值表
 code unsigned char auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/* 扩展变量 ------------------------------------------------------------------*/

/* 私有函数原形 --------------------------------------------------------------*/
static unsigned char MB_JudgeAddr(unsigned short _Addr,unsigned short _RegNum);
static unsigned char MB_JudgeAddr_06(unsigned short _Addr); 
static unsigned char MB_JudgeNum(unsigned short _RegNum,unsigned char _FunCode,unsigned short _ByteNum);
static unsigned short MB_RSP_01H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _CoilNum );
static unsigned short MB_RSP_02H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _CoilNum);
static unsigned char MB_RSP_03H(unsigned short _TxCount,unsigned short _RegNum );
static unsigned char MB_RSP_04H(unsigned short _TxCount,unsigned short _AddrOffset,unsigned short _RegNum );
static unsigned char MB_RSP_05H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _RegDATA);
static unsigned char MB_RSP_06H(unsigned short _TxCount,unsigned short _AddrOffset  );
static unsigned char MB_RSP_10H(unsigned short _TxCount,unsigned short _AddrOffset,unsigned short _RegNum);

/* 函数体 --------------------------------------------------------------------*/


//函数功能：CRC校验计算
//输入参数：_pbuf:参与校验的数据
//         _uslen:数据长度
//返 回 值：CRC16 计算结果
//说    明：已执行高/低CRC字节交换。可用于报文直接发送

unsigned short CRC16_MODBUS(unsigned char *_pbuf, unsigned char _uslen){
   unsigned char ucCRCHi = 0xFF;
   unsigned char ucCRCLo = 0xFF;        //初始化CRC寄存器
   unsigned short usIndex;              //CRC循环索引

   while(_uslen--){
    usIndex = ucCRCHi ^ *_pbuf++;//计算CRC
    ucCRCHi = ucCRCLo ^ auchCRCHi[usIndex];
    ucCRCLo = auchCRCLo[usIndex];
   }
   return ((unsigned short)ucCRCHi << 8 | ucCRCLo);
}

 

//函数功能：提取数据帧，进行解析数据帧
//输入参数：无
//返 回 值：无
//说    明：无

void MB_Parse_Data(unsigned char Idx)
{

  PDUData_TypeDef 		*PduData;//申请结构临时变量 

	#if	DF_isARM_MDK  
	unsigned short crc;//CRC 对调
	#endif
	
  PduData = Modbus_internal_get_handle(Idx); //检查合法性
  if (PduData == NULL) return;			  // 句柄无效
	
	
  PduData->Slave_Addr= Rx_Buf[0];		//从机地址
  PduData->Code = Rx_Buf[1];                   //功能码
  PduData->Addr = ((Rx_Buf[2]<<8) | Rx_Buf[3]);//寄存器起始地址
  PduData->Num  = ((Rx_Buf[4]<<8) | Rx_Buf[5]);//数量(Coil,Input,Holding Reg,Input Reg)

  PduData->_CRC = CRC16_MODBUS(Rx_Buf,RxCount-2);             //本地收到数据计算出来的CRC校验码  //(unsigned char*)&

#if	DF_isARM_MDK  
  if(PduData->CRC_Order == 0)	//CRC高低字节 对调 ==0(大端) 使用默认先发低字节 再发高字节，==1 对调
  {
	 crc=PduData->_CRC;
	 PduData->_CRC =  ((crc >> 8) & 0xFF) | ((crc & 0xFF) << 8);
  }
#endif


	PduData->byteNums = Rx_Buf[6];                                     //获得字节数？？
  PduData->Master_CRC=((Rx_Buf[RxCount-2]<<8) | Rx_Buf[RxCount-1]); //收到主机发来的  CRC结果
  PduData->ValueReg = &Rx_Buf[7];                          //写多个寄存器起始地址  //(unsigned char*)
  PduData->PtrHoldingOffset = PduData->PtrHoldingbase + PduData->Addr; //保持寄存器的起始地址
  PduData->Addr_Original=PduData->Addr;     				//记录原始的 协议地址	 
 

}




 

 /**
  * 函数功能：读取线圈状态：（读/写）
  * 输入参数：_TxCount：发送计数器
  *          _AddrOffset：地址偏移量
  *          _CoilNum：线圈数量
  * 返 回 值：Tx_Buf的数组元素坐标
  * 说    明：读取离散输出，并填充Tx_Buf
  */
static unsigned short MB_RSP_01H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _CoilNum) 
{
 	
	/*----------------------------分割线----------------------------------*/
	
	PDUData_TypeDef 	*PduData = Modbus_internal_get_handle(Now_Idx);  //检查合法性
	if (PduData == NULL) return 0;			  							// 句柄无效	
	
	PduData->Tx_Buf[_TxCount++] = 2; 
  /* 填充返回内容 */ 	
	if (_AddrOffset == COIL_D01)
	{
		PduData->Tx_Buf[_TxCount++] = R_value.D01>>8;
		PduData->Tx_Buf[_TxCount++] = R_value.D01;	
	}
	else if (_AddrOffset == COIL_D02)
	{
		PduData->Tx_Buf[_TxCount++] = R_value.D02>>8;
		PduData->Tx_Buf[_TxCount++] = R_value.D02;	
	}
	else if (_AddrOffset == COIL_D03)
	{
		PduData->Tx_Buf[_TxCount++] = R_value.D03>>8;
		PduData->Tx_Buf[_TxCount++] = R_value.D03;	
	}
	else if (_AddrOffset == COIL_D04)
	{
		PduData->Tx_Buf[_TxCount++] = R_value.D04>>8;
		PduData->Tx_Buf[_TxCount++] = R_value.D04;	
	}
	else
	{
		
		PduData->Tx_Buf[_TxCount++] = 0;
		PduData->Tx_Buf[_TxCount++] = 0;
		
		_CoilNum++;
		
	}		
  return _TxCount;
}



 /**
  * 函数功能：读取离散输入（只读）
  * 输入参数：_TxCount：发送计数器
  *          _AddrOffset：地址偏移量
  *          _CoilNum：线圈数量
  * 返 回 值：Tx_Buf的数组元素坐标
  * 说    明：读取离散输出，并填充Tx_Buf
  */
static unsigned short MB_RSP_02H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _CoilNum){
/**
 * 主机发送：
 * 01 从机地址
 * 02 功能码
 * 00 寄存器起始地址高字节
 * 01 寄存器起始地址低字节
 * 00 寄存器数量搞自己
 * 08 寄存器数量低字节
 * 28 CRC校验高字节
 * 0C CRC校验低字节
 * 
 * 从机应答：1代表ON,0代表OFF。若返回的线圈数不为8的倍数，则在最后数据字节末尾使用0代替，BIT0对应第一个
 * 01 从机地址
 * 02 功能码
 * 01 返回字节数
 * 02 数据1（线圈0002H-线圈0011H）
 * 0D CRC校验高字节
 * 49 CRC校验低字节
 * 
 * 例子1：
 * 发送：01 01 00 02 00 08  9C 0C   查询D02开始的8个继电器状态
 * 返回：01 01 01 02        D0 49   查询到8个状态为：0000 0010
 */
unsigned short i = 0;
	unsigned short m;	
	unsigned char status[10];	

	PDUData_TypeDef 	*PduData = Modbus_internal_get_handle(Now_Idx);  //检查合法性
	if (PduData == NULL) return 0;
	
	
	
  /* 计算返回字节数（_CoilNum变量是以位为单位） */
  m = (_CoilNum+7)/8;
  /* 返回字节数（数量）*/
	PduData->Tx_Buf[_TxCount++] = m; 
	  
	if ((_AddrOffset >= COIL_D01) && (_CoilNum > 0))
  {
		/* 将获取的线圈状态先清零 */
		for (i = 0; i < m; i++)
		{
			status[i] = 0;
		}		
		/* 获取对应线圈状态，并将其写入status[] */
		for (i = 0; i < _CoilNum; i++)
		{
			/* 读LED的状态，写入状态寄存器的每一位 */
//			if (Get_LEDx_State(i + 1 + _AddrOffset - COIL_D01))		
//			{  
//				status[i / 8] |= (1 << (i % 8));
//			}			
		}
	}
	/* 填充发送内容 */
	for (i = 0; i < m; i++)
	{
		/* 继电器状态 */
		PduData->Tx_Buf[_TxCount++] = status[i];	
	}	
  return _TxCount;
}








//========================================================================
// 函数: 
// 描述: 检查写入寄存器的 值是否在范围之内
// 参数: 	
// 返回: ==1 数据出现错误 无法写入EEPROM
// 版本: VER1.0
// 日期: 2024年8月13日
// 备注: 	
//========================================================================
//unsigned char Check_Write_DATA_Confine_TO(void)
//{

//	unsigned char ERROR=0;
// 
//	//对写入的值 进行 范围判定 超出范围判错
////	if(SR_DATA.BigPotWarmTemp_SetDat<0  ||	SR_DATA.BigPotWarmTemp_SetDat>100)	ERROR=1;	// 
////	if(SR_DATA.RAM_COMM_ADDR<01 ||	SR_DATA.RAM_COMM_ADDR>247)	ERROR=1;	//通讯地址 
////	if(SR_DATA.RAM_Temp_AOUT_UP>300)							ERROR=1;	//温度自动上报功能
////	if(SR_DATA.RAM_Temp_Mode<1 	|| SR_DATA.RAM_Temp_Mode>12)	ERROR=1;	//测温模式，S、R、B、K、N、E、J、T、PT100、PT10、CU100、CU50 (从1开始数  4代表K型
////	if(SR_DATA.RAM_TempADJ_A<50 || SR_DATA.RAM_TempADJ_A>1024)	ERROR=1;	//校准数据 A点
////	if(SR_DATA.RAM_TempADJ_B<50 || SR_DATA.RAM_TempADJ_B>1024)	ERROR=1; 	//校准数据 B点
//	 

////	if(ERROR) //将eeprom数据 反写回缓存， 写入无效!!
////	{
////		Sys_MemCpy(&SR_DATA ,&SR_DATA_EEPROM ,sizeof(Fun_ADDR)  ); // 内存复制
////	}
////	else 
////	{ //写入正确 搬走数据
////		Sys_MemCpy(&SR_DATA_EEPROM ,&SR_DATA ,sizeof(Fun_ADDR)  ); // 内存复制
////	}
////	
////	return ERROR;

//}

	
	
	 
 
//========================================================================
// 函数功能：读取保持寄存器（读/写）
// 输入参数：_TxCount：发送计数器
//          _AddrOffset：地址偏移量
//          _CoilNum：线圈数量
// 返 回 值：Tx_Buf的数组元素坐标
// 说    明：读取保持寄存器的内容，并填充Tx_Buf
//======================================================================== 
/**
 * @brief   构建并填充 Modbus 功能码 0x03 (读保持寄存器) 的响应 PDU 数据部分。
 * @details 此函数负责将用户数据从对应的内存区域复制到发送缓冲区 Tx_Buf 中，
 *          从 PduData->Tx_Buf[3] 开始存放。
 * @param   _TxCount: 发送缓冲区中已使用的字节数 (调用前应包含从机地址、功能码)。
 * @param   _RegNum:  要读取的寄存器数量。
 * @return  unsigned short: 更新后的发送缓冲区总字节数 (_TxCount + 1 + _RegNum * 2)。
 * @pre     PduData->Addr 已设置为请求的起始 Modbus 地址 (寄存器地址)。
 * @pre     PduData->Now_Addr_Crew 已设置为匹配请求地址的内存区域索引。
 * @pre     PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Data_ptr 指向有效的结构体实例。
 * @pre     请求的地址范围 [PduData->Addr, PduData->Addr + _RegNum - 1] 必须完全落在
 *          PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew] 定义的区域内。
 * @note    此函数不负责填充从机地址、功能码和最终的 CRC。
 *          它假设 Tx_Buf 有足够的空间容纳即将写入的数据。
 *          Modbus 地址是以 16-bit 寄存器(2 bytes) 为单位的。
 */
static unsigned char MB_RSP_03H(unsigned short _TxCount, unsigned short _RegNum) 
{
    PDUData_TypeDef *PduData = NULL;
    Register_Area_t *CurrentArea = NULL;
    unsigned short start_reg_index = 0; // 起始寄存器在 Data_ptr 指向数据中的索引
    unsigned short *src_data_start_ptr = NULL;

    // 1. 获取当前从机句柄
    PduData = Modbus_internal_get_handle(Now_Idx);
    if (PduData == NULL) {
        // 根据你的原始代码，返回原 _TxCount 表示错误或无响应
        return 0;
    }

    // 2. 安全检查：确保 Now_Addr_Crew 索引有效 (防御性编程)
    if (PduData->Now_Addr_Crew >= PduData->ARM_Count) {
        return 0; // 索引越界，返回原计数
    }
    CurrentArea = &(PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew]);

    // 3. *** 关键修正 ***
    // 计算起始寄存器索引 (相对于该内存区域的起始寄存器地址)
    // Modbus 地址是寄存器地址，所以直接相减得到的就是寄存器索引
    start_reg_index = PduData->Addr - CurrentArea->Start_Address;

    // 4. 获取指向数据结构的 unsigned short 指针
    src_data_start_ptr = (unsigned short *)CurrentArea->Data_ptr;

    // 5. *** 关键修正 ***
    // 计算实际源数据的起始地址：基地址 + 寄存器索引
    // start_reg_index 就是需要跳过的 unsigned short 的数量
    src_data_start_ptr += start_reg_index;

    // 6. 填充字节数 (Byte Count)
    // _TxCount 当前应指向功能码之后的位置
    PduData->Tx_Buf[_TxCount++] = (unsigned char)(_RegNum * 2); // 字节数 = 寄存器数 * 2

    // 7. 内存复制：将用户数据复制到 Tx_Buf
    // 从 PduData->Tx_Buf[_TxCount] (即功能码后的第二个字节) 开始复制
    // 复制 _RegNum 个寄存器的数据，每个寄存器 2 字节
   #if DF_isARM_MDK
	modbus_memcpy(&(PduData->Tx_Buf[_TxCount]), src_data_start_ptr, _RegNum * 2);
   #else
	memcpy(&(PduData->Tx_Buf[_TxCount]), src_data_start_ptr, _RegNum * 2);
   #endif
	
	


    // 8. 更新并返回总的发送字节数
    _TxCount += _RegNum * 2; // 增加数据部分的长度

    return _TxCount;
}




 /**
  * 函数功能：读输入寄存器（与上述03H指令类似，返回内容也类似）只读
  * 输入参数：_TxCount：发送计数器
  *          _AddrOffset：地址偏移量
  *          _CoilNum：线圈数量
  * 返 回 值：Tx_Buf的数组元素坐标
  * 说    明：读取保持寄存器的内容，并填充Tx_Buf
  */
 static unsigned char MB_RSP_04H(unsigned short _TxCount,unsigned short _AddrOffset,unsigned short _RegNum){
/**
 * 主机发送：
 * 01 从机地址
 * 04 功能码
 * 00 寄存器起始地址高字节
 * 20 寄存器起始地址低字节
 * 00 寄存器个数高字节
 * 02 寄存器个数低字节
 * 70 CRC校验高字节
 * 01 CRC校验低字节
 * 
 * 从机应答：保持寄存器的长度为两个字节。对于单个保持寄存器而言，寄存器高字节数据先被传输，
 * 低字节数据后被传输。保持寄存器直接，低地址寄存器先被传输，高地址寄存器后被传输。
 * 01 从机地址
 * 04 功能码
 * 02 字节数
 * 02 数据1高字节（0020H）
 * 03 数据1低字节（0020H）
 * 00 数据2高字节（0021H）
 * 00 数据2低字节（0021H）
 * 82 CRC校验高字节
 * 3C CRC校验低字节
 * 
 * 例子：
 * 发送：01 04 00 20 00 02      70 01    读0020 IN1 开始的两个字节数据内容
 * 返回：01 04 02 02 03 00 00   82 3C    返回：02 03 00 00 四个数据
*/
	unsigned char i;
	unsigned short reg_value[64];
	PDUData_TypeDef 	*PduData = Modbus_internal_get_handle(Now_Idx);  //检查合法性
	if (PduData == NULL) return 0;		
	
  /* 填充返回寄存器数量 */
  PduData->Tx_Buf[_TxCount++] = _RegNum;
	/* 读取保持寄存器内容 */
	for(i = 0; i < _RegNum; i++)
	{
		switch (_AddrOffset)
		{
			/* 测试参数 */
			case REG_IN1:
				reg_value[i] = R_value.IN1;
				break;
		
			default:
				reg_value[i] = 0;
				break;
		}
		_AddrOffset++;
	}
  /* 填充返回内容 */
  for(i = 0;i< _RegNum;i++)
  {
    PduData->Tx_Buf[_TxCount++] = reg_value[i]>>8;
    PduData->Tx_Buf[_TxCount++] = reg_value[i]& 0xFF;
  }
  return _TxCount;
}


 /**
  * 函数功能：写单个线圈（读/写）
  * 输入参数：_TxCount：发送计数器
  *          _AddrOffset：地址偏移量
  *          _CoilNum：线圈数量
  * 返 回 值：Tx_Buf的数组元素坐标
  * 说    明：并填充Tx_Buf
  */
static unsigned char MB_RSP_05H(unsigned short _TxCount,unsigned short _AddrOffset ,unsigned short _RegDATA){
/**
 * 主机发送：写单个线圈寄存器。例如：简单的01~03寄存器地址对应LED1~LED3，将数据存放于D01/D02/D03/D04四个成员中
 * 05H指令设置单个线圈的状态
 * 01 从机地址
 * 05 功能码
 * 00 寄存器地址高字节
 * 01 寄存器地址低字节
 * FF 数据1高字节
 * FF数据1低字节
 * 9D CRC校验高字节
 * BA CRC校验低字节
 * 
 * 从机应答：
 * 01 从机地址
 * 05 功能码
 * 00 寄存器地址高字节
 * 01 寄存器地址低字节
 * FF 寄存器1高字节
 * FF 寄存器1低字节
 * 9D CRC校验高字节
 * BA CRC校验低字节
 * 
 * 例子：
 * 发送：01 05 00 04 FF FF    8D BB   发送数据0xFFFF至0x04地址线圈中
 * 返回：01 05 00 04 FF FF    8D BB   返回原始数据
*/
  /* 填充地址值 */
	PDUData_TypeDef 	*PduData = Modbus_internal_get_handle(Now_Idx);  //检查合法性
	if (PduData == NULL) return 0;		
  
  
  PduData->Tx_Buf[_TxCount++] = _AddrOffset>>8;
  PduData->Tx_Buf[_TxCount++] = _AddrOffset;
	
	if (_AddrOffset == COIL_D01)
	{
		R_value.D01 = _RegDATA;	
   // LED1_ON;		
	}
	else if (_AddrOffset == COIL_D02)
	{
		R_value.D02 = _RegDATA;
	//	LED2_ON;
	}
	else if (_AddrOffset == COIL_D03)
	{
		R_value.D03 = _RegDATA;
	//	LED3_ON;
	}
	else if (_AddrOffset == COIL_D04)
	{
		R_value.D04 = _RegDATA;
		//LED1_TOGGLE;
		//LED2_TOGGLE;
		//LED3_TOGGLE;
	}
	else
	{
	  R_value.D01 = 0;
		R_value.D02 = 0;
		R_value.D03 = 0;
		R_value.D04 = 0;
	}
	PduData->Tx_Buf[_TxCount++] = _RegDATA>>8;
	PduData->Tx_Buf[_TxCount++] = _RegDATA;	
  return _TxCount;
}


  



/**
 * @brief   构建并填充 Modbus 功能码 0x06 (写单个保持寄存器) 的响应 PDU。
 * @details 此函数负责将接收到的单个寄存器数据写入对应的 RAM 和 EEPROM (模拟) 内存区域，
 *          并构建回显响应报文。
 * @param   _TxCount: 发送缓冲区中已使用的字节数 (调用前应包含从机地址、功能码)。
 * @param   _AddrOffset: 请求写入的 Modbus 寄存器地址 (注意：是寄存器地址，不是字节地址)。
 * @return  unsigned char: 更新后的发送缓冲区总字节数 (_TxCount + 4)。
 *                         返回 0 表示检查失败或不应发送响应。
 * @pre     Modbus_Init_Lib 已成功调用。
 * @pre     PduData->Now_Addr_Crew 已由地址判断函数设置为有效的内存区域索引。
 * @pre     RxCount >= 6 (包含地址、功能码、地址、数据、CRC)。
 * @pre     CurrentArea->Check_Write_Data != NULL (由 Modbus_Init_Lib 保证)。
 * @note    此函数假设 Modbus 地址是按寄存器寻址的 (每个寄存器 2 字节)。
 *          写入的数据按网络字节序存储在 Rx_Buf 中，memcpy 保证了正确的字节顺序。
 *          用户检查函数 Check_Write_Data 被调用一次来验证写入操作。
 *          如果 Check_Write_Data 返回 0，则认为写入失败。
 */
static unsigned char MB_RSP_06H(unsigned short _TxCount, unsigned short _AddrOffset)
{
    PDUData_TypeDef     *PduData = NULL;
    Register_Area_t     *CurrentArea = NULL;
    unsigned short      register_offset = 0;   // 相对于区域起始地址的寄存器偏移
    unsigned short      byte_offset = 0;       // 相对于区域起始地址的字节偏移
    unsigned char       *dest_ram_ptr = NULL; // 指向 RAM 中要写入位置的指针
    unsigned char       *dest_eeprom_ptr = NULL; // 指向 EEPROM 中要写入位置的指针
    unsigned short      data_to_write = 0;    // 从 Rx_Buf 提取的待写入数据

    // 1. 获取当前从机句柄
    PduData = Modbus_internal_get_handle(Now_Idx);
    if (PduData == NULL) {
        return 0; // 句柄无效，返回错误
    }

    // 2. 安全检查：确保 Now_Addr_Crew 索引有效
    if (PduData->Now_Addr_Crew >= PduData->ARM_Count) {
        return 0; // 索引越界，返回错误
    }

    // 3. 获取当前内存区域配置
    CurrentArea = &(PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew]);

    // 4. 计算寄存器偏移量 (关键修改)
    // _AddrOffset 和 Start_Address 现在被解释为 Modbus 寄存器地址
    // End_Address 也应该相应地是寄存器地址
    register_offset = _AddrOffset - CurrentArea->Start_Address;

    // 5. 计算字节偏移量 (关键修改)
    // 每个寄存器占 2 字节，所以字节偏移是寄存器偏移的两倍
    byte_offset = register_offset * 2;

    // 6. 计算 RAM 和 EEPROM (模拟) 中的目标地址
    if (CurrentArea->Data_ptr == NULL) { // 冗余检查
        return 0;
    }
    dest_ram_ptr = (unsigned char *)(CurrentArea->Data_ptr) + byte_offset;

    // 7. 从 Rx_Buf 获取要写入的 2 字节数据 (高位在前)
    // Rx_Buf[0]=Addr, Rx_Buf[1]=Func, Rx_Buf[2,3]=Addr, Rx_Buf[4,5]=Data, Rx_Buf[6,7]=CRC
    if (RxCount < 6) {
         return 0; // 数据不完整
    }
    data_to_write = (((unsigned short)Rx_Buf[4]) << 8) | ((unsigned short)Rx_Buf[5]);
    // R_DATA_LONG = 2; // 06H命令是 单长度写入 所以总是写入2 字节 (此变量在此函数中未使用，可删除)

    // 8. 写入 RAM 
	 memcpy(dest_ram_ptr, &data_to_write, sizeof(data_to_write)); //修改缓冲区 内容 
  
    // 9. 调用用户自定义的合法性检查函数
    // 因 Modbus_Init_Lib 已保证 Check_Write_Data != NULL，直接调用
    if (CurrentArea->Check_Write_Data() == 1) { //用户检查写入数据为合法数据
        // 10a. 检查通过，写入 EEPROM (模拟) (如果配置了)
        if (CurrentArea->Data_ptr_EEPROM != NULL) { // 冗余检查
             dest_eeprom_ptr = (unsigned char *)(CurrentArea->Data_ptr_EEPROM) + byte_offset;
 
			memcpy(dest_eeprom_ptr, &data_to_write, sizeof(data_to_write));
  
        }

        // 10b. 构建成功的响应报文 (回显请求的地址和数据)
        // 填充地址 (高位在前)
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_AddrOffset >> 8);
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_AddrOffset & 0xFF);

        // 填充数据 (高位在前，与接收时一致)
        PduData->Tx_Buf[_TxCount++] = Rx_Buf[4]; // Data High Byte
        PduData->Tx_Buf[_TxCount++] = Rx_Buf[5]; // Data Low Byte        

    } else {
        // 11. 用户检查未通过
#ifdef DF_Dbug_Modbus_UR
        USCI0_Dbug_Print("FW06 写入数据非用户期望值\r\n");
#endif
        _TxCount = 0; // 不应发送成功响应
    }    

    // 12. 返回总的发送字节数 (Addr_Hi, Addr_Lo, Data_Hi, Data_Lo)
    // 如果 _TxCount 被置 0，则表示不应发送或发送错误
    return _TxCount;
}



/**
 * @brief   构建并填充 Modbus 功能码 0x10 (写多个保持寄存器) 的响应 PDU。
 * @details 此函数负责将接收到的多个寄存器数据批量写入对应的 RAM 和 EEPROM (模拟) 内存区域，
 *          并构建回显响应报文。依赖于 Modbus_Init_Lib 已成功执行，确保配置有效。
 * @param   _TxCount: 发送缓冲区中已使用的字节数 (调用前应包含从机地址、功能码)。
 * @param   _AddrOffset: 请求写入的起始 Modbus 寄存器地址 (注意：是寄存器地址)。
 * @param   _RegNum: 请求写入的寄存器数量。
 * @return  unsigned char: 更新后的发送缓冲区总字节数 (_TxCount + 4)。
 *                         返回 0 表示检查失败或不应发送响应。
 * @pre     Modbus_Init_Lib 已成功调用。
 * @pre     PduData->Now_Addr_Crew 已由地址判断函数设置为有效的内存区域索引。
 * @pre     Rx_Buf[7] 开始包含 _RegNum * 2 字节的有效数据。
 * @pre     RxCount >= (7 + _RegNum * 2) (包含地址、功能码、地址、数量、字节计数、数据、CRC)。
 * @pre     CurrentArea->Check_Write_Data != NULL (由 Modbus_Init_Lib 保证)。
 * @note    此函数假设 Modbus 地址是按寄存器寻址的 (每个寄存器 2 字节)。
 *          写入的数据按网络字节序存储在 Rx_Buf 中，memcpy 保证了正确的字节顺序。
 *          用户检查函数 Check_Write_Data 被调用一次来验证整个写入操作。
 *          如果 Check_Write_Data 返回 0，则认为写入失败。
 */
static unsigned char MB_RSP_10H(unsigned short _TxCount, unsigned short _AddrOffset, unsigned short _RegNum)
{
    PDUData_TypeDef     *PduData = NULL;
    Register_Area_t     *CurrentArea = NULL;
    unsigned short      register_offset = 0;   // 相对于区域起始地址的寄存器偏移
    unsigned short      byte_offset = 0;       // 相对于区域起始地址的字节偏移
    unsigned char       *dest_ram_ptr = NULL; // 指向 RAM 中要写入位置的指针
    unsigned char       *dest_eeprom_ptr = NULL; // 指向 EEPROM 中要写入位置的指针
    unsigned short      R_DATA_LONG = 0;        // 要写入的总字节数
    unsigned char       byte_count_from_request = 0; // 请求中的字节计数字段

    // 1. 获取当前从机句柄
    PduData = Modbus_internal_get_handle(Now_Idx);
    if (PduData == NULL) {
        return 0; // 句柄无效，返回错误
    }

    // 2. 安全检查：确保 Now_Addr_Crew 索引有效
    if (PduData->Now_Addr_Crew >= PduData->ARM_Count) {
        return 0; // 索引越界，返回错误
    }

    // 3. 获取当前内存区域配置
    CurrentArea = &(PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew]);

    // 4. 计算寄存器偏移量 (关键修改)
    // _AddrOffset 和 Start_Address 现在被解释为 Modbus 寄存器地址
    register_offset = _AddrOffset - CurrentArea->Start_Address;

    // 5. 计算字节偏移量 (关键修改)
    // 每个寄存器占 2 字节，所以字节偏移是寄存器偏移的两倍
    byte_offset = register_offset * 2;

    // 6. 计算 RAM 中的目标地址
    if (CurrentArea->Data_ptr == NULL) { // 冗余检查
        return 0; // 数据指针无效
    }
    dest_ram_ptr = (unsigned char *)(CurrentArea->Data_ptr) + byte_offset;

    // 7. 计算要写入的总字节数 (每个寄存器 2 字节)
    R_DATA_LONG = _RegNum * 2;
    byte_count_from_request = Rx_Buf[6];

    // 8. 数据完整性检查
    if (RxCount < (7 + R_DATA_LONG)) {
         return 0; // 接收到的数据不完整
    }

    // 9. 验证 Rx_Buf[6] (Byte Count) 是否等于 R_DATA_LONG
    if (byte_count_from_request != R_DATA_LONG) {
#ifdef DF_Dbug_Modbus_UR
        USCI0_Dbug_Print("FC10 数据长度不匹配\r\n");
#endif
        return 0; // 字节计数不匹配，协议错误
    }

    // 10. 写入 RAM (批量)
    // 从 Rx_Buf[7] 开始复制 R_DATA_LONG 字节到 dest_ram_ptr
   
		#if DF_isARM_MDK
		 modbus_receive_memcpy(dest_ram_ptr, &Rx_Buf[7], R_DATA_LONG);
		#else
		 memcpy(dest_ram_ptr, &Rx_Buf[7], R_DATA_LONG);
		#endif
    // 11. 调用用户自定义的合法性检查函数
    // 因 Modbus_Init_Lib 已保证 Check_Write_Data != NULL，直接调用
    if (CurrentArea->Check_Write_Data() == 1) { // 检查函数返回 1 (允许写入)
        // 12a. 检查通过，写入 EEPROM (模拟) (如果配置了)
        if (CurrentArea->Data_ptr_EEPROM != NULL) { // 冗余检查
             dest_eeprom_ptr = (unsigned char *)(CurrentArea->Data_ptr_EEPROM) + byte_offset;
			
		#if DF_isARM_MDK
		 modbus_receive_memcpy(dest_eeprom_ptr, &Rx_Buf[7], R_DATA_LONG); // 同样从 Rx_Buf[7] 复制
		#else
		memcpy(dest_eeprom_ptr, &Rx_Buf[7], R_DATA_LONG); // 同样从 Rx_Buf[7] 复制
		#endif
             
        }

        // 12b. 构建成功的响应报文 (回显请求的起始地址和寄存器数量)
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_AddrOffset >> 8);
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_AddrOffset & 0xFF);
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_RegNum >> 8);
        PduData->Tx_Buf[_TxCount++] = (unsigned char)(_RegNum & 0xFF);

        // _TxCount 已在成功分支内增加了 4

    } else {
        // 13. 用户检查未通过
#ifdef DF_Dbug_Modbus_UR
        USCI0_Dbug_Print("FC10 写入数据非用户期望值\r\n");
#endif
        _TxCount = 0; // 不应发送成功响应
    }

    // 14. 返回总的发送字节数
    return _TxCount;
}




  /**
  * 函数功能：异常响应
  * 输入参数：_FunCode：发送异常的功能码
  *          _ExCode：异常码
  * 返 回 值：无
  * 说    明：当通讯数据帧发生异常时，发送异常响应
  */
void MB_Exception_RSP(unsigned char _FunCode,unsigned char _ExCode)
{
	PDUData_TypeDef 		*PduData;//申请结构临时变量 
//	unsigned short TxCount = 0;
	unsigned short crc = 0;
	
	PduData = Modbus_internal_get_handle(Now_Idx); //检查合法性	
	
	TxCount = 0;	//重置 数据长度 
	PduData->Tx_Buf[TxCount++] = PduData->Slave_Hardware_Addr;	/* 从站地址 */
	PduData->Tx_Buf[TxCount++] = _FunCode|0x80;				  	/* 功能码 + 0x80*/	
	PduData->Tx_Buf[TxCount++] = _ExCode ;	          			/* 异常码*/
	
	crc = CRC16_MODBUS(PduData->Tx_Buf,TxCount);//(unsigned char*)
	PduData->Tx_Buf[TxCount++] = crc;	         				 /* crc 低字节 */
	PduData->Tx_Buf[TxCount++] = crc>>8;		      				/* crc 高字节 */

//  UART_Tx((unsigned char*)Tx_Buf, TxCount);
//	 Uart_Send_DATA((unsigned char*)Tx_Buf, TxCount);
	 
	 
	 
//#pragma message " 串口调试 "
 }







// 函数功能：判断功能码 0x06 (写单个寄存器) 的地址是否符合协议范围及权限
// 输入参数：_Addr：要写入的 Modbus 寄存器地址
// 返回值：EX_CODE_NONE (0) 表示合法，其他值为异常码
// 说明：此函数会更新 PduData->Now_Addr_Crew (如果找到匹配区域)
unsigned char MB_JudgeAddr_06(unsigned short _Addr)
{
    unsigned char Excode = EX_CODE_NONE;
    unsigned char Addr_i;
    PDUData_TypeDef *PduData = NULL; // 使用局部变量更清晰

    PduData = Modbus_internal_get_handle(Now_Idx);
    if (PduData == NULL) {
        return EX_CODE_02H; // 或内部错误码
    }

   

    // --- 地址匹配循环 (仅检查单个地址 _Addr 是否存在) ---
    for (Addr_i = 0; Addr_i < PduData->ARM_Count; Addr_i++) 
	{
        Register_Area_t *current_area = &(PduData->Us_Cof_ARM_Num[Addr_i]);

        // 检查单个地址是否在该区域内
        if ((_Addr >= current_area->Start_Address) &&  (_Addr < current_area->End_Address)) 
		{ 

            PduData->Now_Addr_Crew = Addr_i;
#ifdef DF_Dbug_Modbus_UR
            USCI0_Dbug_Print("FC06 内存组匹配 \r\n");
#endif
            break;
        }
		else
		{
			PduData->Now_Addr_Crew = 0xFF; // 初始化为无效索引
		}
    }

    // --- 判断是否找到匹配项 ---
    if (PduData->Now_Addr_Crew == 0xFF) 
	{
        Excode = EX_CODE_02H; // 非法数据地址
#ifdef DF_Dbug_Modbus_UR
        USCI0_Dbug_Print("FC06 地址超范围\r\n");
#endif
        return Excode;
    }
	

#ifdef DF_Dbug_Modbus_UR
    sprintf(Dbug_TX_Buf, "FC06 第%d组内存\r\n", (int)PduData->Now_Addr_Crew);
    USCI0_Dbug_Print(Dbug_TX_Buf);
#endif

    // --- 写入权限检查 ---
    // 功能码 0x06 是写单个寄存器，必须检查可写性
    if (PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Data_Pyte == 0) 
	{ // 该区域只读
        Excode = EX_CODE_02H; // 非法数据地址 (或 EX_CODE_01H? 看具体规范)
#ifdef DF_Dbug_Modbus_UR
        USCI0_Dbug_Print("FC06 该地址禁止写入\r\n");
#endif
        return Excode;
    }


    // --- 检查 4 字节对齐 (如果 Data_Size == 4 且表示必须对齐写入) ---
    // (同 MB_JudgeAddr 的逻辑)
    if (PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Data_Size == 4) 
	{
        unsigned short offset_in_area = _Addr - PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Start_Address;
            
        if ((offset_in_area % 2) != 0) 
		{
            Excode = EX_CODE_02H;
#ifdef DF_Dbug_Modbus_UR
            USCI0_Dbug_Print("FC06 4字节数据起始地址未对齐\r\n");
#endif
            return Excode;
        }
    }


    // --- 所有检查通过 ---
    return Excode; // 返回 EX_CODE_NONE (0)
}
 


   /**
  * 函数功能：判断操作的数量是否符合协议范围
  * 输入参数：_RegNum：寄存器数量
  *          _FunCode：功能码
  *          _ByteNum：字节数量
  * 返 回 值：
  * 说    明：
  */
unsigned char MB_JudgeNum(unsigned short _RegNum,unsigned char _FunCode,unsigned short _ByteNum)
{
  unsigned char Excode = EX_CODE_NONE;
  unsigned short _CoilNum = _RegNum; // 线圈(离散量)的数量
  switch(_FunCode)
  {
    case FUN_CODE_01H: 
    case FUN_CODE_02H:
      if( (_CoilNum<0x0001) || (_CoilNum>0x07D0))
        Excode = EX_CODE_03H;// 异常码03H;
      break;
    case FUN_CODE_03H:
    case FUN_CODE_04H:
      if( (_RegNum<0x0001) || (_RegNum>0x007D))
        Excode = EX_CODE_03H;// 异常码03H;      
      break;
    case FUN_CODE_10H:
      if( (_RegNum<0x0001) || (_RegNum>0x007B))
        Excode = EX_CODE_03H;// 异常码03H
      if( _ByteNum != (_RegNum<<1))
        Excode = EX_CODE_03H;// 异常码03H
      break;
  }
  return Excode;
}








//函数功能：对接收到的信息进行分析并执行
//输入参数：无
//返 回 值：异常码0x00
//说    明：判断功能码，验证地址，数据是否溢出，数据没错误发送响应信号

unsigned char MB_Analyze_Execute(unsigned char Idx)
{
    unsigned char ExCode = EX_CODE_NONE; //默认无异常 
	
	PDUData_TypeDef 		*PduData;//申请结构临时变量  
	PduData = Modbus_internal_get_handle(Idx); //检查合法性 
	if (PduData == NULL)		// 句柄无效
	{
		return ExCode;
	}

  /* 校验码功能 */
  if( IS_NOT_FUNCODE(PduData->Code) ) // 不支持的功能码
  {
    /* MODBUS异常响应 */
    ExCode = EX_CODE_01H;            
    return ExCode;
  }
  
  
  /* 根据功能码分别判断 */
  switch(PduData->Code)
  {
    /* 01H和02H大致是一样的，操作地址可能不一样
     * 这一点结合具体来实现，可以在main函数里申请单独的内存
     * 使用不同的功能码，在实际应用中必须加以区分使用不同的内存空间
     */
/* ---- 01H  02H 读离散输入寄存器(Coil Input)---------------------- */
    case FUN_CODE_01H:
    case FUN_CODE_02H:
      /* 判断线圈数量 */  
      ExCode = MB_JudgeNum(PduData->Num,PduData->Code,1);
      if(ExCode != EX_CODE_NONE )
        return ExCode;      
      
      /* 判断地址 */
      ExCode = MB_JudgeAddr( PduData->Addr,PduData->Num);
      if(ExCode != EX_CODE_NONE )
        return ExCode;  
      break;
/* ---- 03H  04H 读保持/读输入寄存器---------------------- */
    case FUN_CODE_03H:
    case FUN_CODE_04H:
      /* 判断寄存器数量 */
		ExCode = MB_JudgeNum(PduData->Num,PduData->Code,PduData->byteNums);//：判断操作的数量是否符合协议范围
		if(ExCode != EX_CODE_NONE )
		{
			return ExCode;  
		}
	   
			 
			ExCode = MB_JudgeAddr( PduData->Addr,PduData->Num);//判断地址是否符合协议范围（返回内存值，变量值）
			if(ExCode != EX_CODE_NONE )
			{
#ifdef DF_Dbug_Modbus_UR 
				USCI0_Dbug_Print("ARM addr Err \r\n");//发送字符串  
#endif
				return ExCode;  
			}
			
			break;
/* ---- 05H 写单个线圈---------------------- */
    case FUN_CODE_05H:
      break;

/* ---- 06H 写单个保持 ---------------------- */
    case FUN_CODE_06H:
			ExCode = MB_JudgeAddr_06( PduData->Addr);
			if(ExCode != EX_CODE_NONE )
			{
				return ExCode;  
			}
			break;	
	 
// ---- 10H 写多个保持 ----------------------
    case FUN_CODE_10H: 
			//判断寄存器数量 
			ExCode = MB_JudgeNum(PduData->Num,PduData->Code,PduData->byteNums);
			if(ExCode != EX_CODE_NONE )
			{
				return ExCode;
			} 
			// 判断寄存器地址
			ExCode = MB_JudgeAddr( PduData->Addr,PduData->Num);		      		
			if(ExCode != EX_CODE_NONE )
			{
				return ExCode;  	
			} 
			break;

	
	  
	  
  }
  
  
  /* 数据帧没有异常 */
  return ExCode; //   EX_CODE_NONE
}








//函数功能：判断地址是否符合协议范围
//输入参数：_Addr：起始地址
//         _RegNum：寄存器数量
//返 回 值：
//说    明：
 unsigned char MB_JudgeAddr(unsigned short _Addr,unsigned short _RegNum)
{
	unsigned char Excode = EX_CODE_NONE;
	unsigned char Addr_i;//内存 地址范围检查 
	unsigned char Addr_S_or_D;//地址单双 检查  single or double
	
	
	
	PDUData_TypeDef 		*PduData = Modbus_internal_get_handle(Now_Idx); //检查合法性 
	if (PduData == NULL) 
	{
		return	Excode;	// 句柄无效
	}
	
	for(Addr_i=0;Addr_i<PduData->ARM_Count;Addr_i++)//每个协议里面 有多少组协议
	{
		// Modbus起始地址												// Modbus结束地址	 
		if( _Addr>=PduData->Us_Cof_ARM_Num[Addr_i].Start_Address && _Addr < PduData->Us_Cof_ARM_Num[Addr_i].End_Address )//
		{
			PduData->Now_Addr_Crew=Addr_i;
#ifdef DF_Dbug_Modbus_UR 
			USCI0_Dbug_Print("内存组匹配 \r\n");//发送字符串 
#endif
			break; // 找到匹配的区间
		}
		else
		{
			PduData->Now_Addr_Crew=0xFF;
		}	

	}
	//如果遍历完所有区间都没找到
	if(PduData->Now_Addr_Crew == 0xFF)
	{
		Excode = EX_CODE_02H; // 非法数据地址
#ifdef DF_Dbug_Modbus_UR 
		USCI0_Dbug_Print("地址超范围\r\n");//发送字符串 
#endif
		return Excode;
	}

#ifdef DF_Dbug_Modbus_UR 	
		sprintf (Dbug_TX_Buf,"第%d组内存\r\n",(int)PduData->Now_Addr_Crew);
		USCI0_Dbug_Print(Dbug_TX_Buf);	//发送字符串  
#endif	
	
	
	
	//计算当前 获取的数量 是否超出协议的 最大数量？
	if(_RegNum > (PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].End_Address - _Addr))
	{
		 Excode = EX_CODE_02H;// 异常码 02H
#ifdef DF_Dbug_Modbus_UR 
			USCI0_Dbug_Print("索取数据长度超出范围\r\n");//发送字符串 
#endif	
		 return Excode;
	}

	if(PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Data_Size == 4) //4字节时 区分读取的变量 地址是单数还是双数 
	{
		Addr_S_or_D=PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Start_Address &0x01; //在modbus协议中 1个长度 实际占用2字节，所有起始地址是 双数 不可能出现单数地址 反之也一样
		if( (_Addr&0x01) !=Addr_S_or_D )
		{
			Excode = EX_CODE_02H;// 异常码 02H
			return Excode;
		}	

	}	
 
	if( PduData->Code == 06 || PduData->Code == 0x10 ) //寄存器写入 判定
	{
		if(PduData->Us_Cof_ARM_Num[PduData->Now_Addr_Crew].Data_Pyte == 0) //该区域只读 
		{
			Excode = EX_CODE_02H;// 异常码 02H
#ifdef DF_Dbug_Modbus_UR 
			USCI0_Dbug_Print("该地址禁止写入\r\n");//发送字符串 
#endif				
			return Excode;
	
		}
	}
	
 	
  return Excode;
}







//函数功能：正常响应
//输入参数：_FunCode 功能码
//返 回 值：无
//说    明：当通讯数据帧没有异常时并且成功执行后，发送响应数据帧
void MB_RSP(unsigned char _FunCode)
{
  unsigned short crc = 0;
	
	PDUData_TypeDef 		*PduData;//申请结构临时变量 
		
	PduData = Modbus_internal_get_handle(Now_Idx); //检查合法性
		
	if (PduData == NULL) return;			 			 // 句柄无效
	TxCount=0; //重置 发送长度 
	 
	PduData->Tx_Buf[TxCount++] = PduData->Slave_Hardware_Addr;	/* 从站地址 */
	

	PduData->Tx_Buf[TxCount++] = _FunCode;        /* 功能码   */	
  switch(_FunCode)
  {
    case FUN_CODE_01H:
			/* 读取线圈状态 */
			TxCount = MB_RSP_01H(TxCount,PduData->Addr,PduData->Num);
		  break;
    case FUN_CODE_02H:
			/* 读取离散输入 */
      TxCount = MB_RSP_02H(TxCount,PduData->Addr,PduData->Num);
      break;		 
    case FUN_CODE_03H:
			 /* 读取保持寄存器 */ 
			//TxCount = MB_RSP_03H(TxCount,(unsigned short*)PduData.PtrHoldingOffset,PduData.Num);
				TxCount = MB_RSP_03H(TxCount,PduData->Num);
				break;
    case FUN_CODE_04H:
			/* 读取输入寄存器 */
			TxCount =	MB_RSP_04H(TxCount,PduData->Addr,PduData->Num);      
      break;
    case FUN_CODE_05H:
			/* 写单个线圈 */
      TxCount = MB_RSP_05H(TxCount,PduData->Addr,PduData->Num);
      break;
	
    case FUN_CODE_06H: // 写单个保持寄存器     
		TxCount = MB_RSP_06H(TxCount,PduData->Addr);
		break;
    case FUN_CODE_10H://写多个保持寄存器
 
		 TxCount = MB_RSP_10H(TxCount,PduData->Addr,PduData->Num);
		break;
  }

  
  if(TxCount>=2) //当06 10H 写入异常的时候  TxCount 会清零 其余情况正常发送
  { 
	  crc = CRC16_MODBUS(PduData->Tx_Buf,TxCount);//(unsigned char*)

	  if(PduData->CRC_Order == 0)	//CRC高低字节 对调 ==0(大端) 使用默认先发低字节 再发高字节，==1 对调
	  {
	    PduData->Tx_Buf[TxCount++] = crc;	        /* crc 低字节 */ 	  
	    PduData->Tx_Buf[TxCount++] = crc>>8;		/* crc 高字节 */
	  }
	  else
	  {
		PduData->Tx_Buf[TxCount++] = crc>>8;		/* crc 高字节 */
		PduData->Tx_Buf[TxCount++] = crc;			/* crc 低字节 */ 	  
	  }  
  	
		

#ifdef DF_Dbug_Modbus_UR
		PrintHexData(PduData->Tx_Buf, TxCount, "MCU Tx: "); //打印收到的 数据
#endif	  
  }
  else
  {
		MB_Exception_RSP(PduData->Code,EX_CODE_03H); //报告 故障 03
  }  
   
  
}

 

 
/**
 * @brief   获取已构建的Modbus响应数据帧 (包含地址和CRC)
 * @param   idx: 数据索引 (用于区分不同实例或通道)
 * @param   response_frame_buffer: 用户提供的用于接收完整响应帧的缓冲区指针 (可为 NULL，用于查询所需长度)
 * @param   p_actual_length: 指向用于存储实际响应帧长度(字节)的变量的指针。
 *                           此指针不可为 NULL。
 *                           - 调用成功 (返回 MODBUS_Lib_N_Error) 时，*p_actual_length 被设置为实际帧长度。
 *                           - 当返回 MODBUS_TX_ERR_Len_insufficient 时，*p_actual_length 被设置为所需的最小缓冲区大小。
 *                           - 其他错误情况下，*p_actual_length 的值不确定。
 * @retval  ModbusInitError_t:
 *          - MODBUS_Lib_N_Error: 操作成功，数据已复制到 response_frame_buffer (若 buffer 非 NULL)。
 *          - MODBUS_INIT_ERR_NULL_POINTER: p_actual_length 指针为空。
 *          - MODBUS_INIT_ERR_INVALID_COUNT: idx 无效或库未初始化。
 *          - MODBUS_TX_ERR_Len_insufficient: 用户配置的发送缓存长度或提供的 buffer 不足以发送当前数据。
 *          - 其他潜在的初始化错误...
 * @note    在 processModbusRequest 返回指示需要发送响应后调用。
 *          完整的响应帧 (包括从机地址、PDU、CRC) 会被复制到用户提供的 buffer 中。
 *          如果 response_frame_buffer 为 NULL，则不执行复制，仅通过 *p_actual_length 返回所需长度。
 *          用户需保证 response_frame_buffer 指向的缓冲区足够大（当不为 NULL 时）。
 */
ModbusInitError_t Get_Mobus_Response_Data_Lib(unsigned char idx, unsigned char* response_frame_buffer, unsigned short* p_actual_length)
{
    PDUData_TypeDef *current_pdu_data = NULL;

    if (p_actual_length == NULL || response_frame_buffer == NULL) 
	{
        return MODBUS_INIT_ERR_NULL_POINTER; // 输出长度指针为空
    }
 
    current_pdu_data = Modbus_internal_get_handle(idx);
    if (current_pdu_data == NULL) 
	{ 
        return MODBUS_INIT_ERR_INVALID_COUNT; // 无法获取有效的从机句柄 (索引无效或库未初始化)
    }
 
    if (TxCount == 0) 
	{
        *p_actual_length = 0; // 没有数据需要发送，所需长度为0 
        return MODBUS_Lib_N_Error; // 没有数据也视为成功
    }
 
    *p_actual_length = TxCount; // TxCount 是已经包含了地址、功能码、数据、CRC 的完整帧长度

    if (TxCount > current_pdu_data->Comm_Tx_Len_Max) {
        // 如果超出用户定义的最大长度，返回特定错误码
        // *p_actual_length 已在第4步设置为所需长度
        return MODBUS_TX_ERR_Len_insufficient;
    }

 
	// 7. 将库内部的 Tx_Buf 数据复制到用户提供的缓冲区
	// 假设 Tx_Buf 是一个全局指针，指向了包含完整响应帧的缓冲区
	// 并且 TxCount 正确反映了该帧的字节长度
	
	memcpy(response_frame_buffer, current_pdu_data->Tx_Buf, TxCount);
	
	TxCount=0;	//重置 长度
	return MODBUS_Lib_N_Error; // 操作成功
 
}




//========================================================================
// 函数: 
// 描述: 对接收到的 信息进行分析 与 回应
// 参数: idx 解析的 对象，接收的数据，数据长度
// 返回:  
// 版本: VER1.0
// 日期: 2024年8月13日
// 备注: 	
//========================================================================
void processModbusRequest(unsigned char Idx,unsigned char *Read_Data,unsigned short Len)
{
	unsigned char ExCode;
	
	PDUData_TypeDef 		*PduData;//申请结构临时变量 

	RxCount=Len;
	Rx_Buf=Read_Data;//获取接收缓存		
	
#ifdef DF_Dbug_Modbus_UR   
	USCI0_Dbug_Print("开始解码\r\n");//发送字符串   
	PrintHexData(Rx_Buf, RxCount, "收到数据: "); //打印收到的 数据	
#endif

	PduData = Modbus_internal_get_handle(Idx); //检查合法性
	if (PduData == NULL) return;			  // 句柄无效
	Now_Idx   = Idx;     //当前执行的 Idx
	 
	
	
	
	
	
    MB_Parse_Data(Idx);// 解析请求数据
	 
//1. 硬件地址 合法性检查
	if(PduData->Slave_Hardware_Addr != PduData->Slave_Addr)
	{
		if(PduData->Slave_Hardware_Addr_ERR_RET ==1) //从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
		{
			MB_Exception_RSP(PduData->Code, EX_CODE_02H );//从机地址错误  处理....
		}
		else
		{
			TxCount = 0;	//重置 数据长度 
		}	
#ifdef DF_Dbug_Modbus_UR 
		USCI0_Dbug_Print("从机硬件地址\r\n");//发送字符串  
#endif
		return;
	}
	
	
	
	
	
	
	
	
//2. crc校验 合法性检查
	if(PduData->_CRC != PduData->Master_CRC)
	{
		//CRC校验错误 处理....
		MB_Exception_RSP(PduData->Code, EX_CODE_08H );
		
#ifdef DF_Dbug_Modbus_UR 
		USCI0_Dbug_Print("crc 异常\r\n");//发送字符串  
#endif
		return;
	}

 
	
	
	
//3. 分析请求数据并执行相应操作
     ExCode = MB_Analyze_Execute(Idx);
	
    if (ExCode != EX_CODE_NONE) // 如果有异常，可以在这里处理，例如发送异常响应
	{  

#ifdef DF_Dbug_Modbus_UR
		sprintf (Dbug_TX_Buf,"err Code: %d \r\n",(int)ExCode);
		USCI0_Dbug_Print(Dbug_TX_Buf);	//发送字符串  	
#endif

         MB_Exception_RSP(PduData->Code, ExCode);
    }
	else
	{
		//PduData.Addr=Addr_Original;     //记录原始的 协议地址	
		
		MB_RSP(PduData->Code);	// 准备并发送响应数据
	
	}	
#ifdef DF_Dbug_Modbus_UR
	USCI0_Dbug_Print("解析结束\r\n");//发送字符串  
	USCI0_Dbug_Print("\r\n");//发送字符串  
	USCI0_Dbug_Print("\r\n");//发送字符串  
	USCI0_Dbug_Print("\r\n");//发送字符串  
#endif
	
}

 


















