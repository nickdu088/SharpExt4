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
#include <stdlib.h>

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
    struct ext4_mount_stats stats = { 0 };
    ext4_mount_point_stats(root, &stats);
    return gcnew String(stats.volume_name);
}

bool SharpExt4::ExtFileSystem::CanWrite::get()
{
    return !disk->GetBlockDev()->fs->read_only;
}

/// <summary>
/// Get specific file length
/// </summary>
/// <param name="path">file path</param>
/// <returns>file length</returns>
uint64_t SharpExt4::ExtFileSystem::GetFileLength(String^ path)
{
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_file f = { 0 };
    auto r = ext4_fopen(&f, internalPath, "r");
    if (r == EOK)
    {
        auto size = ext4_fsize(&f);
        ext4_fclose(&f);
        return size;
    }

    throw gcnew IOException("Could not open file '" + path + "'.");
}

/// <summary>
/// Create a symbolic link in file system
/// </summary>
/// <param name="target">path of target</param>
/// <param name="path">link name</param>
void SharpExt4::ExtFileSystem::CreateSymLink(String^ target, String^ path)
{
    auto newTarget = (char*)Marshal::StringToHGlobalAnsi(target).ToPointer();
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_fsymlink(newTarget, newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not create symbolic link for '" + path + "'.");
    }
}

/// <summary>
/// Create a hard link in file system
/// </summary>
/// <param name="target">absolute path of target</param>
/// <param name="path">link name</param>
void SharpExt4::ExtFileSystem::CreateHardLink(String^ target, String^ path)
{
    auto newTarget = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, target)).ToPointer();
    auto newPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_flink(newTarget, newPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not create hard lik for '" + path + "'.");
    }
}

/// <summary>
/// Get the permissons of a file or directory
/// </summary>
/// <param name="path">the path to a file or directory</param>
uint32_t SharpExt4::ExtFileSystem::GetMode(String^ path)
{
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    uint32_t mode = 0;
    auto r = ext4_mode_get(internalPath, &mode);
    if (r != EOK)
    {
        throw gcnew IOException("Could not get mode '" + path + "'.");
    }
    return mode;
}

/// <summary>
/// Set the permissons of a file or directory
/// </summary>
/// <param name="path">the path to a file or directory</param>
void SharpExt4::ExtFileSystem::SetMode(String^ path, uint32_t mode)
{
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_mode_set(internalPath, mode);
    if (r != EOK)
    {
        throw gcnew IOException("Could not change mode '" + path + "'.");
    }
}

/// <summary>
/// Get the user and/or group ownership of a file or directory
/// </summary>
/// <param name="path">the path to a file or directory</param>
Tuple<uint32_t, uint32_t>^ SharpExt4::ExtFileSystem::GetOwner(String^ path)
{
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    uint32_t uid = 0;
    uint32_t gid = 0;

    auto r = ext4_owner_get(internalPath, &uid, &gid);
    if (r != EOK)
    {
        throw gcnew IOException("Could not get owner '" + path + "'.");
    }
    return gcnew Tuple<uint32_t, uint32_t>(uid, gid);
}

/// <summary>
/// Set the user and/or group ownership of a file or directory
/// </summary>
/// <param name="path">the path to a file or directory</param>
void SharpExt4::ExtFileSystem::SetOwner(String^ path, uint32_t uid, uint32_t gid)
{
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_owner_set(internalPath, uid, gid);
    if (r != EOK)
    {
        throw gcnew IOException("Could not change mode '" + path + "'.");
    }
}

/// <summary>
/// Shrink or extend the size of given file to the specified size
/// </summary>
/// <param name="path">the path to a file</param>
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

/// <summary>
/// Open a given Linux partition
/// </summary>
/// <param name="path">Partition to open</param>
SharpExt4::ExtFileSystem^ SharpExt4::ExtFileSystem::Open(SharpExt4::ExtDisk^ disk, SharpExt4::Partition^ partition)
{
    auto fs = gcnew ExtFileSystem(disk);

    struct ext4_bcache* bc = nullptr;
    disk->GetBlockDev()->part_offset = partition->Offset;
    disk->GetBlockDev()->part_size = partition->Size;
    itoa(rand(), fs->devName, 16);
    fs->mountPoint = String::Format("/{0}/", gcnew String(fs->devName));
    auto r = ext4_device_register(disk->GetBlockDev(), fs->devName);
    if (r == EOK)
    {
        auto input_name = (char*)Marshal::StringToHGlobalAnsi(fs->mountPoint).ToPointer();
        r = ext4_mount(fs->devName, input_name, false);
        if (r == EOK)
        {
            return fs;
        }
        throw gcnew IOException("Could not mount partition.");
    }

    throw gcnew IOException("Could not register partition.");
}

