#include "power_calculator.h"

#if 0
// HRTIMĘ±Ľä´ÁĘý×é (uint32_t)
uint16_t hrtim_data[] = {
    19004,19388,19772,188, 572, 956, 1340, 1724, 2108, 2492, 2876, 3260, 3644,
    4028, 4412, 4796, 5180, 5564, 5948, 6332, 6716, 7100, 7484,
    7868, 8252, 8636, 9020, 9404, 9788, 10172, 10556, 10940, 11324,
    11708, 12092, 12476, 12860, 13244, 13628, 14012, 14396, 14780, 15164,
    15548, 15932, 16316, 16700, 17084, 17468, 17852, 18236, 18620, 19004,
    19388, 19772,188, 572, 956,
};
const uint16_t HRTIM_DATA_SIZE = sizeof(hrtim_data)/sizeof(hrtim_data);

// ĐłŐńµçÁ÷ADÖµĘý×é (uint16_t)
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
    572,   // ¸ß¶ËżŞÍ¨µă
    9988 ,   // ¸ß¶ËąŘ¶Ďµă (ŐĽżŐ±Č= (300-100)/1000=20%)
    10556 ,   // µÍ¶ËżŞÍ¨µă
    19872,   // ÖÜĆÚ=1000
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
    572,   // ¸ß¶ËżŞÍ¨µă
    8920 ,   // ¸ß¶ËąŘ¶Ďµă (ŐĽżŐ±Č= (300-100)/1000=20%)
    8920+572 ,   // µÍ¶ËżŞÍ¨µă
    26110,   // ÖÜĆÚ=1000
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
    1572,   // ¸ß¶ËżŞÍ¨µă
    6527 ,   // ¸ß¶ËąŘ¶Ďµă (ŐĽżŐ±Č= (300-100)/1000=20%)
    6527+1572 ,   // µÍ¶ËżŞÍ¨µă
    6527*2,   // ÖÜĆÚ=1000
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
    572,   // ¸ß¶ËżŞÍ¨µă
    12602 ,   // ¸ß¶ËąŘ¶Ďµă (ŐĽżŐ±Č= (300-100)/1000=20%)
    12602+572 ,   // µÍ¶ËżŞÍ¨µă
    12602*2,   // ÖÜĆÚ=1000
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
    //     0,  // ÎŢµçŃąĘýľÝ
    //     100, 
    //     ppg_cycle
    // );



