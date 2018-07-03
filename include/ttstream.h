#ifndef TTSTREAM_H
#define TTSTREAM_H

#include <stdio.h>
#include <stdlib.h>

enum TT_Stream_Type
{
    TT_Stream_Type_Memory,
    TT_Stream_Type_File,
};

typedef struct
{
    int type;

    unsigned char *base;
    unsigned long size;
    unsigned long pos;

    // file type only
    FILE *fp;
} TT_Stream;

#include <ttdef.h>
#include <stdio.h>

void ttopenToMem(TT_Stream *stream, const char *filename);
void ttopen(TT_Stream *stream, const char *filename);
size_t ttread(void *result, size_t size, size_t nitems, TT_Stream *stream);
void ttclose(TT_Stream *stream);
long tttell(TT_Stream *stream);
void ttseek(TT_Stream *stream, long pos);

TT_UInt8 readUInt8(TT_Stream *fp);
TT_UInt readUInt(TT_Stream *fp);
TT_UInt16 readUInt16(TT_Stream *fp);
TT_Int16 readInt16(TT_Stream *fp);
void readString(TT_Stream *fp, char *str, int length);
TT_Fixed readFixed(TT_Stream *fp);
TT_FWord readFWord(TT_Stream *fp);
TT_Date readDate(TT_Stream *fp);

#endif
