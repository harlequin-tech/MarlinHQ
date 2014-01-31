#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

#define EXTRUDER_1	0
#define EXTRUDER_2	1
#define EXTRUDER_3	2
#define AUTOTEMP_MIN	0
#define AUTOTEMP_MAX	1
#define AUTOTEMP_FACTOR	2
#define PID_P		0
#define PID_I		1
#define PID_D		2
#define PID_C		3

/*
 * Temperature control menu
 */

void mct_Show(uint8_t line, const char *value)
{
    lcd.setCursor(13,line);
    lcd.print(value);
}

void mct_ClickMenu(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t newMenu)
{
    mainMenu.changeMenu((MainStatus)newMenu);
    beepshort();
}

void mct_AdjustTemp(uint8_t line, volatile long &pos, uint8_t arg)
{
    limitEncoder(pos, 0, 260);
    mct_Show(line, itostr3(pos));
}

void mct_ShowNozzle(uint8_t line, uint8_t extruder)
{
    mct_Show(line, itostr3(intround(degTargetHotend(extruder))));
}

void mct_ClickNozzle(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t extruder)
{
    adjustValue = !adjustValue;
    if (adjustValue) {
	pos = intround(degTargetHotend(extruder));
    } else {
	setTargetHotend(pos, extruder);
    }
}

#ifdef AUTOTEMP
static void mct_AdjustFactor(uint8_t line, volatile long &pos, uint8_t arg)
{
    limitEncoder(pos, 0, 99);
    mct_Show(line, itostr3(pos));
}

static void mct_ShowAutotemp(uint8_t line, uint8_t which)
{
    switch (which) {
    case AUTOTEMP_MIN:
	mct_Show(line, ftostr3(autotemp_min));
	break;

    case AUTOTEMP_MAX:
	mct_Show(line, ftostr3(autotemp_max));
	break;

    case AUTOTEMP_FACTOR:
	mct_Show(line, ftostr3(autotemp_factor));
	break;

    default:
	break;
    }
}

static void mct_ClickAutotemp(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    float *temp = NULL;
    float scale = 1;

    switch (which) {
    case AUTOTEMP_MIN:
	temp = &autotemp_min;
	break;

    case AUTOTEMP_MAX:
	temp = &autotemp_max;
	break;

    case AUTOTEMP_FACTOR:
	temp = &autotemp_factor;
	scale = 100;
	break;

    default:
	return;
    }

    adjustValue = !adjustValue;
    if (adjustValue) {
	pos = intround(*temp * scale);
    } else {
	*temp = pos / scale;
    }
}

static void mct_ShowAutotempStatus(uint8_t line, uint8_t which)
{
    lcd.setCursor(13,line);
    if (autotemp_enabled) {
	lcdprintPGM(MSG_ON);
    } else {
	lcdprintPGM(MSG_OFF);
    }
}

static void mct_ClickAutotempStatus(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    autotemp_enabled=!autotemp_enabled;
    mct_ShowAutotempStatus(line, which);
}
#endif //autotemp

#if defined BED_USES_THERMISTOR || defined BED_USES_AD595
void mct_ShowBed(uint8_t line, uint8_t which)
{
    mct_Show(line, ftostr3(intround(degTargetBed())));
}

void mct_ClickBed(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if (adjustValue) {
	pos = intround(degTargetBed());
    } else {
	setTargetBed(pos);
    }
}

#endif

void mct_ShowFan(uint8_t line, uint8_t which)
{
    mct_Show(line, itostr3(FanSpeed));
}

void mct_ClickFan(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if (adjustValue) {
	pos = FanSpeed;
    } else {
	FanSpeed = pos;
	analogWrite(FAN_PIN, FanSpeed);
    }
}

void mct_AdjustFan(uint8_t line, volatile long &pos, uint8_t arg)
{
    limitEncoder(pos, 0, 255);

    FanSpeed = pos;
    analogWrite(FAN_PIN, FanSpeed);
    mct_Show(line, itostr3(pos));
}