// 32Î»ŐűĘýĆ˝·˝¸ůŁ¨±ÜĂâ¸ˇµăŁ©
static uint32_t sqrt32(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;  // ×î´óÎ»

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

// ÓŕĎŇ˛é±íşŻĘýŁ¨ŐűĘýÔËËăŁ¬·µ»ŘÖµ·Ĺ´ó1000±¶Ł©
static int32_t cos_lookup(int32_t angle_degrees) {
    // ąéŇ»»Ż˝Ç¶Čµ˝0-359¶Č
    angle_degrees %= 360;
    if (angle_degrees < 0) angle_degrees += 360;
    
    // ŔűÓĂÓŕĎŇşŻĘýµÄ¶ÔłĆĐÔ
    bool is_neg = (angle_degrees > 90 && angle_degrees < 270);
    if (angle_degrees > 180) angle_degrees = 360 - angle_degrees;
    
    // 0-90¶ČÓŕĎŇ±íŁ¨Ăż5¶ČŇ»¸öµăŁ¬Öµ·Ĺ´ó1000±¶Ł©
    const int32_t cos_table []= {
        1000, 996, 985, 966, 940, 906, 866, 819, 766, 
        707, 643, 574, 500, 423, 342, 259, 174, 87, 0
    };
    
    // ĽĆËă×î˝Ó˝üµÄË÷Ňý
    uint8_t index = (angle_degrees * 2) / 10;  // Ăż5¶ČŇ»¸öµă
    if (index > 18) index = 18;
    
    return is_neg ? -cos_table[index] : cos_table[index];
}

// Ľě˛âµçÁ÷ąČµă
static bool isValley(const uint16_t* current, uint16_t i) {
    return (current[i-1] >current[i]) && (current[i+1] > current[i]);
}

// ÔÚÖ¸¶¨ÇřĽäÄÚŃ°ŐŇąýÁăµă
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


// ŃéÖ¤ąýÁăµăÓĐĐ§ĐÔ
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

// Ľě˛â·ĺÖµµçÁ÷
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

// ĽĆËăµçÁ÷ĎŕÎ»
static int32_t calculatePhaseAngle(uint16_t zero_cross_idx, uint16_t peak_idx) {
    if (zero_cross_idx >= peak_idx) return 0;
    
    // ĎŕÎ»ÖÍşóĽĆËăŁşĎŕÎ»˝Ç = (·ĺÖµĘ±Ľä - ąýÁăĘ±Ľä) * 18000 / °ëÖÜĆÚµăĘý
    // ĽŮÉč°ëÖÜĆÚ50¸ö˛ÉŃůµă¶ÔÓ¦180¶ČŁ¨¸ůľÝĘµĽĘµ÷ŐűłýĘýŁ©
    return ((peak_idx - zero_cross_idx) * 18000) / 50;  // ·µ»Ř¶Č*100
}

// ĹĐ¶ĎżŐÔŘĚŘĐÔ
static bool isNoLoadCondition(const uint16_t* current, uint16_t count, int32_t peak_current) {
    const uint16_t NO_LOAD_THRESHOLD = 50;  // żŐÔŘµçÁ÷ăĐÖµ
    
    // ĚőĽţ1Łş·ĺÖµµçÁ÷µÍÓÚăĐÖµ
    if (peak_current < NO_LOAD_THRESHOLD) return true;
    
    // ĚőĽţ2ŁşµçÁ÷˛¨¶Ż·¶Î§Đˇ
    uint16_t min = 0xFFFF, max = 0;
    for (uint16_t i = 0; i < count; i++) {
        if (current[i] < min) min = current[i];
        if (current[i] > max) max = current[i];
    }
    return (max - min) < (NO_LOAD_THRESHOLD >> 1);  // ÓŇŇĆ´úĚćłý2
}

// Ö÷ĽĆËăşŻĘý
#if 0
PowerResult CalculatePowerAi(
    const uint16_t* resonant_current,
    const uint16_t* hrtim_values,
    const uint16_t* voltage_data,
    uint16_t count,
    PPGpointDef ppg) 
{
    PowerResult result = {0};
    
    // ˛ÎĘý°˛Č«Ľě˛é
    if (!resonant_current || !hrtim_values || count == 0 || ppg.lowOff >= count) {
        return result;
    }
    
    // 1. ·ĺÖµµçÁ÷Ľě˛â
    PeakInfo peak = findPeakCurrent(resonant_current, ppg.highOn, ppg.highOff);
    result.peak_current = peak.value;
    
    // 2. żŐÔŘĚŘĐÔĹĐ¶Ď
    result.is_no_load = isNoLoadCondition(resonant_current, count, peak.value);
    
    // 3. »ńČˇ¸ßµÍ¶ËÇřĽä

    
    // 4. ąýÁăµăĽě˛â
    result.zero_cross_high = FindZeroCrossing(resonant_current, ppg.highOn, ppg.highOff);
    result.zero_cross_low = FindZeroCrossing(resonant_current, ppg.lowOn, ppg.lowOff);
    
    // 5. ·˝ĎňŃéÖ¤
    if (!ValidateZeroPoint(resonant_current, result.zero_cross_high, true)) {
        result.zero_cross_high = ppg.highOn + (ppg.highOff) * 3 / 4;
    }
    if (!ValidateZeroPoint(resonant_current, result.zero_cross_low, false)) {
        result.zero_cross_low =ppg.lowOn  +  (ppg.lowOff) * 3 / 4;
    }
    
    // 6. ĎŕÎ»ĽĆËă
    result.phase_angle = calculatePhaseAngle(result.zero_cross_high, peak.index);
    
    // 7. ą¦ÂĘĽĆËăŁ¨Č«ŐűĘýÔËËăŁ©
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
        // ĽĆËăRMSµçÁ÷
        uint32_t rms_current = sqrt32(sum_sq / valid_points);
        
        // »ńČˇRMSµçŃąŁ¨Ä¬ČĎ»ň˛âÁżÖµŁ©
        uint16_t rms_voltage = voltage_data ? voltage_data[peak.index] : 2200;  // Ä¬ČĎ220V*10
        
        // ĽĆËăÓĐą¦ą¦ÂĘŁşP = (I_rms * V_rms * cos¦Ő) / 1000
        int32_t cos_phi = cos_lookup(result.phase_angle / 100);  // ×Ş»»ÎŞĘµĽĘ˝Ç¶Č
        int64_t power = (int64_t)rms_current * rms_voltage * cos_phi;
        result.active_power = (int32_t)(power / 1000000);  // łýŇÔ1000*1000Ł¨cos_phi·Ĺ´ó1000±¶Ł©
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
//#define		hrtim_per_adc_1300nS		1040			//1.3us¶ÔÓ¦µÄHRTIMÖµ

//#define		hrtim_per_adc			hrtim_per_adc_500nS
//#define		HRTIM_BASE			1				//˝«1.3us˛ĺÖµÎŞ64·Ý
//#define		HRTIM_BASE_ADJ	hrtim_per_adc/HRTIM_BASE	

#define		HRTIM_BASE			

#include    <stdlib.h>
#include    "api_hrtim.h"

// ÔÚÖ¸¶¨ÇřĽäÄÚŃ°ŐŇąýÁăµă
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


    if((direction&0x3)==0x1)//Ç°Á˝´Î·˝ĎňŁ¬ąŐµăÎ»ÖĂ	
		{			
			uint16_t phasePoint=i-1;		//µĂµ˝ĎŕÎ»µă	


            uint32_t zeroRange=   current[phasePoint-1]+current[phasePoint+1];      //ąýÁăµăÇ°şó¶ţ¸öµăµÄ»ý×îĐˇŁ¬ÎŞąýÁăµă


            if(mivZ>zeroRange)    
            {
                mivZ=zeroRange;             //ŐŇµ˝Ň»¸ö×îĐˇµă
								zeroRange>>=1;			//Á˝¸öµăµÄĆ˝ľůÖµ

                if(current[phasePoint]<=zeroRange)     //ĹĐ¶Ď×îµÍµăÖµĘÇ·ńą»Đˇ)                
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
    uint16_t  dead=input->lowOn;        //ËŔÇřĘ±Ľä
    uint16_t  currentMax=0;
    uint16_t  currentMaxNum;
    uint16_t  currentType=pluseStart;            //µ±Ç°ÍłĽĆÇřĽä
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


        if(currentMax<newValue)
        {
            currentMax=newValue;
            currentMaxSum=currentSum;
            currentMaxNum=num;
        }

		
				
				

		uint16_t hrtim1us=	hrtim_values[num];
    int32_t lastPoint;
				
        switch(currentType)
        {
            case        pluseStart: 
                vcSum+=voltage_data[num];           //ÍłĽĆµçŃąĆ˝ľůÖµ
                vcCnt++;
				lastPoint=(int32_t)end-hrtim1us;	
		        if(lastPoint<hrtim_per_adc)
                {                               //ąŘ¶Ďµă
                    //  if(currentMaxSum<currentSum)
										if(1)
                    {                           //×î´óÖµÔÚąŘ¶ĎµăÇ°łöĎÖŁ¬ŇÔąŘ¶ĎµăÎŞ˝áĘř
                                                //ąŘ¶Ďµă˛ĺÖµĐŢŐý
                        if(lastPoint>0)   
												{
                            lastPoint*=newValue;		                //×îşóŇ»¸öµă˛»ÍęŐűŁ¬ĽÓÉĎĐŢŐýÖµ	
						    // lastPoint/=hrtim_per_adc;
                            currentSumPower=currentSum;
														currentSumPower*=hrtim_per_adc;
                            currentSumPower+= lastPoint;   
                            currentType= pluseLowOn;
                        }
                    }
                    else
                    {                            //×î´óÖµÔÚąŘ¶ĎµăÇ°Ă»łöĎÖŁ¬ĽĚĐřÍłĽĆµ˝¶ÔąÜżŞÍ¨Ł¨ËŔÇřĽĆČëµçÁ÷»ý·ÖŁ©

                        currentType= pluseHighOff;
                    }
  
               
                }
            break; 
            case    pluseHighOff :                //żŞĆôµăŁ¬ÓĂÓÚµČĐłŐńµçÁ÷×î´óÖµ
                if((dead-hrtim1us)<hrtim_per_adc)
                {

                    currentSumPower=currentMaxSum;
                    currentSumPower*=hrtim_per_adc;
                    currentType= pluseLowOn;     //˝áĘřŃ­»·
                }   
            break;    
        } 

        num++;
				if(num>input->end)
				{
						return  result;
				}
				

    }while(currentType<pluseLowOn);

    vcSum/=vcCnt;//µĂµ˝µçŃąĆ˝ľůÖµŁ¬ÓĐĐ§żŞÍ¨¶Î

    result.current=currentSumPower;
    result.voltage=vcSum;

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

    highOffNum+=potNumStart;            //ÉĎąÜąŘ¶ĎĐňşĹµă

//--step1   Ń°ŐŇąýÁăµăŁ¬ ÓĐŔúĘ·µăÓĂĐˇ·¶Î§Ł¬Ă»ÓĐÓĂČ«·¶Î§ 


			zeroStart=potNumStart;
			zeroEnd=highOffNum+2;

    uint16_t phaseUp= FindZeroCrossing(resonant_current, zeroStart, zeroEnd); 


        zeroStart=highOffNum;
        zeroEnd=input->end;



    uint16_t phaseDown= FindZeroCrossing(resonant_current, zeroStart, zeroEnd); 

    if(phaseUp&&phaseDown)
    {                                           //ąýÁăµăČ·ČĎşóĽĆËăÓĐĐ§ą¦ÂĘ
        CalculatorResultDef sumUp= CalculateAuctalCurrent(resonant_current,hrtim_values,voltage_values,phaseUp,input);       //ÍłĽĆÉĎąÜÓĐĐ§µçÁ÷
				
				if(sumUp.current==0)
				{
					return xReturn; 
				}	
			
				uint64_t	sumAll; 
				uint32_t    sumCurrent; 
				if((input->highOff*2+10)<input->lowOff)	
				{		//·Ç¶ÔłĆĘäłö
						CalculatorResultDef sumDown= CalculateAuctalCurrent(resonant_current,hrtim_values,voltage_values,phaseDown,input);    //ÍłĽĆĎÂąÜÓĐĐ§µçÁ÷
						sumAll= sumUp.current*sumUp.voltage+ sumDown.current*sumDown.voltage;
						sumCurrent=sumUp.current+sumDown.current;
				}
				else
				{		//¶ÔłĆĘäłö
						sumAll=sumUp.current*sumUp.voltage*2;
            sumCurrent=sumUp.current*2;
				}	
      
   
				sumAll/=input->lowOff;          //łËÉĎµçŃąµÄą¦ÂĘ

				sumCurrent/=input->lowOff;      //µçÁ÷Öµ

//Ë˛Ę±µçŃą			
				xReturn.voltage=			sumUp.voltage;
				
//Ë˛Ę±ą¦ÂĘ
				xReturn.active_power=sumAll>>4;

//Ë˛Ę±µçÁ÷
        xReturn.active_current=sumCurrent;
//µČĐ§µç×č
        uint32_t esrValue=sumUp.voltage/sumCurrent;         //µçŃąłýŇÔµçÁ÷
        xReturn.esr=   esrValue;         
#if 0 
//ÉĎąÜĎŕÎ»˝Ç
        uint32_t angle=hrtim_values[phaseUp]*180;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©
        angle/=input->highOff;
        xReturn.phase_angleUp=angle;
				
//ĎÂąÜĎŕÎ»˝Ç
				angle=hrtim_values[phaseDown]-input->highOff;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©
        angle*=180;
        angle/=(input->lowOff-input->highOff);
        xReturn.phase_angleDown=angle;
						
					
						
//				xReturn.zero_current=sumUp.current/hrtim_per_adc;
           
            if(phaseUp>potNumStart)
            {
//ÉĎąÜąýÁăµăÎ»ÖĂŁ¨Ęý×éÎ»şĹŁ©							
							
//                xReturn.zero_cross_high=phaseUp-potNumStart;            //Őâ¸öĘÇĎŕ¶ÔÓÚĆđĘĽµăµÄÎ»ÖĂ
	//							 xReturn.zero_cross_high=hrtim_values[phaseUp]/FRE_PER_ADC;
							
							xReturn.zero_cross_high=(hrtim_values[phaseUp]-input->highOn)/FRE_PER_ADC;
							
            }
            if(phaseDown>phaseUp)
            {
//ĎÂąÜąýÁăµăÎ»ÖĂŁ¨Ęý×éÎ»şĹŁ©								
          //      xReturn.zero_cross_low=phaseDown-potNumStart;    
								 xReturn.zero_cross_high=hrtim_values[phaseDown]/FRE_PER_ADC;
							
							
            }
#else
				if(hrtim_values[phaseUp]>input->highOn)
				{

                    uint32_t angle=hrtim_values[phaseUp]-input->highOn;
						
                    xReturn.zero_cross_high=angle/(FRE_PER_ADC/4);

                    angle*=180;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©
                    angle/=input->highOff-input->highOn;
                    xReturn.phase_angleUp=angle;


				}	
				if(hrtim_values[phaseDown]>input->lowOn)
				{
				    uint32_t    angle=hrtim_values[phaseDown]-input->lowOn;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©

					xReturn.zero_cross_low=angle/(FRE_PER_ADC/4);

                    angle*=180;
                    angle/=(input->lowOff-input->lowOn);
                    xReturn.phase_angleDown=angle;

				}	


    }
#endif
		
	return	xReturn;
}


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

	
				
				    // uint16_t		potCh=0;		//ÂŻÍ·şĹŁ¬//ÂŻÍ·ĐňşĹ

				// txaPoint.sumStart=resonant_current[potNum-2];
				// txaPoint.sumStart+=resonant_current[potNum-1];
				// txaPoint.sumStart+=resonant_current[potNum];

					uint32_t 		direction=0xffff;	
					uint8_t     	pluseStep=0;//0	
					uint16_t 		hrtim1us;
                    uint16_t        potNumStart=3;
					uint16_t		preValue,newValue;	
                	// memset(&txaPoint,0,sizeof(CalculatePowerDef));		//ÇĺÁă
#include	"commClass.h"                    
	            MemSetInt((uint32_t*)(&txaPoint),0,sizeof(CalculatePowerDef)/sizeof(int)); 

				preValue=resonant_current[potNumStart-1];

	for(uint16_t potNum=potNumStart;potNum<count;potNum++)        //Ç°ĂćÔ¤ÁôÁËČý¸öĘýľÝ
	{		
						
	
					newValue=resonant_current[potNum];	

//----------------step 1 Í»±äĐŢŐý--------------------------------------------	                   
#if   1 //Í»±äĐŢŐý	                    
					int16_t     delat=newValue-preValue;
					direction<<=1;	
					if(abs(delat)>300)
					{
							uint16_t backValue=resonant_current[potNum+1];
							uint32_t avgValue=(preValue+backValue)/2;
							newValue=avgValue;
					}
					preValue=newValue;			//¸üĐÂÉĎŇ»TxA_A´ÎµÄÖµ

                    // resonant_current[potNum]=newValue;

#endif   //Í»±äĐŢŐý	  


//----------------step 2 ŐŇµ˝ąýÁăµă--------------------------------------------	  
#if   1 //ŐŇµ˝ąýÁăµă			
						if(delat>10)
						{
							direction|=0x1;

						}
						else
						{
                            direction&=~0x1;
			
						}		


						if((direction&0x3f)==0x7)//000111,Ç°ËÄ´Î·˝ĎňŁ¬ąŐµăÎ»ÖĂ	
						{			
					        uint16_t phasePoint=potNum-3;		//µĂµ˝ĎŕÎ»µă	
                            uint32_t mivZ;
                            
                            mivZ=   resonant_current[phasePoint]+resonant_current[phasePoint-1]+resonant_current[phasePoint+1];
                            // if(mivZ<txaPoint.mivZero)    
                            // {
                            //     txaPoint.mivZero=mivZ; 
                            // }



                            if(resonant_current[phasePoint]<10)     //ĹĐ¶Ď×îµÍµăÖµĘÇ·ńą»Đˇ
                            {
                                uint16_t hrtim=hrtim_values[phasePoint];
                                if(pluseStep<pluseLowOn)
                                {//ÉĎąÜąýÁăµă
				


							        txaPoint.phaseUpHrtimN=hrtim-ppgValue->highOn;
							        // txaPoint.phaseUpHrtimP=ppgValue->highOff-hrtim;	//¸şĘýËµĂ÷ĎŕÎ»ÎŞ
							        // phasePoint-=potNumStart;
							        txaPoint.phaseUp=phasePoint;		//ĎŕÎ»ĆđĘĽµă

                                }
                                else
                                {//ĎÂąÜąýÁăµă
		
							        txaPoint.phaseDownHrtimN=hrtim-ppgValue->lowOn;
							        // txaPoint.phaseDownHrtimP=ppgValue->lowOff-hrtim;	//¸şĘýËµĂ÷ĎŕÎ»ÎŞ
							        // phasePoint-=potNumStart;
							        txaPoint.phaseDown=phasePoint;		//ĎŕÎ»ĆđĘĽµă
                                }
                            }    

						}

#endif  //ŐŇµ˝ąýÁăµă	



//----------------step 3 ¸ůľÝppg×´Ě¬ĹĐ¶ĎSTART--------------------------------------------	  


#if 1   //¸ůľÝppg×´Ě¬ĹĐ¶ĎSTART


						switch(pluseStep)	
						{
							case  	pluseStart:		//Ľě˛éżŞĘĽ,Ń°ŐŇÉĎÉýĆđµă//ÉĎąÜËŔÇř
								hrtim1us=	hrtim_values[potNum];
                         
								if(ppgValue->highOn-hrtim1us<hrtim_per_adc)
								{
									pluseStep=pluseHighOn;
								}

	
							
							case	pluseHighOn:

								
								    txaPoint.sumUp+=newValue;	//ĽĚĐřÍłĽĆµ˝ĎÂąÜżŞÍ¨  ÓĂÓÚCEIL QÖµĽĆËă

									hrtim1us=	hrtim_values[potNum];
									if(ppgValue->highOff-hrtim1us<hrtim_per_adc)
									{
                                        uint16_t  		lastPoint;
										lastPoint=ppgValue->highOff-hrtim1us;	
										lastPoint*=newValue;		                //×îşóŇ»¸öµă˛»ÍęŐűŁ¬ĽÓÉĎĐŢŐýÖµ	
										lastPoint/=HRTIM_BASE;
										txaPoint.lastAdjValueUp=lastPoint;

										// txaPoint.sumHighUp=txaPoint.sumUp;			//ÓĂÓÚą¦ÂĘĽĆËă,ŐâŔďSUMUPĂ»ÓĐĽÓNEW
										// txaPoint.upPoint=potNum-txaPoint.phaseUp;		//ÉĎąÜżŞÍ¨µÄĘ±Ľä
										pluseStep=pluseHighOff;
									}	

								break;

							case	pluseHighOff:	//ĎÂąÜËŔÇř 


									txaPoint.sumUp+=newValue;
									hrtim1us=hrtim_values[potNum];
									if(ppgValue->lowOn-hrtim1us<hrtim_per_adc)	
									{
										pluseStep=pluseLowOn;
									}

								
								break;	

							case	pluseLowOn:		//ĎÂ˝µ˝×¶ÎŁ¬Ň»Ö±µ˝ĎÂ˝µąŐµăŁ¬ÍłĽĆ¸ş°ëÖÜą¦ÂĘ
									
									txaPoint.sumDown+=newValue;
									hrtim1us=	hrtim_values[potNum];
									if(ppgValue->lowOff-hrtim1us<hrtim_per_adc)
									{
                                        uint16_t  		lastPoint;
										lastPoint=ppgValue->lowOff-hrtim1us;	
										lastPoint*=newValue;		                //×îşóŇ»¸öµă˛»ÍęŐűŁ¬ĽÓÉĎĐŢŐýÖµ	
										lastPoint/=HRTIM_BASE;
										txaPoint.lastAdjValueDown=lastPoint;

										// txaPoint.sumHighUp=txaPoint.sumDown;			//ÓĂÓÚą¦ÂĘĽĆËă,ŐâŔďSUMUPĂ»ÓĐĽÓNEW
										// txaPoint.upPoint=potNum-txaPoint.phaseUp;		//ÉĎąÜżŞÍ¨µÄĘ±Ľä
										pluseStep=pluseHighOff;
									}		

									
								break;
		
						}
#endif      //¸ůľÝppg×´Ě¬ĹĐ¶ĎEND


//----------------step 4 ĐłŐńµçÁ÷»ý·Östart--------------------------------------------	  

#if 1   //ĐłŐńµçÁ÷»ý·Östart --------------------------------------------------------------

                    if(pluseStep<pluseLowOn)
                    {
                        txaPoint.sumUp+=newValue;	//ĽĚĐřÍłĽĆµ˝ĎÂąÜżŞÍ¨  ÓĂÓÚCEIL QÖµĽĆËă       
					    if(txaPoint.phaseUp)		//ŇÔŐŇµ˝ąýÁăµăŁ¬´ÓąýÁăµăżŞĘĽŔŰĽÓ
					    {		
//µĂµ˝TXA×î´óÖµ
                            txaPoint.sumUpP+=newValue;

						    txaPoint.upAllValue+=newValue;
						    if(txaPoint.xReturn.peak_current<newValue)
						    {
							    txaPoint.xReturn.peak_current=newValue;       //×î´óĐłŐńµçÁ÷
							    txaPoint.TxaMaxNum=potNum;      //ĐłŐńµçÁ÷×î´óµă
							    txaPoint.upHalfValue=txaPoint.upAllValue;//×î´óĐłŐńµăÇ°µÄ»ý·Ö
						    }
					    }


                    }
                    else
                    {
                        txaPoint.sumDown+=newValue;	//ĽĚĐřÍłĽĆµ˝ĎÂąÜżŞÍ¨  ÓĂÓÚCEIL QÖµĽĆËă       
					    if(txaPoint.phaseDown)		//ŇÔŐŇµ˝ąýÁăµăŁ¬´ÓąýÁăµăżŞĘĽŔŰĽÓ
					    {		//¸şĎňµçÁ÷ÍłĽĆ
                            txaPoint.sumDownP+=newValue;
                        }    

                    }

#endif//ĐłŐńµçÁ÷»ý·Öend --------------------------------------------------------------





    }           

//----------------step 5 ĐłŐńµçÁ÷ËŔÇřĐŢŐý ĐłŐńµçÁ÷×î´óÖµłöĎÖÔÚąŘ¶ĎµăÇ°Ł¬ÓĂąŘ¶ĎĘ±µÄ»ý·ÖŁ¬łöĎÖÔÚąŘ¶Ďµăşó ÓĂ×î´óµăĘ±µÄ»ý·ÖÖµ--------------------------------------------	  
            uint32_t 	currentSum=txaPoint.sumUpP;	        //ÉĎąÜ˛ĺÖµĐŢŐý    

			// if(ppgValue->highOff*2+10>ppgValue->lowOff)
			// {
			// 	if(txaPoint.upHalfValue>currentSum)
			// 	{
			// 				currentSum=txaPoint.upHalfValue;        //ÔÚ×î´óµăÖ®Ç°ąŘ¶ĎŁ¬Čˇ×î´óµăÖ®Ç°µÄ»ý·ÖÖµŁ¨ĽÓÉĎÁËËŔÇřĘ±Ľä¶ÎŁ©
			// 	}

			// }

//----------------step 6 ĐłŐńµçÁ÷»ý·Ö˛ĺÖµĐŢŐý --------------------------------------------	  
 #if 1      //˛ĺÖµĐŢŐý     

                // uint32_t 	currentSum=txaPoint.sumUpP;	        //ÉĎąÜ˛ĺÖµĐŢŐý             
				currentSum*=HRTIM_BASE_ADJ;
    			currentSum+=txaPoint.lastAdjValueUp;					//ŐűĚĺŇÔHRTIMĘ±»ůÎŞµĄÎ»
				txaPoint.xReturn.active_power=currentSum;		//ĽÓÉĎĐŢŐýÖµ

                currentSum=txaPoint.sumDownP;	                //ĎÂąÜ˛ĺÖµĐŢŐý        
				currentSum*=HRTIM_BASE_ADJ;
                currentSum+=txaPoint.lastAdjValueDown;
                txaPoint.xReturn.active_power+=currentSum;
                txaPoint.xReturn.active_power/=ppgValue->lowOff/HRTIM_BASE;
#endif

//----------------step 7 ĎŕÎ»˝ÇĘäłö --------------------------------------------	  

 #if 1      //ĎŕÎ»˝ÇĘäłö     
			txaPoint.xReturn.phase_angle=txaPoint.phaseUpHrtimN*180;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©
			txaPoint.xReturn.phase_angle/=ppgValue->highOff;
#endif

            return txaPoint.xReturn;
}

 #if 0      //˛ĺÖµĐŢŐý 								
				//Čçąű˛»ĘÇ¶ÔłĆĘäłöŁ¬ŇŞĽÓÉĎĎŕÎ»ĐŢŐý
				//·Ç¶ÔłĆĎÂąÜĐŢŐý					
				uint16_t 	phaseNum=txaPoint.phaseDown;	
				int16_t 	phaseCount=txaPoint.phaseUp;

				if(phaseCount>1)
				{	

					if(txaPoint.ppgPoint.highOff*2+10<txaPoint.ppgPoint.lowOff)
					{
						if(txaPoint.phaseUp<phaseDownCount)		//µ÷ŐĽżŐ±ČÄŁĘ˝
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
				{//ĎŕÎ»±Ł»¤
				
				
				}	
 #endif     //˛ĺÖµĐŢŐý  

  #if 0      //ĎŕÎ»ĐŢŐý    
				currentSum/=txaPoint.ppgPoint.lowOff/HRTIM_BASE;

				if(txaPoint.phaseDownValue>0)		//ÉĎĎÂąŐµă×ĽČ·Ę¶±đ
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



				



		 		AdcFromApiDma20ms.ceilQ[potCh][count]=downPhaseValue*100/(sumUpValue);	//·ĺÖµÓë»ý·ÖµÄ±ČÖµÎŞQ
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
					AdcFromApiDma20ms.ceilQ[potCh][count]=downPhaseValue*100/(sumUpValue);	//·ĺÖµÓë»ý·ÖµÄ±ČÖµÎŞQ

#endif		//txaHalf

				}
				else
				{
					AdcFromApiDma20ms.ceilQ[potCh][count]=0x10;
				}

  #endif      //ĎŕÎ»ĐŢŐý    


			// int32_t  	phaseDegree=txaPoint.phaseUpHrtim*180;		//ĎŕÎ»˝ÇŁ¨180¶ČÎŞµĄÎ»Ł©
			// phaseDegree/=ppgValue->highOff;
				



#if 0       //Ęäłöµ÷ĘÔĐĹĎ˘

//------------------				
//Ęäłöµ÷ĘÔĐĹĎ˘------------	

		AdcFromApiDma20ms.Txa[potCh][count]	=currentSum;
		AdcFromApiDma20ms.phase[potCh][count]=phaseDegree;//txaPoint.phaseUpHrtim;	//Őâ¸ö˛»ŇŞ±ä
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
			
				adcSize=(hrtimPointOver-hrtimPointStart);		//4¸öŇ»×é

				adcSum[0]=AdcFromApiDma20ms.ceilQ[PotChWork][count];						//ĎŕÎ»Öµ
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
			
#endif       //Ęäłöµ÷ĘÔĐĹĎ˘				
				

#endif
