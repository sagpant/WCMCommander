#ifndef NCFONTS_H
#define NCFONTS_H
#include "swl/swl.h"

extern clPtr<cfont> panelFont;
extern clPtr<cfont> viewerFont;
extern clPtr<cfont> editorFont;
extern clPtr<cfont> dialogFont;
extern clPtr<cfont> terminalFont;
extern clPtr<cfont> helpTextFont;
extern clPtr<cfont> helpBoldFont;
extern clPtr<cfont> helpHeadFont;

void InitFonts();
#endif
