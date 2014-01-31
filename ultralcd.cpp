#include "language.h"
#include "temperature.h"
#include "ultralcd.h"
#ifdef ULTRA_LCD
#include "Marlin.h"
#include "language.h"
#include "temperature.h"
#include "EEPROMwrite.h"
#include <SPI.h>
#include <oled256.h>
#include <math.h>
#include <stdbool.h>
//===========================================================================
//=============================imported variables============================
//===========================================================================


extern volatile int feedmultiply;
extern volatile bool feedmultiplychanged;

extern volatile int extrudemultiply;

extern long position[4];   
#ifdef SDSUPPORT
#include "cardreader.h"
extern CardReader card;
#endif

extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
void encoderInit();

static char lcdErrorStr[17];

static void lcd_showError(void)
{
    lcd.setCursor(16,4);
    lcd.print(lcdErrorStr);
}

void lcd_clearError(void)
{
    memset(lcdErrorStr, ' ', 16);
    lcdErrorStr[16] = 0;

    lcd_showError();
}

void lcd_error(const __FlashStringHelper *error)
{
    lcd_clearError();

    strncpy_P(lcdErrorStr, (const prog_char *)error, 16);
    lcd_showError();
}

static int freeMemory(void) 
{
    int free_memory;

    if((int)__brkval == 0)
      free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
      free_memory = ((int)&free_memory) - ((int)__brkval);

    return free_memory;
}

//===========================================================================
//=============================public variables============================
//===========================================================================

volatile char buttons=0;  //the last checked buttons in a bit array.
volatile long encoderpos=0;
volatile uint8_t lastenc=0;
volatile int8_t lastStep=0;


//===========================================================================
//=============================private  variables============================
//===========================================================================
static char messagetext[LCD_WIDTH]="";

//return for string conversion routines
static char conv[8];

//LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5,LCD_PINS_D6,LCD_PINS_D7);  //RS,Enable,D4,D5,D6,D7 
LcdDisplay lcd(17, 16, 10);	// cs, data_command, reset

static unsigned long previous_millis_lcd=0;
//static long previous_millis_buttons=0;


#ifdef NEWPANEL
uint32_t blocking=0;
#else
long blocking[8]={0,0,0,0,0,0,0,0};
#endif
 
MainMenu mainMenu;

void lcdProgMemprint(const char *str)
{
  char ch=pgm_read_byte(str);
  while(ch)
  {
    lcd.print(ch);
    ch=pgm_read_byte(++str);
  }
}
#define lcdprintPGM(x) lcdProgMemprint(MYPGM(x))


//===========================================================================
//=============================functions         ============================
//===========================================================================

int intround(const float &x){return int(0.5+x);}

void lcd_status(const char* message)
{
  strncpy(messagetext,message,LCD_WIDTH);
  messagetext[strlen(message)]=0;
}

void lcd_statuspgm(const char* message)
{
  char ch=pgm_read_byte(message);
  char *target=messagetext;
  uint8_t cnt=0;
  while(ch &&cnt<LCD_WIDTH)
  {
    *target=ch;
    target++;
    cnt++;
    ch=pgm_read_byte(++message);
  }
  *target=0;
}

void lcd_alertstatuspgm(const char* message)
{
  lcd_statuspgm(message); 
  mainMenu.showStatus(); 
}

FORCE_INLINE void clear()
{
    lcd.clear();
    lcd_showError();
}


void lcd_init()
{
  //beep();
  #if defined(ULTIPANEL) || defined(NEWPANEL)
    buttons_init();
  #endif
  
  byte Degree[8] =
  {
    B01100,
    B10010,
    B10010,
    B01100,
    B00000,
    B00000,
    B00000,
    B00000
  };
  byte Thermometer[8] =
  {
    B00100,
    B01010,
    B01010,
    B01010,
    B01010,
    B10001,
    B10001,
    B01110
  };
  byte uplevel[8]={0x04, 0x0e, 0x1f, 0x04, 0x1c, 0x00, 0x00, 0x00};//thanks joris
  byte refresh[8]={0x00, 0x06, 0x19, 0x18, 0x03, 0x13, 0x0c, 0x00}; //thanks joris
  byte folder [8]={0x00, 0x1c, 0x1f, 0x11, 0x11, 0x1f, 0x00, 0x00}; //thanks joris

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.setColour(LCD_TEXT_COLOUR);
  lcd.setBackground(LCD_TEXT_BACKGROUND);
  lcd.createChar(1,Degree);
  lcd.createChar(2,Thermometer);
  lcd.createChar(3,uplevel);
  lcd.createChar(4,refresh);
  lcd.createChar(5,folder);
#if 0
  MYSERIAL.println(F("echo:lcd_init waiting 4 seconds"));
  delay(5000);
  MYSERIAL.println(F("echo:lcd_init complete"));
#endif
  lcd.setColour(15);
  LCD_MESSAGEPGM(WELCOME_MSG);
  lcd.setColour(LCD_TEXT_COLOUR);
}


