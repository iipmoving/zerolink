
//******************************************************************************
// 	Copyright (c) MOVING
// 	文件名称:
// 	作者 	:  	   	李鹏
// 	模块功能:  	   	
// 	   	   	   	   	电源转接板为从机，电源转接板与灯板之间通讯
//                  用一个IIC串行传送双个炉头信息
// 	   	   	   	   	
// 	局部函数列表:
//  最后更正日期:
//  版本 	:
//  更改记录   	:
/* 1 发送地址					50  1x  x=0 通道0  1通道
	 2 发送初始化数据		50  0x  13 50 0c 0a b4 00 10 8c  	x=bxx00 通道0  x=bxx01	1通道   下发初始化数据
	 3 发送功率数据			50  1x  B4 10 00 AA 00 00   			x=bxx00 通道0  x=bxx01	1通道  下发初始化数据
	 4 接收从机数据			51  80  B4 1x 00 AA 00 00 00 00 00	x=bxx00 通道0  x=bxx01	1通道   电流位低两位代表通道
				


******************************************************************************/

#include	"data_type.h"
#include	"Proto_I2c.h"
#include	"API_i2c.h"
#include	"commClass.h"
//--------全局变量暂时-------------------------------------




	


static 	uint8_t 	FlashValue[10];			//FLASH数据暂存区，只用用PROTOL I2C

uint8_t const txValueInit[]={0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x24};
uint8_t	txValue[20];


void	(*I2cRxControlCallback)(uint8_t *buff, uint8_t len);
void	(*I2cRxInitCallback)(uint8_t *buff, uint8_t len);
uint8_t*	(*I2cSetTxValueCallBack)(uint8_t chn,uint8_t len);

__attribute__((weak))	void API_I2C_RxControlCallback(uint8_t ch,int8_t *buff, uint8_t len)	
{
}
__attribute__((weak))	void API_I2C_RxInitCallback(uint8_t ch,int8_t *buff, uint8_t len)	
{
}
__attribute__((weak))	uint8_t* API_I2C_TxStatusCallback(uint8_t ch, uint8_t len)	
{
	return 0;
}


// void	SetI2cRxControlCallBackFun(void(* TaskAddrss)(uint8_t *buff, uint8_t len))
// {
// 			I2cRxControlCallback=(TaskAddrss);
// }
// void	SetI2cRxInitCallBackFun(void(* TaskAddrss)(uint8_t *buff, uint8_t len))
// {
// 			I2cRxInitCallback=(TaskAddrss);
// }
// void	SetI2cSetTxValueCallBackFun(uint8_t* (* TaskAddrss)(uint8_t chn, uint8_t len))
// {
// 			I2cSetTxValueCallBack=(TaskAddrss);
// }


// uint8_t*		setTxValueInit(uint8_t chn, uint8_t len)
// {
// 	return (uint8_t*)&txValueInit;
// }		




//----------------------------------------------------




#if 0

bit_type IIC1;


beep_dcf_fan_FLAG  	IIC_CMD;
_FLAG 				FLAG;
IH_Status_FLAG 		IH_Status;
_POWER_CTRL			POWER_CTRL;
_DEBUGMODE_CTRL		Debug_Mode;


volatile	u8		u8_PowerSetHB_buf=0,u8_PowerSetLB_buf=0;
volatile	u8		u8_CheckSum;
volatile	u8		u8_PowerMea_HB,u8_PowerMea_LB;	
volatile	u8		u8_ctrl_set_rd,u8_power_set_h_rd,u8_power_set_l_rd;
volatile	u8		u8_I2cError_Cnt=0;
volatile	u8		u8_Commend=0;

#endif



#define	B_INIT_SUC_FLAG	_BIT7	









//INT8U s_test_falg ;//_at_ 0x21;

//INT8U s_test_cnt ;//_at_ 0x22;

//============================
//		静态局部变量定义
//============================

//============================
//		静态局部变量定义
//============================

//============================
//		内部函数声明
//============================


#define C_MASTE_ACK  00
#define C_SLAVE_ACK_RECE  0x88  //SLAVE模式有应答接收
#define C_SLAVE_ACK_SAND  0x8a  //SLAVE模式有应答发送
/*#define C_SLAVE_ADDR	  0x50  //SLAVE模式有应答发送*/
#define C_DATA_ADDR_INIT  0x00	//初始化数据地址
#define C_DATA_ADDR_CTRO  0x10	//控制数据地址
#define C_DATA_ADDR_TM	  0x20	//数据地址
#define C_DATA_ADDR_DEMC  0x30	//数据地址
//#define C_DATA_ADDR_SETPPG	 0X40






