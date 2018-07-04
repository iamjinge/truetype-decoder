#include <ttdef.h>
#include <ttfont.h>
#include <string.h>
#include <stdbool.h>

TT_Table *getTable(TT_Font *font, const char *name)
{
    for (int i = 0; i < font->numTables; i++)
    {
        if (!strcmp(font->tables[i].name, name))
        {
            return &font->tables[i];
        }
    }
    return NULL;
}

bool isNumContourEnd(int num, TT_UInt16 *contours, int length)
{
    for (int ci = 0; ci < length; ci++)
    {
        if (contours[ci] == num)
        {
            return true;
            break;
        }
    }
    return false;
}

void readHeadTable(TT_Font *font)
{
    TT_Table *table = getTable(font, "head");
    printf("search for table name %s\n", table->name);
    ttseek(font->stream, table->offset);
    font->head = (TT_Table_Head *)malloc(sizeof(TT_Table_Head));
    font->head->version = readFixed(font->stream);
    font->head->fontRevision = readFixed(font->stream);
    font->head->checkSumAdjustment = readUInt(font->stream);
    font->head->magicNumber = readUInt(font->stream);
    font->head->flags = readUInt16(font->stream);
    font->head->unitsPerEm = readUInt16(font->stream);
    font->head->created = readDate(font->stream);
    font->head->modified = readDate(font->stream);
    font->head->xMin = readFWord(font->stream);
    font->head->yMin = readFWord(font->stream);
    font->head->xMax = readFWord(font->stream);
    font->head->yMax = readFWord(font->stream);
    font->head->macStyle = readUInt16(font->stream);
    font->head->lowestRecPPEM = readUInt16(font->stream);
    font->head->fontDirectionHint = readInt16(font->stream);
    font->head->indexToLocFormat = readInt16(font->stream);
    font->head->glyphDataFormat = readInt16(font->stream);
    printf("head: magicNumber %8x, indexToLocFormat %d\n",
           font->head->magicNumber, font->head->indexToLocFormat);
}

void readHheaTable(TT_Font *font)
{
    TT_Table *table = getTable(font, "hhea");
    ttseek(font->stream, table->offset);
    font->hhea = (TT_Table_Hhea *)malloc(sizeof(TT_Table_Hhea));
    font->hhea->version = readFixed(font->stream);
    font->hhea->ascent = readFWord(font->stream);
    font->hhea->descent = readFWord(font->stream);
    font->hhea->lineGap = readFWord(font->stream);
    font->hhea->advanceWidthMax = readFWord(font->stream);
    font->hhea->minLeftSideBearing = readFWord(font->stream);
    font->hhea->minRightSideBearing = readFWord(font->stream);
    font->hhea->xMaxExtent = readFWord(font->stream);
    font->hhea->caretSlopeRise = readInt16(font->stream);
    font->hhea->caretSlopeRun = readInt16(font->stream);
    font->hhea->caretOffset = readFWord(font->stream);
    // skip 4 int16 reserved;
    ttseek(font->stream, tttell(font->stream) + 4 * sizeof(TT_Int16));
    font->hhea->metricDataFormat = readInt16(font->stream);
    font->hhea->numOfLongHorMetrics = readUInt16(font->stream);
    printf("hhea: %u %u %u %u", font->hhea->ascent, font->hhea->descent, font->hhea->lineGap, font->hhea->numOfLongHorMetrics);
}

int glyphCount(TT_Font *font)
{
    int old = tttell(font->stream);
    ttseek(font->stream, getTable(font, "maxp")->offset + 4);
    int count = readUInt16(font->stream);
    ttseek(font->stream, old);
    return count;
}

