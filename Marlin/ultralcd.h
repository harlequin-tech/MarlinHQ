#ifndef ULTRALCD_H
#define ULTRALCD_H
#include "Marlin.h"
#ifdef ULTRA_LCD
  #include <oled256.h>
  #include "menu.h"
  void lcd_status();
  void lcd_init();
  void lcd_status(const char* message);
  void beep();
  void buttons_init();
  void buttons_check();

  #define LCD_UPDATE_INTERVAL 100
  #define STATUSTIMEOUT 15000
  //extern LiquidCrystal lcd;
  extern volatile char buttons;  //the last checked buttons in a bit array.
  extern LcdDisplay lcd;
  
  #define lcdprintPGM(x) lcdProgMemprint(MYPGM(x))
  void lcdProgMemprint(const char *str);

  #ifdef NEWPANEL
    #define EN_C (1<<BLEN_C)
    #define EN_B (1<<BLEN_B)
    #define EN_A (1<<BLEN_A)
    
    #define CLICKED (buttons&EN_C)
    #define BLOCK {blocking=millis()+blocktime;}

    #if (SDCARDDETECT > -1)
      #ifdef SDCARDDETECTINVERTED 
        #define CARDINSERTED (READ(SDCARDDETECT)!=0)
      #else
        #define CARDINSERTED (READ(SDCARDDETECT)==0)
      #endif
    #endif  //SDCARDTETECTINVERTED

#ifndef SDSUPPORT
    #define IS_SD_PRINTING false
#endif
  #else

#if 0
    //atomatic, do not change
    #define B_LE (1<<BL_LE)		// left
    #define B_UP (1<<BL_UP)		// right
    #define B_MI (1<<BL_MI)		// middle
    #define B_DW (1<<BL_DW)		// down
    #define B_RI (1<<BL_RI)		// right
    #define B_ST (1<<BL_ST)		// stop?
    #define EN_B (1<<BLEN_B)		// Encoder A
    #define EN_A (1<<BLEN_A)		// Encoder B
    
    #define CLICKED ((buttons&B_MI)||(buttons&B_ST))
    #define BLOCK {blocking[BL_MI]=millis()+blocktime;blocking[BL_ST]=millis()+blocktime;}
#else
    #define CLICKED false
    #define BLOCK false;
#ifndef SDSUPPORT
    #define IS_SD_PRINTING false
#endif
#endif
    
  #endif


    
  // blocking time for recognizing a new keypress of one key, ms
  #define blocktime 500
  #define lcdslow 5
    
  void limitEncoder(long &pos, long min, long max);
  enum MainStatus{Main_Status, Main_Menu, Main_Prepare,Sub_PrepareMove, Main_Control, Main_SD,Sub_TempControl,Sub_MotionControl,Sub_RetractControl, Sub_PreheatPLASettings, Sub_PreheatABSSettings};

  class MainMenu{
  public:
    MainMenu();
    void update();
    int8_t activeline;
    MainStatus status;
    uint8_t displayStartingRow;
    
    void show(const menu_t *menu, uint8_t menuMax);
    void showStatus();
    void showMainMenu();
    void showPrepare();
    void showTune();
    void showControl();
    void showControlMotion();
    void showControlTemp();
    void showControlRetract();
    void showAxisMove();
    void showSD();
    void showPLAsettings();
    void showABSsettings();
    bool force_lcd_update;
    long lastencoderpos;
    int8_t lineoffset;
    int8_t lastlineoffset;
    bool linechanging;
    bool tune;
    
  private:
    void updateActiveLines(const uint8_t maxlines,volatile long &encoderpos);
    void clearIfNecessary(void);
  };

  //conversion routines, could need some overworking
  char *ftostr51(const float &x);
  char *ftostr52(const float &x);
  char *ftostr31(const float &x);
  char *ftostr3(const float &x);


  #define LCD_INIT lcd_init();
  #define LCD_MESSAGE(x) lcd_status(x);
  #define LCD_MESSAGEPGM(x) lcd_statuspgm(MYPGM(x));
  #define LCD_ALERTMESSAGEPGM(x) lcd_alertstatuspgm(MYPGM(x));
  #define LCD_STATUS lcd_status()
#else //no lcd
  #define LCD_INIT
  #define LCD_STATUS
  #define LCD_MESSAGE(x)
  #define LCD_MESSAGEPGM(x)
  #define LCD_ALERTMESSAGEPGM(x)
  FORCE_INLINE void lcd_status() {};

  #define CLICKED false
  #define BLOCK ;
#endif 
  
void lcd_error(const __FlashStringHelper *error);
void lcd_clearError(void);
void lcd_statuspgm(const char* message);
void lcd_alertstatuspgm(const char* message);
  
char *ftostr3(const float &x);
char *itostr2(const uint8_t &x);
char *ftostr31(const float &x);
char *ftostr32(const float &x);
char *itostr31(const int &xx);
char *itostr3(const int &xx);
char *itostr4(const int &xx);
char *ftostr51(const float &x);

void beep();
void beepshort();
int intround(const float &x);

extern MainMenu mainMenu;

#ifdef NEWPANEL
extern uint32_t blocking;
#else
extern long blocking[8];
#endif

#endif //ULTRALCD
