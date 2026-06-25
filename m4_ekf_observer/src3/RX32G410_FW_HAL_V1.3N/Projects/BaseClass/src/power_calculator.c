#include "power_calculator.h"
#include <math.h>
#define		FRE_PER_ADC    384
#if 0
// HRTIM时间戳数组 (uint32_t)
uint16_t hrtim_data[] = {
    19004,19388,19772,188, 572, 956, 1340, 1724, 2108, 2492, 2876, 3260, 3644,
    4028, 4412, 4796, 5180, 5564, 5948, 6332, 6716, 7100, 7484,
    7868, 8252, 8636, 9020, 9404, 9788, 10172, 10556, 10940, 11324,
    11708, 12092, 12476, 12860, 13244, 13628, 14012, 14396, 14780, 15164,
    15548, 15932, 16316, 16700, 17084, 17468, 17852, 18236, 18620, 19004,
    19388, 19772,188, 572, 956,
};
const uint16_t HRTIM_DATA_SIZE = sizeof(hrtim_data)/sizeof(hrtim_data);

// 谐振电流AD值数组 (uint16_t)
uint16_t current_data[] = {
    494, 535, 570,620, 647, 668, 643, 632, 527, 506, 436, 331, 238,
    195, 127, 55, 5, 30, 91, 159, 215, 272, 314,
    379, 415, 463, 500, 539, 581, 606, 645, 660, 637,
    622, 615, 575, 478, 370, 284, 223, 152, 67, 25,
    7, 75, 140, 193, 248, 303, 364, 399, 458, 494,
    535, 570,620, 647, 668,
};
const uint16_t CURRENT_DATA_SIZE = sizeof(current_data)/sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst = {
    3,
    CURRENT_DATA_SIZE,
    572,   // 高端开通点
    9988 ,   // 高端关断点 (占空比= (300-100)/1000=20%)
    10556 ,   // 低端开通点
    19872,   // 周期=1000
    0,
	0,
}; 



#endif

uint16_t voltage_data[] = {

255,
247,
258,
254,
255,
262,
251,
228,
235,
264,
266,
260,
262,
255,
268,
255,
266,
260,
260,
268,
259,
270,
259,
272,
263,
267,
270,
263,
276,
263,
274,
270,
269,
276,
267,
280,
263,
274,
270,
270,
276,
269,
328,
295,
292,
286,
281,
284,
275,
286,
275,
286,
279,
281,
286,
277,
285,
279,
286,
279,
283,
286,
281,
292,
279,
292,
286,
288,
287,
283,
296,
287,
236,
263,
281,
308,
293,



};

uint16_t hrtim_data[] = {
23164,
23548,
23932,
24316,
24700,
25084,
264,
648,
1032,
1416,
1800,
2184,
2568,
2952,
3336,
3720,
4104,
4488,
4872,
5256,
5640,
6024,
6408,
6792,
7176,
7560,
7944,
8328,
8712,
9096,
9480,
9864,
10248,
10632,
11016,
11400,
11784,
12168,
12552,
12920,
13328,
13704,
14072,
14480,
14856,
15216,
15600,
15984,
16368,
16784,
17160,
17544,
17928,
18312,
18704,
19080,
19464,
19848,
20240,
20616,
21000,
21392,
21768,
22152,
22536,
22920,
23304,
23688,
24080,
24456,
24840,
20,
404,
788,
1172,
1556,
1940,




};

uint16_t current_data[] = {
	
44,
47,
50,
55,
55,
52,
59,
63,
67,
72,
71,
55,
52,
41,
40,
31,
24,
23,
6,
12,
5,
8,
7,
4,
12,
1,
14,
15,
24,
27,
30,
38,
37,
44,
47,
52,
55,
58,
62,
62,
63,
65,
63,
59,
56,
38,
39,
31,
27,
24,
15,
8,
10,
4,
24,
5,
8,
7,
4,
14,
13,
24,
23,
29,
31,
24,
46,
44,
52,
51,
57,
59,
62,
63,
68,
70,
57,



};	

#if 0





uint16_t hrtim_data1[] = {
25372,
116,
884,
1652,
2412,
3188,
3956,
4724,
5484,
6260,
7028,
7796,
8556,
9332,
10100,
10868,
11628,
12404,
13164,
13932,
14708,
15476,
16244,
17012,
17780,
18548,
19316,
20084,
20852,
21620,
22388,
23156,
23924,
24692,
25460,
25972,


};    
uint16_t current_data1[] = {
423,
434,
425,
384,
285,
162,
58,
23,
120,
220,
307,
399,
477,
534,
630,
630,
497,
399,
317,
226,
148,
91,
30,
4,
63,
123,
179,
223,
278,
326,
360,
383,
411,
437,
426,
283,
};
const uint16_t CURRENT_DATA_SIZE1 = sizeof(current_data1)/sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst1 = {
    0,
    CURRENT_DATA_SIZE1,
    572,   // 高端开通点
    8920 ,   // 高端关断点 (占空比= (300-100)/1000=20%)
    8920+572 ,   // 低端开通点
    26110,   // 周期=1000
    0,
	0,
}; 
#else


uint16_t hrtim_data1[] = {
	
11556,
12324,
292,
1060,
1828,
2596,
3364,
4132,
4900,
5668,
6444,
7204,
7972,
8740,
9508,
10276,
11044,
11812,
};

uint16_t current_data1[] = {
28,
76,
122,
192,
199,
139,
78,
28,
29,
58,
117,
149,
175,
142,
100,
41,
18,
23,
};
const uint16_t CURRENT_DATA_SIZE1 = sizeof(current_data1)/sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst1 = {
    0,
    CURRENT_DATA_SIZE1,
    1572,   // 高端开通点
    6527 ,   // 高端关断点 (占空比= (300-100)/1000=20%)
    6527+1572 ,   // 低端开通点
    6527*2,   // 周期=1000
    0,
	0, 
	0x180,
	0,
}; 


#endif


const uint16_t CURRENT_DATA_SIZE = sizeof(current_data)/sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst = {
    0,
    CURRENT_DATA_SIZE,
    572,   // 高端开通点
    12602 ,   // 高端关断点 (占空比= (300-100)/1000=20%)
    12602+572 ,   // 低端开通点
    12602*2,   // 周期=1000
    0,
	0,
		0x180,		//adcPer hritm
	0,
}; 




int16_t * Power_Calculator_GetHrtimBuffAddress(uint8_t ch)
{
    if(ch)
    {
        return (int16_t * )hrtim_data1;
    }
    else
    {
        return (int16_t * )hrtim_data;
    }

}
int16_t * Power_Calculator_GetVoltageBuffAddress(uint8_t ch)
{
    if(ch)
    {
        return (int16_t * )voltage_data;
    }
    else
    {
        return (int16_t * )voltage_data;
    }

}

