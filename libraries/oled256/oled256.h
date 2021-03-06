/*-
 * Copyright (c) 2014 Darran Hunt (darran [at] hunt dot net dot nz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OLED256_H_
#define OLED256_H_

#include "Arduino.h"
#include "Print.h"
#include "fonts.h"
#include "fontHQ.h"

/**************************************************
*    LM320Y-256064 (SSD1322 driver)
*
*    Pin   Function     SPI connections
*    ----+------------+----------
*      1   VSS		GND
*      2   VBAT		3.3V-5V
*      3   NC
*      4   D0		SCLK	Serial Clock
*      5   D1		MOSI	Serial Data Input
*      6   D2		NC
*      7   D3 		GND
*      8   D4		GND
*      9   D5		GND
*     10   D6		GND
*     11   D7		GND
*     12   #RD		GND
*     13   #WR		GND
*     14   DC		Data / Command 
*     15   #RESET	
*     16   #CS		Chip select
*
*     Note: All logic pins are 3.3V max.
*
*     BS1 BS0	Mode
*     --------+---------------------
*      0   0	"4 Line SPI" 8-bit + DC pin
*      0   1	"3 Line SPI" 9-bit DC is 9th bit
*      1   0	8-bit 8080 parallel
*      1   1	8-bit 6800 parallel
*
*      Note: SPI mode is write only (MOSI)
*
**************************************************/

#define LCDWIDTH                  256
#define LCDHEIGHT                 64

#define CMD_ENABLE_GRAY_SCALE_TABLE	0x00
#define CMD_SET_COLUMN_ADDR		0x15
#define CMD_WRITE_RAM			0x5C
#define CMD_READ_RAM			0x5D
#define CMD_SET_ROW_ADDR		0x75
#define CMD_SET_REMAP			0xA0
#define CMD_SET_DISPLAY_START_LINE	0xA1
#define CMD_SET_DISPLAY_OFFSET		0xA2
#define CMD_SET_DISPLAY_MODE_OFF	0xA4
#define CMD_SET_DISPLAY_MODE_ON		0xA5
#define CMD_SET_DISPLAY_MODE_NORMAL	0xA6
#define CMD_SET_DISPLAY_MODE_INVERSE	0xA7
#define CMD_ENABLE_PARTIAL_DISPLAY	0xA8
#define CMD_EXIT_PARTIAL_DISPLAY	0xA9
#define CMD_SET_FUNCTION_SELECTION	0xAB
#define CMD_SET_DISPLAY_OFF		0xAE
#define CMD_SET_DISPLAY_ON		0xAF
#define CMD_SET_PHASE_LENGTH		0xB1
#define CMD_SET_CLOCK_DIVIDER		0xB3
#define CMD_DISPLAY_ENHANCEMENT		0xB4
#define CMD_SET_GPIO			0xB5
#define CMD_SET_SECOND_PRECHARGE_PERIOD	0xB6
#define CMD_SET_GRAY_SCALE_TABLE	0xB8
#define CMD_SET_PRECHARGE_VOLTAGE	0xBB
#define CMD_SET_DEFAULT_LINEAR_GRAY_SCALE_TABLE	0xB9
#define CMD_SET_VCOMH_VOLTAGE		0xBE
#define CMD_SET_CONTRAST_CURRENT	0xC1
#define CMD_MASTER_CURRENT_CONTROL	0xC7
#define CMD_SET_MULTIPLEX_RATIO		0xCA
#define CMD_DISPLAY_ENHANCEMENT_B	0xD1
#define CMD_SET_COMMAND_LOCK		0xFD

#define LCD_CHAR_COLS 28
#define LCD_CHAR_ROWS 5

class oled256 : public Print {
public:
    oled256(const uint8_t cs, const uint8_t dc, const uint8_t reset);
    void begin(uint8_t font=FONT_NINE_DOT);
    void init(void);
    void writeCommand(uint8_t reg);
    void writeData(uint8_t data);
    void setColumnAddr(uint8_t start, uint8_t end);
    void setRowAddr(uint8_t start, uint8_t end);
    void fill(uint8_t colour);
    void clear();
    void reset();
    void off(void);
    void on(void);
    bool isOn(void);

    void bitmapDraw(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t *image);

    void setWindow(uint8_t x, uint8_t y, uint8_t xend, uint8_t yend);
    void setFont(uint8_t font);
    void setFontHQ(uint8_t font);
    void setColour(uint8_t colour);
    void setContrast(uint8_t contrast);
    void setBackground(uint8_t colour);
    void setOffset(uint8_t offset);
    uint8_t getOffset(void);
    void setBufHeight(uint8_t rows);
    uint8_t getBufHeight(void);

    void setXY(uint8_t col, uint8_t row);

    uint8_t glyphWidth(char ch);
    uint8_t glyphHeight();
    uint8_t glyphDraw(uint16_t x, uint16_t y, char ch, uint16_t colour, uint16_t bg);

    uint8_t glyphDrawHQ(int16_t x, int16_t y, char ch, uint16_t colour, uint16_t bg);

    virtual size_t write(uint8_t ch);
    virtual size_t write(const char *buf);
    virtual size_t write(const uint8_t *buf, size_t size);
    void printXY(uint8_t col, uint8_t row, const char *string);

    uint8_t cur_x;
    uint8_t cur_y;
    uint8_t foreground;
    uint8_t background;
    bool wrap;

 private:
    uint8_t volatile *port_cs;
    uint8_t volatile *port_dc;
    uint8_t pin_cs;
    uint8_t pin_dc;
    uint8_t _cs;		// chip select
    uint8_t _dc;		// data vs command
    uint8_t _reset;
    uint8_t end_x;
    uint8_t end_y;
    uint8_t cur_col;
    uint8_t cur_row;
    uint8_t _offset;
    uint8_t _bufHeight;
    bool _isOn;

    struct {
	uint8_t xaddr;
	uint16_t pixels;
    } gddram[LCDHEIGHT];

    uint8_t readByte();
    void writeByte(uint8_t data);

    uint8_t _font;
    font_t *_fontHQ;
    bool debug;
};

class LcdDisplay : public oled256 {
    public:
	LcdDisplay(const uint8_t cs, const uint8_t dc, const uint8_t reset);
	void begin(uint8_t cols=32, uint8_t rows=5, uint8_t font=FONT_NINE_DOT);
	void setCursor(int16_t x, int16_t y);
	void createChar(uint8_t, uint8_t[]);
	virtual size_t write(uint8_t ch);

    private:
	uint8_t userChar[8][8];
	uint8_t _cols;
	uint8_t _rows;
};


#endif
