/*
 * Copyright (c) 2021 Nick Du (nick@nickdu.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ExtFileStream.h"
#include "ExtFileSystem.h"
#include <string.h>

using namespace System::Runtime::InteropServices;

SharpExt4::ExtFileStream::ExtFileStream(SharpExt4::ExtFileSystem^ fs, String^ path, FileMode mode, FileAccess access)
    :access(access), mode(mode), fs(fs), path(path)
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
    file = new ext4_file();
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(fs->MountPoint, path)).ToPointer();

    if (!fs->FileExists(path))
    {
        if (mode == FileMode::Open)
        {
            throw gcnew FileNotFoundException("No such file", path);
        }
        else
        {
            //create new file
            ext4_fopen(file, internalPath, "wb+");
        }
    }
    else if (mode == FileMode::CreateNew)
    {
        throw gcnew IOException("File already exists");
    }

    ext4_fopen(file, internalPath, "rb+");

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
    if (ext4_fread(file, buf, count, &rcnt) != EOK)
    {
        throw gcnew IOException("Could not read file '" + path + "'.");
    }
    return rcnt;
}

void SharpExt4::ExtFileStream::Write(array<uint8_t>^ array, int offset, int count)
{
    size_t wcnt = 0;
    pin_ptr<uint8_t> p = &array[offset];
    uint8_t* buf =p;
    if (ext4_fwrite(file, buf, count, &wcnt) != EOK)
    {
        throw gcnew IOException("Could not write file '" + path + "'.");
    }
}

void SharpExt4::ExtFileStream::Close()
{
    if (ext4_fclose(file) != EOK)
    {
        throw gcnew IOException("Could not close file '" + path + "'.");
    }
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