int16_t Power_Calculator_GetTxaBuffSize(uint8_t ch)
{
	
	    if(ch)
    {
       		return	CURRENT_DATA_SIZE1;
    }
    else
    {
        		return	CURRENT_DATA_SIZE;
    }

	
	

}

int16_t * Power_Calculator_GetTxaBuffAddress(uint8_t ch)
{
    if(ch)
    {
        return (int16_t * )current_data1;
    }
    else
    {
        return (int16_t * )current_data;
    }

}


    // PowerResult res = CalculatePower(
    //     current_data, 
    //     hrtim_data, 
    //     0,  // 无电压数据
    //     100, 
    //     ppg_cycle
    // );


#if 0
// 32位整数平方根（避免浮点）
static uint32_t sqrt32(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;  // 最大位

    while (bit > n) bit >>= 2;
    
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

// 余弦查表函数（整数运算，返回值放大1000倍）
static int32_t cos_lookup(int32_t angle_degrees) {
    // 归一化角度到0-359度
    angle_degrees %= 360;
    if (angle_degrees < 0) angle_degrees += 360;
    
    // 利用余弦函数的对称性
    bool is_neg = (angle_degrees > 90 && angle_degrees < 270);
    if (angle_degrees > 180) angle_degrees = 360 - angle_degrees;
    
    // 0-90度余弦表（每5度一个点，值放大1000倍）
    const int32_t cos_table []= {
        1000, 996, 985, 966, 940, 906, 866, 819, 766, 
        707, 643, 574, 500, 423, 342, 259, 174, 87, 0
    };
    
    // 计算最接近的索引
    uint8_t index = (angle_degrees * 2) / 10;  // 每5度一个点
    if (index > 18) index = 18;
    
    return is_neg ? -cos_table[index] : cos_table[index];
}

// 检测电流谷点
static bool isValley(const uint16_t* current, uint16_t i) {
    return (current[i-1] >current[i]) && (current[i+1] > current[i]);
}

// 在指定区间内寻找过零点
static uint16_t FindZeroCrossingAi(const uint16_t* current, uint16_t start, uint16_t end) {
    uint16_t candidate = start;
    uint16_t min_val = current[start];
    
    for (uint16_t i = start + 1; i < end - 1; i++) {
        if (isValley(current, i) && current[i] < min_val) {
            min_val = current[i];
            candidate = i;
        }
    }
    return candidate;
}


// 验证过零点有效性
static bool ValidateZeroPoint(const uint16_t* current, uint16_t idx, bool isHighSide) {
    const uint16_t CHECK_COUNT = 3;
    for (uint16_t i = 1; i <= CHECK_COUNT; i++) {
        if (idx + i >= CHECK_COUNT) break;
        if (isHighSide) {
            if (current[idx+i] <= current[idx+i-1]) return false;
        } else {
            if (current[idx+i] >= current[idx+i-1]) return false;
        }
    }
    return true;
}

// 检测峰值电流
static PeakInfo findPeakCurrent(const uint16_t* current, uint16_t start, uint16_t end) {
    PeakInfo peak = {start, current[start]};
    
    for (uint16_t i = start + 1; i < end; i++) {
        if (current[i] > peak.value) {
            peak.index = i;
            peak.value = current[i];
        }
    }
    return peak;
}

// 计算电流相位
static int32_t calculatePhaseAngle(uint16_t zero_cross_idx, uint16_t peak_idx) {
    if (zero_cross_idx >= peak_idx) return 0;
    
    // 相位滞后计算：相位角 = (峰值时间 - 过零时间) * 18000 / 半周期点数
    // 假设半周期50个采样点对应180度（根据实际调整除数）
    return ((peak_idx - zero_cross_idx) * 18000) / 50;  // 返回度*100
}

// 判断空载特性
static bool isNoLoadCondition(const uint16_t* current, uint16_t count, int32_t peak_current) {
    const uint16_t NO_LOAD_THRESHOLD = 50;  // 空载电流阈值
    
    // 条件1：峰值电流低于阈值
    if (peak_current < NO_LOAD_THRESHOLD) return true;
    
    // 条件2：电流波动范围小
    uint16_t min = 0xFFFF, max = 0;
    for (uint16_t i = 0; i < count; i++) {
        if (current[i] < min) min = current[i];
        if (current[i] > max) max = current[i];
    }
    return (max - min) < (NO_LOAD_THRESHOLD >> 1);  // 右移代替除2
}
#endif
// 主计算函数
#if 0
PowerResult CalculatePowerAi(
    const uint16_t* resonant_current,
    const uint16_t* hrtim_values,
    const uint16_t* voltage_data,
    uint16_t count,
    PPGpointDef ppg) 
{
    PowerResult result = {0};
    
    // 参数安全检查
    if (!resonant_current || !hrtim_values || count == 0 || ppg.lowOff >= count) {
        return result;
    }
    
    // 1. 峰值电流检测
    PeakInfo peak = findPeakCurrent(resonant_current, ppg.highOn, ppg.highOff);
    result.peak_current = peak.value;
    
    // 2. 空载特性判断
    result.is_no_load = isNoLoadCondition(resonant_current, count, peak.value);
    
    // 3. 获取高低端区间

    
    // 4. 过零点检测
    result.zero_cross_high = FindZeroCrossing(resonant_current, ppg.highOn, ppg.highOff);
    result.zero_cross_low = FindZeroCrossing(resonant_current, ppg.lowOn, ppg.lowOff);
    
    // 5. 方向验证
    if (!ValidateZeroPoint(resonant_current, result.zero_cross_high, true)) {
        result.zero_cross_high = ppg.highOn + (ppg.highOff) * 3 / 4;
    }
    if (!ValidateZeroPoint(resonant_current, result.zero_cross_low, false)) {
        result.zero_cross_low =ppg.lowOn  +  (ppg.lowOff) * 3 / 4;
    }
    
    // 6. 相位计算
    result.phase_angle = calculatePhaseAngle(result.zero_cross_high, peak.index);
    
    // 7. 功率计算（全整数运算）
    uint32_t sum_sq = 0;
    uint16_t valid_points = 0;
    const uint16_t start_idx = result.zero_cross_high;
    const uint16_t end_idx = result.zero_cross_low;
    
    for (uint16_t i = start_idx; i < end_idx && i < count; i++) {
        uint32_t val = resonant_current[i];
        sum_sq += val * val;
        valid_points++;
    }
    
    if (valid_points > 0) {
        // 计算RMS电流
        uint32_t rms_current = sqrt32(sum_sq / valid_points);
        
        // 获取RMS电压（默认或测量值）
        uint16_t rms_voltage = voltage_data ? voltage_data[peak.index] : 2200;  // 默认220V*10
        
        // 计算有功功率：P = (I_rms * V_rms * cosφ) / 1000
        int32_t cos_phi = cos_lookup(result.phase_angle / 100);  // 转换为实际角度
        int64_t power = (int64_t)rms_current * rms_voltage * cos_phi;
        result.active_power = (int32_t)(power / 1000000);  // 除以1000*1000（cos_phi放大1000倍）
    }
    
    return result;
}
#endif



typedef	struct 
{
    uint16_t start; 
    uint16_t end;

}WindowsDef;

enum
{
	PLUSE_START=0,
	PLUSE_UP,
	PLUSE_DOWN,

};
enum
{
	pluseStart=0,
	pluseHighOn,
	pluseHighOff,
	pluseLowOn,
	pluseLowOff,
	pluseDown,
	pluseLowScr,
	pluseEnd,
};
#define		DEBUGOUT 0
//#define		hrtim_per_adc_500nS		    FRE_1000K_PWM
//#define		hrtim_per_adc_1000nS		FRE_500K_PWM
//#define		hrtim_per_adc_1300nS		1040			//1.3us对应的HRTIM值

//#define		hrtim_per_adc			hrtim_per_adc_500nS
//#define		HRTIM_BASE			1				//将1.3us插值为64份
//#define		HRTIM_BASE_ADJ	hrtim_per_adc/HRTIM_BASE	

#define		HRTIM_BASE			

#include    <stdlib.h>
//#include    "api_hrtim.h"

// 在指定区间内寻找过零点
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end) {


		if(current==0)
		{
			return 0;
		}	
		if(start>0xff00)
		{
			return 0;
		}	
		
		
		
    uint16_t    candidate = 0;

    uint16_t    direction=0xffff;
    uint16_t	preValue,newValue;	
    preValue=current[start];		
		uint32_t mivZ=0xffffffff;
		
    for (uint16_t i = start + 1; i < end; i++) {

        newValue=current[i];

    	int16_t     delat=newValue-preValue;
        
        direction<<=1;

    	if(delat>0)
		{
			direction|=0x1;
		}
		else
		{
            direction&=~0x1;
		}


        preValue=newValue;


    if((direction&0x3)==0x1)//前两次方向，拐点位置	
		{			
			uint16_t phasePoint=i-1;		//得到相位点	


            uint32_t zeroRange=   current[phasePoint-1]+current[phasePoint+1];      //过零点前后二个点的积最小，为过零点


            if(mivZ>zeroRange)    
            {
                mivZ=zeroRange;             //找到一个最小点
								zeroRange>>=1;			//两个点的平均值

                if(current[phasePoint]<=zeroRange)     //判断最低点值是否够小)                
                {
                    candidate=    phasePoint;
                }
            }

        }    


    }
    return candidate;
}







