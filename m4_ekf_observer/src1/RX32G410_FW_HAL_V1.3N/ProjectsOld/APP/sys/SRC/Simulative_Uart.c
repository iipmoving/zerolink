

#include	<stdint.h>

#include	"simulative_uart.h"

#ifdef DF_Use_Simulative_UART 





//bit  Bit_Simulative_IO=0;  //一个假的 IO 口，专门模拟/替代 某些调试时 被占用的 IO口 







#define				 DF_Sur_TAtest_Sen_Long  18*2   //发送长度控制（1: 从1开始数、 2:  发送数据长度越少更新速度 越快
uint8_t  				Sur_TAtest[DF_Sur_TAtest_Sen_Long]; //18 //总的 缓存长度


//flag8_type     	SYS_MoNi_Ur;//模拟串口 用
typedef struct time_flag
{
		uint8_t uartInit     		:1;    /*1ms时间节点标志*/
		uint8_t	uartTx				:1;			/*过零复位信号*/
		uint8_t uartSendStart     	:1;    /*10ms时间节点标志*/
		uint8_t	uartSendStop			:1;			/*保留*/



} UartMessageFlagDef;

UartMessageFlagDef		UartFlag;

#define  	Bit_Simulative_Ur_Init          UartFlag.uartInit
#define 	Bit_Simulative_Uart_TX          UartFlag.uartTx
#define 	Bit_SEN_START          					UartFlag.uartSendStart
#define 	Bit_SEN_STOP          					UartFlag.uartSendStop




unsigned char Tx_Data_Count=0;// 发送字节长度
unsigned int 	Sen_Interval_Time=0;//发送间隔 计算 
//unsigned int 	system_Time_count=0;
//unsigned int 	system_Time=0; 
unsigned char Tx_Bit_Len=0;//发送位数
unsigned char Tx_Sen_Long=0;//发送时长


//bit  Bit_Simulative_Ur_Init=0;
//bit Bit_Simulative_Uart_TX;
unsigned char Simulative_UR_TxData;//模拟串口 发送 缓存


#define DF_Sen_Interval_SET   80*8 //单位  MS/1


// _波特率2002_停止位1.5_流控/无
#define DF_Simulative_UR_BR  4  //Baud rate 2002

// _波特率8000_停止位1.5_流控/无
//#define DF_Simulative_UR_BR  0  //Baud rate 8000




//========================================================================
// 函数: void Simulative_Uart_Init()
// 描述: 模拟串口 初始化
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2020年3月13日 15:35:16
// 备注: 
//========================================================================
void Simulative_Uart_Init(void)  
{
	
//	P_Simulative_Uart_TX_C ;
//	P_Simulative_Uart_TX 		 =1;
	Bit_Simulative_Ur_Init=1;
}


#if 0
//========================================================================
// 函数: void Simulative_Uart_TX()
// 描述: 模拟串口发送
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2020年3月13日 15:35:16
// 备注: 单字节 发送 信号处理
//======================================================================== 
//extern bit Bit_SD_Delay_OK;
void Simulative_Uart_TX(void)
{
	// static unsigned char Tx_Bit_Len=0;//发送位数
	// static unsigned char Tx_Sen_Long=0;//发送时长
	//static bit Bit_SEN_START=0;
	//static bit Bit_SEN_STOP=0;
	//if(!Bit_SD_Delay_OK)return;//自检 工厂模式 复用
	if(!Bit_Simulative_Ur_Init)
		Simulative_Uart_Init();//IO 初始化一次
	
	
	if(Bit_Simulative_Uart_TX)
		{
			Tx_Sen_Long++;
			if(!Bit_SEN_START)
				{
					P_Simulative_Uart_TX=0; 
					if(Tx_Sen_Long<DF_Simulative_UR_BR)return;
					Bit_SEN_START=1; 
					Tx_Sen_Long=1;
					if(DF_Simulative_UR_BR)Tx_Sen_Long=0;
				}
				
		else	if(!Bit_SEN_STOP)
				{
					
					if(BIT_GET(Simulative_UR_TxData,Tx_Bit_Len))
						{
							P_Simulative_Uart_TX=1;
						}
						else
							{
								P_Simulative_Uart_TX=0;
							}

					if(Tx_Sen_Long>=DF_Simulative_UR_BR)
						{
							Tx_Sen_Long=0;
							Tx_Bit_Len++;
							if(Tx_Bit_Len>7)Bit_SEN_STOP=1;
						}
				}
 			else
 				{
 					
						if(Tx_Sen_Long>DF_Simulative_UR_BR+1)
							{ 
								Tx_Sen_Long=0;
								Tx_Bit_Len=0;
								Bit_SEN_START=0;
								Bit_SEN_STOP=0;
								Bit_Simulative_Uart_TX=0;
							}
							P_Simulative_Uart_TX=1;
 				}
				  
		}

}
#endif
 
