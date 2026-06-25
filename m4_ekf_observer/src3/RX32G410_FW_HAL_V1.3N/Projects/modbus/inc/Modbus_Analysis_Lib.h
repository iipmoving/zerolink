#ifndef _Modbus_Analysis_Lib_H__
#define _Modbus_Analysis_Lib_H__

// --- Modbus 初始化错误码枚举 ---
/**
 * @brief Modbus 库初始化函数可能返回的错误代码。
 */
typedef enum {
    MODBUS_Lib_N_Error         = 0x00, // 不存在错误/ 操作成功 
    MODBUS_INIT_ERR_NULL_POINTER     , //< 输入参数指针为空 
    MODBUS_INIT_ERR_INVALID_COUNT    , //< 从机数量无效 (为0或配置数组指针为空) 
    MODBUS_INIT_ERR_INVALID_DATASIZE , //< Register_Area_t 中 Data_Size 无效 (不是2或4) 
    MODBUS_INIT_ERR_NULL_DATAPTR     , //< Register_Area_t 中 Data_ptr 为空 
    MODBUS_INIT_ERR_NULL_EEPROM_PTR  , //< Register_Area_t 中 Data_Pyte 为 1 (可读可写) 但 Data_ptr_EEPROM 为空 
    MODBUS_INIT_ERR_INVALID_DATATYPE , //< Register_Area_t 中 Data_Pyte 无效 (不是0或1) 
	MODBUS_INIT_ERR_Write_unll ,	   //数据写入 检查函数为空 
	MODBUS_TX_ERR_Len_insufficient, 	//用户配置的 发送缓存 不足 无法发送 数据
	
} ModbusInitError_t; 				// Modbus库 	错误代码类型

//申请 存储 地址 与结构体 数量
//虽然内存是 一整片，但是 用户 协议 高度 自定义 无法 
//所以 由用户去 配置 这些 缓冲区 
typedef struct {
	
    unsigned short Start_Address;  // Modbus起始地址
	unsigned short End_Address;    // Modbus结束地址	 
    void* Data_ptr;          	   // 指向实际数据的指针 （用户 申请）
	void* Data_ptr_EEPROM;         // 指向实际数据的指针 （用户 申请）	
	unsigned char (*Check_Write_Data)(void);    // 写入数据检查函数，(需要用户实现，检查写入数据范围 是否合规 （==1 允许写入，==0 禁止写入）
	unsigned char  Data_Size;      // 每个寄存器的大小(字节)，2 或者4，
    unsigned char  Data_Pyte;      // 数据类型==0只读，==1可读可写，
} Register_Area_t;
//例如  用户可以设置 地址 02 --- 0xA 是ih状态 （只读）
//例如  用户可以设置 地址 1000 --- 0x1006 是监控状态 （只读）
//例如  用户可以设置 地址 1100 --- 0x1100 是基本设置 （可读可写）
//....等等...用户可以 设置 无限个 这样的 参数

/* 类型定义 ------------------------------------------------------------------*/
typedef struct 
{
	Register_Area_t		*Us_Cof_ARM_Num;			//用户配置 内存片区数量
	unsigned char 		 ARM_Count;  				//每个协议里面 有多少组(ARM)协议
	unsigned char 		 Slave_Hardware_Addr;		//从机硬件地址设置
	unsigned char 		 Slave_Hardware_Addr_ERR_RET;	//从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
	
	unsigned char 		 *Tx_Buf;			//发送缓存（回应缓存）
	unsigned short 		 Comm_Tx_Len_Max;	//发送缓存 长度
	unsigned char 		 CRC_Order;	//CRC高低字节 对调 ==0 使用默认先发低字节 再发高字节，==1 对调



	
	
//以下是 lib库 的解析 全局变量，无需配置(!!!且用户 禁止修改)
	unsigned char 	Slave_Addr; //从机地址(硬件地址)
    unsigned char 	Code ;  	           //功能码
    unsigned char 	byteNums; 	         //字节数
    unsigned short	Addr ;             		//操作内存的起始地址
    unsigned short	Addr_Original;     //记录原始的 协议地址（主要是 防止函数执行修改了内存地址  导致反馈错误 多一层记录 手段而已） 
    
	unsigned short 	Num; 	           //查询 寄存器或线圈的数量
    unsigned short	 _CRC;       	   //CRC校验码
	unsigned short	 Master_CRC;       //收到主机发来的  CRC结果
	unsigned char 	Now_Addr_Crew;		//当前的内存组员（指的是“ARM_Count”第几组内存 ）
	
	
    unsigned char 	*ValueReg; 	       //10H功能码的数据(写多个寄存器起始地址)
    unsigned short 	*PtrHoldingbase;   //保持寄存器的首地址
    unsigned short 	*PtrHoldingOffset; //保持寄存器的偏移地址
	

}PDUData_TypeDef;

 
 
 
/**
 * @brief   初始化Modbus协议库
 * @param   num: 用户提供的modbus 使用数量
 * @param   pdu_data_struct: 用户分配并初始化的 PDUData_TypeDef 结构体指针
 *                           用户必须预先分配好结构体本身及其中所有指针成员
 *                           (如 Us_Cof_ARM_Num, ValueReg, PtrHoldingbase 等) 所指向的内存。
 * @retval  int: 0 表示成功, -1 表示参数错误或初始化失败
 * @note    必须在调用其他库函数前执行。库将直接操作用户提供的结构和缓冲区。
 */
unsigned char Modbus_Init_Lib(PDUData_TypeDef* pdu_data_struct,unsigned char num);

/**
 * @brief   处理接收到的完整Modbus RTU请求帧 (包含地址和CRC)
 * @param   rx_frame_buffer: 指向接收到的完整RTU帧数据缓冲区 (含地址和CRC)
 * @param   rx_frame_length: 接收到的完整帧数据长度
 * @retval  int: 0 表示成功处理并准备好响应数据,
 *              1 表示请求非法或不支持(已生成异常响应),
 *             -1 表示内部错误或初始化未完成
 * @note    用户在接收到完整帧后调用此函数。
 *          处理结果将准备在内部发送缓冲区中，供 Get_Mobus_Response_Data_Lib 获取。
 *          库会验证CRC、地址，并解析PDU。
 */
void processModbusRequest(unsigned char Idx,unsigned char *Read_Data,unsigned short Len);

/**
 * @brief   获取已构建的Modbus响应数据帧 (包含地址和CRC)
 * @param   idx: 数据索引 (用于区分不同实例或通道, 当前版本暂未使用)
 * @param   response_frame_buffer: 用户提供的用于接收完整响应帧的缓冲区指针
 * @retval  unsigned short: 响应帧的实际长度(字节), 0表示无有效响应或错误
 * @note    在 processModbusRequest 返回 0 或 1 后调用。
 *          完整的响应帧 (包括从机地址、PDU、CRC) 会被复制到用户提供的 buffer 中。
 *          用户需保证 response_frame_buffer 足够大以容纳完整响应。
 */
ModbusInitError_t Get_Mobus_Response_Data_Lib(unsigned char idx, unsigned char* response_frame_buffer, unsigned short* p_actual_length);


#endif /* _MUDBUS_485_H */
