void beep()
{
  //return;
  #ifdef ULTIPANEL
	#if (BEEPER > -1)
	{
		pinMode(BEEPER,OUTPUT);
		for(int8_t i=0;i<20;i++){
		WRITE(BEEPER,HIGH);
		delay(5);
		WRITE(BEEPER,LOW);
		delay(5);
		}
	}
        #endif
  #endif
}

void beepshort()
{
  //return;
  #ifdef ULTIPANEL
	#if (BEEPER > -1)
	{
		pinMode(BEEPER,OUTPUT);
		for(int8_t i=0;i<10;i++){
		WRITE(BEEPER,HIGH);
		delay(3);
		WRITE(BEEPER,LOW);
		delay(3);
		}
	}
        #endif
  #endif  
}

void lcd_status()
{
#if defined(ULTIPANEL) || defined(NEWPANEL)
    static uint8_t oldbuttons=0;
    uint32_t now=millis();
#ifndef NEWPANEL
    for(int8_t i=0; i<8; i++) {
	if((blocking[i] > now))
	    buttons &= ~(1<<i);
    }
#endif
    if ((buttons != oldbuttons) || ((now - previous_millis_lcd) >= LCD_UPDATE_INTERVAL)) {
	previous_millis_lcd = now;
	oldbuttons = buttons;
	mainMenu.update();
    }
#else
    if ((now - previous_millis_lcd) >= LCD_UPDATE_INTERVAL) {
	mainMenu.update();
    }
#endif
}
#if defined(ULTIPANEL) || defined(NEWPANEL)


void buttons_init()
{
    pinMode(4,OUTPUT);  // must be output for SPI master mode
    pinMode(A0,OUTPUT);  // must be output for SPI master mode
    digitalWrite(4, HIGH);
    digitalWrite(A0, HIGH);
  #ifdef NEWPANEL
    pinMode(BTN_EN1,INPUT);
    pinMode(BTN_EN2,INPUT); 
    pinMode(BTN_ENC,INPUT); 
    WRITE(BTN_EN1,HIGH);	// pullup
    WRITE(BTN_EN2,HIGH);	// pullup
    WRITE(BTN_ENC,HIGH);	// pullup
    #if (SDCARDDETECT > -1)
    pinMode(SDCARDDETECT,INPUT);
    WRITE(SDCARDDETECT,HIGH);
    #endif
  #else
    pinMode(SHIFT_CLK,OUTPUT);
    pinMode(SHIFT_LD,OUTPUT);
    pinMode(SHIFT_EN,OUTPUT);
    pinMode(SHIFT_OUT,INPUT);
    WRITE(SHIFT_OUT,HIGH);
    WRITE(SHIFT_LD,HIGH); 
    WRITE(SHIFT_EN,LOW); 
  #endif
    encoderInit();
}

#define ERR	 0
#define CW_HALF  0
#define CW_FULL  1
#define CW_END   0
#define ACW_HALF 0 
#define ACW_FULL -1
#define ACW_END  0

/*
* Clockwise:      (BA) 00 -> 01 -> 11 -> 10 -> 00
* Anti-clockwise: (BA) 00 -> 10 -> 11 -> 01 -> 00
*                                Detect
*
* Note: Encoder A and B signals are active low
*/
int8_t encState[4][4] = {
 // BA:    00       01         10        11  current
     {         0,  CW_HALF, ACW_HALF,     ERR  }, // 00
     {  ACW_HALF,        0,      ERR, CW_FULL  }, // 01
     {    CW_END,      ERR,        0, ACW_FULL }, // 10
     {       ERR,  ACW_HALF, CW_HALF,        0 }, // 11
};

typedef struct {
    uint32_t time;
    uint8_t lastValue;
} debounce_t;

uint8_t debounce(uint8_t value, uint16_t period, debounce_t &state)
{
    uint32_t now = millis();

    if (value != state.lastValue) {
	if (state.time) {
	    if ((now - state.time) < period) {
		return state.lastValue;
	    } else {
		// debounced
		state.lastValue = value;
		state.time = 0;
	    }
	} else {
	    // got a change
	    state.time = now;
	}
    } else {
	// reset
	state.time = 0;
    }

    return state.lastValue;
}