void readCmap(TT_Font *font)
{
    TT_Table *table = getTable(font, "cmap");
    font->cmap = (TT_Table_Cmap *)malloc(sizeof(TT_Table_Cmap));
    ttseek(font->stream, table->offset);
    font->cmap->version = readUInt16(font->stream);
    font->cmap->numberSubtables = readUInt16(font->stream);
    font->cmap->subTables = (TT_Table_Cmap_Sub *)malloc(
        sizeof(TT_Table_Cmap_Sub) * font->cmap->numberSubtables);

    printf("cmap %d, %d, %d, %d\n", table->offset, table->length,
           font->cmap->version, font->cmap->numberSubtables);

    for (int i = 0; i < font->cmap->numberSubtables; i++)
    {
        TT_Table_Cmap_Sub *sub = font->cmap->subTables + i;
        sub->platformID = readUInt16(font->stream);
        sub->platformSpecificID = readUInt16(font->stream);
        sub->offset = readUInt(font->stream) + table->offset;
        for (int j = i - 1; j >= 0; j--)
        {
            if (font->cmap->subTables[j].offset == sub->offset)
            {
                sub->subType = TT_Cmap_SUB_Repeat;
                break;
            }
        }
        if (sub->subType == TT_Cmap_SUB_Repeat)
        {
            continue;
        }
        int old = tttell(font->stream);
        ttseek(font->stream, sub->offset);
        TT_UInt16 format = readUInt16(font->stream);
        if (format == 6)
        {
            sub->subType = TT_Cmap_SUB_6;
            sub->body = malloc(sizeof(TT_Table_Cmap_Sub6));
            Cmap_Read_6(sub->body, font->stream);
        }
        else if (format == 4)
        {
            sub->subType = TT_Cmap_SUB_4;
            sub->body = malloc(sizeof(TT_Table_Cmap_Sub4));
            Cmap_Read_4(sub->body, font->stream);
        }
        printf("%u, \t%d\n", sub->offset, format);
        ttseek(font->stream, old);
    }
}

int getGlyphOffset(TT_Font *font, int index)
{
    printf("get glyph offset index %d\n", index);
    TT_Table *loca = getTable(font, "loca");
    int offset = 0, old = 0;
    old = tttell(font->stream);
    if (font->head->indexToLocFormat == 1)
    {
        ttseek(font->stream, loca->offset + index * 4);
        offset = readUInt(font->stream);
    }
    else
    {
        ttseek(font->stream, loca->offset + index * 2);
        offset = readUInt16(font->stream) * 2;
    }
    ttseek(font->stream, old);

    printf("get glyph offset %d\n", offset);
    TT_Table *glyf = getTable(font, "glyf");
    return offset + glyf->offset;
}

void getGlyphMetric(TT_Font *font, TT_Glyph *glyph, int index)
{
    TT_Table *table = getTable(font, "hmtx");
    if (index < font->hhea->numOfLongHorMetrics)
    {
        ttseek(font->stream, table->offset + index * 4);
        glyph->advanceWidth = readUInt16(font->stream);
        glyph->leftSideBearing = readInt16(font->stream);
    }
    else
    {
        ttseek(font->stream, table->offset + (font->hhea->numOfLongHorMetrics - 1) * 4);
        glyph->advanceWidth = readUInt16(font->stream);
        ttseek(font->stream, table->offset + font->hhea->numOfLongHorMetrics * 4 +
                                 (index - font->hhea->numOfLongHorMetrics) * 2);
        glyph->leftSideBearing = readInt16(font->stream);
    }
}

void scaleGlyph(TT_Font *font, TT_Glyph *glyph)
{
    glyph->xMin = glyph->xMin * font->metrics.xScale / TT_Fixed_1;
    glyph->yMin = glyph->yMin * font->metrics.yScale / TT_Fixed_1;
    glyph->xMax = glyph->xMax * font->metrics.xScale / TT_Fixed_1;
    glyph->yMax = glyph->yMax * font->metrics.yScale / TT_Fixed_1;
    glyph->advanceWidth = glyph->advanceWidth * font->metrics.xScale / TT_Fixed_1;
    glyph->leftSideBearing = glyph->leftSideBearing * font->metrics.xScale / TT_Fixed_1;
}

