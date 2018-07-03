#include <ttstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ttopenToMem(TT_Stream *stream, const char *filename)
{
    stream->type = TT_Stream_Type_Memory;
    stream->fp = NULL;
    FILE *fp = fopen(filename, "rb");
    stream->pos = 0;
    fseek(fp, 0, SEEK_END);
    stream->size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    stream->base = malloc(stream->size);
    fread(stream->base, 1, stream->size, fp);
    fclose(fp);
}

void ttopen(TT_Stream *stream, const char *filename)
{
    stream->type = TT_Stream_Type_File;
    stream->fp = fopen(filename, "rb");
}

size_t ttread(void *result, size_t size, size_t nitems, TT_Stream *stream)
{
    if (stream->type == TT_Stream_Type_File)
    {
        return fread(result, size, nitems, stream->fp);
    }
    else
    {
        long allSize = size * nitems;
        void *after = memcpy(result, stream->base + stream->pos, allSize);
        stream->pos += allSize;
        return nitems;
    }
}

void ttclose(TT_Stream *stream)
{
    if (stream->type == TT_Stream_Type_File)
    {
        fclose(stream->fp);
    }
    else
    {
        free(stream->base);
    }
}

long tttell(TT_Stream *stream)
{
    if (stream->type == TT_Stream_Type_File)
    {
        return ftell(stream->fp);
    }
    else
    {
        return stream->pos;
    }
}

void ttseek(TT_Stream *stream, long pos)
{
    if (stream->type == TT_Stream_Type_File)
    {
        fseek(stream->fp, pos, SEEK_SET);
    }
    else
    {
        stream->pos = pos;
    }
}

TT_UInt8 readUInt8(TT_Stream *fp)
{
    TT_UInt8 result;
    ttread(&result, sizeof(result), 1, fp);
    return result;
}

TT_UInt readUInt(TT_Stream *fp)
{
    TT_UInt result = (readUInt8(fp) << 24) |
                     (readUInt8(fp) << 16) |
                     (readUInt8(fp) << 8) |
                     (readUInt8(fp));
    return result;
}

TT_UInt16 readUInt16(TT_Stream *fp)
{
    TT_UInt16 result = (readUInt8(fp) << 8) |
                       (readUInt8(fp));
    return result;
}

TT_Int16 readInt16(TT_Stream *fp)
{
    return readUInt16(fp);
}

void readString(TT_Stream *fp, char *str, int length)
{
    for (int i = 0; i < length; i++)
    {
        str[i] = readUInt8(fp);
    }
    str[length] = 0;
}

TT_Fixed readFixed(TT_Stream *fp)
{
    TT_Fixed result = readUInt(fp);
    return result;
}

TT_FWord readFWord(TT_Stream *fp)
{
    return readInt16(fp);
}

TT_Date readDate(TT_Stream *fp)
{
    TT_Date result = ((long)readUInt(fp) << 32) |
                     ((long)readUInt(fp));
    return result;
}