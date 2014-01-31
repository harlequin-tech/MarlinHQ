#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

/*
 * Control menu
 */

static void mc_ClickLoad(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t loadDefaults)
{
    EEPROM_RetrieveSettings(loadDefaults);
}

static const menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_MAIN_WIDE,        NULL,  mct_ClickMenu,  NULL,  Main_Menu },
    { MSG_TEMPERATURE_WIDE, NULL,  mct_ClickMenu,  NULL,  Sub_TempControl },
    { MSG_MOTION_WIDE,      NULL,  mct_ClickMenu,  NULL,  Sub_MotionControl },
#ifdef FWRETRACT
    { MSG_RECTRACT_WIDE,    NULL,  mct_ClickMenu,  NULL,  Sub_RetractControl },
#endif
    { MSG_LOAD_EEPROM,      NULL,  mc_ClickLoad,   NULL,  false },
    { MSG_RESTORE_FAILSAFE, NULL,  mc_ClickLoad,   NULL,  true }
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showControl()
{
    show(menu, MENU_MAX);
}
#endif
