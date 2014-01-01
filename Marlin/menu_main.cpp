#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

#include "cardreader.h"
extern CardReader card;

/*
 * Main menu
 */

static void main_ShowPrepare(uint8_t line, uint8_t arg)
{
    lcd.setCursor(0, line);
    if (mainMenu.tune) {
	lcdprintPGM(MSG_TUNE);
    } else {
	lcdprintPGM(MSG_PREPARE);
    }
}

static void main_ShowFile(uint8_t line, uint8_t arg)
{
    lcd.setCursor(0, line);
#ifdef CARDINSERTED
    if (!CARDINSERTED) {
	lcdprintPGM(MSG_NO_CARD); 
	return;
    }
#endif
    if (card.sdprinting) {
	lcdprintPGM(MSG_STOP_PRINT);
    } else {
	lcdprintPGM(MSG_CARD_MENU);
    }
}

static void main_ClickFile(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
#ifdef CARDINSERTED
    if (!CARDINSERTED) return;
#endif

    card.printingHasFinished();
    mainMenu.status = Main_SD;
    beepshort();
}

static void main_ShowPause(uint8_t line, uint8_t arg)
{
#ifdef CARDINSERTED
    if (!CARDINSERTED) return;
#endif

    if (card.sdprinting) {
	lcdprintPGM(MSG_PAUSE_PRINT);
    } else {
	lcdprintPGM(MSG_RESUME_PRINT);
    }
}

static void main_ClickPause(uint8_t line, long &pos, bool &adjustValue, uint8_t which)
{
#ifdef CARDINSERTED
    if (!CARDINSERTED) return;
#endif

    if (card.sdprinting) {
	card.pauseSDPrint();
    } else {
	card.startFileprint();
	starttime=millis();
    }
    beepshort();
    mainMenu.status = Main_Status;
}

static const menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_WATCH,           NULL,             mct_ClickMenu,     NULL,           Main_Status },
    { "",                  main_ShowPrepare, mct_ClickMenu,     NULL,           Main_Prepare },
    { MSG_CONTROL_ARROW,   NULL,             mct_ClickMenu,     NULL,           Main_Control },
#ifdef SDSUPPORT
    { "",                  main_ShowFile,    main_ClickFile,    NULL,           0 },
    { "",                  main_ShowPause,   main_ClickPause,   NULL,           0 },
#endif
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showMainMenu()
{
#ifndef ULTIPANEL
    force_lcd_update=false;
#endif
    if (tune) {
	if (!(movesplanned() || IS_SD_PRINTING)) {
	    force_lcd_update=true;
	    tune=false;
	}
    } else {
	if (movesplanned() || IS_SD_PRINTING) {
	    force_lcd_update=true;
	    tune=true;
	}
    } 

    show(menu, MENU_MAX);
}

#endif
