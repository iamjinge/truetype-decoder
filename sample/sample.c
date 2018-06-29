#include <stdio.h>
#include <file_reader.h>
#include <ttdef.h>
#include <ttfont.h>

const char *filename;
FILE *fp;

void testRead()
{
    TT_Font font;
    TT_Glyph glyph;
    TT_New_Font(&font, fp);
    int index = TT_Get_Char_Index(&font, fp, 'B');
    TT_Get_Glyph(&font, fp, &glyph, index);

    TT_Done(&font);
}

int main(int argc, char const *argv[])
{
    printf("args count %d\n", argc);
    filename = argv[1];

    fp = fopen(filename, "rb");
    testRead();

    fclose(fp);
    return 0;
}
