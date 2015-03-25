/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include <dirent.h>
#include "color-style.h"
#include "string-util.h"

//надо нормально разукрасить

using namespace wal;
/*
How the color scheme works:

Colors are defined by a set of the statements in the rules text:
CONTROL_CLASSID@ITEMID:CONDITION[:CONDITION2...] {COLOR_NAME: 0xBBGGRR; ...}

ITEMID and CONDITION are optional, and can be omitted. To define default colorset for all classes
that are not explicitly defined in the rules use '*' for CONTROL_CLASSID

String tokens for CONTROL_CLASSID,ITEMID,CONDITION,COLOR_NAME from the  rules are converted to integers
upon parsing the rules text on startup. These integers can be retrieved by color consumers via
int ::GetUiID(char* itemName)

String for CONTROL_CLASSID is defined by control C++ class with Win::UiGetClassId() or its overrides.

CONDITION is set/reset with UiCondList::Set()

Finally 0xBBGGRR is retrieved via
Win::UiGetColor( int CONTROL_CLASSID, int ITEMID, UiCondList* conditionList, unsigned default0xBBGGRR)

The 0xBBGGRR is used in
GC::SetTextColor()/GC::SetFillColor()/etc

*/

static char uiDefaultWcmRules[] =
   "* {color: 0; background: 0xD8E9EC; hotkey-color:0x0000FF ;focus-frame-color : 0x00A000; button-color: 0xD8E9EC;  }"
   "*@item:odd		{color: 0; background: 0xE0FFE0; }"
   "*@item:current-item	{color: 0xFFFFFF; background: 0x800000; }"
   "*@item			{color: 0; background: 0xFFFFFF; }"

   "ScrollBar { button-color: 0xD8E9EC;  }"
   "EditLine:!enabled { background: 0xD8E9EC; }"
   "EditLine:focus {color: 0; background: 0x909000; mark-color: 0xFFFFFF; mark-background : 0x800000; frame-color: 0xD8E9EC }"
   "EditLine {color: 0; background: 0x909000; frame-color: 0xD8E9EC; mark-color: 0xFFFFFF; mark-background : 0x808080; }"

   "#variable { color : 0x800000 }"
   "#value { color : 0x009000 }"

   "Progress { color:0xA00000; frame-color: 0x808080; background: 0xD8E9EC }"

   "command-line:!enabled { background: 0; }"
   "command-line:focus {color: 0xFFFFFF; background: 0; mark-color: 0xFFFFFF; mark-background : 0x800000; frame-color: 0xD8E9EC }"
   "command-line {color: 0xFFFFFF; background: 0; frame-color: 0xD8E9EC; mark-color: 0xFFFFFF; mark-background : 0x808080; }"

   "Autocomplete:!enabled { background: 0xB0B000; }"
   "Autocomplete:current-item {color: 0xFFFFFF; background: 0x000000; mark-color: 0xFFFFFF; mark-background : 0x000000; frame-color: 0x000000 }"
   "Autocomplete:current-item:odd {color: 0xFFFFFF; background: 0x000000; mark-color: 0xFFFFFF; mark-background : 0x000000; frame-color: 0x000000 }"
   "Autocomplete:odd {color: 0xFFFFFF; background: 0xA9A900; frame-color: 0xFFFFFF; mark-color: 0xFFFFFF; mark-background : 0x000000; }"
   "Autocomplete {color: 0xFFFFFF; background: 0xB0B000; frame-color: 0xFFFFFF; mark-color: 0xFFFFFF; mark-background : 0x000000; }"

   "Panel { color: 0xFFFF00; background: 0x800000; }"             // default foreground color for all items - light cyan; RGB(0,255,255)
   "Panel@item:selected-panel:current-item { color: 0x000000 }"         // default foreground color for all items in under cursor - black
   "Panel@item:selected:current-item { color: 0x00FFFF }"            // default foreground color for all selected items in under cursor - light yellow

   "Panel@item:exe { color: 0x00FF00 }"                           // foreground color for executables - light green
   "Panel@item:exe:selected-panel:current-item { color: 0x00FF00 }"     // foreground color for executables under cursor - light green
   "Panel@item:exe:selected:current-item { color: 0x00FFFF }"        // foreground color for selected executables under cursor - light yellow

   "Panel@item:dir { color: 0xFFFFFF }"                           // foreground color for dir names - white
   "Panel@item:dir:selected-panel:current-item { color: 0xFFFFFF }"     // foreground color for dir names under cursor - white
   "Panel@item:dir:selected:current-item { color: 0x00FFFF }"        // foreground color for selected dir names under cursor - light yellow

   "Panel@item:hidden { color: 0x808000 }"                        // foreground color for "hidden" items - dark cyan
   "Panel@item:hidden:selected-panel:current-item { color: 0x808080 }"  // foreground color for "hidden" items under cursor - dark gray; RGB(128,128,182)
   "Panel@item:hidden:selected:current-item { color: 0x00FFFF }"     // foreground color for selected "hidden" items under cursor - light yellow

   "Panel@item:link { color: 0xBBBB00 }"                          // foreground color for links - dark cyan
   "Panel@item:link :selected-panel:current-item { color: 0xBBBB00 }"      // foreground color for links under cursor - dark cyan
   "Panel@item:link:selected:current-item { color: 0x00FFFF }"          // foreground color for selected links under cursor - light yellow

   "Panel@item:bad { color: 0xA0 }"
   "Panel@item:selected { color: 0x00FFFF }"
   "Panel@item:selected-panel:current-item { background: 0x808000 }" // background color of cursor - dark cyan; RGB(0,128,128)
