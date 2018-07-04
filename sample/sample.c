#include <stdio.h>
// #include <file_reader.h>
#include <ttdef.h>
#include <ttfont.h>

const char *filename;

void MoveToFunc(TT_Point *p, void *user)
{
    printf("M %d %d\n", p->x, p->y);
}

void LineToFunc(TT_Point *p, void *user)
{
    printf("L %d %d\n", p->x, p->y);
}

void QuadToFunc(TT_Point *cp, TT_Point *p, void *user)
{
    printf("Q %d %d, %d %d\n", cp->x, cp->y, p->x, p->y);
}

void testRead()
{
    TT_Font font;
    TT_Glyph glyph;
    TT_Decompose_Funcs callback;
    callback.MoveToFunc = MoveToFunc;
    callback.LineToFunc = LineToFunc;
    callback.QuadToFunc = QuadToFunc;
    TT_New_Font(&font, filename);
    TT_Set_Size(&font, 16);
    int index = TT_Get_Char_Index(&font, L'二');
    TT_Get_Glyph(&font, &glyph, index);
    printf("glyph width %d\n", glyph.advanceWidth);
    TT_Decompose_Glyph(&font, &glyph, &callback);
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
