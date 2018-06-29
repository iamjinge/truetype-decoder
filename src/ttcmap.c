#include <ttcmap.h>

/*  functions for format 6  */
void Cmap_Read(TT_Table_Cmap_Sub6 *sub6, FILE *fp)
{
    sub6->length = readUInt16(fp);
    sub6->language = readUInt16(fp);
    sub6->firstCode = readUInt16(fp);
    sub6->entryCount = readUInt16(fp);
    sub6->glyphIndexArray = (TT_UInt16 *)malloc(sizeof(TT_UInt16) * sub6->entryCount);
    for (int i = 0; i < sub6->entryCount; i++)
    {
        sub6->glyphIndexArray[i] = readUInt16(fp);
    }
    printf("read sub 6: %d, %d, %d\n", sub6->length, sub6->firstCode, sub6->entryCount);
}

int Cmap_GetGlyph(TT_Table_Cmap_Sub6 *sub6, TT_ULong code)
{
    TT_UInt16 offset = code - sub6->firstCode;
    if (offset >= 0 && offset < sub6->entryCount)
    {
        return sub6->glyphIndexArray[offset];
    }
    return 0;
}