//"Panel@item:selected-panel:current-item:oper-state { background: 0xFF }"
   "Panel ScrollBar, Viewer ScrollBar, Editor ScrollBar {  background: 0xB0B000; button-color: 0x909000 }"

   // border-color1...4 - color of external borders, 1 is outmost, 4 is inmost. Make 2 and 4 light cyan RGB(0,255,255) and 1 and 3 equal to the color of background (dark blue)
   "Panel { border-color1: 0x800000; border-color2: 0xFFFF00; border-color3: 0x800000; border-color4: 0xFFFF00;"

   // vline-color1..3 - color of lines - colums separators. Make first two of them light cyan, 2 pixel thick and third line paint to the color of background (dark blue)
   " vline-color1: 0xFFFF00; vline-color2:0xFFFF00; vline-color3:0x800000;"

   // line-color - color of horizontal top and bottom pane above and below items list. Make it light cyan
   " line-color:0xFFFF00;"

   "summary-color: 0xFFFFFF; }"

   "Panel@footer { color: 0xFFFF00; background: 0x800000; }"         // footer - full info (name, size, date etc.) at the bottom of panel about the item under cursor - light cyan on dark blue

   "Panel { summary-color: 0xFFFF00; }"                           // summary - total size of files in current forlder at the bottom of panel; light cyan
   "Panel:have-selected { summary-color: 0x00FFFF; }"             // total size of selected items at the bottom of panel, light yellow
   "Panel:oper-state { summary-color: 0xFF; }"

   "ButtonWin { color: 0xFFFFFF; background: 0; }"
   "ButtonWin Button { color: 0; background: 0xB0B000; }"
   "EditLine#command-line {color: 0xFFFFFF; background: 0; mark-color: 0; mark-background : 0x800000; }"
   "ToolTip { color: 0; background: 0x80FFFF; }"
   "ToolBar { color: 0; background: 0xB0B000; }"
   "MenuBar { color: 0; background: 0xB0B000; current-item-frame-color:0x00FF00; }"
   "MenuBar:current-item { color: 0xFFFFFF; background: 0;  } "
   "PopupMenu@item {color: 0xFFFFFF; background: 0xA0A000; pointer-color:0; line:0; } "
   "PopupMenu@item:current-item { background: 0x404000; } "
   "PopupMenu { background: 0xC0C000; frame-color: 0xFFFFFF; } "

   "drive-dlg { background: 0xB0B000; }"
   "drive-dlg * { background: 0xB0B000; color: 0xFFFFFF; }"
   "drive-dlg * @item { color: 0xFFFFFF; background: 0xB0B000; comment-color: 0xCCCCCC; first-char-color:0xFFFF }"
   "drive-dlg * @item:current-item { color: 0xFFFFFF;  background: 0; comment-color: 0xCCCCCC; first-char-color:0xFFFF }"

   "#messagebox-red { color: 0xFFFFFF; background: 0xFF; }"
   "#messagebox-red Static { color: 0xFFFFFF; background: 0xFF; }"
   "HelpWin@style-head { color: 0xFFFFFF; }"
   "HelpWin@style-def  { color: 0xFFFFFF; }"
   "HelpWin@style-red  { color: 0xFF; }"
   "HelpWin@style-bold { color: 0xFFFF; }"
   "HelpWin { background: 0x808000; }"

   "Viewer { background: 0x800000; color:0xFFFFFF; ctrl-color:0xFF0000; "
   "	makr-color:	0; mark-background: 0xAAAA00; line-color:0x00FFFF; "
   "	hid: 0xB00000; load-background:	0xD00000; load-color: 0xFF;}"
   "Editor { background: 0x800000; color:0xFFFFFF; mark-background: 0xAAAA00; mark-color: 0; cursor-color: 0x00FFFF; "
   "	DEF:0xFFFFFF; KEYWORD:0x00FFFF; COMMENT:0xFF8080; STRING:0xFFFF00; PRE:0x00FF00; NUM:0xFFFF80; OPER:0x20FFFF; ATTN:0x0000FF; }"
   "StringWin { color: 0xFFFF00; background: 0; }"
   "EditorHeadWin, ViewerHeadWin { color: 0; background: 0xB0B000; prefix-color:0xFFFFFF; cs-color:0xFFFFFF;}"

   "TabDialog { background: 0x606060; color:0xFFFFFF; }"
   "TabDialog * { background: 0x606060; color:0xFFFFFF; button-color: 0x808080; }"
   "TabDialog EditLine { background: 0xD0D0D0; color:0; frame-color: 0x606060; }"
   "TabDialog *@item:dir {color: 0xFFFF00; background: 0; }"
   "TabDialog *@item:current-item	{background: 0x808080; }"
   "TabDialog *@item {color: 0xFFFFFF; background: 0; }"

   "Shortcuts VListWin { background: 0xB0B000; first-char-color:0xFFFF; }"
   "Shortcuts VListWin@item {color: 0; background: 0xB0B000; }"
   "Shortcuts VListWin@item:current-item {color: 0xFFFFFF; background: 0x800000; }"

   ;

