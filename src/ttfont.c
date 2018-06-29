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

int glyphCount(TT_Font *font, FILE *fp)
{
    int old = ftell(fp);
    fseek(fp, getTable(font, "maxp")->offset + 4, SEEK_SET);
    int count = readUInt16(fp);
    fseek(fp, old, SEEK_SET);
    return count;
}

void readCmap(TT_Font *font, FILE *fp)
{
    TT_Table *table = getTable(font, "cmap");
    font->cmap = (TT_Table_Cmap *)malloc(sizeof(TT_Table_Cmap));
    fseek(fp, table->offset, SEEK_SET);
    font->cmap->version = readUInt16(fp);
    font->cmap->numberSubtables = readUInt16(fp);
    font->cmap->subTables = (TT_Table_Cmap_Sub *)malloc(
        sizeof(TT_Table_Cmap_Sub) * font->cmap->numberSubtables);

    printf("cmap %d, %d, %d, %d\n", table->offset, table->length,
           font->cmap->version, font->cmap->numberSubtables);

    for (int i = 0; i < font->cmap->numberSubtables; i++)
    {
        TT_Table_Cmap_Sub *sub = font->cmap->subTables + i;
        sub->platformID = readUInt16(fp);
        sub->platformSpecificID = readUInt16(fp);
        sub->offset = readUInt(fp) + table->offset;
        int old = ftell(fp);
        fseek(fp, sub->offset, SEEK_SET);
        TT_UInt16 format = readUInt16(fp);
        if (format == 6)
        {
            sub->subType = SUB_6;
            sub->body = malloc(sizeof(TT_Table_Cmap_Sub6));
            Cmap_Read((TT_Table_Cmap_Sub6 *)(sub->body), fp);
        }
        printf("\t%d\n", format);
        fseek(fp, old, SEEK_SET);
    }
}

int getGlyphOffset(TT_Font *font, FILE *fp, int index)
{
    printf("get glyph offset index %d\n", index);
    TT_Table *loca = getTable(font, "loca");
    int offset = 0, old = 0;
    old = ftell(fp);
    if (font->head->indexToLocFormat == 1)
    {
        fseek(fp, loca->offset + index * 4, SEEK_SET);
        offset = readUInt(fp);
    }
    else
    {
        fseek(fp, loca->offset + index * 2, SEEK_SET);
        offset = readUInt16(fp) * 2;
    }
    fseek(fp, old, SEEK_SET);

    printf("get glyph offset %d\n", offset);
    TT_Table *glyf = getTable(font, "glyf");
    return offset + glyf->offset;
}

void readSimpleGlyph(TT_Font *font, FILE *fp, TT_Glyph *glyph)
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
        TT_UInt16 contour = readUInt16(fp);
        printf("contour %d\n", contour);
        if (contour > maxContour)
        {
            maxContour = contour;
        }
    }
    TT_UInt16 numPoints = maxContour + 1;
    // TODO skip instructions
    TT_UInt16 offset = readUInt16(fp);
    fseek(fp, offset, SEEK_CUR);
    TT_UInt8 *flags = (TT_UInt8 *)malloc(sizeof(TT_UInt8) * numPoints);
    TT_Point *points = (TT_Point *)malloc(sizeof(TT_Point) * numPoints);

    for (int i = 0; i < numPoints; i++)
    {
        flags[i] = readUInt8(fp);
        if (flags[i] & REPEAT)
        {
            TT_UInt8 count = readUInt8(fp);
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
                     ? ((flags[i] & X_DELTA) ? readUInt8(fp) : -readUInt8(fp))
                     : ((flags[i] & X_DELTA) ? 0 : readInt16(fp));
        points[i].x = value;
    }

    value = 0;
    for (int i = 0; i < numPoints; i++)
    {
        value += (flags[i] & Y_IS_BYTE)
                     ? ((flags[i] & Y_DELTA) ? readUInt8(fp) : -readUInt8(fp))
                     : ((flags[i] & Y_DELTA) ? 0 : readInt16(fp));
        points[i].y = value;
    }

    printf("points: \n");
    for (int i = 0; i < numPoints; i++)
    {
        printf("%d: %d(%d, %d)\n", i, flags[i] & ON_CURVE, points[i].x, points[i].y);
    }

    free(flags);
}

