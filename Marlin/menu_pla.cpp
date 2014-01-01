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

#define TUNE_SPEED 0

#define PLA_FANSPEED	0
#define PLA_HOTENDTEMP	1
#define PLA_HPBTEMP	2
#define ABS_FANSPEED	3
#define ABS_HOTENDTEMP	4
#define ABS_HPBTEMP	5

#define NUM_PARMS	6
static int *parameter[NUM_PARMS] = {
    &plaPreheatFanSpeed,
    &plaPreheatHotendTemp,
    &plaPreheatHPBTemp,
    &absPreheatFanSpeed,
    &absPreheatHotendTemp,
    &absPreheatHPBTemp
};

static int limit[NUM_PARMS] = {
    255,	// PLA fan
    210,	// PLA hotend
     90,	// PLA bed
    255,	// ABS fan
    260,	// ABS hotend
    120		// ABS bed
};

static void mp_Show(uint8_t line, const char *value)
{
    lcd.setCursor(13,line);
    lcd.print(value);
}

static void mp_ShowPreheat(uint8_t line, uint8_t which) 
{
    if (which < NUM_PARMS) {
	mp_Show(line, ftostr3(*parameter[which]));
    }
}

static void mp_ClickPreheat(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
    if (which >= NUM_PARMS) return;

    adjustValue = !adjustValue;
    if (adjustValue) {
	pos = *parameter[which];
    } else {
	*parameter[which] = pos;
    }
}

static void mp_AdjustPreheat(uint8_t line, long &pos, uint8_t which)
{
    if (which >= NUM_PARMS) return;
    limitEncoder(pos, 0, limit[which]);
    mp_Show(line, itostr3(pos));
}

static void mp_ClickStore(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
    EEPROM_StoreSettings();
}

static const menu_t menu_pla[] __attribute__((__progmem__)) = {
    { MSG_TEMPERATURE_RTN, NULL,           mct_ClickMenu,   NULL,             Sub_TempControl },
    { MSG_FAN_SPEED,       mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, PLA_FANSPEED },
    { MSG_NOZZLE,          mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, PLA_HOTENDTEMP },
    { MSG_BED,             mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, PLA_HPBTEMP },
    { MSG_STORE_EEPROM,    NULL,           mp_ClickStore,   NULL,             0 },
};
#define MENU_PLA_MAX (sizeof(menu_pla) / sizeof(menu_pla[0]))

static const menu_t menu_abs[] __attribute__((__progmem__)) = {
    { MSG_TEMPERATURE_RTN, NULL,           mct_ClickMenu,   NULL,             Sub_TempControl },
    { MSG_FAN_SPEED,       mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, ABS_FANSPEED },
    { MSG_NOZZLE,          mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, ABS_HOTENDTEMP },
    { MSG_BED,             mp_ShowPreheat, mp_ClickPreheat, mp_AdjustPreheat, ABS_HPBTEMP },
    { MSG_STORE_EEPROM,    NULL,           mp_ClickStore,   NULL,             0 },
};
#define MENU_ABS_MAX (sizeof(menu_abs) / sizeof(menu_abs[0]))

void MainMenu::showPLAsettings()
{
#ifdef ULTIPANEL
    show(menu_pla, MENU_PLA_MAX);
#endif
}

void MainMenu::showABSsettings()
{
#ifdef ULTIPANEL
    show(menu_abs, MENU_ABS_MAX);
#endif
}

#endif