/// <summary>
/// Override ToString
/// </summary>
String^ SharpExt4::ExtFileSystem::ToString()
{
    return Name;
}

/// <summary>
/// ExtFileSystem destructor
/// </summary>
SharpExt4::ExtFileSystem::~ExtFileSystem()
{
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(mountPoint).ToPointer();
    ext4_umount(input_name);
    ext4_device_unregister(devName);

    ext4_block_fini(disk->GetBlockDev());
    delete[] devName;
}

/// <summary>
/// ExtFileSystem constructor
/// </summary>
SharpExt4::ExtFileSystem::ExtFileSystem(ExtDisk^ disk)
{
    this->disk = disk;
    ext4_block_init(disk->GetBlockDev());
    devName = new char[CONFIG_EXT4_MAX_BLOCKDEV_NAME];
    memset(devName, 0, CONFIG_EXT4_MAX_BLOCKDEV_NAME);
}

Regex^ SharpExt4::ConvertWildcardsToRegEx(String^ pattern)
{
    if (!pattern->Contains("."))
    {
        pattern += ".";
    }

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
    if (String::IsNullOrEmpty(sourceFile) || String::IsNullOrEmpty(destinationFile))
    {
        throw gcnew ArgumentNullException("sourceFileName or destFileName is null.");
    }

    if (!FileExists(sourceFile))
    {
        throw gcnew IOException("Could not open file '" + sourceFile + "'.");
    }

    if (FileExists(destinationFile))
    {
        if (!overwrite)
        {
            throw gcnew IOException("File exists '" + destinationFile + "'.");
        }

        DeleteFile(destinationFile);
    }

    int bufferSize = 64 * 1024; //64k buffer
    auto sour = OpenFile(sourceFile, FileMode::Open, FileAccess::Read);
    auto dest = OpenFile(destinationFile, FileMode::CreateNew, FileAccess::Write);
    auto buf = gcnew array<Byte>(bufferSize);

    int bytesRead = -1;
    while ((bytesRead = sour->Read(buf, 0, bufferSize)) > 0)
    {
        dest->Write(buf, 0, bytesRead);
    }

    sour->Close();
    dest->Close();
}

void SharpExt4::ExtFileSystem::RenameFile(String^ sourceFileName, String^ destFileName)
{
    if (String::IsNullOrEmpty(sourceFileName) || String::IsNullOrEmpty(destFileName))
    {
        throw gcnew ArgumentNullException("sourceFileName or destFileName is null.");
    }

    if (!FileExists(sourceFileName))
    {
        throw gcnew FileNotFoundException("Could not find file '" + sourceFileName + "'.");
    }

    if (FileExists(destFileName))
    {
        throw gcnew IOException("'" + destFileName + " already exists.");
    }

    auto newSourcePath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, sourceFileName)).ToPointer();
    auto newDestPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, destFileName)).ToPointer();
    if (ext4_frename(newSourcePath, newDestPath) != EOK)
    {
        throw gcnew IOException("Could not move file '" + sourceFileName +"'.");
    }
}

void SharpExt4::ExtFileSystem::DeleteDirectory(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_dir_rm(internalPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not delete directory '" + path + "'.");
    }
}


void SharpExt4::ExtFileSystem::DeleteFile(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_fremove(internalPath);
    if (r != EOK)
    {
        throw gcnew IOException("Could not delete file '" + path + "'.");
    }
}

bool SharpExt4::ExtFileSystem::DirectoryExists(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_dir d = { 0 };
    if (ext4_dir_open(&d, internalPath) == EOK)
    {
        ext4_dir_close(&d);
        return true;
    }
    return false;
}

bool SharpExt4::ExtFileSystem::FileExists(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    ext4_file f = { 0 };
    if (ext4_fopen(&f, internalPath, "rb") == EOK)
    {
        ext4_fclose(&f);
        return true;
    }
    return false;
}