CalculatorResultDef CalculateAuctalCurrent(uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_data,
    uint16_t start,
    PowerCalculatorInputDef* input)
{
    uint16_t num=start;

    CalculatorResultDef result={0};
	uint16_t	hrtim_per_adc=input->perAdc;
    uint16_t  end=input->highOff;
    uint16_t  dead=input->lowOn;        //死区时间
    uint16_t  currentMax=0;
    uint16_t  currentMaxNum;
    uint16_t  currentType=pluseStart;            //当前统计区间
    uint32_t  currentMaxSum;
    uint32_t  currentSumPower;
    uint16_t  vcCnt=1;

    if(hrtim_values[start]>end)
    {
        end=input->lowOff;
        dead=input->highOn;
    }


    uint32_t currentSum=resonant_current[num];
    uint32_t  vcSum=voltage_data[num];
    num++;
    do
    {
        uint16_t newValue=resonant_current[num];
        currentSum+=newValue;


if(currentMax < newValue)
{
    int16_t d1, d2, d3, avg;
    d1 = newValue - resonant_current[num-1];        /* 最后一步 (正数=上升) */
    d2 = resonant_current[num-1] - resonant_current[num-2];
    d3 = resonant_current[num-2] - resonant_current[num-3];
    
    if(d2 < 0) d2 = 0;
    if(d3 < 0) d3 = 0;
    
    avg = (d2 + d3) / 2;
    
    if(d1 > avg)
    {
        newValue = resonant_current[num-1] + avg;   /* 限幅到平均值 */
    }
    
    currentMax = newValue;
    currentMaxSum = currentSum;
    currentMaxNum = num;
}
		
				
				

		uint16_t hrtim1us=	hrtim_values[num];
    int32_t lastPoint;
				
        switch(currentType)
        {
            case        pluseStart: 
                vcSum+=voltage_data[num];           //统计电压平均值
                vcCnt++;
				lastPoint=(int32_t)end-hrtim1us;	
		        if(lastPoint<hrtim_per_adc)
                {                               //关断点
                    //  if(currentMaxSum<currentSum)
										if(1)
                    {                           //最大值在关断点前出现，以关断点为结束
                                                //关断点插值修正
                        if(lastPoint>0)   
												{
                            lastPoint*=newValue;		                //最后一个点不完整，加上修正值	
						    // lastPoint/=hrtim_per_adc;
                            currentSumPower=currentSum;
														currentSumPower*=hrtim_per_adc;
                            currentSumPower+= lastPoint;   
                            currentType= pluseLowOn;
                        }
                    }
                    else
                    {                            //最大值在关断点前没出现，继续统计到对管开通（死区计入电流积分）

                        currentType= pluseHighOff;
                    }
  
               
                }
            break; 
            case    pluseHighOff :                //开启点，用于等谐振电流最大值
                if((dead-hrtim1us)<hrtim_per_adc)
                {

                    currentSumPower=currentMaxSum;
                    currentSumPower*=hrtim_per_adc;
                    currentType= pluseLowOn;     //结束循环
                }   
            break;    
        } 

        num++;
				if(num>input->end)
				{
						return  result;
				}
				

    }while(currentType<pluseLowOn);

    vcSum/=vcCnt;//得到电压平均值，有效开通段

    result.current=currentSumPower;
    result.voltage=vcSum;
		result.peak_current=currentMax;
		result.peak_num=currentMaxNum;

    return  result;
}    


PowerCalculatorInputDef*     Power_Calculator_GetInputArrayAddress(uint8_t ch)
{
    if(ch)
    {

        return  &inputArrayConst1;
    }
    else
    {
        return  &inputArrayConst;
    }

}