enum
{
		I2cCammandByte=0,			//命令字节
		I2cStatusByte,			//状态设置字				
		I2cPowerSwitchByte,		
		I2cPowerSetMByte,	
		I2cFanSpeedMByte,
		I2cKvalueByte,	
		I2cHalfPowerByte,
};	


I2cStructDef		I2cRam;			//当前暂存数据


#define	s_com_lost_count	I2cRam.comLostCount
#define	comp_rec_byte_save	I2cRam.comByteSave				//保存地址字长，用于暂存比较	

#define	I2cChn			I2cRam.i2cChn												//当前I2C 通道号
#define	s_com_byte_cnt	I2cRam.comByte								//新数据的字长
#define s_com_rece_buf	I2cRam.comBuffNow							//新数据指针（当前读到的数据）
#define	s_com_buf		I2cRam.comBuffLast								//缓存的数据指针（上一次读到的数据）
#define		ProtoI2cSuccess			I2cRam.I2cSuccess	
#define		ProtoI2cFail			I2cRam.I2cFail	


#define 	I2C_CHN  0x5	//i2c通道数




typedef	struct
{	
	
	union {										//POWER模块标志
		unsigned char  	   	byte;      	
		I2cFlagDef			bit;
	}flag;
	uint8_t				compCount;				//比较确认次数
	uint8_t				comAddressSave;			//保存回读的地址（命令字）			
	uint8_t				comTxLengthSave;		//保存回读的长度
	UserStringDef*		I2cRxData;				//上次读到的数据(需要缓存)
	UserStringDef*		I2cTxData;				//新发送的数据（实际子炉头保存地址）

}I2cSaveDef;				//子炉头数据保存

I2cSaveDef		I2cSave[I2C_CHN];				//子炉头实际保存地址

#define B_neg_flag							I2cSave[I2cChn].flag.bit.negBit					//数据以求反标志
#define b_com_init_suc_flag		I2cSave[I2cChn].flag.bit.initBit
#define	comp_count							I2cSave[I2cChn].compCount
//#define	comp_rec_address_save		I2cSave[I2cChn].comAddressSave		//保存地址数据，用于暂存比较
//#define	comp_rec_length_save		I2cSave[I2cChn].comTxLengthSave		//保存地址数据，用于暂存比较




void	comm_rece_buff_set(INT8U	t_iic_neg);
INT8U	iic_data_comp(INT8U *p,INT8U lens);



void 	ProtoI2cNull(uint8_t *buff, uint8_t len)
{
}	


void	I2cSlaveInit(void)
{
	for(uint8_t i=0;i<I2C_CHN; i++)
	{
		I2cSave[i].I2cRxData	=(UserStringDef*)MemNew(I2cSave[i].I2cRxData,20);				//初始化当前数据缓存（使用前需初始化)
		I2cSave[i].I2cTxData	=(UserStringDef*)MemNew(I2cSave[i].I2cTxData,20);
	}
	
	// I2cSetTxValueCallBack=	setTxValueInit;
	// I2cRxInitCallback		=ProtoI2cNull;
	// I2cRxControlCallback	=ProtoI2cNull;
}	


void*	getFlashValue(INT8U start_addr)				//得到FLASH值 
{
#ifdef	flash_enable	
		if(eeprom_read(0,FlashValue))
		{	
			return	FlashValue;
		}else
		{
			return NULL;
		}			
#else
		return NULL;
#endif
}

//void	set_IIC_slave(INT8U t_addr)
//{
//	C_SLAVE_ADDR=t_addr;
//}



//*************************************************************
// 函数名	：__Bool	iic_data_comp(INT8U *p,INT8U lens)
// 作者		：
// 功能		：指定数组与接收数组比对
// 参数		：无
// 返回值	：0 不等 1相等
// 调用全局变量 ：
// 修改全局变量 ：
// 备注：	
//************************************************************

INT8U	iic_data_comp(INT8U *p,INT8U lens)
{
	INT8U i;
	INT8U f,t_checksum,temp;
	f=1;
	temp=0;
	t_checksum=0;
	for(i=0;i<lens;i++)
	{
		temp=s_com_rece_buf[i];
		t_checksum+=temp;

		if(s_com_rece_buf[i]!=p[i])		//数据比对
		{
			p[i] = s_com_rece_buf[i];
			f =0;
		}
	}

	t_checksum-=temp;
	t_checksum=~t_checksum;


	if(temp==t_checksum)
	{
	 	f=2;
	}

	return f;
}