String^ SharpExt4::ExtFileSystem::ReadSymLink(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }
    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    char buf[4096] = { 0 };
    size_t rcnt = 0;
    if (ext4_readlink(internalPath, buf, 4096, &rcnt) != EOK)
    {
        throw gcnew IOException("Could not read file '" + path +"'.");
    }
    return gcnew String(buf);
}

array<String^>^ SharpExt4::ExtFileSystem::GetDirectories(String^ path, String^ searchPattern, SearchOption searchOption)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto re = ConvertWildcardsToRegEx(searchPattern);

    auto dirs = gcnew List<String^>();
    auto internalPath = CombinePaths(mountPoint, path);
    DoSearch(dirs, internalPath, re, searchOption == SearchOption::AllDirectories, true, false);
    return dirs->ToArray();
}

array<String^>^ SharpExt4::ExtFileSystem::GetFiles(String^ path, String^ searchPattern, SearchOption searchOption)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto re = ConvertWildcardsToRegEx(searchPattern);

    auto results = gcnew List<String^>();
    auto internalPath = CombinePaths(mountPoint, path);
    DoSearch(results, internalPath, re, searchOption == SearchOption::AllDirectories, false, true);
    return results->ToArray();
}

void SharpExt4::ExtFileSystem::MoveDirectory(String^ sourceDirectoryName, String^ destinationDirectoryName)
{
    if (String::IsNullOrEmpty(sourceDirectoryName) || String::IsNullOrEmpty(destinationDirectoryName))
    {
        throw gcnew ArgumentNullException("sourceFileName or destFileName is null.");
    }

    if (!DirectoryExists(sourceDirectoryName))
    {
        throw gcnew FileNotFoundException("Could not find directory '" + sourceDirectoryName + "'.");
    }

    if (DirectoryExists(destinationDirectoryName))
    {
        throw gcnew IOException("'" + destinationDirectoryName + " already exists.");
    }

    auto newSourcePath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, sourceDirectoryName)).ToPointer();
    auto newDestPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, destinationDirectoryName)).ToPointer();

    if (ext4_dir_mv(newSourcePath, newDestPath) != EOK)
    {
        throw gcnew IOException("Could not move directory '" + sourceDirectoryName +"'");
    }
}

SharpExt4::ExtFileStream^ SharpExt4::ExtFileSystem::OpenFile(String^ path, FileMode mode, FileAccess access)
{
    return gcnew ExtFileStream(this, path, mode, access);
}

DateTime^ SharpExt4::ExtFileSystem::GetCreationTime(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    uint32_t ctime = 0;
    if (ext4_ctime_get(internalPath, &ctime) != EOK)
    {
        throw gcnew IOException("Could not get last access time.");
    }
    return TheEpoch.AddSeconds(ctime);
}

void SharpExt4::ExtFileSystem::SetCreationTime(String^ path, DateTime newTime)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_ctime_set(internalPath, (TheEpoch - newTime).TotalSeconds);
    if (r != EOK)
    {
        throw gcnew IOException("Could not set creation time.");
    }
}

DateTime SharpExt4::ExtFileSystem::GetLastAccessTime(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    uint32_t atime = 0;
    if (ext4_atime_get(internalPath, &atime) != EOK)
    {
        throw gcnew IOException("Could not get last access time.");
    }
    return TheEpoch.AddSeconds(atime);
}

void SharpExt4::ExtFileSystem::SetLastAccessTime(String^ path, DateTime newTime)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_atime_set(internalPath, (TheEpoch - newTime).TotalSeconds);
    if (r != EOK)
    {
        throw gcnew IOException("Could not set last access time.");
    }
}

DateTime SharpExt4::ExtFileSystem::GetLastWriteTime(String^ path)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    uint32_t mtime = 0;
    if (ext4_mtime_get(internalPath, &mtime) != EOK)
    {
        throw gcnew IOException("Could not get last access time.");
    }
    return TheEpoch.AddSeconds(mtime);
}

void SharpExt4::ExtFileSystem::SetLastWriteTime(String^ path, DateTime newTime)
{
    if (String::IsNullOrEmpty(path))
    {
        throw gcnew ArgumentNullException("path is null.");
    }

    auto internalPath = (char*)Marshal::StringToHGlobalAnsi(CombinePaths(mountPoint, path)).ToPointer();
    auto r = ext4_mtime_set(internalPath, (TheEpoch - newTime).TotalSeconds);
    if (r != EOK)
    {
        throw gcnew IOException("Could not set last write time.");
    }
}

