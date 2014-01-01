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
long encoderpos=0;
uint8_t lastenc=0;
int8_t lastStep=0;


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
  SPI.setClockDivider(SPI_CLOCK_DIV4);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.createChar(1,Degree);
  lcd.createChar(2,Thermometer);
  lcd.createChar(3,uplevel);
  lcd.createChar(4,refresh);
  lcd.createChar(5,folder);
#if 0
  MYSERIAL.println(F("echo:lcd_init waiting 5 seconds"));
  delay(5000);
  MYSERIAL.println(F("echo:lcd_init complete"));
#endif
  LCD_MESSAGEPGM(WELCOME_MSG);
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
    //static long previous_millis_buttons=0;
    //static long previous_lcdinit=0;
    buttons_check(); // Done in temperature interrupt
    //previous_millis_buttons=millis();
#if 1	/* XXX check */
    uint32_t ms=millis();
    for(int8_t i=0; i<8; i++) {
      #ifndef NEWPANEL
      if((blocking[i]>ms))
        buttons &= ~(1<<i);
      #else
      if((blocking>ms))
        buttons &= ~(1<<i);        
      #endif
    }
#endif
    if((buttons==oldbuttons) &&  ((millis() - previous_millis_lcd) < LCD_UPDATE_INTERVAL)   )
      return;
    oldbuttons=buttons;
  #else
  
    if(((millis() - previous_millis_lcd) < LCD_UPDATE_INTERVAL)   )
      return;
  #endif
    
  previous_millis_lcd=millis();
  mainMenu.update();
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
}

#if 1
void buttons_check()
{
#ifdef NEWPANEL
    uint8_t newbutton=0;
    if(READ(BTN_EN1)==0)  newbutton|=EN_A;
    if(READ(BTN_EN2)==0)  newbutton|=EN_B;
    if((blocking<millis()) &&(READ(BTN_ENC)==0))
	newbutton|=EN_C;
    buttons=newbutton;
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
	enc = 1;
    }
    if (buttons & EN_B) {
	enc |= 2;
    }

    uint8_t delta = (enc - lastenc) % 4;
    switch(delta) {
	case 0:	// no change
	    break;

	case 1: // clockwise step
	    encoderpos++;
	    lastStep = 1;
	    break;
	    
	case 2: // 2 steps (missed pulse)
	    // assume its the same direction as the last step
	    encoderpos += 2*lastStep;
	    break;

	case 3: // counter-clockwise step
	    encoderpos--;
	    lastStep = -1;
	    break;

	default:
	    break;
    }
    lastenc=enc;
}
#else
uint32_t debounce_EN_A=0;
uint32_t debounce_EN_B=0;
uint32_t debounce_EN_C=0;
uint8_t debounce=0;
#define DEBOUNCE_PERIOD	5

