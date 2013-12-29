#ifndef _MB_SANGUINO_H_
#define _MB_SANGUINO_H_

// ATMEL ATMEGA1284P
//
//                       +---\/---+
//           (D 0) PB0  1|        |40  PA0 (AI 0 / D 31)
//           (D 1) PB1  2|        |39  PA1 (AI 1 / D 30)
//      INT2 (D 2) PB2  3|        |38  PA2 (AI 2 / D 29)
//       PWM (D 3) PB3  4|        |37  PA3 (AI 3 / D 28)
//    PWM/SS (D 4) PB4  5|        |36  PA4 (AI 4 / D 27)
//      MOSI (D 5) PB5  6|        |35  PA5 (AI 5 / D 26)
//  PWM/MISO (D 6) PB6  7|        |34  PA6 (AI 6 / D 25)
//   PWM/SCK (D 7) PB7  8|        |33  PA7 (AI 7 / D 24)
//                 RST  9|        |32  AREF
//                 VCC 10|        |31  GND 
//                 GND 11|        |30  AVCC
//               XTAL2 12|        |29  PC7 (D 23)
//               XTAL1 13|        |28  PC6 (D 22)
//      RX0 (D 8)  PD0 14|        |27  PC5 (D 21) TDI
//      TX0 (D 9)  PD1 15|        |26  PC4 (D 20) TDO
// RX1/INT0 (D 10) PD2 16|        |25  PC3 (D 19) TMS
// TX1/INT1 (D 11) PD3 17|        |24  PC2 (D 18) TCK
//      PWM (D 12) PD4 18|        |23  PC1 (D 17) SDA
//      PWM (D 13) PD5 19|        |22  PC0 (D 16) SCL
//      PWM (D 14) PD6 20|        |21  PD7 (D 15) PWM
//                       +--------+
//
//   PCINT15-8: D7-0  : bit 1
//   PCINT31-24: D15-8  : bit 3
//   PCINT23-16: D23-16 : bit 2
//   PCINT7-0: D31-24   : bit 0

#define KNOWN_BOARD 1
#ifndef __AVR_ATmega644P__
#ifndef __AVR_ATmega1284P__
#error Oops!  Make sure you have 'Sanguino' selected from the 'Tools -> Boards' menu.
#endif
#endif

#define X_STEP_PIN         15
#define X_DIR_PIN          21
#if X_HOME_DIR < 0
# define X_MIN_PIN          18 
# define X_MAX_PIN          -1
#else
# define X_MIN_PIN          -1
# define X_MAX_PIN          18
#endif

#define Y_STEP_PIN         22
#define Y_DIR_PIN          23
#if Y_HOME_DIR < 0
# define Y_MIN_PIN          19 
# define Y_MAX_PIN          -1
#else
# define Y_MIN_PIN          -1
# define Y_MAX_PIN          19
#endif

#define Z_STEP_PIN         3
#define Z_DIR_PIN          2
#if Z_HOME_DIR < 0
# define Z_MIN_PIN          20 
# define Z_MAX_PIN          -1
#else
# define Z_MIN_PIN          -1
# define Z_MAX_PIN          20
#endif

#define E0_STEP_PIN         1
#define E0_DIR_PIN          0

#define LED_PIN            -1

#define FAN_PIN            -1 

#ifdef MELZI
#define LED_PIN            28
#define FAN_PIN            4
#endif

#define PS_ON_PIN          -1
#define KILL_PIN           -1

#define HEATER_0_PIN       13 // (extruder)
#define HEATER_1_PIN       -1
#define HEATER_2_PIN       -1

#ifdef SANGUINOLOLU_V_1_2

#define HEATER_BED_PIN     12 // (bed)
#define X_ENABLE_PIN       14
#define Y_ENABLE_PIN       14
#define Z_ENABLE_PIN       26
#define E0_ENABLE_PIN       14

#else

#define HEATER_BED_PIN       14  // (bed)
#define X_ENABLE_PIN       -1
#define Y_ENABLE_PIN       -1
#define Z_ENABLE_PIN       -1
#define E0_ENABLE_PIN       -1

#endif

#define TEMP_0_PIN          7   // MUST USE ANALOG INPUT NUMBERING NOT DIGITAL OUTPUT NUMBERING!!!!!!!!! (pin 33 extruder)
#define TEMP_1_PIN         -1
#define TEMP_2_PIN         -1
#define TEMP_BED_PIN        6   // MUST USE ANALOG INPUT NUMBERING NOT DIGITAL OUTPUT NUMBERING!!!!!!!!! (pin 34 bed)
#define SDPOWER            -1
#define SDSS               11
#define SDCARDDETECT -1

#ifdef MELZI
#define SDSS               24
#endif

// Sanguinololu panel settings
#ifdef NEWPANEL
    #define BTN_EN1 29	// A
    #define BTN_EN2 28	// B
    #define BTN_ENC 27  //the click
    
    #define BLEN_A 0	// just bits in buttons field
    #define BLEN_B 1	// just bits in buttons field
    #define BLEN_C 2	// just bits in buttons field
    
    //encoder rotation values
    #define encrot0 0
    #define encrot1 2
    #define encrot2 3
    #define encrot3 1
#endif

#endif