void		iic_init_func()
{		

		if((s_com_rece_buf[I2cCammandByte]&0xf0)==C_DATA_ADDR_INIT)	//初始化数据接收处理
		{
//			IHStatus &= ~B_INIT_SUC_FLAG;


		  if(s_com_byte_cnt == 9||s_com_byte_cnt == 10)
			{
//				if(iic_data_comp(s_com_buf,s_com_byte_cnt))		//初始化数据比对
//				{
						
						
							
					b_com_init_suc_flag = 1;
//					IHStatus |= B_INIT_SUC_FLAG; 
				
				
//-----------------------将数据传送到APP_power输入区  回调函数----				
//					(*I2cRxInitCallback)(&s_com_buf[0],s_com_byte_cnt);
				API_I2C_RxInitCallback(I2cChn,(int8_t*)&s_com_buf[1],s_com_byte_cnt-1);
//-----------------------将数据传送到APP_power输入区  回调函数----				
////					for(i=1;i<s_com_byte_cnt;i++)				//初始化数据比对成功，数据处理
////					{

////						g_sys_para_init[i-1] = s_com_buf[i];			//第一位是命令字
//////						memcpy(g_sys_para_init,s_com_buf,8);
////					}
//					set_OVP_da();		//更新比较器值	
																			
//				}
			}
		}
	
 }


void	iic_demc_func()				//标定数据
{
	
#if 1	
	uint8_t	t_num,t_temp;							
	if((s_com_rece_buf[I2cCammandByte]&0xf0)==C_DATA_ADDR_DEMC)
	{
#ifdef	flash_enable			
			eeprom_write_command_set(s_com_buf[1]);			//写保护开关值置位 

			if(s_com_buf[1]==0x1a)					//初始地址固定为1A
			{
				t_num=s_com_buf[2];	  		//数据区长度
				t_num*=2;							//数据区长度WORD
				t_temp=eeprom_data_checksum(&s_com_buf[3],t_num);
				if(t_temp)
				{
					 eeprom_operate(s_com_buf,&s_com_buf[1],s_com_byte_cnt-1,(uint8_t*)&FlashValue);			//写入FLASH
//					//待写数组地址，偏移地址，字节数

				}

			}
			eeprom_write_command_set(0);			//写保护开关值清除 
#endif		
		
	}
#endif
}




void	iic_ctro_func(void)					//控制	
{
		if((s_com_rece_buf[I2cCammandByte]&0xf0)==C_DATA_ADDR_CTRO)				//控制命令处理
		{
									//P14=~P14;
			if((s_com_byte_cnt>=5)&&(s_com_byte_cnt<= 10))		//ControlSet 4位 4位正反码
			{	
									


//				if(iic_data_comp(s_com_buf,s_com_byte_cnt))		//比对判断
//				{		
										
					if(((s_com_buf[I2cStatusByte]>>4)&0x0f)!=(((~s_com_buf[I2cStatusByte])&0x0f)))	
					{
							return;
					}

//					comp_count++;
//					if(comp_count>=2)
//					{
//						comp_count=2;
//												

						if(((s_com_buf[I2cStatusByte]>>4)&0x0f)==(((~s_com_buf[I2cStatusByte])&0x0f)))	
						{
							uint8_t  rxBuff[10];
							rxBuff[I2cCammandByte]=	s_com_buf[I2cCammandByte]&0x0f;
							rxBuff[I2cStatusByte] = s_com_buf[I2cStatusByte]&0x0f;		//风机位不管正反校验，保证每次有效																																																			
												
							rxBuff[I2cPowerSwitchByte] = s_com_buf[I2cPowerSwitchByte];			//PowerSwitch
							rxBuff[I2cPowerSetMByte] = s_com_buf[I2cPowerSetMByte];			//PowerSetM

							rxBuff[I2cFanSpeedMByte]=0xff; //fan						//如果通讯没有这两位则初始化使其无效 2019 9.4 修改 moving
 							rxBuff[I2cKvalueByte]=0xff; //K
							rxBuff[I2cHalfPowerByte]=0x00; //half 						//SET_PPG_VALUE
							rxBuff[8]=0x00;
							if(s_com_byte_cnt>=6)
							{
								rxBuff[I2cFanSpeedMByte] = s_com_buf[I2cFanSpeedMByte];			//fan_speed_in
							}


							if(s_com_byte_cnt>=7)
							{
								rxBuff[I2cKvalueByte] = s_com_buf[I2cKvalueByte];			//K_value_in
							}

							if(s_com_byte_cnt>=8)
							{
								rxBuff[I2cHalfPowerByte] = s_com_buf[I2cHalfPowerByte];			//ppg相对当前功率调整值

							}
							if(s_com_byte_cnt>=9)
							{
								rxBuff[7] = s_com_buf[7];			//ppg相对当前功率调整值
							}
//-----------------------将数据传送到APP_power 输入区  回调函数----
//								(*I2cRxControlCallback)(rxBuff,8);	
								API_I2C_RxControlCallback(I2cChn,(int8_t* )&rxBuff[1],7);
							
//-------------------------------------------------------	---------						
						}	
						else
						{	
							comp_count=0;
//							s_main_reset++;
//							if(s_main_reset>120)
//							{
//								s_main_reset=120;
//							}

						}
			}
		}

 }

 
 
 
 //*************************************************************