void buttons_check()
{
  #ifdef NEWPANEL
    //uint8_t newbutton=0;
    if (READ(BTN_EN1)==0) {
	if (!buttons & EN_A) {
	    if (debounce & EN_A) {
		if ((millis() - debounce_EN_A) > DEBOUNCE_PERIOD) {
		    buttons |= EN_A;
		    debounce &= ~EN_A;
		}
	    } else {
		debounce_EN_A = millis();
		debounce |= EN_A;
	    }
	} else {
	    debounce &= ~EN_A;
	}
    } else {
	if (buttons & EN_A) {
	    if (debounce & EN_A) {
		if ((millis() - debounce_EN_A) > DEBOUNCE_PERIOD) {
		    buttons &= ~EN_A;
		    debounce &= ~EN_A;
		}
	    } else {
		debounce_EN_A = millis();
		debounce |= EN_A;
	    }
	} else {
	    debounce &= ~EN_A;
	}
    }

    if (READ(BTN_EN2)==0) {
	if (!buttons & EN_B) {
	    if (debounce & EN_B) {
		if ((millis() - debounce_EN_B) > DEBOUNCE_PERIOD) {
		    buttons |= EN_B;
		    debounce &= ~EN_B;
		}
	    } else {
		debounce_EN_B = millis();
		debounce |= EN_B;
	    }
	} else {
	    debounce &= ~EN_B;
	}
    } else {
	if (buttons & EN_B) {
	    if (debounce & EN_B) {
		if ((millis() - debounce_EN_B) > DEBOUNCE_PERIOD) {
		    buttons &= ~EN_B;
		    debounce &= ~EN_B;
		}
	    } else {
		debounce_EN_B = millis();
		debounce |= EN_B;
	    }
	} else {
	    debounce &= ~EN_B;
	}
    }

    if (READ(BTN_ENC)==0) {
	if (!buttons & EN_C) {
	    if (debounce & EN_C) {
		if ((millis() - debounce_EN_C) > DEBOUNCE_PERIOD) {
		    buttons |= EN_C;
		    debounce &= ~EN_C;
		}
	    } else {
		debounce_EN_C = millis();
		debounce |= EN_C;
	    }
	} else {
	    debounce &= ~EN_C;
	}
    } else {
	if (buttons & EN_C) {
	    if (debounce & EN_C) {
		if ((millis() - debounce_EN_C) > DEBOUNCE_PERIOD) {
		    buttons &= ~EN_C;
		    debounce &= ~EN_C;
		}
	    } else {
		debounce_EN_C = millis();
		debounce |= EN_C;
	    }
	} else {
	    debounce &= ~EN_C;
	}
    }
    //if((blocking<millis()) &&(READ(BTN_ENC)==0))
     // newbutton|=EN_C;
    //buttons=newbutton;
  #else   //read it from the shift register
    uint8_t newbutton=0;
    WRITE(SHIFT_LD,LOW);
    WRITE(SHIFT_LD,HIGH);
    unsigned char tmp_buttons=0;
    for(int8_t i=0;i<8;i++)
    { 
      newbutton = newbutton>>1;
      if(READ(SHIFT_OUT))
        newbutton|=(1<<7);
      WRITE(SHIFT_CLK,HIGH);
      WRITE(SHIFT_CLK,LOW);
    }
    buttons=~newbutton; //invert it, because a pressed switch produces a logical 0
  #endif
  
  //manage encoder rotation
  char enc=0;
  if(buttons&EN_A)
    enc|=(1<<0);
  if(buttons&EN_B)
    enc|=(1<<1);
  if(enc!=lastenc)
	{
    switch(enc)
    {
    case encrot0:
      if(lastenc==encrot3)
        encoderpos++;
      else if(lastenc==encrot1)
        encoderpos--;
      break;
    case encrot1:
      if(lastenc==encrot0)
        encoderpos++;
      else if(lastenc==encrot2)
        encoderpos--;
      break;
    case encrot2:
      if(lastenc==encrot1)
        encoderpos++;
      else if(lastenc==encrot3)
        encoderpos--;
      break;
    case encrot3:
      if(lastenc==encrot2)
        encoderpos++;
      else if(lastenc==encrot0)
        encoderpos--;
      break;
    default:
      ;
    }
  }
  lastenc=enc;
}
#endif

#endif

MainMenu::MainMenu()
{
  status=Main_Status;
  displayStartingRow=0;
  activeline=0;
  force_lcd_update=true;
  linechanging=false;
  tune=false;
}

