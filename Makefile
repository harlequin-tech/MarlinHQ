#ARDUINODIR:=/Applications/Arduino.marlin.app/Contents/Resources/Java
#AVRTOOLSPATH := $(ARDUINODIR)/hardware/tools/avr/bin
#SOURCES := AthenaHQ.ino ultralcd.cpp EEPROMwrite.cpp cardreader.cpp \
	    menu_main.cpp menu_motion.cpp menu_moveaxis.cpp menu_pla.cpp \
	    menu_prepare.cpp menu_sd.cpp menu_tempcontrol.cpp menu_tune.cpp \
	    motion_control.cpp planner.cpp stepper.cpp temperature.cpp watchdog.cpp \
	    menu_control.cpp
LIBRARIES := SPI oled256 SD
#BOARD := mighty_opt
#BOARD := atmega328
#BOARD := mega2560
BOARD := atmega1284p

#CPPFLAGS := -DHQ_AEON
#CPPFLAGS := -DHQ_TRINITY
CPPFLAGS := -DHQ_ATHENA

include arduino.mk
#include ../arduino.mk
