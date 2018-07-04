#ifndef TTCMAP_H
#define TTCMAP_H

#include <ttdef.h>

enum
{
    // only support format 4 and 6!
    TT_Cmap_SUB_Invalid,
    TT_Cmap_SUB_0,
    TT_Cmap_SUB_2,
    TT_Cmap_SUB_4,
    TT_Cmap_SUB_6,
    TT_Cmap_SUB_8,
    TT_Cmap_SUB_10,
    TT_Cmap_SUB_Repeat,
};

typedef struct
{
    TT_UInt16 platformID;
    TT_UInt16 platformSpecificID;
    TT_UInt32 offset;
    TT_UInt8 subType;
    void *body;
} TT_Table_Cmap_Sub;

typedef struct
{
    TT_UInt16 format;
    TT_UInt16 length;
    TT_UInt16 language;
    TT_UInt16 segCountX2;
    TT_UInt16 searchRange;
    TT_UInt16 entrySelector;
    TT_UInt16 rangeShift;
    TT_UInt16 *endCode;
    TT_UInt16 reservedPad;
    TT_UInt16 *startCode;
    TT_Int16 *idDelta;
    TT_UInt16 *idRangeOffset;
    TT_UInt16 *glyphIndexArray;

    TT_UInt16 segCount;
} TT_Table_Cmap_Sub4;

typedef struct
{
    TT_UInt16 format;
    TT_UInt16 length;
    TT_UInt16 language;
    TT_UInt16 firstCode;
    TT_UInt16 entryCount;
    TT_UInt16 *glyphIndexArray;
} TT_Table_Cmap_Sub6;

// skip format, it has been read!
void Cmap_Read_4(TT_Table_Cmap_Sub4 *sub4, TT_Stream *fp);

int Cmap_GetGlyph_4(TT_Table_Cmap_Sub4 *sub4, TT_ULong code);

void Cmap_Free_4(TT_Table_Cmap_Sub4 *sub4);

// skip format, it has been read!
void Cmap_Read_6(TT_Table_Cmap_Sub6 *sub6, TT_Stream *fp);

int Cmap_GetGlyph_6(TT_Table_Cmap_Sub6 *sub6, TT_ULong code);

#endif
