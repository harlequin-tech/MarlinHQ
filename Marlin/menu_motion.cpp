#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

extern long position[4];   

/*
 * Motion menu
 */


#define MOTION_ACCELERATION	0
#define MOTION_XYJERK		1
#define MOTION_VMAXX		2
#define MOTION_VMAXY		3
#define MOTION_VMAXZ		4
#define MOTION_VMAXE		5
#define MOTION_VMINFEED		6
#define MOTION_VMINTRAVEL	7
#define MOTION_ARETRACT		8
#define MOTION_XSTEPS		9
#define MOTION_YSTEPS		10
#define MOTION_ZSTEPS		11
#define MOTION_ESTEPS		12

static struct {
    float *parm;
    float scale;
    int16_t min;
    int32_t max;
} fparm[] = {
    { &acceleration,		    0.01,  5, 990, },
    { &max_xy_jerk,		    1,	   1, 990, },
    { &max_feedrate[X_AXIS],	    1,	   1, 990, },
    { &max_feedrate[Y_AXIS],	    1,	   1, 990, },
    { &max_feedrate[Z_AXIS],	    1,	   1, 990, },
    { &max_feedrate[E_AXIS],	    1,	   1, 990, },
    { &minimumfeedrate,		    1,	   0, 990, },
    { &mintravelfeedrate,	    1,	   1, 990, },
    { &retract_acceleration,	    0.01, 10, 990, },
    { &axis_steps_per_unit[X_AXIS], 100,   5, 999999, },
    { &axis_steps_per_unit[Y_AXIS], 100,   5, 999999, },
    { &axis_steps_per_unit[Z_AXIS], 100,   5, 999999, },
    { &axis_steps_per_unit[E_AXIS], 100,   5, 999999, },
};

static void mm_ShowFloat(uint8_t line, uint8_t which) 
{
    if (which < MOTION_XSTEPS) {
	lcd.setCursor(13,line);
	if (fparm[which].scale < 1) {
	    lcd.print(itostr3(*fparm[which].parm/100));
	    lcdprintPGM("00");
	} else {
	    lcd.print(itostr3(*fparm[which].parm));
	}
    } else {
	lcd.setCursor(11,line);
	lcd.print(ftostr52(*fparm[which].parm));
    }
}

static void mm_ShowAccel(uint8_t line, uint8_t which)
{
    lcd.setCursor(13,line);
    lcd.print(itostr3(max_acceleration_units_per_sq_second[which]/100));
    lcdprintPGM("00");
}

static void mm_ClickFloat(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if(adjustValue) {
	pos = *fparm[which].parm * fparm[which].scale;
    } else {
	if ((which >= MOTION_XSTEPS) && (which <= MOTION_ESTEPS)) {
            float factor = float(pos) / 100.0 / *fparm[which].parm;
            position[which - MOTION_XSTEPS] = lround(position[which - MOTION_XSTEPS]*factor);
	}
	*fparm[which].parm = pos / fparm[which].scale;
    }
}

static void mm_ClickAccel(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if(adjustValue) {
	pos = max_acceleration_units_per_sq_second[X_AXIS] / 100;
    } else {
	max_acceleration_units_per_sq_second[X_AXIS] = pos * 100;
    }
}

void mm_AdjustFloat(uint8_t line, long &pos, uint8_t which)
{
    limitEncoder(pos, fparm[which].min, fparm[which].max);

    if (which >= MOTION_XSTEPS && (which <= MOTION_ESTEPS)) {
	lcd.setCursor(11, line);
	lcd.print(ftostr52(pos/100.0));
    } else {
	lcd.setCursor(13,line);
	lcd.print(itostr3(pos));
	if (fparm[which].scale < 1) {
	    lcdprintPGM("00");
	}
    }
}

void mm_AdjustAccel(uint8_t line, long &pos, uint8_t axis)
{
    limitEncoder(pos, 1, 990);
    mct_Show(line, itostr3(pos));
    lcdprintPGM("00");
}

static const menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_CONTROL,   NULL,	   mct_ClickMenu,   NULL,		Main_Control },
    { MSG_ACC,       mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_ACCELERATION },
    { MSG_VXY_JERK,  mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_XYJERK },
    { MSG_X,         mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMAXX },
    { MSG_Y,         mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMAXY },
    { MSG_Z,         mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMAXZ },
    { MSG_E,         mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMAXE },
    { MSG_VMIN,      mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMINFEED },
    { MSG_VTRAV_MIN, mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_VMINTRAVEL },

    { MSG_X,         mm_ShowAccel,   mm_ClickAccel,   mm_AdjustAccel,	X_AXIS },
    { MSG_Y,         mm_ShowAccel,   mm_ClickAccel,   mm_AdjustAccel,	Y_AXIS },
    { MSG_Z,         mm_ShowAccel,   mm_ClickAccel,   mm_AdjustAccel,	Z_AXIS },
    { MSG_E,         mm_ShowAccel,   mm_ClickAccel,   mm_AdjustAccel,	E_AXIS },

    { MSG_A_RETRACT, mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_ARETRACT },
    { MSG_XSTEPS,    mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_XSTEPS },
    { MSG_YSTEPS,    mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_YSTEPS },
    { MSG_ZSTEPS,    mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_ZSTEPS },
    { MSG_ESTEPS,    mm_ShowFloat,   mm_ClickFloat,   mm_AdjustFloat,	MOTION_ESTEPS },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showControlMotion()
{
    show(menu, MENU_MAX);
}

#endif