// 函数名	：void  iic_address_func()
// 作者		：
// 功能		：接收地址设置
// 参数		：
// 返回值	：
// 调用全局变量 ：
// 修改全局变量 ：
// 备注：	1、0x50 0x1x 单地址方式， 1 代表回读数据地址， x 代表子炉头号   代表接收字长为16，第16位为校验和
//        2、0x50 0x1x 06  带字长方式，  1 代表回读数据地址， x 代表子炉头号 06 代表要回读的字长，最后一个字节是校验和
//************************************************************
 
 
void  iic_address_func()
{
	/*	INT8U	t_iic_temp;*/

						if((s_com_rece_buf[I2cCammandByte]&0xf0)==0x70)						//通讯中断，快速重发
						{
					/*	   m_low_rool_flag=1;		//下一个START清记数
						   s_com_rece_buf[I2cCammandByte]&=0x1f;	//改为0X10*/
						}

 						if((s_com_rece_buf[I2cCammandByte]&0xf0)==0x10)							//检查地址,只用于控制指令
						{
							
							if(s_com_byte_cnt==2)	
							{
								comp_rec_byte_save=s_com_rece_buf[I2cStatusByte]&0xf;
							}			
							else
							{	
								comp_rec_byte_save=16;								//50 10  代表接收字长为16，第16位为校验和
							}
						}
						if((s_com_rece_buf[I2cCammandByte]&0xf0)==0x80)						//一组数据发送完成
						{
					/*	   m_low_rool_flag=1;		//下一个START清记数
						   s_com_rece_buf[I2cCammandByte]&=0x1f;	//改为0X10*/
						}						
						
						
						
}


uint8_t	I2cSuccessCount(void)
{
	
	uint8_t xReturn=0;
	
	ProtoI2cSuccess++;
	if(ProtoI2cSuccess>200)				//长时间通讯不成功重新初始化I2C
	{
	
#ifdef COMM_UART

#else		

			ProtoI2cSuccess=0;
			API_I2C_Init();

		
			ProtoI2cFail++;						//两次重置总线，停止功率输出
			if(ProtoI2cFail>=2)
			{	
					xReturn=1;
			}	
#endif
			
	}		
	return xReturn;
}	






//******************************************************************
// 函数名	：void	iic_bus_updata(void)
// 作者		：
// 功能		：IIC 数据处理
// 参数		：无
// 返回值	：无
// 调用全局变量 ：
// 修改全局变量 ：
// 备注：	
//******************************************************************\




