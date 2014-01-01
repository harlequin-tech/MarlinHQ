#ifndef MENU_H_
#define MENU_H_


typedef void (*click_t)(uint8_t line, long &pos, bool &adjustValue, uint8_t arg);
typedef void (*show_t)(uint8_t line, uint8_t arg);
typedef void (*adjust_t)(uint8_t line, long &pos, uint8_t arg);

typedef struct {
    char name[24];
    show_t show;
    click_t click;
    adjust_t adjust;
    uint8_t arg;
} menu_t;

void mct_ClickMenu(uint8_t line, long &pos, bool &adjustValue, uint8_t newMenu);
void mct_Show(uint8_t line, const char *value);
void mct_AdjustTemp(uint8_t line, long &pos, uint8_t arg);

void mct_ShowNozzle(uint8_t line, uint8_t extruder);
void mct_ClickNozzle(uint8_t line, long &pos, bool &adjustValue, uint8_t extruder);

void mct_ShowBed(uint8_t line, uint8_t which);
void mct_ClickBed(uint8_t line, long &pos, bool &adjustValue, uint8_t which);

void mct_ShowFan(uint8_t line, uint8_t which);
void mct_ClickFan(uint8_t line, long &pos, bool &adjustValue, uint8_t which);
void mct_AdjustFan(uint8_t line, long &pos, uint8_t arg);

#endif
