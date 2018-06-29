#ifndef TTDEF_H
#define TTDEF_H

typedef int TT_Error;

typedef unsigned char   TT_UInt8;

typedef unsigned short  TT_UInt16;
typedef unsigned short  TT_2Dot14; // 2.14
typedef unsigned short  TT_FWord;
typedef signed short    TT_Int16;

typedef signed int      TT_Int;
typedef unsigned int    TT_UInt;
typedef TT_UInt         TT_UInt32;
typedef unsigned int    TT_Fixed; // 16.16

typedef long            TT_Date;
typedef unsigned long   TT_ULong;

#include <stdlib.h>
#include <stdio.h>

#include <file_reader.h>
#include <tterr.h>

#endif