void decomposeSimpleGlyph(TT_Font *font, TT_Glyph *glyph, TT_Decompose_Funcs *callback)
{
    TT_UInt16 *contours = malloc(sizeof(TT_UInt16) * glyph->numberOfContours);

    TT_UInt8 ON_CURVE = 1,
             X_IS_BYTE = 2,
             Y_IS_BYTE = 4,
             REPEAT = 8,
             X_DELTA = 16,
             Y_DELTA = 32;

    TT_UInt16 maxContour = 0;
    for (int i = 0; i < glyph->numberOfContours; i++)
    {
        contours[i] = readUInt16(font->stream);
        if (contours[i] > maxContour)
            maxContour = contours[i];
        printf("contour %d\n", contours[i]);
    }
    TT_UInt16 numPoints = maxContour + 1;
    // skip instructions
    TT_UInt16 offset = readUInt16(font->stream);
    ttseek(font->stream, tttell(font->stream) + offset);
    TT_UInt8 *flags = (TT_UInt8 *)malloc(sizeof(TT_UInt8) * numPoints);
    TT_Point *points = (TT_Point *)malloc(sizeof(TT_Point) * numPoints);

    for (int i = 0; i < numPoints; i++)
    {
        flags[i] = readUInt8(font->stream);
        if (flags[i] & REPEAT)
        {
            TT_UInt8 count = readUInt8(font->stream);
            for (int j = 1; j <= count; j++)
            {
                flags[i + j] = flags[i];
            }
            i += count;
        }
    }

    int value = 0;
    for (int i = 0; i < numPoints; i++)
    {
        value += (flags[i] & X_IS_BYTE)
                     ? ((flags[i] & X_DELTA) ? readUInt8(font->stream) : -readUInt8(font->stream))
                     : ((flags[i] & X_DELTA) ? 0 : readInt16(font->stream));
        points[i].x = value * font->metrics.xScale / TT_Fixed_1;
    }

    value = 0;
    for (int i = 0; i < numPoints; i++)
    {
        value += (flags[i] & Y_IS_BYTE)
                     ? ((flags[i] & Y_DELTA) ? readUInt8(font->stream) : -readUInt8(font->stream))
                     : ((flags[i] & Y_DELTA) ? 0 : readInt16(font->stream));
        points[i].y = value * font->metrics.yScale / TT_Fixed_1;
    }

    printf("points: \n");
    for (int i = 0; i < numPoints; i++)
    {
        printf("%d: %d(%d, %d)\n", i, flags[i] & ON_CURVE, points[i].x, points[i].y);
    }

    int startIndex = 0;
    TT_Point temp;
    bool inQuad = false;
    for (int i = 0; i < numPoints; i++)
    {
        bool isContourEnd = isNumContourEnd(i, contours, glyph->numberOfContours);
        if (i == startIndex)
        {
            callback->MoveToFunc(points + i, callback->user);
            inQuad = false;
        }
        else if (~flags[i] & ON_CURVE)
        {
            if (inQuad)
            {
                temp.x = (points[i - 1].x + points[i].x) / 2;
                temp.y = (points[i - 1].y + points[i].y) / 2;
                callback->QuadToFunc(points + i - 1, &temp, callback->user);
                inQuad = false;
            }
            if (isContourEnd)
            {
                callback->QuadToFunc(points + i, points + startIndex, callback->user);
                startIndex = i + 1;
            }
            else
            {
                inQuad = true;
            }
        }
        else
        {
            if (inQuad)
            {
                callback->QuadToFunc(points + i - 1, points + i, callback->user);
                inQuad = false;
            }
            else
            {
                callback->LineToFunc(points + i, callback->user);
            }
            if (isContourEnd)
            {
                callback->LineToFunc(points + startIndex, callback->user);
                startIndex = i + 1;
            }
        }
    }

    free(contours);
    free(flags);
    free(points);
}

TT_Error TT_New_Font(TT_Font *font, const char *filename)
{
    font->metrics.xScale = TT_Fixed_1;
    font->metrics.yScale = TT_Fixed_1;
    font->stream = malloc(sizeof(TT_Stream));
    // ttopen(font->stream, filename);
    ttopenToMem(font->stream, filename);
    font->scaleType = readUInt(font->stream);
    font->numTables = readUInt16(font->stream);
    font->searchRange = readUInt16(font->stream);
    font->entrySelector = readUInt16(font->stream);
    font->rangeShift = readUInt16(font->stream);

    printf("font: %d, %d, %d, %d, %d\n", font->scaleType, font->numTables,
           font->searchRange, font->entrySelector, font->rangeShift);

    font->tables = (TT_Table *)malloc(sizeof(TT_Table) * font->numTables);

    for (int i = 0; i < font->numTables; i++)
    {
        TT_Table *table = &font->tables[i];
        table->name = (char *)malloc(sizeof(char) * 5);
        readString(font->stream, table->name, 4);
        table->checkSum = readUInt(font->stream);
        table->offset = readUInt(font->stream);
        table->length = readUInt(font->stream);
        printf("name is %s, \t%u, \t%d, \t%d\n",
               table->name, table->checkSum, table->offset, table->length);
    }

    readHeadTable(font);
    readHheaTable(font);

    printf("glyph count %d\n", glyphCount(font));
    readCmap(font);
    return TT_Error_OK;
}