#ifdef PIDTEMP
static void mct_ShowPid(uint8_t line, uint8_t which)
{
    switch (which) {
    case PID_P:
	mct_Show(line, itostr4(Kp));
	break;

    case PID_I:
	mct_Show(line, ftostr51(Ki/PID_dT));
	break;

    case PID_D:
	mct_Show(line, itostr4(Kd*PID_dT));
	break;

    case PID_C:
	mct_Show(line, itostr3(Kc));
	break;

    default:
	break;
    }
}

static void mct_ClickPid(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    adjustValue = !adjustValue;
    if (adjustValue) {
	switch (which) {
	case PID_P: pos = Kp; break;
	case PID_I: pos = (Ki*10/PID_dT); break;
	case PID_D: pos = (Kd/5.0/PID_dT); break;
	case PID_C: pos = Kc; break;
	default: break;
	}
    } else {
	switch (which) {
	case PID_P: Kp = pos; break;
	case PID_I: Ki = pos/10.0*PID_dT; break;
	case PID_D: Kd = pos*5.0*PID_dT; break;
	case PID_C: Kc = pos; break;
	default: break;
	}
    }
}

static void mct_AdjustPid(uint8_t line, volatile long &pos, uint8_t which)
{
    if (which == PID_P) {
	if (pos<1) pos=1;
    } else {
	if (pos<0) pos=0;
    }
    if (which == PID_C) {
	if (pos>990) pos=990;
	mct_Show(line, itostr3(pos));
    } else {
	if (pos>9990) pos=9990;
	mct_Show(line, itostr4(pos));
    }
}
#endif // PIDTEMP

static menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_CONTROL, NULL,	   mct_ClickMenu,   NULL,		Main_Control },
    { MSG_NOZZLE,  mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_1 },
#if EXTRUDERS > 1
    { MSG_NOZZLE1, mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_2 },
#endif
#if EXTRUDERS > 2
    { MSG_NOZZLE2, mct_ShowNozzle, mct_ClickNozzle, mct_AdjustTemp,	EXTRUDER_3 },
#endif
#ifdef AUTOTEMP
    { MSG_MIN,	    mct_ShowAutotemp,       mct_ClickAutotemp,       mct_AdjustTemp,   AUTOTEMP_MIN },
    { MSG_MAX,	    mct_ShowAutotemp,       mct_ClickAutotemp,       mct_AdjustTemp,   AUTOTEMP_MAX },
    { MSG_FACTOR,   mct_ShowAutotemp,       mct_ClickAutotemp,       mct_AdjustFactor, AUTOTEMP_FACTOR },
    { MSG_AUTOTEMP, mct_ShowAutotempStatus, mct_ClickAutotempStatus, NULL,             0 },
#endif
#if defined BED_USES_THERMISTOR || defined BED_USES_AD595
    { MSG_BED,      mct_ShowBed,            mct_ClickBed,            mct_AdjustTemp,   0 },
#endif
    { MSG_FAN_SPEED,mct_ShowFan,            mct_ClickFan,            mct_AdjustFan,    0 },
    { MSG_PID_P,    mct_ShowPid,            mct_ClickPid,            mct_AdjustPid,    PID_P },
    { MSG_PID_I,    mct_ShowPid,            mct_ClickPid,            mct_AdjustPid,    PID_I },
    { MSG_PID_D,    mct_ShowPid,            mct_ClickPid,            mct_AdjustPid,    PID_D },
    { MSG_PID_C,    mct_ShowPid,            mct_ClickPid,            mct_AdjustPid,    PID_C },
    { MSG_PREHEAT_PLA_SETTINGS, NULL,       mct_ClickMenu,           NULL,    Sub_PreheatPLASettings },
    { MSG_PREHEAT_ABS_SETTINGS, NULL,       mct_ClickMenu,           NULL,    Sub_PreheatABSSettings },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showControlTemp()
{
    show(menu, MENU_MAX);
}

#endif
