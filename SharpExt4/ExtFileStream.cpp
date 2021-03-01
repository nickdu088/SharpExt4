#include "ExtFileStream.h"
#include "ExtFileSystem.h"
#include <string.h>

using namespace System::Runtime::InteropServices;

SharpExt4::ExtFileStream::ExtFileStream(SharpExt4::ExtFileSystem^ fs, String^ path, FileMode mode, FileAccess access)
    :access(access), mode(mode), fs(fs)
{
    if (!fs->CanWrite)
    {
        if (mode != FileMode::Open)
        {
            throw gcnew NotSupportedException("Only existing files can be opened");
        }

        if (access != FileAccess::Read)
        {
            throw gcnew NotSupportedException("Files cannot be opened for write");
        }
    }
    auto newPath = CombinePaths(ExtFileSystem::MountPoint, path);
    file = new ext4_file();
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();

    if (!fs->FileExists(path))
    {
        if (mode == FileMode::Open)
        {
            throw gcnew FileNotFoundException("No such file", path);
        }
        else
        {
            //create new file
            ext4_fopen(file, input_name, "wb+");
        }
    }
    else if (mode == FileMode::CreateNew)
    {
        throw gcnew IOException("File already exists");
    }

    ext4_fopen(file, input_name, "rb+");

    if (mode == FileMode::Create || mode == FileMode::Truncate)
    {
        SetLength(0);
    }
}

bool SharpExt4::ExtFileStream::CanRead::get()
{
    return (access != FileAccess::Write);
}

bool SharpExt4::ExtFileStream::CanWrite::get()
{
    return (access != FileAccess::Read);
}

bool SharpExt4::ExtFileStream::CanSeek::get()
{
    return true;
}

int64_t SharpExt4::ExtFileStream::Length::get()
{
    return file->fsize;
}

int64_t SharpExt4::ExtFileStream::Position::get()
{
    return file->fpos;
}

void SharpExt4::ExtFileStream::Position::set(int64_t value)
{
    file->fpos = value;
}


SharpExt4::ExtFileStream::~ExtFileStream()
{
    Close();
}

int SharpExt4::ExtFileStream::Read(array<uint8_t>^ array, int offset, int count)
{
    size_t rcnt = 0;
    pin_ptr<uint8_t> p = &array[offset];
    uint8_t* buf = p;
    ext4_fread(file, buf, count, &rcnt);
    return rcnt;
}

void SharpExt4::ExtFileStream::Write(array<uint8_t>^ array, int offset, int count)
{
    size_t wcnt = 0;
    pin_ptr<uint8_t> p = &array[offset];
    uint8_t* buf =p;
    ext4_fwrite(file, buf, count, &wcnt);
}

void SharpExt4::ExtFileStream::Close()
{
    ext4_fclose(file);
    delete file;
}

int64_t SharpExt4::ExtFileStream::Seek(int64_t offset, SeekOrigin origin)
{
    return ext4_fseek(file, offset, (int)origin);
}

void SharpExt4::ExtFileStream::Flush()
{
}

void SharpExt4::ExtFileStream::SetLength(int64_t value)
{
    file->fsize = 0;
}