void TT_Set_Size(TT_Font *font, TT_UInt size)
{
    font->metrics.xScale = TT_Fixed_1 * TT_Size_Multi * size / font->head->unitsPerEm;
    font->metrics.yScale = TT_Fixed_1 * TT_Size_Multi * size / font->head->unitsPerEm;
}

int TT_Get_Char_Index(TT_Font *font, long code)
{
    TT_Table_Cmap *cmap = font->cmap;

    for (int i = 0; i < cmap->numberSubtables; i++)
    {
        int index;
        switch (cmap->subTables[i].subType)
        {
        case TT_Cmap_SUB_4:
            index = Cmap_GetGlyph_4(cmap->subTables[i].body, code);
            break;
        case TT_Cmap_SUB_6:
            index = Cmap_GetGlyph_6(cmap->subTables[i].body, code);
            break;
        default:
            index = 0;
        }
        if (index > 0)
        {
            printf("get char index for %ld, %d\n", code, index);
            return index;
        }
    }
    return 0;
}

TT_Error TT_Get_Glyph(TT_Font *font, TT_Glyph *glyph, int index)
{
    int offset = getGlyphOffset(font, index);
    TT_Table *glyf = getTable(font, "glyf");
    if (offset < glyf->offset || offset >= glyf->offset + glyf->length)
    {
        return TT_Error_NoGlyph;
    }
    ttseek(font->stream, offset);
    glyph->numberOfContours = readInt16(font->stream);
    glyph->xMin = readFWord(font->stream);
    glyph->yMin = readFWord(font->stream);
    glyph->xMax = readFWord(font->stream);
    glyph->yMax = readFWord(font->stream);
    glyph->offset = tttell(font->stream);

    getGlyphMetric(font, glyph, index);
    scaleGlyph(font, glyph);
    printf("glyph: %d, %d, %d, %d, %d\n", glyph->numberOfContours,
           glyph->xMin, glyph->yMin, glyph->xMax, glyph->yMax);
    return TT_Error_OK;
}

TT_Error TT_Decompose_Glyph(TT_Font *font, TT_Glyph *glyph, TT_Decompose_Funcs *callback)
{
    // TODO deal with compound glyph
    if (glyph->numberOfContours == -1)
    {
        printf("compoud glyph unsupport");
    }
    else
    {
        ttseek(font->stream, glyph->offset);
        decomposeSimpleGlyph(font, glyph, callback);
    }
    return TT_Error_OK;
}

void TT_Done(TT_Font *font)
{
    for (int i = 0; i < font->numTables; i++)
    {
        TT_DoneTable(&font->tables[i]);
    }
    free(font->tables);
    ttclose(font->stream);
    free(font->stream);
    free(font->head);
    free(font->hhea);
    freeCmap(font->cmap);
}

void TT_DoneTable(TT_Table *table)
{
    free(table->name);
}

void freeCmap(TT_Table_Cmap *cmap)
{
    for (int i = 0; i < cmap->numberSubtables; i++)
    {
        switch (cmap->subTables[i].subType)
        {
        case TT_Cmap_SUB_4:
            Cmap_Free_4(cmap->subTables[i].body);
            break;
        case TT_Cmap_SUB_6:
            free(((TT_Table_Cmap_Sub6 *)(cmap->subTables[i].body))->glyphIndexArray);
            break;
        }
        free(cmap->subTables[i].body);
    }
    free(cmap->subTables);
    free(cmap);
}