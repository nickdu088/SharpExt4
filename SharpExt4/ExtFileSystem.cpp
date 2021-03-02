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

#include <cstring>
#include "ExtFileSystem.h"
#include "../lwext4/include/ext4.h"
#include "../lwext4/include/ext4_fs.h"
#include "ExtFileStream.h"
#include "io_raw.h"

String^ SharpExt4::ExtFileSystem::MountPoint::get()
{
    return mountPoint;
}

String^ SharpExt4::ExtFileSystem::Name::get()
{
    return String::Empty;
}

String^ SharpExt4::ExtFileSystem::Description::get()
{
    return String::Empty;
}

String^ SharpExt4::ExtFileSystem::VolumeLabel::get()
{
    auto root = (char*)Marshal::StringToHGlobalAnsi(mountPoint).ToPointer();
    struct ext4_mount_stats stats;
    ext4_mount_point_stats(root, &stats);
    return gcnew String(stats.volume_name);
}

bool SharpExt4::ExtFileSystem::CanWrite::get()
{
    return !bd->fs->read_only;
}

uint64_t SharpExt4::ExtFileSystem::GetFileLength(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_file* f = nullptr;
    auto r = ext4_fopen(f, newPath, "r");
    if (r == EOK)
    {
        auto size = ext4_fsize(f);
        ext4_fclose(f);
        return size;
    }

    throw gcnew IOException("Could not open file '" + path + "'.");
}

void SharpExt4::ExtFileSystem::CreateSymLink(String^ target, String^ path)
{
    auto newTarget = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, target)).ToPointer();
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_fsymlink(newTarget, newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not create symbolic link for '" + path + "'.");
    }
}

void SharpExt4::ExtFileSystem::CreateHardLink(String^ path, String^ hardPath)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto newHardPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, hardPath)).ToPointer();
    auto r = ext4_flink(newPath, newHardPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not create hard lik for '" + path + "'.");
    }
}

void SharpExt4::ExtFileSystem::ChangeMode(String^ path, uint32_t mode)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_chmod(newPath, mode);
    if (r != EOK)
    {
        throw gcnew IOException("Could not change mode '" + path + "'.");
    }
}

void SharpExt4::ExtFileSystem::ChangeOwner(String^ path, uint32_t uid, uint32_t gid)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_chown(newPath, uid, gid);
    if (r != EOK)
    {
        throw gcnew IOException("Could not change mode '" + path + "'.");
    }
}

void SharpExt4::ExtFileSystem::Truncate(String^ path, uint64_t size)
{
    if (FileExists(path))
    {
        auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
        ext4_file f = { 0 };
        if (ext4_fopen(&f, newPath, "") == EOK)
        {
            ext4_ftruncate(&f, size);
            ext4_fclose(&f);
        }
        throw gcnew IOException("Could not open file '" + path + "'.");
    }
    throw gcnew FileNotFoundException("Could not find file '" + path + "'.");
}


SharpExt4::ExtFileSystem^ SharpExt4::ExtFileSystem::Open(SharpExt4::Partition^ partition)
{
    auto fs = gcnew ExtFileSystem();

    struct ext4_bcache* bc = nullptr;
    fs->bd->part_offset = partition->Offset;
    fs->bd->part_size = partition->Size;

    auto r = ext4_device_register(fs->bd, bc ? bc : 0, "ext4_fs");
    if (r == EOK)
    {
        auto input_name = (char*)Marshal::StringToHGlobalAnsi(fs->mountPoint).ToPointer();
        r = ext4_mount("ext4_fs", input_name, false);
        if (r == EOK)
        {
            return fs;
        }
    }

    throw gcnew IOException("Could not mount partition.");
}

String^ SharpExt4::ExtFileSystem::ToString()
{
    return Name;
}

SharpExt4::ExtFileSystem::~ExtFileSystem()
{
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(mountPoint).ToPointer();
    ext4_umount(input_name);

    ext4_block_fini(bd);
}

SharpExt4::ExtFileSystem::ExtFileSystem()
{
    bd = ext4_io_raw_dev_get();
    ext4_block_init(bd);
}

Regex^ SharpExt4::ConvertWildcardsToRegEx(String^ pattern)
{
    if (!pattern->Contains("."))
    {
        pattern += ".";
    }

    //String^ query = "^" + Regex::Escape(pattern)->Replace("\\*", ".*")->Replace("\\?", "[^.]") + "$";
    String^ query = "^" + Regex::Escape(pattern)->Replace("\\*", ".*")->Replace("\\.", ".*") + "$";
    return gcnew Regex(query, RegexOptions::IgnoreCase | RegexOptions::CultureInvariant);
}

String^ SharpExt4::CombinePaths(String^ a, String^ b)
{
    if (String::IsNullOrEmpty(a))
    {
        return b;
    }
    else if (String::IsNullOrEmpty(b))
    {
        return a;
    }
    else
    {
        return a->TrimEnd('/') + "/" + b->TrimStart('/');
    }
}

