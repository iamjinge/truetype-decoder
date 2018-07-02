#include <ttdef.h>
#include <ttfont.h>
#include <string.h>

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
        int old = tttell(font->stream);
        ttseek(font->stream, sub->offset);
        TT_UInt16 format = readUInt16(font->stream);
        if (format == 6)
        {
            sub->subType = SUB_6;
            sub->body = malloc(sizeof(TT_Table_Cmap_Sub6));
            Cmap_Read_6((TT_Table_Cmap_Sub6 *)(sub->body), font->stream);
        }
        else if (format == 4)
        {
            sub->subType = SUB_4;
            sub->body = malloc(sizeof(TT_Table_Cmap_Sub4));
            Cmap_Read_4(sub->body, font->stream);
        }
        printf("\t%d\n", format);
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

void readSimpleGlyph(TT_Font *font, TT_Glyph *glyph)
{
    char ON_CURVE = 1,
         X_IS_BYTE = 2,
         Y_IS_BYTE = 4,
         REPEAT = 8,
         X_DELTA = 16,
         Y_DELTA = 32;

    TT_UInt16 maxContour = 0;
    for (int i = 0; i < glyph->numberOfContours; i++)
    {
        TT_UInt16 contour = readUInt16(font->stream);
        printf("contour %d\n", contour);
        if (contour > maxContour)
        {
            maxContour = contour;
        }
    }
    TT_UInt16 numPoints = maxContour + 1;
    // TODO skip instructions
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
            printf("repead at %d, %2x, %d\n", i, flags[i], count);
            for (int j = 1; j <= count; j++)
            {
                flags[i + j] = flags[i];
            }
            i += count;
        }
    }
    printf("flags:");
    for (int i = 0; i < numPoints; i++)
    {
        printf("\t%2x", flags[i]);
    }
    printf("\n");

    int value = 0;
    for (int i = 0; i < numPoints; i++)
    {
        value += (flags[i] & X_IS_BYTE)
                     ? ((flags[i] & X_DELTA) ? readUInt8(font->stream) : -readUInt8(font->stream))
                     : ((flags[i] & X_DELTA) ? 0 : readInt16(font->stream));
        points[i].x = value;
    }

    value = 0;
    for (int i = 0; i < numPoints; i++)
    {
        value += (flags[i] & Y_IS_BYTE)
                     ? ((flags[i] & Y_DELTA) ? readUInt8(font->stream) : -readUInt8(font->stream))
                     : ((flags[i] & Y_DELTA) ? 0 : readInt16(font->stream));
        points[i].y = value;
    }

    printf("points: \n");
    for (int i = 0; i < numPoints; i++)
    {
        printf("%d: %d(%d, %d)\n", i, flags[i] & ON_CURVE, points[i].x, points[i].y);
    }

    free(flags);
}

void TT_New_Font(TT_Font *font, const char *filename)
{
    font->stream = malloc(sizeof(TT_Stream));
    ttopen(font->stream, filename);
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
        printf("name is %s, %d, %d, %d\n",
               table->name, table->checkSum, table->offset, table->length);
    }

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

    printf("glyph count %d\n", glyphCount(font));
    readCmap(font);
}

int TT_Get_Char_Index(TT_Font *font, long code)
{
    TT_Table_Cmap *cmap = font->cmap;

    for (int i = 0; i < cmap->numberSubtables; i++)
    {
        int index;
        switch (cmap->subTables[i].subType)
        {
        case SUB_4:
            index = Cmap_GetGlyph_4(cmap->subTables[i].body, font->stream, code);
            break;
        case SUB_6:
            index = Cmap_GetGlyph_6(cmap->subTables[i].body, code);
            break;
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
    printf("glyph: %d, %d, %d, %d, %d\n", glyph->numberOfContours,
           glyph->xMin, glyph->yMin, glyph->xMax, glyph->yMax);

    // TODO deal with compound glyph
    if (glyph->numberOfContours == -1)
    {
        printf("compoud glyph unsupport");
    }
    else
    {
        readSimpleGlyph(font, glyph);
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
        case 4:
            Cmap_Free_4(cmap->subTables[i].body);
            break;
        case 6:
            free(((TT_Table_Cmap_Sub6 *)(cmap->subTables[i].body))->glyphIndexArray);
            break;
        }
        free(cmap->subTables[i].body);
    }
    free(cmap->subTables);
    free(cmap);
}