#if 0
void Simulative_UR_TxDispose(void)
{
  	uint8_t i;
  
	
				if(Sen_Interval_Time<DF_Sen_Interval_SET)
				{
					 Sen_Interval_Time++;
					 return;
				}

					if(!Bit_Simulative_Uart_TX)
					{
						Bit_Simulative_Uart_TX=1;
						Tx_Data_Count++;
 
						if(Tx_Data_Count==1) //头帧
						{
								Simulative_UR_TxData=0x55;
						} 
						else if(Tx_Data_Count>= DF_Sur_TAtest_Sen_Long*2+2)
						{
								Simulative_UR_TxData=0;
								Tx_Data_Count=0;
								Sen_Interval_Time=0; 
						}
							 
						else	if(Tx_Data_Count>=2 && Tx_Data_Count<=DF_Sur_TAtest_Sen_Long*2+1)
							{
								if(!(Tx_Data_Count & 0x01))
								{
									i=Tx_Data_Count-(Tx_Data_Count/2+1);
									Simulative_UR_TxData=Sur_TAtest[i]>>8&0x00FF;
								}
								else
								{
									i=Tx_Data_Count-(Tx_Data_Count/2+2);
									Simulative_UR_TxData=Sur_TAtest[i]&0X00FF;
								}
							 
							}
	
 
					}


}

#endif
#endif


//2个头数据+1个通道数+2*通道数+2个校验码+两个尾数据
#ifdef	DebugOutPc


uint16_t DebugOutBuff[DEBUG_OUT_CH_NUM];
uint8_t DebugHead[2] = {0x3C, 0xDD};
uint8_t DebugTail[2] = {0x88, 0x99};
uint8_t DebugCrc[2];

UserStringDef*		DebugBuff;


void	setDebugOutBuff(int16_t value,uint8_t ch)
{
		DebugOutBuff[ch]=value;
}	


//发送调试数据
UserStringDef* Debug_OutToPc(void)
{
    uint16_t sum;
    uint8_t i;

		DebugBuff=(UserStringDef*)MemNew(DebugBuff,sizeof(int)*(DEBUG_OUT_CH_NUM+4));
	
		uint8_t*	buff=(uint8_t*)&DebugBuff->p;
	
//    Uart_WriteStr(DebugHead, 2);	
		*buff++=DebugHead[0];
		*buff++=DebugHead[1];	
//    Uart_WriteByte(DEBUG_OUT_CH_NUM);
		*buff++=DEBUG_OUT_CH_NUM;


    sum = 0x67;
    for(i = 0; i < DEBUG_OUT_CH_NUM; i++)
    {
        *buff++=(DebugOutBuff[i] >> 8);
 				*buff++=(DebugOutBuff[i] & 0xff);
        sum += (DebugOutBuff[i] >> 8) ^ 0x3d;
        sum += (DebugOutBuff[i] & 0xff) ^ 0x3d;
    }


    DebugCrc[0] = (sum >> 8) & 0xff;
    DebugCrc[1] = sum & 0xff;
		
//    Uart_WriteStr(DebugCrc, 2);
		*buff++=DebugCrc[0];
		*buff++=DebugCrc[1];		
		

//    Uart_WriteStr(DebugTail, 2);
		*buff++=DebugTail[0];
		*buff++=DebugTail[1];	
		
		DebugBuff->len=(buff-(uint8_t*)&DebugBuff->p);			//得到
		
		return	DebugBuff;
}

//回读控制数据
#if 0
void Debug_GetData(uint8_t *data)
{
    if(data[0] == 0x3C && data[1] == 0xDD && data[6] == 0x88 && data[7] == 0x99)
    {
        if((data[2] + data[3] + data[4]) == data[5])
        {
            switch(data[2])
            {
                case 0:
                {
                    int para = (data[3] << 8) + data[4];
                }
                break;
            }
        }
    }
}
#endif

#endif