PowerResult CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input
    ) 
{	


    PowerResult     xReturn;

	xReturn.zero_cross_high=0;
	xReturn.zero_cross_low=0;    




		uint16_t				hrtim_per_adc=input->perAdc;
    uint16_t        potNumStart=input->start+4;
    uint16_t        zeroStart,zeroEnd;
    uint16_t        highOffNum=input->highOff/hrtim_per_adc;

    highOffNum+=potNumStart;            //上管关断序号点

//--step1   寻找过零点， 有历史点用小范围，没有用全范围 


			zeroStart=potNumStart;
			zeroEnd=highOffNum+2;

    uint16_t phaseUp= FindZeroCrossing(resonant_current, zeroStart, zeroEnd); 


        zeroStart=highOffNum;
        zeroEnd=input->end;



    uint16_t phaseDown= FindZeroCrossing(resonant_current, zeroStart, zeroEnd); 

    if(phaseUp&&phaseDown)
    {                                           //过零点确认后计算有效功率
        CalculatorResultDef sumUp= CalculateAuctalCurrent(resonant_current,hrtim_values,voltage_values,phaseUp,input);       //统计上管有效电流
				
				if(sumUp.current==0)
				{
					return xReturn; 
				}	
			
				uint64_t	sumAll; 
				uint32_t    sumCurrent; 
				if((input->highOff*2+10)<input->lowOff)	
				{		//非对称输出
						CalculatorResultDef sumDown= CalculateAuctalCurrent(resonant_current,hrtim_values,voltage_values,phaseDown,input);    //统计下管有效电流
//						sumAll= sumUp.current*sumUp.voltage+ sumDown.current*sumDown.voltage;
						sumCurrent=sumUp.current+sumDown.current;
				}
				else
				{		//对称输出
//						sumAll=sumUp.current*sumUp.voltage*2;
            sumCurrent=sumUp.current*2;
				}	
      
   
				sumAll/=input->lowOff;          //乘上电压的功率

				sumCurrent/=input->lowOff;      //电流值

//瞬时电压			
				xReturn.voltage=			sumUp.voltage;
				
////瞬时功率
//				xReturn.active_power=sumAll>>4;

//瞬时电流
        xReturn.active_current=sumCurrent;

				 xReturn.peak_current=sumUp.peak_current;

				

				if(hrtim_values[phaseUp]>input->highOn)
				{

                    uint32_t angle=hrtim_values[phaseUp]-input->highOn;
						
//                    xReturn.zero_cross_high=angle/(FRE_PER_ADC/4);
					xReturn.zero_cross_high=hrtim_values[phaseUp];	
					
					
                    angle*=PHASE_DEG_BASE;		//相位角（180度为单位）
                    angle/=input->highOff-input->highOn;
                    xReturn.phase_angleUp=angle;


				}	
				if(hrtim_values[phaseDown]>input->lowOn)
				{
				    uint32_t    angle=hrtim_values[phaseDown]-input->lowOn;		//相位角（180度为单位）

//						xReturn.zero_cross_low=angle/(FRE_PER_ADC/4);
						 xReturn.zero_cross_low=hrtim_values[phaseDown];	
                    angle*=180;
                    angle/=(input->lowOff-input->lowOn);
                    xReturn.phase_angleDown=angle;

				}	


    }

		
	return	xReturn;
}
#if 0
/* ========================================================================
 * CalculatePower_FPU — 单周期功率计算 (FPU, 1ms 快速路径)
 *
 * 算法: I×Vdc 直接积分, 关断点线性插值, 对称性检测.
 *       不依赖 phi, V_fund 或 C — 仅依赖 ADC 标定.
 *       返回扩展 PowerResult (含 float 字段).
 * ======================================================================== */

/* ---- FPU 辅助: 电流零偏估计 ---- */
static float _EstimateIzero(const uint16_t* adc, uint16_t start, uint16_t end)
{
    /* 单遍扫描找最小 3 值, O(n) 无额外内存.
       整流后电流零点 = 波形底部, 取最小 3 值均值抗噪. */
    float m1 = 1e9f, m2 = 1e9f, m3 = 1e9f;
    for (uint16_t i = start; i < end; i++) {
        float v = (float)adc[i];
        if (v < m1)      { m3 = m2; m2 = m1; m1 = v; }
        else if (v < m2) { m3 = m2; m2 = v; }
        else if (v < m3) { m3 = v; }
    }
    return (m1 + m2 + m3) / 3.0f;
}

/* ---- FPU 辅助: 谷值检测 (方向跟踪法) ---- */
static uint16_t _FindValley_f(const uint16_t* adc, uint16_t start, uint16_t end,
                               uint16_t* out_idx)
{
    uint16_t candidate = 0;
    uint16_t direction = 0xffff;
    float pre_val = (float)adc[start];
    uint32_t min_sum = 0xffffffff;

    for (uint16_t i = start + 1; i < end && i < start + 200; i++) {
        float cur_val = (float)adc[i];
        direction <<= 1;
        if (cur_val > pre_val) direction |= 0x1;
        else                   direction &= ~0x1;

        if ((direction & 0x3) == 0x1) {
            uint16_t pi = i - 1;
            if (pi > start && (pi + 1) < end) {
                uint32_t zr = (uint32_t)adc[pi - 1] + (uint32_t)adc[pi + 1];
                uint32_t mid2 = (uint32_t)adc[pi] * 2;
                if (mid2 <= zr && zr < min_sum) {
                    min_sum = zr;
                    candidate = pi;
                }
            }
        }
        pre_val = cur_val;
    }

    *out_idx = candidate;
    return candidate;
}

/* ---- FPU 辅助: I x Vdc 积分 (关断点插值) ---- */
static float _IntegratePwr(const uint16_t* adc_i, const uint16_t* hrtim,
                           const uint16_t* adc_v, uint16_t start,
                           PowerCalculatorInputDef* input, float I_zero,
                           float* Vdc_sum, uint16_t* vdc_n)
{
    uint16_t i = start;
    uint16_t end_cnt;
    float sum = 0.0f;
    float vsum = 0.0f;
    uint16_t vn = 0;
    uint16_t per = input->perAdc;
    if (per == 0) per = 1;

    if (hrtim[start] > input->highOff) {
        end_cnt = input->lowOff;
    } else {
        end_cnt = input->highOff;
    }

    while (i < input->end) {
        float I_act = ((float)adc_i[i] - I_zero) * I_SCALE;
        float Vdc   = (float)adc_v[i] * VDC_SCALE;

        float weight = 1.0f;
        if (i + 1 < input->end) {
            int32_t dCNT = (int32_t)hrtim[i + 1] - (int32_t)hrtim[i];
            if (dCNT > 0 && dCNT < 10000) {
                int32_t dist = (int32_t)end_cnt - (int32_t)hrtim[i];
                if (dist > 0 && dist < dCNT)
                    weight = (float)dist / (float)dCNT;
            }
        }
        sum  += I_act * Vdc * weight;
        vsum += Vdc;
        vn++;

        if (hrtim[i] >= end_cnt) break;
        i++;
    }

    if (Vdc_sum) *Vdc_sum = vsum;
    if (vdc_n)   *vdc_n   = vn;
    return sum;
}

