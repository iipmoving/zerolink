#ifndef __SMG_Disp_General_Lib_H_
#define __SMG_Disp_General_Lib_H_



// 功能目录  
  
//void Disp_Error_CODE();//故障代码显示

//void Disp_Hot();// 显示高温  4位数码管 显示hout

//void Disp_Convention();// 功率显示 

//bit Disp_Time(); //定时时间显示

//HEX16_Code_TO_Ascii(); // ASCII 码 转16进制码 ,

//void Dsp_Font_ASCII(); //显示字符串  仅支持0-9 a-z A-Z，空格 -，  
  
//Disp_Hex_H_L( );   // 描述: 16进制显示  BYTE 类型， 支持2个byte
 
//	Disp_Flash____   4位数码管 显示 - - - -   并且由下 往上 跑马 

//  Disp_SMG_WaiKuang(BYTE *SMG_Disp_DATA);   //只显示外壳  不跑马 
//  Disp_Flash_BianKuang(BYTE *SMG_Disp_DATA,BYTE Speed,BYTE IDx);  数码管 边框 跑马显示




typedef struct _SMG_Disp_General_STATE 
{
	//函数“Disp_Flash_FanGun” 的运行内存
	unsigned char    FG_Based_Speed;	//基础速度(翻滚 动画
	unsigned char    FG_Frame;		//帧数(翻滚 动画

	//函数 “Disp_Flash_BianKuang” 的运行内存
	unsigned char    BK_Based_Speed;//基础速度（边框 动画
	unsigned char    BK_Frame;//帧数（边框 动画

	//函数 “Disp_Flash____” 的运行内存
	unsigned char   __Based_Speed;	//基础速度  (--动画
	unsigned char   __Frame;		//帧数 		(--动画

	//函数 “Disp_Hot” 的运行内存
	unsigned char   HOT_Based_Speed;	//基础速度(hot 动画
	unsigned char   HOT_Frame;			//帧数 (hot 动画
	unsigned char   HOT_Bit_F_Add_Sub; 		//(hot 动画
	
} SMG_Disp_General_STATE_;

//========================================================================
// 函数:  
// 描述: 通用显示 内存 初始化
// 参数: User_Disp 指向用户配置的 内存数量 Disp_Count 设置 检测数量
// 返回: 初始化失败 == 0xFF ，初始化成功 == 0
// 版本: VER1.0
// 日期: 2025年3月27日
// 备注: 
//========================================================================
unsigned char  Disp_General_Init(SMG_Disp_General_STATE_ *User_Disp, unsigned char Disp_Count);
//========================================================================
// 函数: 	
// 描述: 闪烁同步功能，需要跟 系统 0.5秒闪烁标志 状态同步
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2022年11月19日 14:51:03
// 备注: 
//========================================================================
void SMG_Disp_General_Lib_Flash_STA(unsigned char Flash);
 
extern  unsigned char const  SMG_Disp_Code[];  //数码管 码表

#define	SMG_SEG_A		0x01
#define SMG_SEG_B		0x02
#define SMG_SEG_C		0x04
#define SMG_SEG_D		0x08
#define SMG_SEG_E		0x10
#define SMG_SEG_F		0x20
#define SMG_SEG_G		0x40
#define SMG_SEG_H		0x80

#define	SMG_Bit_A		0
#define SMG_Bit_B		1
#define SMG_Bit_C		2
#define SMG_Bit_D		3
#define SMG_Bit_E		4
#define SMG_Bit_F		5
#define SMG_Bit_G		6
#define SMG_Bit_H		7



#define SMG_Disp_0			0x3F
#define SMG_Disp_1			0x06
#define SMG_Disp_2			0x5B
#define SMG_Disp_3			0x4F
#define SMG_Disp_4			0x66
#define SMG_Disp_5			0x6D
#define SMG_Disp_6			0x7D
#define SMG_Disp_7			0x07
#define SMG_Disp_8			0x7F
#define SMG_Disp_9			0x6F
#define SMG_Disp_A 			0x77
#define SMG_Disp_B			0x7C
#define SMG_Disp_C 			0x39
#define SMG_Disp_D			0x5E
#define SMG_Disp_E 			0x79
#define SMG_Disp_F			0x71 

