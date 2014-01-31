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

#ifdef FWRETRACT
extern bool autoretract_enabled;
extern bool retracted;
extern float retract_length;
extern float retract_feedrate;
extern float retract_zlift;
extern float retract_recover_length;
extern float retract_recover_feedrate;

#define MR_LENGTH		0
#define MR_FEEDRATE		1
#define MR_ZLIFT		2
#define MR_RECOVER_LENGTH	3
#define MR_RECOVER_FEEDRATE	4

static struct {
    float *parm;
    float scale;
    int16_t min;
    int32_t max;
} fparm[] = {
    { &retract_length,		 100,  1, 990, },
    { &retract_feedrate,	 0.2,  1, 990, },
    { &retract_zlift,		 10,   0, 990, },
    { &retract_recover_length,	 100,  0, 990, },
    { &retract_recover_feedrate, 0.2,  1, 990, },
};


static void mr_ShowAuto(uint8_t line, uint8_t arg) 
{
    lcd.setCursor(13,line);
    if (autoretract_enabled) {
	lcdprintPGM(MSG_ON);
    } else {
	lcdprintPGM(MSG_OFF);
    }
}

static void mr_ClickAuto(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t arg)
{
    autoretract_enabled =! autoretract_enabled;
    mr_ShowAuto(line, arg);
}

static void mr_ShowFloat(uint8_t line, uint8_t which) 
{
    if ((which == MR_FEEDRATE) || (which == MR_RECOVER_FEEDRATE)) {
	mct_Show(line, itostr4(*fparm[which].parm));
    } else {
	mct_Show(line, ftostr52(*fparm[which].parm));
    }
}

static void mr_ClickFloat(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if(adjustValue) {
	pos = *fparm[which].parm * fparm[which].scale;
    } else {
	*fparm[which].parm = pos / fparm[which].scale;
    }
}

void mr_AdjustFloat(uint8_t line, volatile long &pos, uint8_t which)
{
    limitEncoder(pos, fparm[which].min, fparm[which].max);

    lcd.setCursor(13,line);
    if ((which == MR_FEEDRATE) || (which == MR_RECOVER_FEEDRATE)) {
	mct_Show(line, itostr4(pos / fparm[which].scale));
    } else {
	mct_Show(line, ftostr52(pos / fparm[which].scale));
    }
}

static const menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_CONTROL,                  NULL,	    mct_ClickMenu,   NULL,		Main_Control },
    { MSG_AUTORETRACT,              mr_ShowAuto,    mr_ClickAuto,    NULL,	        0 },
    { MSG_CONTROL_RETRACT,          mr_ShowFloat,   mr_ClickFloat,   mr_AdjustFloat,	MR_LENGTH },
    { MSG_CONTROL_RETRACTF,         mr_ShowFloat,   mr_ClickFloat,   mr_AdjustFloat,	MR_FEEDRATE },
    { MSG_CONTROL_RETRACT_ZLIFT,    mr_ShowFloat,   mr_ClickFloat,   mr_AdjustFloat,	MR_ZLIFT },
    { MSG_CONTROL_RETRACT_RECOVER,  mr_ShowFloat,   mr_ClickFloat,   mr_AdjustFloat,	MR_RECOVER_LENGTH },
    { MSG_CONTROL_RETRACT_RECOVERF, mr_ShowFloat,   mr_ClickFloat,   mr_AdjustFloat,	MR_RECOVER_FEEDRATE },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showControlRetract()
{
    show(menu, MENU_MAX);
}
#else
void MainMenu::showControlRetract()
{
}
#endif

#endif