debounce_t debEncA = { 0, 0 };
debounce_t debEncB = { 0, 0 };
debounce_t debEncC = { 0, 0 };

uint8_t lastEnc=0;
void encoderInit()
{
    lastEnc = 0;
    if (READ(BTN_EN1) == 0) {
	lastEnc |= 1;
	debEncA.lastValue = 1;
    }
    if (READ(BTN_EN2) == 0) {
	lastEnc |= 2;
	debEncB.lastValue = 2;
    }
}



/*
* Clockwise:      (BA) 00 -> 01 -> 11 -> 10 -> 00
* Anti-clockwise: (BA) 00 -> 10 -> 11 -> 01 -> 00
*                                Detect
*
* Note: Encoder A and B signals are active low
*/
int8_t encoderChange(uint8_t curEnc)
{
    static uint32_t lastTick = 0;
    static int8_t lastStep = 0;
    int8_t step = encState[lastEnc & 0x3][curEnc & 0x3];
    uint32_t now = millis();
#if 0
    if (curEnc != lastEnc) {
	MYSERIAL.print(F("ENC: "));
	MYSERIAL.print(lastEnc);
	MYSERIAL.print(F(" -> "));
	MYSERIAL.print(curEnc);
	MYSERIAL.print(F(" = "));
	MYSERIAL.println(step);
    }
#endif
    lastEnc = curEnc;

    if (mainMenu.linechanging && step) {
	// provide some faster updates when the encoder is spun faster
	if ((lastStep == step) && ((now - lastTick) < 100)) {
	    lastStep = step;
	    step *= 10;
	    if ((now - lastTick) < 20) {
		step *= 10;
	    }
	} else {
	    lastStep = step;
	}
	lastTick = now;
    }
    return step;
}

#define DEBOUNCE_MS 2
#define DEBOUNCE_CLICK_MS 2

bool encoderClicked()
{
    static bool off = true;

    if (off) {
	if (debounce(buttons & EN_C, DEBOUNCE_MS, debEncC)) {
	    off = false;
	    return true;
	}
    } else {
	if (!debounce(buttons & EN_C, DEBOUNCE_MS, debEncC)) {
	    off = true;;
	}
    }

    return false;
}

 
// Note: May be called from an ISR
void buttons_check()
{
#ifdef NEWPANEL
    uint8_t newbutton=0;
    if (READ(BTN_EN1)==0) newbutton |= EN_A;
    if (READ(BTN_EN2)==0) newbutton |= EN_B;
    if (READ(BTN_ENC)==0) newbutton |= EN_C;
#if 0
    if ((blocking<millis()) &&(READ(BTN_ENC)==0))
	newbutton|=EN_C;
#endif
    buttons = newbutton;
#else   //read it from the shift register
    uint8_t newbutton=0;
    WRITE(SHIFT_LD,LOW);
    WRITE(SHIFT_LD,HIGH);
    unsigned char tmp_buttons=0;
    for(int8_t i=0;i<8;i++) { 
	newbutton = newbutton>>1;
	if(READ(SHIFT_OUT))
	    newbutton|=(1<<7);
	WRITE(SHIFT_CLK,HIGH);
	WRITE(SHIFT_CLK,LOW);
    }
    buttons=~newbutton; //invert it, because a pressed switch produces a logical 0
#endif

    //manage encoder rotation
    uint8_t enc=0;

    if (buttons & EN_A) {
	enc = debounce(1, DEBOUNCE_MS, debEncA);
    } else {
	enc = debounce(0, DEBOUNCE_MS, debEncA);
    }

    if (buttons & EN_B) {
	enc |= debounce(2, DEBOUNCE_MS, debEncB);
    } else {
	enc = (enc & 0x1) | debounce(0, DEBOUNCE_MS, debEncB);
    }

    encoderpos += encoderChange(enc);
}

#endif

MainMenu::MainMenu()
{
    status = Main_Status;
    activeline = 0;
    force_lcd_update = true;
    linechanging = false;
    tune = false;
    lastChange = 0;
    currentMenu = NULL;
    showMenuLine = &MainMenu::showLine;
    lcdOffset = 0;
    //lcd.setBufHeight(6*lcd.glyphHeight());
}

