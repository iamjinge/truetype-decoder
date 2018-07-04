#ifndef FONT_H_
#define FONT_H_

#include <ttdef.h>
#include <ttcmap.h>

typedef struct
{
    TT_Int x;
    TT_Int y;
} TT_Point;

typedef struct
{
    char *name;
    TT_UInt checkSum;
    TT_UInt offset;
    TT_UInt length;
} TT_Table;

typedef struct
{
    TT_Fixed version;
    TT_Fixed fontRevision;
    TT_UInt checkSumAdjustment;
    TT_UInt magicNumber;
    TT_UInt16 flags;
    TT_UInt16 unitsPerEm;
    TT_Date created;
    TT_Date modified;
    TT_FWord xMin;
    TT_FWord yMin;
    TT_FWord xMax;
    TT_FWord yMax;
    TT_UInt16 macStyle;
    TT_UInt16 lowestRecPPEM;
    TT_Int16 fontDirectionHint;
    TT_Int16 indexToLocFormat;
    TT_Int16 glyphDataFormat;
} TT_Table_Head;

typedef struct
{
    TT_UInt16 version;
    TT_UInt16 numberSubtables;
    TT_Table_Cmap_Sub *subTables;
} TT_Table_Cmap;

typedef struct
{
    TT_Fixed version;
    TT_FWord ascent;
    TT_FWord descent;
    TT_FWord lineGap;
    TT_FWord advanceWidthMax;
    TT_FWord minLeftSideBearing;
    TT_FWord minRightSideBearing;
    TT_FWord xMaxExtent;
    TT_Int16 caretSlopeRise;
    TT_Int16 caretSlopeRun;
    TT_FWord caretOffset;
    // 4 int16 reserved;
    TT_Int16 metricDataFormat;
    TT_UInt16 numOfLongHorMetrics;
} TT_Table_Hhea;

typedef struct
{
    TT_Fixed xScale;
    TT_Fixed yScale;
} TT_Font_Metrics;

typedef struct
{
    TT_Int16 numberOfContours;
    TT_FWord xMin;
    TT_FWord yMin;
    TT_FWord xMax;
    TT_FWord yMax;
    TT_UInt offset;

    TT_UInt16 advanceWidth;
    TT_Int16 leftSideBearing;
} TT_Glyph;

typedef struct
{
    TT_UInt scaleType;
    TT_UInt16 numTables;
    TT_UInt16 searchRange;
    TT_UInt16 entrySelector;
    TT_UInt16 rangeShift;
    TT_Table *tables;

    TT_Font_Metrics metrics;

    TT_Stream *stream;

    TT_Table_Head *head;
    TT_Table_Hhea *hhea;

    TT_Table_Cmap *cmap;
} TT_Font;

typedef struct
{
    void *user;
    void (*MoveToFunc)(TT_Point *, void *);
    void (*LineToFunc)(TT_Point *, void *);
    void (*QuadToFunc)(TT_Point *, TT_Point *, void *);
    void (*CubicToFunc)(TT_Point *, TT_Point *, TT_Point *, void *);
} TT_Decompose_Funcs;

TT_Error TT_New_Font(TT_Font *font, const char *filename);

void TT_Set_Size(TT_Font *font, TT_UInt size);

int TT_Get_Char_Index(TT_Font *font, long code);

TT_Error TT_Get_Glyph(TT_Font *font, TT_Glyph *glyph, int index);

TT_Error TT_Decompose_Glyph(TT_Font *font, TT_Glyph *glyph, TT_Decompose_Funcs *callback);

void TT_Done(TT_Font *font);

void TT_DoneTable(TT_Table *table);

void freeCmap(TT_Table_Cmap *cmap);

#endif