PowerResult CalculatePower_FPU(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input
    )
{
    PowerResult r;
    memset(&r, 0, sizeof(PowerResult));

    uint16_t start_i = input->start + 4;
    uint16_t end_i   = input->end;
    if (end_i <= start_i + 4) return r;

    uint16_t per = input->perAdc;
    if (per == 0) per = 1;

    /* 1. I_zero */
    float I_zero = _EstimateIzero(resonant_current, start_i, end_i);

    /* 2. 谷值检测 */
    uint16_t v_up = 0, v_dn = 0;
    uint16_t co_idx = input->highOff / per + start_i;
    if (co_idx >= end_i) co_idx = end_i - 2;

    _FindValley_f(resonant_current, start_i, co_idx + 2, &v_up);
    _FindValley_f(resonant_current, co_idx, end_i, &v_dn);

    if (!v_up) v_up = start_i;
    if (!v_dn) v_dn = co_idx + 1;

    r.zero_cross_high = v_up;
    r.zero_cross_low  = v_dn;

    /* 3. I x Vdc 积分 */
    float Vdc_s = 0.0f;
    uint16_t vn = 0;
    float s_up = _IntegratePwr(resonant_current, hrtim_values, voltage_values,
                                v_up, input, I_zero, &Vdc_s, &vn);

    /* 4. 对称性 */
    int sym = ((input->highOff * 2 + 10) >= input->lowOff);
    float s_dn = 0.0f;
    if (!sym) {
        s_dn = _IntegratePwr(resonant_current, hrtim_values, voltage_values,
                              v_dn, input, I_zero, NULL, NULL);
    }

    /* 5. P_W — 归一化: 累加和 ÷ (lowOff/perAdc) 转为周期平均功率 */
    float N_cycle = (float)input->lowOff / (float)per;
    if (N_cycle < 1.0f) N_cycle = 1.0f;
    float P_W;
    if (sym)       P_W = s_up * 2.0f / N_cycle;
    else if (s_dn) P_W = (s_up + s_dn) / N_cycle;
    else           P_W = s_up / N_cycle;
    if (P_W < 0.0f) P_W = 0.0f;

    /* 6. Float fields */
    r.P_W       = P_W;
    r.Vdc_mean  = (vn > 0) ? Vdc_s / (float)vn : 0.0f;

    /* 6. I_peak / I_rms — 峰值检测法 */
    float I_peak_A = 0.0f;
    {
        uint16_t I_max = 0;
        for (uint16_t k = 0; k < end_i; k++) {
            if (resonant_current[k] < 60000 && resonant_current[k] > I_max)
                I_max = resonant_current[k];
        }
        if (I_max > I_zero)
            I_peak_A = ((float)I_max - I_zero) * I_SCALE;
    }
    r.I_peak = I_peak_A;
    r.I_rms  = I_peak_A * 0.70710678f;

    /* 7. phi */
    if (v_up > start_i && hrtim_values[v_up] > input->highOn) {
        uint32_t dist = hrtim_values[v_up] - input->highOn;
        uint32_t T_sw = input->lowOff;
        if (T_sw > 0) {
            r.phi_deg = (float)dist / (float)T_sw * 360.0f;
            if (r.phi_deg > 180.0f) r.phi_deg -= 360.0f;
            if (r.phi_deg < -180.0f) r.phi_deg += 360.0f;
        }
    }

    /* 8. Int fields (compat) */
    r.active_power  = (int32_t)(P_W * 16.0f);
    r.active_current = (int32_t)(r.I_rms * 100.0f);
    r.voltage        = (uint16_t)(r.Vdc_mean / VDC_SCALE);
    r.peak_current   = (uint16_t)(I_peak_A / I_SCALE + I_zero);
    r.zero_current   = (uint16_t)I_zero;
    if (r.I_rms > 0.001f)
        r.esr = (uint16_t)(r.Vdc_mean / r.I_rms);

    if (v_up > start_i && hrtim_values[v_up] > input->highOn) {
        int32_t a = (int32_t)(r.phi_deg * 10.0f);
        if (a < 0) a += 3600;
        r.phase_angleUp = (int16_t)a;
    }

    return r;
}

/* ========================================================================
 * Kalman 滤波器: L 平滑 + 突变检测
 * ======================================================================== */

static KalmanLState g_Kalman[4];

static float _L_KalmanStep(uint8_t head, float L_meas, float I_rms)
{
    KalmanLState* s = &g_Kalman[head];
    float R = KALMAN_R0 / ((I_rms > 0.1f) ? (I_rms / 10.0f) : 1.0f);

    if (!s->initialized) {
        s->x_hat    = L_meas;
        s->P        = 1.0f;
        s->sigma_run = 2.0f;
        s->n_samples = 1;
        s->initialized = 1;
        return L_meas;
    }

    /* Predict */
    float x_pred = s->x_hat;
    float P_pred = s->P + KALMAN_Q;

    /* Update */
    float innov = L_meas - x_pred;
    float K = P_pred / (P_pred + R);
    s->x_hat = x_pred + K * innov;
    s->P = (1.0f - K) * P_pred;

    /* Running sigma */
    if (s->n_samples < 20) {
        s->n_samples++;
        float alpha = 1.0f / (float)s->n_samples;
        s->sigma_run = (1.0f - alpha) * s->sigma_run
                     + alpha * (innov > 0 ? innov : -innov) * 1.4826f;
    } else {
        float alpha = 0.05f;
        s->sigma_run = (1.0f - alpha) * s->sigma_run
                     + alpha * (innov > 0 ? innov : -innov) * 1.4826f;
    }

    return s->x_hat;
}

static uint8_t _L_AnomalyCheck(uint8_t head, float L_meas, float I_rms)
{
    KalmanLState* s = &g_Kalman[head];
    if (!s->initialized || s->sigma_run < 0.01f) return 0;

    float R = KALMAN_R0 / ((I_rms > 0.1f) ? (I_rms / 10.0f) : 1.0f);
    float innov = L_meas - s->x_hat;
    float innov_std = sqrtf(s->P + R);
    if (innov_std < 0.01f) return 0;

    return ((innov > 0 ? innov : -innov) > KALMAN_ANOMALY_THRESH * s->sigma_run) ? 1 : 0;
}

/* ========================================================================
 * L B-H 修正表 (I_rms 非线性补偿)
 * ======================================================================== */