#define SMG_Disp_N		SMG_SEG_A|SMG_SEG_B|SMG_SEG_C|SMG_SEG_E|SMG_SEG_F
#define SMG_Disp_n		SMG_SEG_C|SMG_SEG_E|SMG_SEG_G

#define SMG_Disp_d		SMG_SEG_B|SMG_SEG_C|SMG_SEG_D|SMG_SEG_E|SMG_SEG_G
#define SMG_Disp_V		SMG_SEG_B|SMG_SEG_C|SMG_SEG_D|SMG_SEG_E|SMG_SEG_F
#define SMG_Disp_L		SMG_SEG_D|SMG_SEG_E|SMG_SEG_F

#define SMG_Disp_o		SMG_SEG_C|SMG_SEG_D|SMG_SEG_E|SMG_SEG_G
#define SMG_Disp_t		SMG_SEG_D|SMG_SEG_E|SMG_SEG_F|SMG_SEG_G

#define SMG_Disp_U		SMG_SEG_B|SMG_SEG_C|SMG_SEG_D|SMG_SEG_E|SMG_SEG_F
#define SMG_Disp_H  	0x76
#define SMG_Disp_P		0x73
#define SMG_Disp_OFF 	0x00

#define SMG_Disp_c			SMG_SEG_D|SMG_SEG_E|SMG_SEG_G	//小写c
#define SMG_Disp_UPc		SMG_SEG_A|SMG_SEG_F|SMG_SEG_G  //小写c在上方显示
 
#define SMG_Disp_Xo			SMG_SEG_C|SMG_SEG_E|SMG_SEG_G|SMG_SEG_D  //小写o在下方显示
#define SMG_Disp_UPo		SMG_SEG_A|SMG_SEG_B|SMG_SEG_F|SMG_SEG_G  //小写o在 上 方显示

#define SMG_Disp_u_			SMG_SEG_B|SMG_SEG_D|SMG_SEG_F|SMG_SEG_G  //无锅显示 U加一横在下面  


#define SMG_Disp_Heng  		0x40	



//========================================================================
// 函数: 	
// 描述: 故障代码显示
// 参数: CODE（故障码E几F几D几这些开头）,Number,*Disp_DATA（指向显示目标）,  SET（设置闪烁/预留其他状态升级）
// 返回: none.
// 版本: VER1.0
// 日期: 2022年11月19日 14:51:03
// 备注: 
//========================================================================
extern void Disp_Error_CODE(unsigned char DCode,unsigned char Number,unsigned char *Disp_DATA,unsigned char SET)  ;
 
 
//调用范例：  Disp_Error_CODE(SMG_Disp_E,0x5,Disp_DATA,1);// 显示E5， SET==1 表示闪烁，0==常显示
 
//========================================================================
// 函数: 
// 描述: 显示 高温  4位数码管 显示hout
// 参数: Based_Speed(跑马速度）、IDx 炉头号
// 返回: none.
// 版本: VER1.0
// 日期: 2020年1月9日 10:43:16
// 备注: 跑马速度和 调用速度有关，显示调用尽量是 固定的。
//========================================================================
void Disp_Hot(unsigned char *SMG_Disp_DATA,unsigned char Based_Speed,unsigned char IDx);
 

//========================================================================
// 函数: 
// 描述: 4位数码管 显示 - - - -   并且由下 往上 跑马
// 参数: Based_Speed(跑马速度）、IDx 炉头号
// 返回: none.
// 版本: VER1.0
// 日期: 2020年1月9日 10:43:16
// 备注: 
//========================================================================
void Disp_Flash____(unsigned char *SMG_Disp_DATA,unsigned char Speed,unsigned char IDx);
 

