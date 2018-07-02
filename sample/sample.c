#include <stdio.h>
// #include <file_reader.h>
#include <ttdef.h>
#include <ttfont.h>

const char *filename;

void testRead()
{
    TT_Font font;
    TT_Glyph glyph;
    TT_New_Font(&font, filename);
    int index = TT_Get_Char_Index(&font, L'二');
    TT_Get_Glyph(&font, &glyph, index);
    getchar();
    TT_Done(&font);
}

int main(int argc, char const *argv[])
{
    printf("args count %d\n", argc);
    filename = argv[1];

    testRead();

    getchar();
    return 0;
}