static float _L_CorrectB_H(float L_raw, float I_rms)
{
    /* 4段线性修正系数 (来源: TEST_REPORT 电感-电流曲线) */
    static const struct { float lo, hi, k, Iref; } bands[] = {
        {  0.0f,  8.0f, 1.12f,  5.0f },
        {  8.0f, 14.0f, 1.08f, 11.0f },
        { 14.0f, 20.0f, 1.05f, 17.0f },
        { 20.0f, 99.0f, 1.03f, 25.0f },
    };
    int n = sizeof(bands) / sizeof(bands[0]);
    for (int i = 0; i < n; i++) {
        if (I_rms >= bands[i].lo && I_rms < bands[i].hi) {
            float ratio = I_rms / bands[i].Iref;
            return L_raw / (1.0f + bands[i].k * (ratio - 1.0f));
        }
    }
    return L_raw;
}

/* ========================================================================
 * CalculateElecParams_20ms — 20ms 电参数计算 (4炉头统一)
 * ======================================================================== */

void CalculateElecParams_20ms(
    uint16_t* current_buf[4],
    uint16_t* hrtim_buf[4],
    uint16_t* voltage_buf[4],
    PowerCalculatorInputDef* input[4],
    ElecParamsDef elec[4]
    )
{
    for (uint8_t h = 0; h < 4; h++) {
        ElecParamsDef* e = &elec[h];
        memset(e, 0, sizeof(ElecParamsDef));

        if (!input[h] || input[h]->end == 0 || input[h]->highOff == 0)
            continue;

        PowerCalculatorInputDef* in = input[h];
        uint16_t* adc_i = current_buf[h];
        uint16_t* hrtim = hrtim_buf[h];
        uint16_t* adc_v = voltage_buf[h];

        /* 调用 FPU 功率计算获取基础值 */
        PowerResult pr = CalculatePower_FPU(adc_i, hrtim, adc_v, in);

        e->P_W      = pr.P_W;
        e->I_rms    = pr.I_rms;
        e->I_peak   = pr.I_peak;
        e->Vdc_mean = pr.Vdc_mean;
        e->phi_deg  = pr.phi_deg;
        e->valid    = (pr.I_rms > 0.001f) ? 1 : 0;
        if (!e->valid) continue;

        /* ---- f_sw (HRTIM period) ---- */
        uint32_t period_cnt = in->lowOff;
        if (period_cnt > 0 && in->perAdc > 0) {
            e->f_sw_kHz = 2000.0f * (float)in->perAdc / (float)period_cnt;
        }

        /* ---- D_U, DT1, DT2 ---- */
        uint32_t CU = in->highOn, CO = in->highOff;
        uint32_t LN = in->lowOn,  LO = in->lowOff;
        if (period_cnt > 0 && in->perAdc > 0) {
            float tpc = 0.5f / (float)in->perAdc;  /* us/count */
            e->D_U_pct = (float)(CO - CU) / (float)period_cnt * 100.0f;
            e->DT1_us  = (float)(LN - CO) * tpc;
            int32_t dt2 = (int32_t)CU - (int32_t)LO;
            if (dt2 < 0) dt2 += (int32_t)period_cnt;
            e->DT2_us  = (float)dt2 * tpc;
        }

        /* ---- cos_phi ---- */
        e->cos_phi = cosf(e->phi_deg * 0.01745329252f);

        /* ---- 阻抗 (KVL dI/dt 封闭解, dI/dt 验证) ---- */
        /* L: Vdc/2 + V_C_peak = L × I_peak × ω_sw
         *     V_C_peak = I_peak / (ω_sw × C)                         */
        float C_F     = C_RES_uF * 1e-6f;
        float omega_sw = 2.0f * 3.14159265f * e->f_sw_kHz * 1000.0f;
        float V_C_peak = e->I_peak / (omega_sw * C_F);
        e->L_uH = (e->Vdc_mean * 0.5f + V_C_peak) / (e->I_peak * omega_sw) * 1e6f;
        if (e->L_uH < 0) e->L_uH = 0;

        /* f_res = 1/(2π√(LC)) — 从 L 反推, 非谷值间隔 */
        if (e->L_uH > 0.001f) {
            e->f_res_kHz = 1.0f / (2.0f * 3.14159265f
                * sqrtf(e->L_uH * 1e-6f * C_F)) / 1000.0f;
        } else {
            e->f_res_kHz = e->f_sw_kHz;
        }

        /* Q = tan(φ) / (f_sw/f_res - f_res/f_sw) */
        {
            float tan_phi   = tanf(e->phi_deg * 0.01745329252f);
            float ratio     = e->f_sw_kHz / e->f_res_kHz;
            float ratio_inv = e->f_res_kHz / e->f_sw_kHz;
            float denom     = ratio - ratio_inv;
            if (fabsf(denom) > 0.001f)
                e->Q_factor = tan_phi / denom;
            else
                e->Q_factor = 0.0f;
        }

        /* R = ω_res × L / Q */
        {
            float omega_res = 2.0f * 3.14159265f * e->f_res_kHz * 1000.0f;
            if (e->Q_factor > 0.001f)
                e->R_ohm = omega_res * e->L_uH * 1e-6f / e->Q_factor;
            else
                e->R_ohm = 0.0f;
        }

        /* 阻抗 @ f_sw (工作频率, 非谐振点) */
        {
            float X_L_sw = omega_sw * e->L_uH * 1e-6f;
            float X_C_sw = 1.0f / (omega_sw * C_F);
            e->X_ohm = X_L_sw - X_C_sw;
            e->Z_mag_ohm = sqrtf(e->R_ohm * e->R_ohm + e->X_ohm * e->X_ohm);
        }

        /* ---- L B-H 修正 ---- */
        e->L_corr_uH = _L_CorrectB_H(e->L_uH, e->I_rms);

        /* ---- Kalman ---- */
        e->L_kalman_uH = _L_KalmanStep(h, e->L_corr_uH, e->I_rms);
        e->anomaly = _L_AnomalyCheck(h, e->L_corr_uH, e->I_rms);
    }
}
#endif

