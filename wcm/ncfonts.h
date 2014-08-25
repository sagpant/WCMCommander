#ifndef NCFONTS_H
#define NCFONTS_H
#include "swl/swl.h"

extern cptr<cfont> panelFont;
extern cptr<cfont> viewerFont;
extern cptr<cfont> editorFont;
extern cptr<cfont> dialogFont;
extern cptr<cfont> terminalFont;
extern cptr<cfont> helpTextFont;
extern cptr<cfont> helpBoldFont;
extern cptr<cfont> helpHeadFont;

void InitFonts();
#endif
