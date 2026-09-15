#ifndef MC_GUI_RT_H
#define MC_GUI_RT_H

/* From-scratch GUI runtime (no third-party dependencies). */

int GuiOpen(int w, int h, const char *title);
int GuiClose(void);
int GuiClear(int color);
int GuiRect(int x, int y, int w, int h, int color);
int GuiText(int x, int y, const char *s, int color, int scale);
int GuiPresent(void);
int GuiPoll(void);
int GuiEventX(void);
int GuiEventY(void);
int GuiEventKey(void);
int GuiSleep(int ms);
int GuiSave(const char *path);

#endif