#if 0
void CalculatePowerU1(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_data,
    uint16_t count,
    PPGpointDef* ppgValue,
) 
{	


//#ifdef  DEBUG_POWER_OUT
#if 0	
    resonant_current=current_data;
    hrtim_values=hrtim_data;
    voltage_data=0;
    count=CURRENT_DATA_SIZE;
    ppgValue=&ppg_cycle;

#endif

	
				
				    // uint16_t		potCh=0;		//炉头号，//炉头序号

				// txaPoint.sumStart=resonant_current[potNum-2];
				// txaPoint.sumStart+=resonant_current[potNum-1];
				// txaPoint.sumStart+=resonant_current[potNum];

					uint32_t 		direction=0xffff;	
					uint8_t     	pluseStep=0;//0	
					uint16_t 		hrtim1us;
                    uint16_t        potNumStart=3;
					uint16_t		preValue,newValue;	
                	// memset(&txaPoint,0,sizeof(CalculatePowerDef));		//清零
#include	"commClass.h"                    
	            MemSetInt((uint32_t*)(&txaPoint),0,sizeof(CalculatePowerDef)/sizeof(int)); 

				preValue=resonant_current[potNumStart-1];

	for(uint16_t potNum=potNumStart;potNum<count;potNum++)        //前面预留了三个数据
	{		
						
	
					newValue=resonant_current[potNum];	

//----------------step 1 突变修正--------------------------------------------	                   
#if   1 //突变修正	                    
					int16_t     delat=newValue-preValue;
					direction<<=1;	
					if(abs(delat)>300)
					{
							uint16_t backValue=resonant_current[potNum+1];
							uint32_t avgValue=(preValue+backValue)/2;
							newValue=avgValue;
					}
					preValue=newValue;			//更新上一TxA_A次的值

                    // resonant_current[potNum]=newValue;

#endif   //突变修正	  


//----------------step 2 找到过零点--------------------------------------------	  
#if   1 //找到过零点			
						if(delat>10)
						{
							direction|=0x1;

						}
						else
						{
                            direction&=~0x1;
			
						}		


						if((direction&0x3f)==0x7)//000111,前四次方向，拐点位置	
						{			
					        uint16_t phasePoint=potNum-3;		//得到相位点	
                            uint32_t mivZ;
                            
                            mivZ=   resonant_current[phasePoint]+resonant_current[phasePoint-1]+resonant_current[phasePoint+1];
                            // if(mivZ<txaPoint.mivZero)    
                            // {
                            //     txaPoint.mivZero=mivZ; 
                            // }



                            if(resonant_current[phasePoint]<10)     //判断最低点值是否够小
                            {
                                uint16_t hrtim=hrtim_values[phasePoint];
                                if(pluseStep<pluseLowOn)
                                {//上管过零点
				


							        txaPoint.phaseUpHrtimN=hrtim-ppgValue->highOn;
							        // txaPoint.phaseUpHrtimP=ppgValue->highOff-hrtim;	//负数说明相位为
							        // phasePoint-=potNumStart;
							        txaPoint.phaseUp=phasePoint;		//相位起始点

                                }
                                else
                                {//下管过零点
		
							        txaPoint.phaseDownHrtimN=hrtim-ppgValue->lowOn;
							        // txaPoint.phaseDownHrtimP=ppgValue->lowOff-hrtim;	//负数说明相位为
							        // phasePoint-=potNumStart;
							        txaPoint.phaseDown=phasePoint;		//相位起始点
                                }
                            }    

						}

#endif  //找到过零点	



//----------------step 3 根据ppg状态判断START--------------------------------------------	  


#if 1   //根据ppg状态判断START


						switch(pluseStep)	
						{
							case  	pluseStart:		//检查开始,寻找上升起点//上管死区
								hrtim1us=	hrtim_values[potNum];
                         
								if(ppgValue->highOn-hrtim1us<hrtim_per_adc)
								{
									pluseStep=pluseHighOn;
								}

	
							
							case	pluseHighOn:

								
								    txaPoint.sumUp+=newValue;	//继续统计到下管开通  用于CEIL Q值计算

									hrtim1us=	hrtim_values[potNum];
									if(ppgValue->highOff-hrtim1us<hrtim_per_adc)
									{
                                        uint16_t  		lastPoint;
										lastPoint=ppgValue->highOff-hrtim1us;	
										lastPoint*=newValue;		                //最后一个点不完整，加上修正值	
										lastPoint/=HRTIM_BASE;
										txaPoint.lastAdjValueUp=lastPoint;

										// txaPoint.sumHighUp=txaPoint.sumUp;			//用于功率计算,这里SUMUP没有加NEW
										// txaPoint.upPoint=potNum-txaPoint.phaseUp;		//上管开通的时间
										pluseStep=pluseHighOff;
									}	

								break;

							case	pluseHighOff:	//下管死区 


									txaPoint.sumUp+=newValue;
									hrtim1us=hrtim_values[potNum];
									if(ppgValue->lowOn-hrtim1us<hrtim_per_adc)	
									{
										pluseStep=pluseLowOn;
									}

								
								break;	

							case	pluseLowOn:		//下降阶段，一直到下降拐点，统计负半周功率
									
									txaPoint.sumDown+=newValue;
									hrtim1us=	hrtim_values[potNum];
									if(ppgValue->lowOff-hrtim1us<hrtim_per_adc)
									{
                                        uint16_t  		lastPoint;
										lastPoint=ppgValue->lowOff-hrtim1us;	
										lastPoint*=newValue;		                //最后一个点不完整，加上修正值	
										lastPoint/=HRTIM_BASE;
										txaPoint.lastAdjValueDown=lastPoint;

										// txaPoint.sumHighUp=txaPoint.sumDown;			//用于功率计算,这里SUMUP没有加NEW
										// txaPoint.upPoint=potNum-txaPoint.phaseUp;		//上管开通的时间
										pluseStep=pluseHighOff;
									}		

									
								break;
		
						}
#endif      //根据ppg状态判断END


//----------------step 4 谐振电流积分start--------------------------------------------	  

#if 1   //谐振电流积分start --------------------------------------------------------------

                    if(pluseStep<pluseLowOn)
                    {
                        txaPoint.sumUp+=newValue;	//继续统计到下管开通  用于CEIL Q值计算       
					    if(txaPoint.phaseUp)		//以找到过零点，从过零点开始累加
					    {		
//得到TXA最大值
                            txaPoint.sumUpP+=newValue;

						    txaPoint.upAllValue+=newValue;
						    if(txaPoint.xReturn.peak_current<newValue)
						    {
							    txaPoint.xReturn.peak_current=newValue;       //最大谐振电流
							    txaPoint.TxaMaxNum=potNum;      //谐振电流最大点
							    txaPoint.upHalfValue=txaPoint.upAllValue;//最大谐振点前的积分
						    }
					    }


                    }
                    else
                    {
                        txaPoint.sumDown+=newValue;	//继续统计到下管开通  用于CEIL Q值计算       
					    if(txaPoint.phaseDown)		//以找到过零点，从过零点开始累加
					    {		//负向电流统计
                            txaPoint.sumDownP+=newValue;
                        }    

                    }

#endif//谐振电流积分end --------------------------------------------------------------





    }           

//----------------step 5 谐振电流死区修正 谐振电流最大值出现在关断点前，用关断时的积分，出现在关断点后 用最大点时的积分值--------------------------------------------	  
            uint32_t 	currentSum=txaPoint.sumUpP;	        //上管插值修正    

			// if(ppgValue->highOff*2+10>ppgValue->lowOff)
			// {
			// 	if(txaPoint.upHalfValue>currentSum)
			// 	{
			// 				currentSum=txaPoint.upHalfValue;        //在最大点之前关断，取最大点之前的积分值（加上了死区时间段）
			// 	}

			// }

//----------------step 6 谐振电流积分插值修正 --------------------------------------------	  
 #if 1      //插值修正     

                // uint32_t 	currentSum=txaPoint.sumUpP;	        //上管插值修正             
				currentSum*=HRTIM_BASE_ADJ;
    			currentSum+=txaPoint.lastAdjValueUp;					//整体以HRTIM时基为单位
				txaPoint.xReturn.active_power=currentSum;		//加上修正值

                currentSum=txaPoint.sumDownP;	                //下管插值修正        
				currentSum*=HRTIM_BASE_ADJ;
                currentSum+=txaPoint.lastAdjValueDown;
                txaPoint.xReturn.active_power+=currentSum;
                txaPoint.xReturn.active_power/=ppgValue->lowOff/HRTIM_BASE;
#endif

//----------------step 7 相位角输出 --------------------------------------------	  

 #if 1      //相位角输出     
			txaPoint.xReturn.phase_angle=txaPoint.phaseUpHrtimN*180;		//相位角（180度为单位）
			txaPoint.xReturn.phase_angle/=ppgValue->highOff;
#endif

            return txaPoint.xReturn;
}

 #if 0      //插值修正 								
				//如果不是对称输出，要加上相位修正
				//非对称下管修正					
				uint16_t 	phaseNum=txaPoint.phaseDown;	
				int16_t 	phaseCount=txaPoint.phaseUp;

				if(phaseCount>1)
				{	

					if(txaPoint.ppgPoint.highOff*2+10<txaPoint.ppgPoint.lowOff)
					{
						if(txaPoint.phaseUp<phaseDownCount)		//调占空比模式
						{	

							int32_t 	sum=txaPoint.sumDown;	
							phaseCount--;
							for(uint8_t i=0;i<phaseCount;i++)
							{
							sum-=TxA_ADC_AdcDmaBuff.Txa[potCh].Hrtim[phaseNum-i];
							}
						
							if(sum>0)
							{	
							txaPoint.phaseSum=sum;
							currentSum+=txaPoint.phaseSum*HRTIM_BASE_ADJ;
							}
					
						}
					}
					


				}	
				else
				{//相位保护
				
				
				}	
 #endif     //插值修正  

  #if 0      //相位修正    
				currentSum/=txaPoint.ppgPoint.lowOff/HRTIM_BASE;

				if(txaPoint.phaseDownValue>0)		//上下拐点准确识别
				{


#define	TXA_HALF	0
#if	TXA_HALF		//txaHalf
		uint32_t 	sumUpValue=txaPoint.sumHighUp;	
		uint32_t	downPhaseValue=txaPoint.sumDown;
		
		if((txaPoint.ppgPoint.highOff*2+10)<(txaPoint.ppgPoint.lowOff))
		{
				downPhaseValue*=txaPoint.ppgPoint.highOff;
				downPhaseValue/=(txaPoint.ppgPoint.lowOff-txaPoint.ppgPoint.highOff);
				sumUpValue=txaPoint.sumUp;

		}
		else
		{
			if(txaPoint.ppgPoint.highOff<FRE_500K_PWM)
			{



			}

		}



				



		 		AdcFromApiDma20ms.ceilQ[potCh][count]=downPhaseValue*100/(sumUpValue);	//峰值与积分的比值为Q
#else	//txaHalf



					uint32_t	downPhaseValue=txaPoint.phaseDownValue;
					if(txaPoint.ppgPoint.highOff<FRE_55K_PWM)
					{
//						sumUpValue+=FRE_1000K_PWM*2;
//						downPhaseValue-=FRE_1000K_PWM*2;
					}


					if((txaPoint.ppgPoint.highOff*2+10)<(txaPoint.ppgPoint.lowOff))
					{
						downPhaseValue*=txaPoint.ppgPoint.highOff;
						downPhaseValue/=txaPoint.ppgPoint.lowOff;
//				downPhaseValue/=(txaPoint.ppgPoint.lowOff-txaPoint.ppgPoint.highOff);
					}
					AdcFromApiDma20ms.ceilQ[potCh][count]=downPhaseValue*100/(sumUpValue);	//峰值与积分的比值为Q

#endif		//txaHalf

				}
				else
				{
					AdcFromApiDma20ms.ceilQ[potCh][count]=0x10;
				}

  #endif      //相位修正    


			// int32_t  	phaseDegree=txaPoint.phaseUpHrtim*180;		//相位角（180度为单位）
			// phaseDegree/=ppgValue->highOff;
				



