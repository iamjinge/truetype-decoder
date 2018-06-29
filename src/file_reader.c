#include <file_reader.h>

TT_UInt8 readUInt8(FILE *fp)
{
    TT_UInt8 result;
    fread(&result, 1, sizeof(result), fp);
    return result;
}

TT_UInt readUInt(FILE *fp)
{
    TT_UInt result = (readUInt8(fp) << 24) |
                     (readUInt8(fp) << 16) |
                     (readUInt8(fp) << 8) |
                     (readUInt8(fp));
    return result;
}

TT_UInt16 readUInt16(FILE *fp)
{
    TT_UInt16 result = (readUInt8(fp) << 8) |
                       (readUInt8(fp));
    return result;
}

TT_Int16 readInt16(FILE *fp)
{
    return readUInt16(fp);
}

void readString(FILE *fp, char *str, int length)
{
    for (int i = 0; i < length; i++)
    {
        str[i] = readUInt8(fp);
    }
    str[length] = 0;
}

TT_Fixed readFixed(FILE *fp)
{
    TT_Fixed result = readUInt(fp);
    return result;
}

TT_FWord readFWord(FILE *fp)
{
    return readInt16(fp);
}

TT_Date readDate(FILE *fp)
{
    TT_Date result = ((long)readUInt(fp) << 32) |
                     ((long)readUInt(fp));
    return result;
}