void MainMenu::showStatus()
{ 
#if LCD_HEIGHT>=4
    static int olddegHotEnd0=-1;
    static int oldtargetHotEnd0=-1;
    //initial display of content
    if(force_lcd_update)  {
	encoderpos=feedmultiply;
	clear();
	lcd.setCursor(0,0);lcdprintPGM("\002---/---\001 ");
#if defined BED_USES_THERMISTOR || defined BED_USES_AD595 
	lcd.setCursor(10,0);lcdprintPGM("B---/---\001 ");
#elif EXTRUDERS > 1
	lcd.setCursor(10,0);lcdprintPGM("\002---/---\001 ");
#endif
    }

    int tHotEnd0=intround(degHotend(0));
    if ((tHotEnd0!=olddegHotEnd0)||force_lcd_update) {
	lcd.setCursor(1,0);
	lcd.print(ftostr3(tHotEnd0));
	olddegHotEnd0=tHotEnd0;
    }
    int ttHotEnd0=intround(degTargetHotend(0));
    if ((ttHotEnd0!=oldtargetHotEnd0)||force_lcd_update) {
	lcd.setCursor(5,0);
	lcd.print(ftostr3(ttHotEnd0));
	oldtargetHotEnd0=ttHotEnd0;
    }
#if defined BED_USES_THERMISTOR || defined BED_USES_AD595 
    static int oldtBed=-1;
    static int oldtargetBed=-1; 
    int tBed=intround(degBed());
    if ((tBed!=oldtBed)||force_lcd_update) {
	lcd.setCursor(11,0);
	lcd.print(ftostr3(tBed));
	oldtBed=tBed;
    }
    int targetBed=intround(degTargetBed());
    if ((targetBed!=oldtargetBed)||force_lcd_update) {
	lcd.setCursor(15,0);
	lcd.print(ftostr3(targetBed));
	oldtargetBed=targetBed;
    }
#elif EXTRUDERS > 1
    static int olddegHotEnd1=-1;
    static int oldtargetHotEnd1=-1;
    int tHotEnd1=intround(degHotend1());
    if ((tHotEnd1!=olddegHotEnd1)||force_lcd_update) {
	lcd.setCursor(11,0);
	lcd.print(ftostr3(tHotEnd1));
	olddegHotEnd1=tHotEnd1;
    }
    int ttHotEnd1=intround(degTargetHotend(1));
    if ((ttHotEnd1!=oldtargetHotEnd1)||force_lcd_update) {
	lcd.setCursor(15,0);
	lcd.print(ftostr3(ttHotEnd1));
	oldtargetHotEnd1=ttHotEnd1;
    }
#endif
    //starttime=2;
    static uint16_t oldtime=0;
    if (starttime!=0) {
	lcd.setCursor(0,1);
	uint16_t time=millis()/60000-starttime/60000;

	if (starttime!=oldtime) {
	    lcd.print(itostr2(time/60));lcdprintPGM("h ");lcd.print(itostr2(time%60));lcdprintPGM("m");
	    oldtime=time;
	}
    }
    static int oldzpos=0;
    int currentz=current_position[2]*100;
    if ((currentz!=oldzpos)||force_lcd_update) {
	lcd.setCursor(10,1);
	lcdprintPGM("Z:");lcd.print(ftostr52(current_position[2]));
	oldzpos=currentz;
    }

    static int oldfeedmultiply=0;
    int curfeedmultiply=feedmultiply;

    if (feedmultiplychanged == true) {
	feedmultiplychanged = false;
	encoderpos = curfeedmultiply;
    }

    if (encoderpos!=curfeedmultiply||force_lcd_update) {
	curfeedmultiply=encoderpos;
	if (curfeedmultiply<10)
	    curfeedmultiply=10;
	if (curfeedmultiply>999)
	    curfeedmultiply=999;
	feedmultiply=curfeedmultiply;
	encoderpos=curfeedmultiply;
    }

    if ((curfeedmultiply!=oldfeedmultiply)||force_lcd_update) {
	oldfeedmultiply=curfeedmultiply;
	lcd.setCursor(0,2);
	lcd.print(itostr3(curfeedmultiply));lcdprintPGM("% ");
    }

    if (messagetext[0]!='\0') {
	lcd.setCursor(0,LCD_HEIGHT-1);
	lcd.print(messagetext);
	uint8_t n=strlen(messagetext);
	for(int8_t i=0;i<LCD_WIDTH-n;i++)
	    lcd.print(" ");
	messagetext[0]='\0';
    }
#ifdef SDSUPPORT
    static uint8_t oldpercent=101;
    uint8_t percent=card.percentDone();
    if (oldpercent!=percent ||force_lcd_update) {
	lcd.setCursor(10,2);
	lcd.print(itostr3((int)percent));
	lcdprintPGM("%SD ");
	if (card.sdprinting) {
	    uint32_t time = card.timeLeft();
	    lcd.print(time/3600);
	    time -= (time/3600) * 3600;
	    lcd.print(':');
	    lcd.print(itostr2(time/60));
	    time -= (time/60) * 60;
	    lcd.print(':');
	    lcd.print(itostr2(time));
	    lcd.print(F("  "));
	} else {
	    lcd.print(F("        "));
	}
    }
    if (card.sdprinting) {
	lcd.setCursor(1,3);
	if (card.longFilename[0]) {
	    lcd.print(card.longFilename);
	} else {
	    lcd_status(card.filename);
	}
    }
#endif
    static uint8_t memColour=15;
    static int8_t memInc = -1;
    lcd.setCursor(18,3);
    lcd.print(F("mem: "));

    lcd.setColour(memColour);
    memColour += memInc;
    if (memInc < 0 && memColour < 4) {
	memInc = 1;
    }
    if (memInc > 0 && memColour > 14) {
	memInc = -1;
    }
    lcd.print(freeMemory());

    lcd.setColour(LCD_TEXT_COLOUR);
    lcd.print(F("   "));
#else //smaller LCDS----------------------------------
    static int olddegHotEnd0=-1;
    static int oldtargetHotEnd0=-1;
    //initial display of content
    if(force_lcd_update) {
	encoderpos=feedmultiply;
	lcd.setCursor(0,0);lcdprintPGM("\002---/---\001 ");
    }

    int tHotEnd0=intround(degHotend(0));
    int ttHotEnd0=intround(degTargetHotend(0));

    if ((abs(tHotEnd0-olddegHotEnd0)>1)||force_lcd_update) {
	lcd.setCursor(1,0);
	lcd.print(ftostr3(tHotEnd0));
	olddegHotEnd0=tHotEnd0;
    }
    if ((ttHotEnd0!=oldtargetHotEnd0)||force_lcd_update) {
	lcd.setCursor(5,0);
	lcd.print(ftostr3(ttHotEnd0));
	oldtargetHotEnd0=ttHotEnd0;
    }

    if (messagetext[0]!='\0') {
	lcd.setCursor(0,LCD_HEIGHT-1);
	lcd.print(messagetext);
	uint8_t n=strlen(messagetext);
	for(int8_t i=0;i<LCD_WIDTH-n;i++)
	    lcd.print(" ");
	messagetext[0]='\0';
    }

#endif
    force_lcd_update=false;
}