#if 0       //输出调试信息

//------------------				
//输出调试信息------------	

		AdcFromApiDma20ms.Txa[potCh][count]	=currentSum;
		AdcFromApiDma20ms.phase[potCh][count]=phaseDegree;//txaPoint.phaseUpHrtim;	//这个不要变
		AdcFromApiDma20ms.phaseValue[potCh][count]=ppgValue->highOn;//txaPoint.phaseUpHrtimP;
		AdcFromApiDma20ms.phaseDown[potCh][count]=txaPoint.phaseUpHrtimN;//txaPoint.phaseUp;
		


			uint16_t * 	buff0=(uint16_t *)&(TxA_ADC_AdcDmaBuff.Txa[potCh].Hrtim[hrtimPointStart]);
//			uint32_t* 	buff0=(uint32_t *)&(AdcFromApiDma20ms.phaseDown[0][0]);
			uint16_t*	buff1=(uint16_t *)&(TxA_ADC_AdcDmaBuff.Txa[potCh].Hrtim[hrtimPointStart]);
			uint32_t* 	buff2=(uint32_t *)&(AdcFromApiDma20ms.ceilQ[0][0]);
			
				 if(AdcFromApiDma20ms.count==5)
//			if(AdcFromApiDma20ms.ceilQ[PotChWork][count]>0xa0)

//			if(AdcFromApiDma20ms.phase[PotChWork][count]>70)
			{
			
				adcSize=(hrtimPointOver-hrtimPointStart);		//4个一组

				adcSum[0]=AdcFromApiDma20ms.ceilQ[PotChWork][count];						//相位值
				adcSum[1]=AdcFromApiDma20ms.phaseValue[PotChWork][count];
				adcSum[2]=AdcFromApiDma20ms.phaseDown[PotChWork][count];
				adcSum[3]=AdcFromApiDma20ms.phase[PotChWork][count];				
				
					for(uint8_t i=0;i<adcSize;i++)
					{
							adcBuff[0][i]=buff0[i];
//							adcBuff[0][i]=buff0[i];
							adcBuff[1][i]=buff1[i];
							adcBuff[2][i]=buff2[i];

					}

					
			}
//			API_GPIO_WritePin(DebugA_pin,0);		
			
#endif       //输出调试信息				
				

#endif