void MainMenu::showStatus()
{ 
#if LCD_HEIGHT>=4
  static int olddegHotEnd0=-1;
  static int oldtargetHotEnd0=-1;
  //force_lcd_update=true;
  if(force_lcd_update)  //initial display of content
  {
    encoderpos=feedmultiply;
    clear();
    lcd.setCursor(0,0);lcdprintPGM("\002---/---\001 ");
    #if defined BED_USES_THERMISTOR || defined BED_USES_AD595 
      lcd.setCursor(10,0);lcdprintPGM("B---/---\001 ");
    #elif EXTRUDERS > 1
      lcd.setCursor(10,0);lcdprintPGM("\002---/---\001 ");
    #endif
  }
    
  int tHotEnd0=intround(degHotend0());
  if((tHotEnd0!=olddegHotEnd0)||force_lcd_update)
  {
    lcd.setCursor(1,0);
    lcd.print(ftostr3(tHotEnd0));
    olddegHotEnd0=tHotEnd0;
  }
  int ttHotEnd0=intround(degTargetHotend0());
  if((ttHotEnd0!=oldtargetHotEnd0)||force_lcd_update)
  {
    lcd.setCursor(5,0);
    lcd.print(ftostr3(ttHotEnd0));
    oldtargetHotEnd0=ttHotEnd0;
  }
  #if defined BED_USES_THERMISTOR || defined BED_USES_AD595 
    static int oldtBed=-1;
    static int oldtargetBed=-1; 
    int tBed=intround(degBed());
    if((tBed!=oldtBed)||force_lcd_update)
    {
      lcd.setCursor(11,0);
      lcd.print(ftostr3(tBed));
      oldtBed=tBed;
    }
    int targetBed=intround(degTargetBed());
    if((targetBed!=oldtargetBed)||force_lcd_update)
    {
      lcd.setCursor(15,0);
      lcd.print(ftostr3(targetBed));
      oldtargetBed=targetBed;
    }
  #elif EXTRUDERS > 1
    static int olddegHotEnd1=-1;
    static int oldtargetHotEnd1=-1;
    int tHotEnd1=intround(degHotend1());
    if((tHotEnd1!=olddegHotEnd1)||force_lcd_update)
    {
      lcd.setCursor(11,0);
      lcd.print(ftostr3(tHotEnd1));
      olddegHotEnd1=tHotEnd1;
    }
    int ttHotEnd1=intround(degTargetHotend1());
    if((ttHotEnd1!=oldtargetHotEnd1)||force_lcd_update)
    {
      lcd.setCursor(15,0);
      lcd.print(ftostr3(ttHotEnd1));
      oldtargetHotEnd1=ttHotEnd1;
    }
  #endif
  //starttime=2;
  static uint16_t oldtime=0;
  if(starttime!=0)
  {
    lcd.setCursor(0,1);
    uint16_t time=millis()/60000-starttime/60000;
    
    if(starttime!=oldtime)
    {
      lcd.print(itostr2(time/60));lcdprintPGM("h ");lcd.print(itostr2(time%60));lcdprintPGM("m");
      oldtime=time;
    }
  }
  static int oldzpos=0;
  int currentz=current_position[2]*100;
  if((currentz!=oldzpos)||force_lcd_update)
  {
    lcd.setCursor(10,1);
    lcdprintPGM("Z:");lcd.print(ftostr52(current_position[2]));
    oldzpos=currentz;
  }
  
  static int oldfeedmultiply=0;
  int curfeedmultiply=feedmultiply;
  
  if(feedmultiplychanged == true) {
    feedmultiplychanged = false;
    encoderpos = curfeedmultiply;
  }
  
  if(encoderpos!=curfeedmultiply||force_lcd_update)
  {
   curfeedmultiply=encoderpos;
   if(curfeedmultiply<10)
     curfeedmultiply=10;
   if(curfeedmultiply>999)
     curfeedmultiply=999;
   feedmultiply=curfeedmultiply;
   encoderpos=curfeedmultiply;
  }
  
  if((curfeedmultiply!=oldfeedmultiply)||force_lcd_update)
  {
   oldfeedmultiply=curfeedmultiply;
   lcd.setCursor(0,2);
   lcd.print(itostr3(curfeedmultiply));lcdprintPGM("% ");
  }
  
  if(messagetext[0]!='\0')
  {
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
  if(oldpercent!=percent ||force_lcd_update)
  {
     lcd.setCursor(10,2);
    lcd.print(itostr3((int)percent));
    lcdprintPGM("%SD");
  }
#endif
  lcd.setCursor(18,2);
  lcd.print(freeMemory());
  lcd.print(F("   "));
#else //smaller LCDS----------------------------------
  static int olddegHotEnd0=-1;
  static int oldtargetHotEnd0=-1;
  if(force_lcd_update)  //initial display of content
  {
    encoderpos=feedmultiply;
    lcd.setCursor(0,0);lcdprintPGM("\002---/---\001 ");
  }
    
  int tHotEnd0=intround(degHotend0());
  int ttHotEnd0=intround(degTargetHotend0());


  if((abs(tHotEnd0-olddegHotEnd0)>1)||force_lcd_update)
  {
    lcd.setCursor(1,0);
    lcd.print(ftostr3(tHotEnd0));
    olddegHotEnd0=tHotEnd0;
  }
  if((ttHotEnd0!=oldtargetHotEnd0)||force_lcd_update)
  {
    lcd.setCursor(5,0);
    lcd.print(ftostr3(ttHotEnd0));
    oldtargetHotEnd0=ttHotEnd0;
  }

  if(messagetext[0]!='\0')
  {
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

//any action must not contain a ',' character anywhere, or this breaks:
#define MENUITEM(repaint_action, click_action) \
  {\
    if(force_lcd_update)  { lcd.setCursor(0,line);  repaint_action; } \
    if((activeline==line) && CLICKED) {click_action} \
  }


enum {
  ItemR_exit,
  ItemR_autoretract,
  ItemR_retract_length,ItemR_retract_feedrate,ItemR_retract_zlift,
  ItemR_unretract_length,ItemR_unretract_feedrate,
  
};



void MainMenu::showControlRetract()
{
#ifdef FWRETRACT
 uint8_t line=0;
 clearIfNecessary();
 for(int8_t i=lineoffset;i<lineoffset+LCD_HEIGHT;i++)
 {
  switch(i)
  {
    case ItemR_exit:
      MENUITEM(  lcdprintPGM(MSG_CONTROL)  ,  BLOCK;status=Main_Control;beepshort(); ) ;
      break;
    
      //float retract_length=2, retract_feedrate=1200, retract_zlift=0.4;
  //float retract_recover_length=0, retract_recover_feedrate=500;
      case ItemR_autoretract:
      {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_AUTORETRACT);
          lcd.setCursor(13,line);
          if(autoretract_enabled)
            lcdprintPGM(MSG_ON);
          else
            lcdprintPGM(MSG_OFF);
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          autoretract_enabled=!autoretract_enabled;
          lcd.setCursor(13,line);
          if(autoretract_enabled)
            lcdprintPGM(MSG_ON);
          else
            lcdprintPGM(MSG_OFF);
          BLOCK;
        }
        
      }break;  
    
      case ItemR_retract_length:
    {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_CONTROL_RETRACT);
          lcd.setCursor(13,line);lcd.print(ftostr52(retract_length));
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          linechanging=!linechanging;
          if(linechanging)
          {
              encoderpos=(long)(retract_length*100);
          }
          else
          {
            retract_length= encoderpos/100.;
            encoderpos=activeline*lcdslow;
              
          }
          BLOCK;
          beepshort();
        }
        if(linechanging)
        {
          if(encoderpos<1) encoderpos=1;
          if(encoderpos>990) encoderpos=990;
          lcd.setCursor(13,line);lcd.print(ftostr52(encoderpos/100.));
        }
        
      }break;
      case ItemR_retract_feedrate:
    {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_CONTROL_RETRACTF);
          lcd.setCursor(13,line);lcd.print(itostr4(retract_feedrate));
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          linechanging=!linechanging;
          if(linechanging)
          {
              encoderpos=(long)(retract_feedrate/5);
          }
          else
          {
            retract_feedrate= encoderpos*5.;
            encoderpos=activeline*lcdslow;
              
          }
          BLOCK;
          beepshort();
        }
        if(linechanging)
        {
          if(encoderpos<1) encoderpos=1;
          if(encoderpos>990) encoderpos=990;
          lcd.setCursor(13,line);lcd.print(itostr4(encoderpos*5));
        }
        
      }break;
      case ItemR_retract_zlift://float retract_acceleration = 7000;
    {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_CONTROL_RETRACT_ZLIFT);
          lcd.setCursor(13,line);lcd.print(ftostr52(retract_zlift));;
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          linechanging=!linechanging;
          if(linechanging)
          {
              encoderpos=(long)(retract_zlift*10);
          }
          else
          {
            retract_zlift= encoderpos/10.;
            encoderpos=activeline*lcdslow;
              
          }
          BLOCK;
          beepshort();
        }
        if(linechanging)
        {
          if(encoderpos<0) encoderpos=0;
          if(encoderpos>990) encoderpos=990;
          lcd.setCursor(13,line);lcd.print(ftostr52(encoderpos/10.));
        }
        
      }break;
      case ItemR_unretract_length:
    {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_CONTROL_RETRACT_RECOVER);
          lcd.setCursor(13,line);lcd.print(ftostr52(retract_recover_length));;
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          linechanging=!linechanging;
          if(linechanging)
          {
              encoderpos=(long)(retract_recover_length*100);
          }
          else
          {
            retract_recover_length= encoderpos/100.;
            encoderpos=activeline*lcdslow;
              
          }
          BLOCK;
          beepshort();
        }
        if(linechanging)
        {
          if(encoderpos<0) encoderpos=0;
          if(encoderpos>990) encoderpos=990;
          lcd.setCursor(13,line);lcd.print(ftostr52(encoderpos/100.));
        }
        
      }break;
      
      case ItemR_unretract_feedrate:
    {
        if(force_lcd_update)
        {
          lcd.setCursor(0,line);lcdprintPGM(MSG_CONTROL_RETRACT_RECOVERF);
          lcd.setCursor(13,line);lcd.print(itostr4(retract_recover_feedrate));
        }
        
        if((activeline!=line) )
          break;
        
        if(CLICKED)
        {
          linechanging=!linechanging;
          if(linechanging)
          {
              encoderpos=(long)retract_recover_feedrate/5;
          }
          else
          {
            retract_recover_feedrate= encoderpos*5.;
            encoderpos=activeline*lcdslow;
              
          }
          BLOCK;
          beepshort();
        }
        if(linechanging)
        {
          if(encoderpos<1) encoderpos=1;
          if(encoderpos>990) encoderpos=990;
          lcd.setCursor(13,line);lcd.print(itostr4(encoderpos*5));
        }
        
      }break;
    
    default:   
      break;
  }
  line++;
 }
 updateActiveLines(ItemR_unretract_feedrate,encoderpos);