void MainMenu::update()
{
#ifdef CARDINSERTED
    static bool oldcardstatus=false;
    if ((CARDINSERTED != oldcardstatus)) {
	force_lcd_update=true;
	oldcardstatus=CARDINSERTED;
	lcd_init(); // to maybe revive the lcd if static electricty killed it.
	//Serial.println("echo: SD CHANGE");
	if(CARDINSERTED) {
	    card.initsd();
	    LCD_MESSAGEPGM(MSG_SD_INSERTED);
	} else {
	    card.release();
	    LCD_MESSAGEPGM(MSG_SD_REMOVED);
	}
    }
#endif

#if 0
    for (uint8_t ind=0; ind<8; ind++) {
	lcd.setOffset(lcdOffset++);
	if (lcdOffset > 63) {
	    lcdOffset = 0;
	}
	delay(5);
    }
#endif

    switch(status) {
    case Main_Status: 
	showStatus();
	if (CLICKED) {
	    linechanging=false;
	    BLOCK;
	    changeMenu(Main_Menu);
	}
	break;
    case Main_Menu: 
	showMainMenu();
	linechanging = false;
	break;
    case Main_Prepare: 
	if(tune) {
	    showTune();
	} else {
	    showPrepare(); 
	}
	break;
    case Sub_PrepareMove:
	showAxisMove();
	break;
    case Main_Control:
	showControl(); 
	break;
    case Sub_MotionControl:
	showControlMotion(); 
	break;
    case Sub_RetractControl:
	showControlRetract(); 
	break;
    case Sub_TempControl:
	showControlTemp(); 
	break;
    case Main_SD: 
	showSD();
	break;
    case Sub_PreheatPLASettings: 
	showPLAsettings();
	break;
    case Sub_PreheatABSSettings: 
	showABSsettings();
	break;
    default:
	break;
    }

    if ((encoderpos != lastencoderpos) || (buttons & EN_B)) {
	lastChange = millis();
	lastencoderpos = encoderpos;
    }

    if ((status != Main_Status) && ((millis() - lastChange) > STATUSTIMEOUT)) {
	changeMenu(Main_Status);
	lastChange = millis();
    }
}

//**********************************************************************************************************
//  convert float to string with +123.4 format
char *ftostr3(const float &x)
{
  //sprintf(conv,"%5.1f",x);
  int xx=x;
  conv[0]=(xx/100)%10+'0';
  conv[1]=(xx/10)%10+'0';
  conv[2]=(xx)%10+'0';
  conv[3]=0;
  return conv;
}