//========================================================================
// 函数: 
// 描述: 数码管 边框 跑马显示
// 参数: Based_Speed(跑马速度）、IDx 炉头号
// 返回: none.
// 版本: VER1.0
// 日期: 2023年5月20日 10:47:34
// 备注: 
//========================================================================
void Disp_SMG_WaiKuang(unsigned char *SMG_Disp_DATA);   //只显示外壳  不跑马
  
void Disp_Flash_BianKuang(unsigned char *SMG_Disp_DATA,unsigned char Speed,unsigned char IDx); //边框显示 

void Disp_Flash_FanGun(unsigned char *SMG_Disp_DATA,unsigned char Speed,unsigned char IDx); //翻滚 动画
//========================================================================
// 函数:  
// 描述: 常规显示 合集...
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2022年11月21日 16:07:55
// 备注: 
//========================================================================
enum Disp_Power_Type //功率显示
{ 
	 DF_Dip_def=0,	  //方式0：1---9999  ........
	 DF_Dip_123 ,     //方式1： 1 2 3 4........  
	 DF_Dip_999 ,     //方式1： 0-999 三数码管	 
	 DF_Dip_P0102 ,	  //方式2：P01  P02 P03.............  
	 DF_Dip_P1,		  //方式3：P01  P02 P03.............  
	 DF_Dip_L1,		  //方式3：L1  L2 L3.............在数码管 2..3位显示（实际是三位数码管，第一位只能显示“1”的那种）
	 DF_Dip_16Hex,	  //方式4：16进制显示 (16位  
	 
	
	DF_Dip_o,	//方式1：xxx°
	DF_Dip_C,	//方式2：xxxC
	DF_Dip_c,	//方式3：xxxc
	DF_Dip_UPc,	//方式4：xxxc，c在上半数码管
	DF_Dip_F,	//方式4：xxxF，华氏度
	
	 
	
};

void Disp_Convention(unsigned char *SMG_Disp_DATA, unsigned char Disp_MODE, unsigned int i);

//调用范例：Disp_Convention(Disp_DATA,DF_Dip_P1,5);// 显示P05


//========================================================================
// 函数:  
// 描述: 有符号类型 显示 集合
// 参数: *，显示方式，显示值
// 返回: none.
// 版本: VER1.0
// 日期: 2025年6月13日 11:09:20
// 备注: 
//========================================================================
enum Disp_Signed_Power_Type //显示
{ 
	DF_Dip_S_def=0,	  //方式0：-999到9999  ........ 
	DF_Dip_S_o,	//方式1：xxx°
	DF_Dip_S_C,	//方式2：xxxC
	DF_Dip_S_c,	//方式3：xxxc
	DF_Dip_S_UPc,	//方式4：xxxc，c在上半数码管
	DF_Dip_S_F,	//方式4：xxxF，华氏度
};
void Disp_Signed_Convention(unsigned char *SMG_Disp_DATA,unsigned char Disp_MODE ,signed short num);

//========================================================================
// 函数:  
// 描述: 时间 显示 集合
// 参数: *、时间显示方式、闪烁方式、时间（输入是分钟）
// 返回: 返回“：”状态自行判断 需不需要闪，闪烁是谁？，每个点所在的位置都不一样。
// 版本: VER1.0
// 日期: 2022年11月21日 16:07:55
// 备注: 
//========================================================================
enum Disp_Timer_Type //定时显示
{ 
	
