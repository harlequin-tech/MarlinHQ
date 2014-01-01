#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

/*
 * Axis movement menu
 */

static void mma_Show(uint8_t line, const char *value)
{
    lcd.setCursor(11,line);
    lcd.print(value);
}

static void mma_ShowAxis(uint8_t line, uint8_t axis) 
{
    mma_Show(line, ftostr52(current_position[axis]));
}

static void mma_ClickAxis(uint8_t line, long &pos, bool &adjustValue, uint8_t axis)
{
    adjustValue = !adjustValue;
    if(adjustValue) {
	enquecommand("G91");
	pos = 0;
    } else {
      enquecommand("G90");
    }
}

#define MMA_EXTRUDE 1
#define MMA_RETRACT 2

static void mma_ClickExtruder(uint8_t line, long &pos, bool &adjustValue, uint8_t direction)
{
    if (direction == MMA_EXTRUDE) {
	enquecommand("G92 E0");
	enquecommand("G1 F70 E1");
	beepshort();
    } else {
	enquecommand("G92 E0");
	enquecommand("G1 F700 E-1");
	beepshort();
    }
}

void mma_AdjustAxis(uint8_t line, long &pos, uint8_t axis)
{
    if (pos > 0) {
	switch (axis) {
	case X_AXIS: enquecommand("G1 F700 X0.1"); break;
	case Y_AXIS: enquecommand("G1 F700 Y0.1"); break;
	case Z_AXIS: enquecommand("G1 F70 Z0.1"); break;
	default: break;
	}
    } else if (pos < 0) {
	switch (axis) {
	case X_AXIS: enquecommand("G1 F700 X-0.1"); break;
	case Y_AXIS: enquecommand("G1 F700 Y-0.1"); break;
	case Z_AXIS: enquecommand("G1 F70 Z-0.1"); break;
	default: break;
	}
    }

    mma_ShowAxis(line, axis);
}

static const menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_PREPARE_ALT, NULL,	   mct_ClickMenu,   NULL,		Main_Prepare },
    { " X:",       mma_ShowAxis,   mma_ClickAxis,   mma_AdjustAxis,	X_AXIS },
    { " Y:",       mma_ShowAxis,   mma_ClickAxis,   mma_AdjustAxis,	Y_AXIS },
    { " Z:",       mma_ShowAxis,   mma_ClickAxis,   mma_AdjustAxis,	Z_AXIS },
    { MSG_EXTRUDE, mma_ShowAxis,   mma_ClickExtruder, NULL,	        MMA_EXTRUDE },
    { MSG_RETRACT, mma_ShowAxis,   mma_ClickExtruder, NULL,	        MMA_RETRACT },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showAxisMove()
{
    show(menu, MENU_MAX);
}

#endif
