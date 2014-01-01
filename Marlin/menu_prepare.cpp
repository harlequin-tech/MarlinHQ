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

extern long encoderpos;

static void menuClickMain()
{
    mainMenu.status = Main_Menu;
    beepshort();
}

static void menuClickAutostart()
{
#ifdef SDSUPPORT
    card.lastnr=0;
    card.setroot();
    card.checkautostart(true);
#endif
    beepshort();
}

static void menuClickDisableSteppers()
{
    enquecommand("M84");
    beepshort();
}

static void menuClickAutoHome()
{
    enquecommand("G28");
    beepshort();
}


static void menuClickSetOrigin()
{
    enquecommand("G92 X0 Y0 Z0");
    beepshort();
}

static void menuClickPreheatPLA()
{
    setTargetHotend0(plaPreheatHotendTemp);
    setTargetBed(plaPreheatHPBTemp);
#if FAN_PIN > -1
    FanSpeed = plaPreheatFanSpeed;
    analogWrite(FAN_PIN,  FanSpeed);
#endif
    beepshort();
}

static void menuClickPreheatABS()
{
    setTargetHotend0(absPreheatHotendTemp);
    setTargetBed(absPreheatHPBTemp); 
#if FAN_PIN > -1
    FanSpeed = absPreheatFanSpeed;
    analogWrite(FAN_PIN,  FanSpeed);
#endif
    beepshort();
}

static void menuClickCooldown()
{
    setTargetHotend0(0);
    setTargetHotend1(0);
    setTargetHotend2(0);
    setTargetBed(0);
    beepshort();
}

static void menuClickPrepareMove()
{
    mainMenu.status=Sub_PrepareMove;
    beepshort();
}

static const struct {
    char name[20];
    void (*action)();
} menu[] __attribute__((__progmem__)) = {
    { MSG_MAIN,			menuClickMain },
    { MSG_AUTOSTART,		menuClickAutostart },
    { MSG_DISABLE_STEPPERS,	menuClickDisableSteppers },
    { MSG_AUTO_HOME,		menuClickAutoHome },
    { MSG_SET_ORIGIN,		menuClickSetOrigin }, 
    { MSG_PREHEAT_PLA,		menuClickPreheatPLA },
    { MSG_PREHEAT_ABS,		menuClickPreheatABS },
    { MSG_COOLDOWN,		menuClickCooldown },
    { MSG_MOVE_AXIS,		menuClickPrepareMove },
};

#define MENU_PREPARE_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showPrepare()
{
    uint8_t line = activeline + lineoffset;

    updateActiveLines(MENU_PREPARE_MAX, encoderpos);

    if (CLICKED && (line < MENU_PREPARE_MAX)) {
	BLOCK;
	void (*action)() = (void (*)())pgm_read_dword(&menu[line].action);
	action();
    }

    clearIfNecessary();
    if (force_lcd_update) {
	for (line=lineoffset; line<lineoffset+LCD_HEIGHT; line++) {
	    lcd.setCursor(0, line-lineoffset);
	    lcdProgMemprint(menu[line].name);
	}
    }
}

#endif