	//显示格式 设定
	DF_Dip_H_M=0,//方式1：hour  :  minute （3或4位数码管）   时间输入单位是 分钟  
	DF_Dip_H_M_Sec,//方式1：hour  :  minute （3或4位数码管）时间输入单位是 秒   
	DF_Dip_H_M_Sec_Z,//方式1：hour  :  minute （3或4位数码管）时间输入单位是 秒   采用正计时的 方式
	DF_Dip_1xH_Sec_Z,//188 型 三位数码管 ，显示时间，xxH(输入时间单位 /sec
	
	
	DF_Dip_99M,//方式2：0-99分钟（2位数码管、在1...2位体现）
	DF_Dip_999M,//方式2：0-199分钟（3位数码管、在1...2...3位体现）	
	DF_Dip_999MZ,//方式2：0-199分钟（3位数码管、在1...2...3位体现）	正计时 输入单位是 秒	
	DF_Dip_9H9M,//方式3：0-59 分钟，>=1H、1.1  1.2 1.3..........1.9每0.1代表6分钟     （2位数码管、在1...2位体现）    
	DF_Dip_9H10M,//方式4：0-59分钟  >=1H、1.1 1.2 1.3.........1.5  每0.1代表10分钟	  （2位数码管、在1...2位体现）
	DF_Dip_99M_Sec,//方式2：0-99分钟（2位数码管、在1...2位体现）输入单位是 秒	
	DF_Dip_99M_SecZ,	//方式2：0-99分钟（2位数码管、在1...2位体现）正计时 输入单位是 秒	
	//DF_Dip_M_S,
	//方式5：0-59分钟 (多炉头显示 最小时间
	DF_Dip_FH_FM,//方式6：hour  :  minute （3或4位数码管） 但是 设置时  小时跟分钟 分开显示 

	
	//-----------------------------
	//闪烁规则设定
	DF_Fla_ON,	//代表 时间不在设置状态，正常怎么显示 “：”该怎么闪就怎么闪
	//↓↓↓↓↓   针对时间在设置的过程中
	DF_Fla_All, 	//时间“：”同步闪烁
//	DF_Fla_N_DP, 	//时间 ：不 闪烁
//	DF_Fla_Y_DP, 	//时间 ：  闪烁
	DF_Fla_YH_NDP,	//时间 H闪烁M不闪烁、“：”不闪烁
	DF_Fla_YM_NDP,	//时间 M闪烁H不闪烁、“：”不闪烁
	
	 
};

unsigned char Disp_Time(unsigned char *SMG_Disp_DATA,unsigned char Disp_MODE, unsigned char Flash,  unsigned long Timer);


//调用案例
//if(  Disp_Time(Disp_DATA,DF_Dip_H_M,DF_Fla_ON,90) )
//{
//	 SMG_Disp_Dp; //点亮“：”  （ 因为不确定你的 点在那个位置，要自己定义，
//}



/********************************************************************************
*name         :  
*author       : rsl
*function     : 显示字符串
*para         : ASCII 码 转16进制码 ,
*return       : 无
*brief        :  
********************************************************************************/
unsigned char HEX16_Code_TO_Ascii(unsigned char IN_HEX);


////调用案例
//XXX=HEX16_Code_TO_Ascii('5');   //返回 0x05


/********************************************************************************
*name         :  
*author       : rsl
*function     : 显示字符串
*para         : font 字符串缓冲
*return       : 无
*brief        : 字符串要是ASCII 仅支持0-9 A-Z(不区分大小写)，空格 -， 
********************************************************************************/ 
void Dsp_Font_ASCII(unsigned char *LedStr , char *font);

//调用案例
//Dsp_Font_ASCII(Disp_DATA,"A-31"); //显示 A-31


 

//========================================================================
// 函数: 
// 描述: 16进制显示  unsigned char 类型， 支持2个byte
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2020年1月9日 10:43:16
// 备注: 
//========================================================================
void Disp_Hex_H_L(unsigned char *SMG_Disp_DATA,unsigned char data_h, unsigned char data_l);

//调用案例
//Disp_Hex_H_L(Disp_DATA,0x55,0x12);   //显示 55 12
 
//========================================================================
// 函数: 
// 描述: 16进制显示  unsigned char 类型，中间两位 显示
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2020年1月9日 10:43:16
// 备注: 
//========================================================================
void Disp_Hex_ZJ(unsigned char *SMG_Disp_DATA,unsigned char data_Disp);

 
#endif



