void TT_New_Font(TT_Font *font, FILE *fp)
{
    font->scaleType = readUInt(fp);
    font->numTables = readUInt16(fp);
    font->searchRange = readUInt16(fp);
    font->entrySelector = readUInt16(fp);
    font->rangeShift = readUInt16(fp);

    printf("font: %d, %d, %d, %d, %d\n", font->scaleType, font->numTables,
           font->searchRange, font->entrySelector, font->rangeShift);

    font->tables = (TT_Table *)malloc(sizeof(TT_Table) * font->numTables);

    for (int i = 0; i < font->numTables; i++)
    {
        TT_Table *table = &font->tables[i];
        table->name = (char *)malloc(sizeof(char) * 5);
        readString(fp, table->name, 4);
        table->checkSum = readUInt(fp);
        table->offset = readUInt(fp);
        table->length = readUInt(fp);
        printf("name is %s, %d, %d, %d\n",
               table->name, table->checkSum, table->offset, table->length);
    }

    TT_Table *table = getTable(font, "head");
    printf("search for table name %s\n", table->name);
    fseek(fp, table->offset, SEEK_SET);
    font->head = (TT_Table_Head *)malloc(sizeof(TT_Table_Head));
    font->head->version = readFixed(fp);
    font->head->fontRevision = readFixed(fp);
    font->head->checkSumAdjustment = readUInt(fp);
    font->head->magicNumber = readUInt(fp);
    font->head->flags = readUInt16(fp);
    font->head->unitsPerEm = readUInt16(fp);
    font->head->created = readDate(fp);
    font->head->modified = readDate(fp);
    font->head->xMin = readFWord(fp);
    font->head->yMin = readFWord(fp);
    font->head->xMax = readFWord(fp);
    font->head->yMax = readFWord(fp);
    font->head->macStyle = readUInt16(fp);
    font->head->lowestRecPPEM = readUInt16(fp);
    font->head->fontDirectionHint = readInt16(fp);
    font->head->indexToLocFormat = readInt16(fp);
    font->head->glyphDataFormat = readInt16(fp);
    printf("head: magicNumber %8x, indexToLocFormat %d\n",
           font->head->magicNumber, font->head->indexToLocFormat);

    printf("glyph count %d\n", glyphCount(font, fp));
    readCmap(font, fp);
}

int TT_Get_Char_Index(TT_Font *font, FILE *fp, long code)
{
    TT_Table_Cmap *cmap = font->cmap;

    for (int i = 0; i < cmap->numberSubtables; i++)
    {
        if (cmap->subTables[i].subType == SUB_6)
        {
            TT_Table_Cmap_Sub6 *sub6 = cmap->subTables[i].body;
            int index = Cmap_GetGlyph(sub6, code);
            printf("get char index for %ld, %d\n", code, index);
            return index;
        }
    }
    return 0;
}

TT_Error TT_Get_Glyph(TT_Font *font, FILE *fp, TT_Glyph *glyph, int index)
{
    int offset = getGlyphOffset(font, fp, index);
    TT_Table *glyf = getTable(font, "glyf");
    if (offset < glyf->offset || offset >= glyf->offset + glyf->length)
    {
        return TT_Error_NoGlyph;
    }
    fseek(fp, offset, SEEK_SET);
    glyph->numberOfContours = readInt16(fp);
    glyph->xMin = readFWord(fp);
    glyph->yMin = readFWord(fp);
    glyph->xMax = readFWord(fp);
    glyph->yMax = readFWord(fp);
    printf("glyph: %d, %d, %d, %d, %d\n", glyph->numberOfContours,
           glyph->xMin, glyph->yMin, glyph->xMax, glyph->yMax);

    // TODO deal with compound glyph
    if (glyph->numberOfContours == -1)
    {
    }
    else
    {
        readSimpleGlyph(font, fp, glyph);
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
            // TODO release cmap for format 4
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