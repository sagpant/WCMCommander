#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library  library = 0;

int main()
{
	if ( FT_Init_FreeType( &library ) ) { return 1; }

	return 0;
}

