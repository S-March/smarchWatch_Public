//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
// PLL LUT utilities
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

#ifndef _PLLVCOCAL_LUT_H_
#define _PLLVCOCAL_LUT_H_
#if LUT_PATCH_ENABLED

// Can be placed in the c file
#define GET_MAXCALCAP()  		(calcap_minmax & 0x000F)																	//Can be used in 16 bit operations			
#define GET_MINCALCAP()  		((calcap_minmax >> 4) & 0x000F)														//Can be used in 16 bit operations
#define SET_MAXCALCAP(cc)  	( (calcap_minmax)=((calcap_minmax & 0xF0) + (cc)) )				//cc is expected to be in the range [0,15]
#define SET_MINCALCAP(cc)  	( (calcap_minmax)=((calcap_minmax & 0x0F) + ((cc)<<4)) )	//cc is expected to be in the range [0,15]

#define VCOCAL_LUT_SIZE	        40    // Should be fixed to 40. Provided here for debugging
#define VCOCAL_LUT_SIZE_BYTES	20    // Should be fixed to 20. Provided here for debugging
#define NUM_CALCAPS_FOR_LUT 	8     // max number of calcaps under consideration (In the rf_cal_cap LUT we expect to see a max of 4 different calcaps)
#define HW_LUT_MAXNUM_CALCAPS   4     // max number of different calcaps supported by the HW LUT

#define CHAN_ZERO_FREQ         2402   // Freq in MHz of Channel 0
#define CHAN_LAST_FREQ         2480   // Freq in MHz of Channel 39 in BLE 
#define CHAN_WIDTH             2     // Channel Width in MHz

#define LUT_SAVE_CFG 	1			
#define LUT_RESTORE 	2			




// Visible to the user
#define LUT_INIT 		0			
#define LUT_UPDATE 	    1		

#define VCOCAL_SUCCESS  0x00
#define VCOCAL_ERR_1    0x01
#define VCOCAL_ERR_2    0x02
#define VCOCAL_ERR_3    0x04
#define VCOCAL_ERR_4    0x08
#define VCOCAL_ERR_5    0x10
#define VCOCAL_ERR_6    0x20
#define VCOCAL_ERR_7    0x40
#define VCOCAL_ERR_8    0x80


struct LUT_CFG_struct
{
	int8_t 	    HW_LUT_MODE	; 							    // 1: HW LUT mode, 0: SW LUT mode using ISR
	int8_t 	    RX_HSI_ENABLED	; 					    // 1: HSI mode, 0:LSI mode
	int8_t 	    PA_PULLING_OFFSET	; 				    // in channel numbers
	int8_t 	    NR_CCUPD_1ST	; 	              // max number of real time iterations during the first min_cn calcap update routine
	int8_t 	    NR_CCUPD_REST	;                 // max number of real time iterations during the rest min_cn / max_cn updates
	int8_t 	    NR_CCUPD_OL	;                   // Extra number of iterations during the LUT INIT (non real time mode). Should be less than 126 in total
	int8_t 	    BLE_BAND_MARGIN	; 					    // Extention of BLE band in MHz, to make sure that we keep updating enough number of calcaps
	int8_t 	    EST_HALF_OVERLAP	; 				    // Estimated half overlap between successive calcaps, in MHz
	int8_t 	    REQUIRED_CHAN_OVERLAP	; 				// Min required overlap in channels
	uint8_t 	PLL_LOCK_VTUNE_NUMAVGPOW	; 				
	int16_t 	PLL_LOCK_VTUNE_LIMIT_LO	; 				
	int16_t 	PLL_LOCK_VTUNE_LIMIT_HI	; 				
	int16_t 	PLL_LOCK_VTUNE_P2PVAR	; 				
	int16_t 	PLL_LOCK_TIMING	; 				
	uint16_t 	VCO_CALCNT_STARTVAL	; 				
	uint16_t 	VCO_CALCNT_TIMEOUT	; 				
};


struct calcap_range
{
	int8_t 	min_cn	; // signed as min_cn might have to store channels below 0.
	int8_t 	max_cn	; // signed as max_cn might have to store channels below 0.
};

extern volatile struct calcap_range vco_calcap_ranges[NUM_CALCAPS_FOR_LUT];		// In Retention memory!
extern volatile uint8_t calcap_minmax;																				// In Retention memory! (MSNibble holds min calcap, LSNibble holds max calcap)
extern volatile uint8_t rf_cal_cap[VCOCAL_LUT_SIZE_BYTES];																				// In Retention memory!
extern volatile uint16_t vcocal_ctrl_reg_val;

uint8_t  pll_vcocal_LUT_InitUpdate(uint8_t mode);
void set_rf_cal_cap(uint16_t cn); // This is used for overrule based testing

#endif //LUT_PATCH_ENABLED
#endif //_PLLVCOCAL_LUT_H_
