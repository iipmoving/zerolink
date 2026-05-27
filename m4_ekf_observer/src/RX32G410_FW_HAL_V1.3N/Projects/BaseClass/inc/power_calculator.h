#ifndef POWER_CALCULATOR_H
#define POWER_CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>

// ¹¤×÷ÖÜÆÚ¶¨Òå
typedef struct {
    uint16_t start;         //HRTIM¿ªÊ¼µã
    uint16_t end;            //HRTIM½áÊøµã   

	
    uint16_t highOn;		    //ËÀÇøºó¸ß¶Ë¿ªÍ¨HRTIMÖµ
	uint16_t highOff;			//¸ß¶Ë¹Ø±Õ£¬Ò²¾ÍÊÇDUTY PPGÕ¼¿Õ±È 
	
    uint16_t lowOn;		        //ËÀÇøºóµÍ¶Ë¿ªÍ¨
	uint16_t lowOff;			//µÍ¶Ë¹Ø±Õ£¬Ò²¾ÍÊÇprioed	PPGÖÜÆÚ
   
    uint16_t zero_cross_high; // ¸ß¶Ë¹ıÁãµãË÷Òı
    uint16_t zero_cross_low;  // µÍ¶Ë¹ıÁãµãË÷Òı
	
		uint16_t perAdc;				//Ã¿¸öADC¶ÔÓ¦µÄHRTIMÖµ
		uint16_t lagDuty;  // æ»åè‡‚å ç©ºæ¯”CMPå€¼ï¼ˆå…¨æ¡¥ï¼‰
}PowerCalculatorInputDef;	        //Ğ³ÕñµçÁ÷¼ÆËãÊäÈë²ÎÊı

// ·åÖµµçÁ÷ĞÅÏ¢
typedef struct {
    uint16_t index;   // ·åÖµË÷Òı
    uint16_t value;   // ·åÖµµçÁ÷Öµ
}PeakInfo;


typedef struct {
    uint32_t voltage;   // ·åÖµË÷Òı
    uint32_t current;   // ·åÖµµçÁ÷Öµ
}CalculatorResultDef;



// ¹¦ÂÊ¼ÆËã½á¹û
typedef struct {
    int32_t active_power;     // ÓĞ¹¦¹¦ÂÊ
    int32_t active_current;   // ÎŞ¹¦¹¦ÂÊ

		uint16_t peak_current;    	// ·åÖµµçÁ÷
    uint16_t zero_current;    	// ¹ıÁãµãµçÁ÷Öµ   
	
		int16_t phase_angleUp;      // ÏàÎ»½Ç£¨¶È*100£©
    int16_t phase_angleDown;

    uint16_t zero_cross_high; // ¸ß¶Ë¹ıÁãµãË÷Òı
    uint16_t zero_cross_low;  // µÍ¶Ë¹ıÁãµãË÷Òı

    uint16_t esr;               //µÈĞ§µç×è
    uint16_t voltage; 

}PowerResult;

/**
 * @brief ¼ÆËã¹¦ÂÊ¼°Ïà¹Ø²ÎÊı
 * @param resonant_current Ğ³ÕñµçÁ÷Êı×é
 * @param hrtim_values HRTIMÊ±¼ä´ÁÊı×é
 * @param voltage_data µçÑ¹Êı¾İ£¨¿ÉÑ¡£©
 * @param count Êı¾İµãÊıÁ¿
 * @param ppg ¹¤×÷ÖÜÆÚ¶¨Òå
 * @return PowerResult ¼ÆËã½á¹û½á¹¹Ìå
 */
PowerResult CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_data,
    PowerCalculatorInputDef* input
);
int16_t * Power_Calculator_GetVoltageBuffAddress(uint8_t ch);		//µÃµ½µçÑ¹µ÷ÊÔÊı¾İ
int16_t * Power_Calculator_GetHrtimBuffAddress(uint8_t ch);       //µÃµ½µ÷ÊÔHRTIMµØÖ·
int16_t * Power_Calculator_GetTxaBuffAddress(uint8_t ch);       //µÃµ½µ÷ÊÔÊı¾İµØÖ·
int16_t Power_Calculator_GetTxaBuffSize(uint8_t ch);			//µÃµ½µ÷ÊÔÊı¾İ´óĞ¡
PowerCalculatorInputDef*	Power_Calculator_GetInputArrayAddress(uint8_t ch);


#define     ZERO_WINDOW_MIN     8           //¹ıÁã×îĞ¡¼ì²â´°¿Ú£¬+- 2* ZERO_WINDOW_MIN+1 ¸öµã

#define testCh	0

#endif // POWER_CALCULATOR_H