List<SharpExt4::ExtDirEntry^>^ SharpExt4::ExtFileSystem::GetDirectory(String^ path)
{
    auto result = gcnew List<ExtDirEntry^>();
    char sss[255];
    ext4_dir d;
    const ext4_direntry* de;

    auto input_name = (char*)Marshal::StringToHGlobalAnsi(path).ToPointer();

    ext4_dir_open(&d, input_name);
    de = ext4_dir_entry_next(&d);

    while (de) {
        memcpy(sss, de->name, de->name_length);
        sss[de->name_length] = 0;
        result->Add(gcnew ExtDirEntry(gcnew String(sss), de->entry_length, EntryType(de->inode_type)));
        de = ext4_dir_entry_next(&d);
    }
    ext4_dir_close(&d);
    return result;
}

void SharpExt4::ExtFileSystem::DoSearch(List<String^>^ results, String^ path, Regex^ regex, bool subFolders, bool dirs, bool files)
{
    auto parentDir = GetDirectory(path);
    if (parentDir == nullptr)
    {
        throw gcnew DirectoryNotFoundException(String::Format("The directory '{0}' was not found", path));
    }

    String^ resultPrefixPath = path->Substring(mountPoint->Length - 1);

    for each(auto de in parentDir)
    {
        if (de->Name->EndsWith("."))
            continue;

        bool isDir = (de->Type == EntryType::DIR);

        if ((isDir && dirs) || (!isDir && files))
        {
            if (regex->IsMatch(de->Name))
            {
                results->Add(CombinePaths(resultPrefixPath, de->Name));
            }
        }

        if (subFolders && isDir)
        {
            DoSearch(results, CombinePaths(path, de->Name), regex, subFolders, dirs, files);
        }
    }
}

void SharpExt4::ExtFileSystem::CreateDirectory(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_dir_mk(newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not create directory '" + path + "'.");
    }
}

void SharpExt4::ExtFileSystem::CopyFile(String^ sourceFile, String^ destinationFile, bool overwrite)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::DeleteDirectory(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_dir_rm(newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not delete directory '" + path + "'.");
    }
}


void SharpExt4::ExtFileSystem::DeleteFile(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_fremove(newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not delete file '" + path + "'.");
    }
}

bool SharpExt4::ExtFileSystem::DirectoryExists(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_dir d = { 0 };
    if (ext4_dir_open(&d, newPath) == EOK)
    {
        ext4_dir_close(&d);
        return true;
    }
    return false;
}

bool SharpExt4::ExtFileSystem::FileExists(String^ path)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_file f = { 0 };
    if (ext4_fopen(&f, newPath, "rb") == EOK)
    {
        ext4_fclose(&f);
        return true;
    }
    return false;
}

array<String^>^ SharpExt4::ExtFileSystem::GetDirectories(String^ path, String^ searchPattern, SearchOption searchOption)
{
    auto re = ConvertWildcardsToRegEx(searchPattern);

    auto dirs = gcnew List<String^>();
    auto newPath = CombinePaths(mountPoint, path);
    DoSearch(dirs, newPath, re, searchOption == SearchOption::AllDirectories, true, false);
    return dirs->ToArray();
}

array<String^>^ SharpExt4::ExtFileSystem::GetFiles(String^ path, String^ searchPattern, SearchOption searchOption)
{
    auto re = ConvertWildcardsToRegEx(searchPattern);

    auto results = gcnew List<String^>();
    auto newPath = CombinePaths(mountPoint, path);
    DoSearch(results, newPath, re, searchOption == SearchOption::AllDirectories, false, true);
    return results->ToArray();
}

void SharpExt4::ExtFileSystem::MoveDirectory(String^ sourceDirectoryName, String^ destinationDirectoryName)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::MoveFile(String^ sourceName, String^ destinationName, bool overwrite)
{
    throw gcnew System::NotImplementedException();
}

SharpExt4::ExtFileStream^ SharpExt4::ExtFileSystem::OpenFile(String^ path, FileMode mode, FileAccess access)
{
    return gcnew ExtFileStream(this, path, mode, access);
}

FileAttributes SharpExt4::ExtFileSystem::GetAttributes(String^ path)
{
    throw gcnew System::NotImplementedException(); return FileAttributes();
}

void SharpExt4::ExtFileSystem::SetAttributes(String^ path, FileAttributes newValue)
{
    throw gcnew System::NotImplementedException();
}

DateTime^ SharpExt4::ExtFileSystem::GetCreationTime(String^ path)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::SetCreationTime(String^ path, DateTime^ newTime)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_file_set_ctime(newPath, newTime->ToFileTime());
    if (r != EOK)
    {
        throw gcnew IOException("Could not set creation time.");
    }
}

DateTime^ SharpExt4::ExtFileSystem::GetLastAccessTime(String^ path)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::SetLastAccessTime(String^ path, DateTime^ newTime)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_file_set_atime(newPath, newTime->ToFileTime());
    if (r != EOK)
    {
        throw gcnew IOException("Could not set last access time.");
    }
}

DateTime^ SharpExt4::ExtFileSystem::GetLastWriteTime(String^ path)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::SetLastWriteTime(String^ path, DateTime^ newTime)
{
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_file_set_mtime(newPath, newTime->ToFileTime());
    if (r != EOK)
    {
        throw gcnew IOException("Could not set last write time.");
    }
}