char *itostr2(const uint8_t &x)
{
  //sprintf(conv,"%5.1f",x);
  int xx=x;
  conv[0]=(xx/10)%10+'0';
  conv[1]=(xx)%10+'0';
  conv[2]=0;
  return conv;
}

//  convert float to string with +123.4 format
char *ftostr31(const float &x)
{
  int xx=x*10;
  conv[0]=(xx>=0)?'+':'-';
  xx=abs(xx);
  conv[1]=(xx/1000)%10+'0';
  conv[2]=(xx/100)%10+'0';
  conv[3]=(xx/10)%10+'0';
  conv[4]='.';
  conv[5]=(xx)%10+'0';
  conv[6]=0;
  return conv;
}

char *ftostr32(const float &x)
{
  long xx=x*100;
  conv[0]=(xx>=0)?'+':'-';
  xx=abs(xx);
  conv[1]=(xx/100)%10+'0';
  conv[2]='.';
  conv[3]=(xx/10)%10+'0';
  conv[4]=(xx)%10+'0';
  conv[6]=0;
  return conv;
}

char *itostr31(const int &xx)
{
  conv[0]=(xx>=0)?'+':'-';
  conv[1]=(xx/1000)%10+'0';
  conv[2]=(xx/100)%10+'0';
  conv[3]=(xx/10)%10+'0';
  conv[4]='.';
  conv[5]=(xx)%10+'0';
  conv[6]=0;
  return conv;
}

char *itostr3(const int &xx)
{
  conv[0]=(xx/100)%10+'0';
  conv[1]=(xx/10)%10+'0';
  conv[2]=(xx)%10+'0';
  conv[3]=0;
  return conv;
}

char *itostr4(const int &xx)
{
  conv[0]=(xx/1000)%10+'0';
  conv[1]=(xx/100)%10+'0';
  conv[2]=(xx/10)%10+'0';
  conv[3]=(xx)%10+'0';
  conv[4]=0;
  return conv;
}

//  convert float to string with +1234.5 format
char *ftostr51(const float &x)
{
  long xx=x*10;
  conv[0]=(xx>=0)?'+':'-';
  xx=abs(xx);
  conv[1]=(xx/10000)%10+'0';
  conv[2]=(xx/1000)%10+'0';
  conv[3]=(xx/100)%10+'0';
  conv[4]=(xx/10)%10+'0';
  conv[5]='.';
  conv[6]=(xx)%10+'0';
  conv[7]=0;
  return conv;
}

//  convert float to string with +123.45 format
char *ftostr52(const float &x)
{
  long xx=x*100;
  conv[0]=(xx>=0)?'+':'-';
  xx=abs(xx);
  conv[1]=(xx/10000)%10+'0';
  conv[2]=(xx/1000)%10+'0';
  conv[3]=(xx/100)%10+'0';
  conv[4]='.';
  conv[5]=(xx/10)%10+'0';
  conv[6]=(xx)%10+'0';
  conv[7]=0;
  return conv;
}

void MainMenu::updateActiveLines(const uint8_t maxlines,volatile long &encoderpos)
{
    uint8_t maxLine = min(maxlines-lineoffset-1, LCD_HEIGHT-1);
    uint8_t lastActiveLine = activeline;

    if (linechanging) return; // an item is changint its value, do not switch lines hence

    if (abs(encoderpos) >= lcdslow) { 
	activeline += encoderpos/lcdslow;
	encoderpos -= encoderpos/lcdslow;

	if (currentMenu) {
	    hideCursor(lastActiveLine);
	} else {
	    lcd.setCursor(0,lastActiveLine);
	    lcd.print(' ');
	}

	if (activeline < 0) {
	    if (lineoffset > 0) {
		lineoffset += activeline;
		if (lineoffset < 0) lineoffset = 0; 
		force_lcd_update = true;
	    }
	    activeline = 0;   
	} else if (activeline > maxLine) {
	    if ((activeline+lineoffset) >= maxlines) {
		activeline = maxlines-lineoffset-1;
	    }
	    if (activeline > maxLine) {
		lineoffset += activeline - maxLine;
		force_lcd_update = true;
	    }
	    activeline = maxLine;
	}

	if (currentMenu) {
	    showCursor(activeline);
	} else {
	    lcd.setCursor(0,activeline);lcd.print((activeline+lineoffset)?'>':'\003');    
	}

	lastChange = millis();
    } 
}

