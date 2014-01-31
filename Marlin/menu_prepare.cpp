#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

#include "cardreader.h"
extern CardReader card;

#define MP_DISABLE_STEPPERS 1
#define MP_AUTO_HOME        2
#define MP_SET_ORIGIN       3

static void mp_ClickAutostart(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
#ifdef SDSUPPORT
    card.lastnr=0;
    card.setroot();
    card.checkautostart(true);
#endif
    beepshort();
}

static void mp_ClickEnqueue(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    switch (which) {
    case MP_SET_ORIGIN:
	enquecommand("G92 X0 Y0 Z0");
	break;
    case MP_AUTO_HOME:
	enquecommand("G28");
	break;
    case MP_DISABLE_STEPPERS:
	enquecommand("M84");
	break;
    default:
	break;
    }
    beepshort();
}

static void mp_ClickPreheatPLA(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    setTargetHotend(plaPreheatHotendTemp, 0);
    setTargetBed(plaPreheatHPBTemp);
#if FAN_PIN > -1
    FanSpeed = plaPreheatFanSpeed;
    analogWrite(FAN_PIN,  FanSpeed);
#endif
    beepshort();
}

static void mp_ClickPreheatABS(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    setTargetHotend(absPreheatHotendTemp, 0);
    setTargetBed(absPreheatHPBTemp); 
#if FAN_PIN > -1
    FanSpeed = absPreheatFanSpeed;
    analogWrite(FAN_PIN,  FanSpeed);
#endif
    beepshort();
}

static void mp_ClickCooldown(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    setTargetHotend(0,0);
    setTargetHotend(0,1);
    setTargetHotend(0,2);
    setTargetBed(0);
    beepshort();
}

static menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_MAIN,            NULL,        mct_ClickMenu,           NULL,  Main_Menu },
    { MSG_PREHEAT_PLA,     NULL,	mp_ClickPreheatPLA,      NULL,   0 },
    { MSG_PREHEAT_ABS,     NULL,	mp_ClickPreheatABS,      NULL,   0 },
    { MSG_COOLDOWN,        NULL,	mp_ClickCooldown,        NULL,   0 },
    { MSG_MOVE_AXIS,       NULL,	mct_ClickMenu,           NULL,   Sub_PrepareMove },
    { MSG_DISABLE_STEPPERS,NULL,	mp_ClickEnqueue,         NULL,   MP_DISABLE_STEPPERS },
    { MSG_AUTO_HOME,       NULL,	mp_ClickEnqueue,         NULL,   MP_AUTO_HOME },
    { MSG_SET_ORIGIN,      NULL,	mp_ClickEnqueue,         NULL,   MP_SET_ORIGIN }, 
    { MSG_AUTOSTART,       NULL,	mp_ClickAutostart,       NULL,   0 },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showPrepare()
{
    show(menu, MENU_MAX);
}

#endif
