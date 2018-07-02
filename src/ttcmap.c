#include <ttcmap.h>

int findEndCodeIndex(TT_UInt16 target, TT_UInt16 *array, int length)
{
    int low, high, middle;
    low = 0;
    high = length - 1;

    while (low != high)
    {
        middle = (low + high) / 2;
        printf("%d, %d, %d, %d\n", low, high, array[middle], array[middle + 1]);
        if (array[middle] <= target)
            low = middle + 1;
        else
            high = middle;
    }
    printf("%d\n", low);
    if (array[low] >= target)
        return low;
    else
        return -1;
}

void Cmap_Read_4(TT_Table_Cmap_Sub4 *sub4, TT_Stream *fp)
{
    sub4->length = readUInt16(fp);
    sub4->language = readUInt16(fp);
    sub4->segCountX2 = readUInt16(fp);
    sub4->segCount = sub4->segCountX2 / 2;
    sub4->searchRange = readUInt16(fp);
    sub4->entrySelector = readUInt16(fp);
    sub4->rangeShift = readUInt16(fp);

    sub4->endCode = malloc(sub4->segCountX2);
    for (int i = 0; i < sub4->segCount; i++)
    {
        sub4->endCode[i] = readUInt16(fp);
    }
    sub4->reservedPad = readUInt16(fp);

    sub4->startCode = malloc(sub4->segCountX2);
    for (int i = 0; i < sub4->segCount; i++)
    {
        sub4->startCode[i] = readUInt16(fp);
    }
    sub4->idDelta = malloc(sub4->segCountX2);
    for (int i = 0; i < sub4->segCount; i++)
    {
        sub4->idDelta[i] = readInt16(fp);
    }
    sub4->idRangeOffset = malloc(sub4->segCountX2);
    for (int i = 0; i < sub4->segCount; i++)
    {
        sub4->idRangeOffset[i] = readUInt16(fp);
    }
    sub4->glyphIndexOffset = tttell(fp);
}

int Cmap_GetGlyph_4(TT_Table_Cmap_Sub4 *sub4, TT_Stream *fp, TT_ULong code)
{
    int endCodeIndex = findEndCodeIndex(code, sub4->endCode, sub4->segCount);
    if (endCodeIndex < 0 || endCodeIndex >= sub4->segCount - 1)
        return 0;
    if (sub4->startCode[endCodeIndex] > code)
        return 0;

    int glyphIndex;
    if (sub4->idRangeOffset[endCodeIndex] != 0)
    {
        // TODO unchecked code!!
        glyphIndex = *(&(sub4->idRangeOffset[endCodeIndex]) +
                       sub4->idRangeOffset[endCodeIndex] / 2 +
                       (code - sub4->startCode[endCodeIndex]));
    }
    else
    {
        glyphIndex = sub4->idDelta[endCodeIndex] + code;
    }
    if (glyphIndex >= 0)
        return glyphIndex;
    else
        return 0;
}

void Cmap_Free_4(TT_Table_Cmap_Sub4 *sub4)
{
    free(sub4->endCode);
    free(sub4->startCode);
    free(sub4->idDelta);
    free(sub4->idRangeOffset);
    free(sub4->glyphIndexArray);
}

/*  functions for format 6  */
void Cmap_Read_6(TT_Table_Cmap_Sub6 *sub6, TT_Stream *fp)
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

int Cmap_GetGlyph_6(TT_Table_Cmap_Sub6 *sub6, TT_ULong code)
{
    TT_UInt16 offset = code - sub6->firstCode;
    if (offset >= 0 && offset < sub6->entryCount)
    {
        return sub6->glyphIndexArray[offset];
    }
    return 0;
}