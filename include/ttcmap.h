#ifndef TTCMAP_H
#define TTCMAP_H

#include <ttdef.h>

enum
{
    // only support format 4 and 6!
    SUB_0,
    SUB_2,
    SUB_4,
    SUB_6,
    SUB_8,
    SUB_10,
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
void Cmap_Read(TT_Table_Cmap_Sub6 *sub6, FILE *fp);

int Cmap_GetGlyph(TT_Table_Cmap_Sub6 *sub6, TT_ULong code);

#endif