#endif
}



enum {
  ItemC_exit,ItemC_temp,ItemC_move,
#ifdef FWRETRACT
  ItemC_rectract,
#endif
  ItemC_store, ItemC_load,ItemC_failsafe
};

void MainMenu::showControl()
{
 uint8_t line=0;
 clearIfNecessary();
 for(int8_t i=lineoffset;i<lineoffset+LCD_HEIGHT;i++)
 {
  switch(i)
  {
    case ItemC_exit:
      MENUITEM(  lcdprintPGM(MSG_MAIN_WIDE)  ,  BLOCK;status=Main_Menu;beepshort(); ) ;
      break;
    case ItemC_temp:
      MENUITEM(  lcdprintPGM(MSG_TEMPERATURE_WIDE)  ,  BLOCK;status=Sub_TempControl;beepshort(); ) ;
      break;
   case ItemC_move:
      MENUITEM(  lcdprintPGM(MSG_MOTION_WIDE)  ,  BLOCK;status=Sub_MotionControl;beepshort(); ) ;
      break;
#ifdef FWRETRACT
    case ItemC_rectract:
      MENUITEM(  lcdprintPGM(MSG_RECTRACT_WIDE)  ,  BLOCK;status=Sub_RetractControl;beepshort(); ) ;
      break;
#endif
    case ItemC_store:
    {
      if(force_lcd_update)
      {
        lcd.setCursor(0,line);lcdprintPGM(MSG_STORE_EEPROM);
      }
      if((activeline==line) && CLICKED)
      {
        //enquecommand("M84");
        beepshort();
        BLOCK;
        EEPROM_StoreSettings();
      }
    }break;
    case ItemC_load:
    {
      if(force_lcd_update)
      {
        lcd.setCursor(0,line);lcdprintPGM(MSG_LOAD_EEPROM);
      }
      if((activeline==line) && CLICKED)
      {
        //enquecommand("M84");
        beepshort();
        BLOCK;
        EEPROM_RetrieveSettings();
      }
    }break;
    case ItemC_failsafe:
    {
      if(force_lcd_update)
      {
        lcd.setCursor(0,line);lcdprintPGM(MSG_RESTORE_FAILSAFE);
      }
      if((activeline==line) && CLICKED)
      {
        //enquecommand("M84");
        beepshort();
        BLOCK;
        EEPROM_RetrieveSettings(true);
      }
    }break;
    default:   
      break;
  }
  line++;
 }
 updateActiveLines(ItemC_failsafe,encoderpos);
}


