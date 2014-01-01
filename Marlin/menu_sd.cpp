#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"

/*
 * Main menu
 */

#ifdef SDSUPPORT
#include "cardreader.h"
extern CardReader card;

static char *dirname = "/";

static void sd_ShowDir(uint8_t line, uint8_t arg)
{
    lcd.print(" ");
    dirname = card.getWorkDirName();
    if (sizeof(dirname) >= LCD_WIDTH) {
	dirname[LCD_WIDTH-1] = 0;
    }
    if (dirname[0]=='/') {
	lcdprintPGM(MSG_REFRESH);
    } else {
	lcd.print("\005");
	lcd.print(dirname);
	lcd.print("/..");
    }
}

static void sd_ClickDir(uint8_t line, volatile long &pos, bool &adjustValue, uint8_t which)
{
    if (dirname[0] == '/') {
	if (SDCARDDETECT == -1) {
	    card.initsd();
	}
    }
    card.popDir();
    mainMenu.force_lcd_update=true;
    mainMenu.lineoffset=0;
    beepshort();
}

static menu_t menu[] __attribute__((__progmem__)) = {
    { MSG_MAIN,            NULL,             mct_ClickMenu,     NULL,           Main_Menu },
    { "",                  sd_ShowDir,       sd_ClickDir,       NULL,           0 },
};

#define MENU_MAX (sizeof(menu) / sizeof(menu[0]))

void MainMenu::showSD()
{
    static uint8_t nrfiles=0;

    uint8_t curline = activeline + lineoffset;
    uint8_t arg=0;

    if (curline < MENU_MAX) {
	arg = pgm_read_byte(&menu[curline].arg);
    }

    clearIfNecessary();
    if (mainMenu.force_lcd_update) {
	if (card.cardOK) {
	    nrfiles=card.getnrfilenames();
	} else {
	    nrfiles=0;
	    lineoffset=0;
	}
    }
    if (force_lcd_update) {
	for (uint8_t line=lineoffset; line<lineoffset+LCD_HEIGHT; line++) {
	    if (line < MENU_MAX) {
		show_t show = (show_t)(pgm_read_word(&menu[line].show));
		lcd.setCursor(0, line-lineoffset);
	        lcdProgMemprint(menu[line].name);
		if (show) {
		    show(line-lineoffset, pgm_read_byte(&menu[line].arg));
		}
	    } else {
		uint16_t fileno = line-MENU_MAX;
		card.getfilename(fileno);
#ifdef DEBUG
		MYSERIAL.print(F("line["));
		MYSERIAL.print(line);
		MYSERIAL.print(F("] "));
		MYSERIAL.print(F("Filenr:"));MYSERIAL.print(fileno);
		MYSERIAL.print(F(" = ")); MYSERIAL.println(card.longFilename);
#endif
		lcd.setCursor(0,line-lineoffset);
		lcdprintPGM(" ");
		if (card.filenameIsDir) {
		    lcd.print("\005");
		}
		if (sizeof(card.longFilename) >= LCD_WIDTH) {
		    card.longFilename[LCD_WIDTH-1] = '\0';
		}
		lcd.print(card.longFilename);
	    }
	}
	showCursor();
	force_lcd_update = false;
    }

    if (CLICKED) {
	BLOCK;	// XXX fix this
	if (curline < MENU_MAX) {
	    click_t click = (click_t)(pgm_read_word(&menu[curline].click));
	    click(activeline, encoderpos, linechanging, arg);
	} else {
	    // check for selected file
	    uint16_t fileno = curline-MENU_MAX;
	    card.getfilename(fileno);
	    for (int8_t ind=0; card.filename[ind]; ind++) {
		card.filename[ind] = tolower(card.filename[ind]);
	    }
	    if (card.filenameIsDir) {
		card.pushDir(card.filename, fileno);
		lineoffset = 0;
		mainMenu.force_lcd_update=true;
	    } else {
		char cmd[50];
		snprintf(cmd, sizeof(cmd), "M23 %s", card.filename);		// select file for printing
		//sprintf(cmd,"M115");
		enquecommand(cmd);
		enquecommand("M24");				// start / resume print
#if 0
		MYSERIAL.println(F("Getting duration"));
		uint16_t layers;
		uint32_t time = printDuration(card.filename, &layers);
		MYSERIAL.print(F("Duration "));
		MYSERIAL.print(time);
		MYSERIAL.print(F(" seconds,  "));
		MYSERIAL.print(layers);
		MYSERIAL.println(F(" layers"));
#endif
		beep(); 
		mainMenu.changeMenu(Main_Status);
		if (card.longFilename[0]) {
		    if (sizeof(card.longFilename) > LCD_WIDTH) {
			// truncate filename to LCD width
			card.longFilename[LCD_WIDTH-1] = '\0';
		    }
		    lcd_status(card.longFilename);
		} else {
		    lcd_status(card.filename);
		}
	    }
	}
    }

    updateActiveLines(MENU_MAX+nrfiles-2,encoderpos);
}
#else
void MainMenu::sdShow() { }
#endif

#endif
