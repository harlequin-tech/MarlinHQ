#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

extern volatile int feedmultiply;
extern volatile bool feedmultiplychanged;
extern volatile int extrudemultiply;
extern long encoderpos;
extern long position[4];   

/*
 * Axis movement menu
 */

#define TUNE_SPEED 0
#define EXTRUDER_1	0
#define EXTRUDER_2	1
#define EXTRUDER_3	2

static void mt_Show(uint8_t line, const char *value)
{
    lcd.setCursor(13,line);
    lcd.print(value);
}

static void mt_ShowSpeed(uint8_t line, uint8_t arg) 
{
    mt_Show(line, ftostr3(feedmultiply));
}

static void mt_ClickSpeed(uint8_t line, long &pos, bool &adjustValue, uint8_t arg)
{
    adjustValue = !adjustValue;
    if(adjustValue) {
	pos = feedmultiply;
    }
}

void mt_AdjustSpeed(uint8_t line, long &pos, uint8_t arg)
{
    limitEncoder(pos, 1, 400);
    feedmultiply = encoderpos;
    feedmultiplychanged = true;
    mt_Show(line, itostr3(encoderpos));
}

static void mt_ShowFlow(uint8_t line, uint8_t arg) 
{
    mt_Show(line, ftostr3(feedmultiply));
}

static void mt_ClickFlow(uint8_t line, long &pos, bool &adjustValue, uint8_t arg)
{
    adjustValue =! adjustValue;
    if (adjustValue) {
	pos = axis_steps_per_unit[E_AXIS]*100.0;
    } else {
	float factor = float(encoderpos)/100.0/float(axis_steps_per_unit[E_AXIS]);
	position[E_AXIS] = lround(position[E_AXIS]*factor);
	axis_steps_per_unit[E_AXIS] = pos/100.0;
    }
}

void mt_AdjustFlow(uint8_t line, long &pos, uint8_t arg)
{
    limitEncoder(pos, 5, 999999);
    mt_Show(line, ftostr52(encoderpos/100.0));
}

static menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_MAIN,    NULL,	   mct_ClickMenu,   NULL,		Main_Menu },
    { MSG_SPEED,   mt_ShowSpeed,   mt_ClickSpeed,   mt_AdjustSpeed,	0 },
    { MSG_FLOW,    mt_ShowFlow,    mt_ClickFlow,    mt_AdjustFlow,      0 },
    { MSG_NOZZLE,  mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_1 },
#if EXTRUDERS > 1
    { MSG_NOZZLE1, mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_2 },
#endif
#if EXTRUDERS > 2
    { MSG_NOZZLE2, mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_3 },
#endif
#if defined BED_USES_THERMISTOR || defined BED_USES_AD595
    { MSG_BED,      mct_ShowBed,   mct_ClickBed,    mct_AdjustTemp,      0 },
#endif
    { MSG_FAN_SPEED,mct_ShowFan,   mct_ClickFan,    mct_AdjustFan,       0 },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showTune()
{
    show(menu, MENU_MAX);
}

#endif