void MainMenu::update()
{
    static MainStatus oldstatus=Main_Menu;  //init automatically causes foce_lcd_update=true
    static uint32_t timeoutToStatus=0;
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

    if (status!=oldstatus) {
	force_lcd_update = true;
	encoderpos = 0;
	lineoffset = 0;
	oldstatus = status;
    }
    if ((encoderpos != lastencoderpos) || CLICKED) {
	timeoutToStatus = millis()+STATUSTIMEOUT;
    }

    switch(status) {
    case Main_Status: 
	showStatus();
	if (CLICKED) {
	    linechanging=false;
	    BLOCK;
	    status = Main_Menu;
	    timeoutToStatus = millis()+STATUSTIMEOUT;
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

    if (timeoutToStatus<millis())
	status=Main_Status;
    //force_lcd_update=false;
    lastencoderpos = encoderpos;
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
    long curencoderpos=encoderpos;  

    if (linechanging) return; // an item is changint its value, do not switch lines hence

    lastlineoffset=lineoffset; 
    force_lcd_update=false;

    if (abs(curencoderpos-lastencoderpos) < lcdslow) { 
	lcd.setCursor(0,activeline);
	lcd.print((activeline+lineoffset)?' ':' '); 
	if (curencoderpos<0)  {  
	    lineoffset--; 
	    if(lineoffset<0) lineoffset=0; 
	    curencoderpos=lcdslow-1;
	} 
	if (curencoderpos>(LCD_HEIGHT)*lcdslow) { 
	    lineoffset++; 
	    curencoderpos=(LCD_HEIGHT-1)*lcdslow; 
	    if(lineoffset>(maxlines+1-LCD_HEIGHT)) 
		lineoffset=maxlines+1-LCD_HEIGHT; 
	    if(curencoderpos>maxlines*lcdslow) 
		curencoderpos=maxlines*lcdslow; 
	} 
	lastencoderpos=encoderpos=curencoderpos;
	activeline=curencoderpos/lcdslow;

	if (activeline<0) activeline=0;
	if (activeline>LCD_HEIGHT-1) activeline=LCD_HEIGHT-1;
	if (activeline>maxlines) {
	    activeline=maxlines;
	    curencoderpos=maxlines*lcdslow;
	}

	if (lastlineoffset!=lineoffset) {
	    force_lcd_update=true;
	}
	lcd.setCursor(0,activeline);lcd.print((activeline+lineoffset)?'>':'\003');    
    } 
}

void MainMenu::clearIfNecessary()
{
    if (lastlineoffset!=lineoffset ||force_lcd_update) {
	force_lcd_update=true;
	clear();
    } 
}

void MainMenu::show(const menu_t *menu, uint8_t menuMax)
{
    uint8_t line = activeline + lineoffset;

    if (line >= menuMax) {
	return;
    }

    adjust_t adjust = (adjust_t)pgm_read_dword(&menu[line].adjust);
    bool adjusting = linechanging;
    uint8_t arg = pgm_read_byte(&menu[line].arg);

    if (CLICKED) {
	BLOCK;	// XXX fix this
	click_t click = (click_t)pgm_read_dword(&menu[line].click);
	click(activeline, encoderpos, linechanging, arg);
	if (adjusting && !linechanging) {
	    // restore correct encoder position
	    encoderpos = activeline*lcdslow;
	    beepshort();
	}
    }

    if (linechanging) {
	// user is changing the parameter value of the current line
	if (adjust) {
	    adjust(activeline, encoderpos, arg);
	}
    } else {
	updateActiveLines(menuMax, encoderpos);
    }

    clearIfNecessary();
    if (force_lcd_update) {
	for (line=lineoffset; line<lineoffset+LCD_HEIGHT; line++) {
	    show_t show = (show_t)pgm_read_dword(&menu[line].show);
	    lcd.setCursor(0, line-lineoffset);
	    lcdProgMemprint(menu[line].name);
	    if (show) {
		show(line-lineoffset, pgm_read_byte(&menu[line].arg));
	    }
	}
    }
}

void limitEncoder(long &pos, long min, long max)
{
    if (pos<min) {
	pos = 1;
    } else if (pos > max) {
	pos = max;
    }
}

#else
void lcd_error(const __FlashStringHelper *error)
{
    /* No LCD */
}

void lcd_clearError(void)
{
}
#endif //ULTRA_LCD


