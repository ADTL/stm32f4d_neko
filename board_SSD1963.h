/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/SSD1963/board_SSD1963_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the SSD1963 display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/* Using FSMC A16 (PD11) as DC */
#define GDISP_REG       (*((volatile uint8_t *) 0x60000000)) /* DC = 0 */
#define GDISP_RAM       (*((volatile uint8_t *) 0x60020000)) /* DC = 1 */

#define SET_RST         palSetPad(GPIOA, 1);
#define CLR_RST         palClearPad(GPIOA, 1);

/**
 * @brief	LCD panel specs
 *
 * @note	The timings need to follow the datasheet for your particular TFT/LCD screen
 * 			(the actual screen, not the controller).
 * @note	Datasheets normally use a specific set of timings and acronyms, their value refers
 * 			to the number of pixel clocks. Non-display periods refer to pulses/timings that occur
 * 			before or after the timings that actually put pixels on the screen. Display periods
 * 			refer to pulses/timings that directly put pixels on the screen.
 * @note	HDP: Horizontal Display Period, normally the width - 1<br>
 * 			HT: Horizontal Total period (display + non-display)<br>
 * 			HPS: non-display period between the start of the horizontal sync (LLINE) signal
 * 				and the first display data<br>
 * 			LPS: horizontal sync pulse (LLINE) start location in pixel clocks<br>
 * 			HPW: Horizontal sync Pulse Width<br>
 * 			VDP: Vertical Display period, normally height - 1<br>
 * 			VT: Vertical Total period (display + non-display)<br>
 * 			VPS: non-display period in lines between the start of the frame and the first display
 * 				data in number of lines<br>
 * 			FPS: vertical sync pulse (LFRAME) start location in lines.<br>
 * 			VPW: Vertical sync Pulse Width
 * @note	Here's how to convert them:<br>
 * 				SCREEN_HSYNC_FRONT_PORCH = ( HT - HPS ) - GDISP_SCREEN_WIDTH<br>
 * 				SCREEN_HSYNC_PULSE = HPW<br>
 * 				SCREEN_HSYNC_BACK_PORCH = HPS - HPW<br>
 * 				SCREEN_VSYNC_FRONT_PORCH = ( VT - VPS ) - GDISP_SCREEN_HEIGHT<br>
 * 				SCREEN_VSYNC_PULSE = VPW<br>
 * 				SCREEN_VSYNC_BACK_PORCH = VPS - LPS<br>
 */

static const LCD_Parameters	DisplayTimings[] = {
	// You need one of these array elements per display
	{
		320, 240,								// Panel width and height
		0, 0, 68,								// Horizontal Timings (back porch, front porch, pulse)
		440,				// Total Horizontal Period (calculated from above line)
		0, 0, 18,								// Vertical Timings (back porch, front porch, pulse)
		265,				// Total Vertical Period (calculated from above line)
		CALC_FPR(320,240,0,0,18,0,0,68,60ULL)	// FPR - the 60ULL is the frames per second. Note the ULL!
	},
};

/**
 * @brief   Initialise the board for the display.
 *
 * @param[in] g			The GDisplay structure
 *
 * @note	Set the g->board member to whatever is appropriate. For multiple
 * 			displays this might be a pointer to the appropriate register set.
 *
 * @notapi
 */
static inline void init_board(GDisplay *g) {
	(void) g;
        unsigned char FSMC_Bank;

        /* STM32F4 FSMC init */
        rccEnableAHB3(RCC_AHB3ENR_FSMCEN, 0);

        /* Group pins */
        IOBus busD = {GPIOD, (1<<0) | (1<<1) | (1<<4) | (1<<5) | (1<<7) |
                      (1<<11) | (1<<14) | (1<<15), 0};

        IOBus busE = {GPIOE, (1<<7) | (1<<8) | (1<<9) | (1<<10), 0};

        /* FSMC is an alternate function 12 (AF12) */
        palSetBusMode(&busD, PAL_MODE_ALTERNATE(12));
        palSetBusMode(&busE, PAL_MODE_ALTERNATE(12));

        FSMC_Bank = 0;

        /* FSMC timing */
        FSMC_Bank1->BTCR[FSMC_Bank+1] = (FSMC_BTR1_ADDSET_1 | FSMC_BTR1_ADDSET_3) \
                        | (FSMC_BTR1_DATAST_1 | FSMC_BTR1_DATAST_3) \
                        | (FSMC_BTR1_BUSTURN_1 | FSMC_BTR1_BUSTURN_3) ;

        /* Bank1 NOR/SRAM control register configuration
         * This is actually not needed as already set by default after reset */
        FSMC_Bank1->BTCR[FSMC_Bank] = FSMC_BCR1_MWID_0 | FSMC_BCR1_WREN | FSMC_BCR1_MBKEN;
        /* Reset pin initialisation */
        palSetPadMode(GPIOA, 1, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); 
}

/**
 * @brief   After the initialisation.
 *
 * @param[in] g			The GDisplay structure
 *
 * @notapi
 */
static inline void post_init_board(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Set or clear the lcd reset pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 * 
 * @notapi
 */
static inline void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	if (state) {
                CLR_RST;
        } else {
                SET_RST;
        }
}

/**
 * @brief   Take exclusive control of the bus
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void acquire_bus(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Release exclusive control of the bus
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void release_bus(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Send data to the index register.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] index			The index register to set
 *
 * @notapi
 */
static inline void write_index(GDisplay *g, uint8_t index) {
	(void) g;
	GDISP_REG = index;
}

/**
 * @brief   Send data to the lcd.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] data			The data to send
 * 
 * @notapi
 */
static inline void write_data(GDisplay *g, uint8_t data) {
	(void) g;
	GDISP_RAM = data;
}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