bool hasEnding ( std::string const &fullString, std::string const &ending )
{
   if ( fullString.length() >= ending.length() )
   {
      return ( 0 == fullString.compare( fullString.length() - ending.length(), ending.length(), ending ) );
   }
   return false;
}

std::string STYLES_PATH = UNIX_CONFIG_DIR_PATH "/styles/";
const std::string STYLE_EXTENSION = ".style";

std::vector<std::string> GetColorStyles()
{
   std::vector<std::string> styleNames;

   DIR *dir;
   struct dirent *ent;
   if ( ( dir = opendir( STYLES_PATH.c_str() ) ) != NULL )
   {
      while ( ( ent = readdir( dir ) ) != NULL )
      {
         std::string styleName( ent->d_name );
         if ( hasEnding( styleName, STYLE_EXTENSION ) )
         {
            styleName = styleName.substr( 0, styleName.size() - STYLE_EXTENSION.size() );
            styleNames.push_back( styleName );
         }
      }
      closedir( dir );
   }
   else
   {
      perror ( "" ); // FIXME: What is wcm error reporting style?
   }
   return styleNames;
}

void SetDefaultColorStyle( )
{
   try
   {
      UiReadMem( uiDefaultWcmRules );
   }
   catch ( cexception* ex )
   {
      fprintf( stderr, "error:%s\n", ex->message() );
      ex->destroy();
   }
}

void SetColorStyle( std::string style )
{
	try
	{
		std::string filename = STYLES_PATH + style + STYLE_EXTENSION;
		UiReadFile( filename.c_str() );
	}
	catch ( cexception* ex )
	{
		fprintf( stderr, "error:%s\n", ex->message() );
		ex->destroy();
		SetDefaultColorStyle();
	}
}