void	iic_bus_updata(void)
{
	INT8U i;
	INT8U m_iic_neg;
	INT8U	temp;	

	UserStringDef*	rxNewData;					//新读到的数据			当前运算指针

	
	rxNewData= 	getI2cRxBuffAdr();			//I2C读到的数据	


	s_com_rece_buf=(uint8_t*)&(rxNewData->p);
	s_com_byte_cnt=rxNewData->len;						//得到接收数据长度
	
	if(s_com_byte_cnt>20)
	{

		s_com_byte_cnt=0;
		rxNewData->num=0;
	
		
	}		
	
	
 	if(rxNewData->num)						//有新数据
	{
		rxNewData->num=0;			//更新索引
		
		ProtoI2cSuccess=0;
		ProtoI2cFail=0;
		memcpy(s_com_rece_buf,&(rxNewData->p),rxNewData->len);				//数据保存到接收数组（方便查看）
		 
		//****************如果命令字为反数据取反--两次数据进行反码交替校验** 2019 9.4 修改 moving********
		if(s_com_rece_buf[I2cCammandByte]>0x80)				 //地址不大于0x80
		{
			m_iic_neg=0xff;						//回送也用反码	，回送反码标志
			for(i=0;i<=s_com_byte_cnt;i++)	  		
			{
				s_com_rece_buf[i]=~s_com_rece_buf[i];			//得到的反码，取反	 		
			}
	
		}
		else
		{

			m_iic_neg=0;						//正码回送也用正码			

		}

//-------判断通道--------------------------------------
		
		uint8_t addressSave=s_com_rece_buf[I2cCammandByte];


		
		I2cChn=		getI2cChn(addressSave&0x7);	//得到当前通讯炉头号
		
	
		s_com_buf=(uint8_t*)&(I2cSave[I2cChn].I2cRxData->p);			//上一次缓存的数据

		
//**************************************************************************************************
//		准备回送数据
//**************************************************************************************************
			
		comm_rece_buff_set(m_iic_neg);	  				//校对不成功前进行数据反码切换		准备回送的数据，（有正反码，以上次下发数据命令字为准


//**************************************************************************************************
//		数据
//**************************************************************************************************		
		
		
		
		
				 if(s_com_byte_cnt <= 2)
				 {
					   iic_address_func();				//设置地址
				 }
				 else 							//接收数据处理
				 {
				
					temp=	iic_data_comp(s_com_buf,s_com_byte_cnt);
					if(temp)
					{
						comp_count+=temp;
					}
   	   	  else         	
   	   	  {
   	   	   	  comp_count=0;
   	   	  }

   	   	   	   	   	  

   	      if(comp_count>=2)
       	  {
      	   	comp_count=2;
    			
						memcpy(txValue,	s_com_rece_buf,	s_com_byte_cnt);					
			 			iic_init_func();				//初始化数据

						iic_demc_func();				//标定
						
						iic_ctro_func();				//功率控制

				   }
				}

						
		s_com_byte_cnt=0;		//通讯完成
   	  
	}


}







void	comm_rece_buff_set(INT8U	t_iic_neg)
{
			INT8U	i;
			INT8U	temp,checksum_temp,temp1;  
			INT8U	t_address_offset;
			uint8_t *	s_com_sand_buf	=(uint8_t *)(&(I2cSave[I2cChn].I2cTxData->p));
//			uint8_t*  status=I2cSetTxValueCallBack(I2cChn,comp_rec_byte_save);		//读取回送的数据
			uint8_t  t_potnum;

	
		uint8_t*  status=API_I2C_TxStatusCallback(I2cChn,comp_rec_byte_save);

					temp=0xFF;
					temp1=0;
					if(B_neg_flag)						//以经取反，不再取反  BIT5为求反位， 
    			{
						temp=0x0;															
					}
					checksum_temp=0;
//******************修正，按通讯下发的偏移地址重置数据，****************************************************************************************
					t_address_offset=0;//comp_rec_address&0x0f;			//偏移地址
					
					for(i=0;i<comp_rec_byte_save;i++)
					{	
							if(t_iic_neg)
							{
 								B_neg_flag=1;	  									//此时为反码	上一次的数据
								s_com_sand_buf[i-t_address_offset]^=temp;			//当数据以经取反，与0异或，不取反
							
							}
							else
							{
					
								B_neg_flag=0;
																			//此时为正码

								temp1=status[i];
#if		CURRENTADDRESS								
								if(i==2)//电流位加上地址标志
								{
									temp1&=0xf8;

									temp1|=	I2cChn&0x7;	

									t_potnum=temp1;				//校验需要XOR这一字节
								}	
#endif
								checksum_temp+=temp1;
								s_com_sand_buf[i-t_address_offset] = temp1;			//通讯数据只在正码时更新
									
							}
//***************************************************************************************************************
					}
					i--;   	
		      if(!B_neg_flag)
   	   	  {
						checksum_temp-=temp1;
						checksum_temp=~checksum_temp;
						checksum_temp^=t_potnum;
						s_com_sand_buf[i-t_address_offset]=checksum_temp;  //最后一位为验检和
					}
					I2cSave[I2cChn].I2cTxData->len=comp_rec_byte_save;															
		//	}				
					memcpy(getI2cTxBuffAdr(),I2cSave[I2cChn].I2cTxData,I2cSave[I2cChn].I2cTxData->max+sizeof(int));		//复制到I2C发送缓存


}		 

uint8_t 	getI2cChn(uint8_t ch)
{
		uint8_t	chn=ch;
		chn&=03;
//		if(ch&8)
//		{
//			chn+=1;
//		}		
		return chn;
}	

uint8_t 	setI2cChn(uint8_t ch)
{
		uint8_t	chn=ch;
		chn&=03;
//		if(ch&8)
//		{
//			chn+=1;
//		}		
		return chn;
}	
