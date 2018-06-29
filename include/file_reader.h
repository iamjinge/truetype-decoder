#ifndef FILE_READER_H
#define FILE_READER_H

#include <ttdef.h>
#include <stdio.h>

TT_UInt8 readUInt8(FILE *fp);
TT_UInt readUInt(FILE *fp);
TT_UInt16 readUInt16(FILE *fp);
TT_Int16 readInt16(FILE *fp);
void readString(FILE *fp, char *str, int length);
TT_Fixed readFixed(FILE *fp);
TT_FWord readFWord(FILE *fp);
TT_Date readDate(FILE *fp);

#endif