void MainMenu::hideCursor(uint8_t line)
{
    uint8_t saveActiveLine = activeline;

#ifdef DEBUG
    uint8_t menuline = line + lineoffset;
    MYSERIAL.print(F("hideCursor: line "));
    MYSERIAL.print(line);
    MYSERIAL.print(F(" menuline "));
    MYSERIAL.print(menuline);

    if (menuline < menuMax) {
	MYSERIAL.println((const __FlashStringHelper *)currentMenu[menuline].name);
    }
#endif

    activeline = 255;
    (this->*showMenuLine)(line);
    activeline = saveActiveLine;
}

void MainMenu::showCursor(uint8_t line)
{
#ifdef DEBUG
    uint8_t menuline = line + lineoffset;
    MYSERIAL.print(F("showCursor: line "));
    MYSERIAL.print(line);
    MYSERIAL.print(F(" menuline "));
    MYSERIAL.print(menuline);

    if (menuline < menuMax) {
	MYSERIAL.println((const __FlashStringHelper *)currentMenu[menuline].name);
    }
#endif

    (this->*showMenuLine)(line);
}

void MainMenu::clearIfNecessary()
{
    if (force_lcd_update) {
	clear();
    } 
}

#ifdef DEBUG
void dumpMenuEntry(menu_t *menu)
{
    adjust_t adjust = (adjust_t)pgm_read_word(&menu->adjust);
    show_t show = (show_t)pgm_read_word(&menu->show);
    click_t click = (click_t)pgm_read_word(&menu->click);
    arg = pgm_read_byte(&menu->arg);
    MYSERIAL.print(F("sizeof(click_t) = "));
    MYSERIAL.println(sizeof(click_t));
    MYSERIAL.print(F("line["));
    MYSERIAL.print(line);
    MYSERIAL.print(F("] \""));
    MYSERIAL.print((const __FlashStringHelper *)menu->name);
    MYSERIAL.println(F("\""));
    MYSERIAL.print(F("    show   = "));
    MYSERIAL.println((uint32_t)show, HEX);
    MYSERIAL.print(F("    click = "));
    MYSERIAL.println((uint32_t)click, HEX);
    MYSERIAL.print(F("    adjust = "));
    MYSERIAL.println((uint32_t)adjust, HEX);
    MYSERIAL.print(F("    arg    = "));
    MYSERIAL.println((int)arg);
}
#endif

void MainMenu::showLine(uint8_t line)
{
    uint8_t menuline = line + lineoffset;

    if (menuline >= currentMenuMax) {
	return;
    }

    show_t show = (show_t)pgm_read_word(&currentMenu[menuline].show);
#ifdef DEBUG
    dumpMenuEntry(&currentMenu[menuline]);
#endif
    lcd.setCursor(0, line);
    if (line == activeline) {
	lcd.setBackground(LCD_CURSOR_BACKGROUND);
	lcd.setColour(LCD_CURSOR_COLOUR);
    }
    lcdProgMemprint(currentMenu[menuline].name);
    if (show) {
	show(line, pgm_read_byte(&currentMenu[menuline].arg));
    }
    if (line == activeline) {
	lcd.setCursor(0,line);
	lcd.print((line+lineoffset)?'>':'\003');    
	lcd.setBackground(LCD_TEXT_BACKGROUND);
	lcd.setColour(LCD_TEXT_COLOUR);
    }
}

void MainMenu::show(const menu_t *menu, uint8_t menuMax)
{
    uint8_t curline = activeline + lineoffset;

    currentMenu = menu;
    currentMenuMax = menuMax;

    if (curline >= menuMax) {
	return;
    }

    adjust_t adjust = (adjust_t)pgm_read_word(&menu[curline].adjust);
    bool adjusting = linechanging;
    uint8_t arg;

    clearIfNecessary();
    if (force_lcd_update) {
	for (uint8_t line=0; line<LCD_HEIGHT; line++) {
	    (this->*showMenuLine)(line);
	}
	force_lcd_update = false;
    }

    if (CLICKED) {
	BLOCK;	// XXX fix this
	click_t click = (click_t)pgm_read_word(&menu[curline].click);
	arg = pgm_read_byte(&menu[curline].arg);
	click(activeline, encoderpos, linechanging, arg);
	if (adjusting && !linechanging) {
	    // restore correct encoder position
	    encoderpos = 0;
	    beepshort();
	}
    }

    if (linechanging) {
	// user is changing the parameter value of the current line
	if (adjust) {
	    arg = pgm_read_byte(&menu[curline].arg);
	    adjust(activeline, encoderpos, arg);
	}
    } else {
	updateActiveLines(menuMax, encoderpos);
    }

}

