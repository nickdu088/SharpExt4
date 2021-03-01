#include "ExtDirEntry.h"

SharpExt4::ExtDirEntry::ExtDirEntry(String^ name, uint64_t length, EntryType type)
{
    Name = name;
    Length = length;
    Type = type;
}