void MainMenu::changeMenu(MainStatus newMenu)
{
    status = newMenu;
    force_lcd_update = true;
    activeline = 0;
    lineoffset = 0;
    encoderpos = 0;
    if (newMenu == Main_SD) {
	showMenuLine = &MainMenu::showSDLine;
    } else {
	showMenuLine = &MainMenu::showLine;
    }
    lastChange = millis();
}

void limitEncoder(volatile long &pos, long min, long max)
{
    if (pos<min) {
	pos = 1;
    } else if (pos > max) {
	pos = max;
    }
}

#if 0
bool getGParm(char key, char *fbuf, float &res)
{
    while (*fbuf != key) {
	if (*fbuf == 0) return false;
	fbuf++;
    }

    res = strtod(fbuf+1, NULL);

    return true;
}

#if 0
int16_t sd_fgets(char *str, int16_t num, char *delim=0) 
{
    char ch;
    int16_t n = 0;
    int16_t r = -1;
    while ((n + 1) < num && (r = card.file.read(&ch, 1)) == 1) {
	// delete CR
	if (ch == '\r') continue;
	    str[n++] = ch;
	if (!delim) {
	    if (ch == '\n') break;
	} else {
	    if (strchr(delim, ch)) break;
	}
    }
    if (r < 0) {
	// read error
	return -1;
    }
    str[n] = '\0';
    return n;
}
#else
char sdBuf[512];
int16_t sdBufSize=0;
int16_t sdInd=0;
char *sd_fgets(char *str, int16_t num)
{
    char *res = str;
    while (num > 2) {
	if (sdInd >= sdBufSize) {
	    sdBufSize = card.file.read(sdBuf, sizeof(sdBuf));
	    if (sdBufSize <= 0) {
		return NULL;
	    }
	    sdInd = 0;
	}

	char ch = sdBuf[sdInd++];
	if (ch == '\r') continue;
	*str++ = ch;
	if (ch == '\n') break;
	num--;
    }
    *str = '\0';

    return res;
}
#endif

uint32_t printDuration(char *filename, uint16_t *layers)
{
    float lastx=0;
    float lasty=0;
    float lastz=0;
    float laste=0;
    float lastf=0;
    float x=0;
    float y=0;
    float z=lastz;
    float e=0;
    float f=0;
    float currenttravel = 0;
    float moveduration = 0;
    float totalduration = 0;
    float acceleration = 1500;	// get this from settings
    float layerbeginduration = 0;
    float distance=0;
    uint32_t layercount = 0;
    char fbuf[80];

    card.openFile(filename, true);
    while (!card.eof()) {
	if (sd_fgets(fbuf, sizeof(fbuf)) != NULL) {
	    if (fbuf[0] != 'G') continue;
	    if (fbuf[1] == '4') {
		if (getGParm('P', &fbuf[2], moveduration)) {
		    moveduration /= 1000.0;
		} else if (!getGParm('S', &fbuf[2], moveduration)) {
		    continue;
		}
	    } else if (fbuf[1] == '1') {
		if (!getGParm('X', &fbuf[2], x)) x = lastx;
		if (!getGParm('Y', &fbuf[2], y)) y = lasty;
		if (!getGParm('Z', &fbuf[2], z)) z = lastz;
		if (!getGParm('E', &fbuf[2], e)) e = laste;
		if (!getGParm('F', &fbuf[2], f)) {
		    f = lastf;
		} else {
		    f /= 60.0;
		}

		currenttravel = hypot(lastx-x, hypot(lasty-y, lastz-z));
                distance = abs(2* ((lastf+f) * (f-lastf) * 0.5 ) / acceleration);
                if ((distance <= currenttravel) && ((lastf + f) != 0) && (f != 0)) {
                    moveduration = 2 * distance / ( lastf + f );
                    currenttravel -= distance;
                    moveduration += currenttravel/f;
		} else {
                    moveduration = sqrt(2 * distance / acceleration);
		}
	    }

            totalduration += moveduration;

            if (z > lastz) {
                layercount++;
#if 0
		MYSERIAL.print(F("layer "));
		MYSERIAL.print(lastz);
		MYSERIAL.print(F(" duration "));
		MYSERIAL.print(int(totalduration - layerbeginduration));
		MYSERIAL.println(F(" seconds "));
#endif
                layerbeginduration = totalduration;
	    }

            lastx = x;
            lasty = y;
            lastz = z;
            laste = e;
            lastf = f;
	} else {
	    break;
	}
    }
    if (layers) {
	*layers = int(layercount);
    }
    card.closefile();
    return (int)totalduration;
}
#endif

#else
void lcd_error(const __FlashStringHelper *error)
{
    /* No LCD */
}

void lcd_clearError(void)
{
}
#endif //ULTRA_LCD

