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
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(mountPoint).ToPointer();
    struct ext4_mount_stats stats;
    ext4_mount_point_stats(input_name, &stats);
    return gcnew String(stats.volume_name);
}

bool SharpExt4::ExtFileSystem::CanWrite::get()
{
    return !bd->fs->read_only;
}

uint64_t SharpExt4::ExtFileSystem::GetFileLength(String^ path)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_file* f = nullptr;
    ext4_fopen(f, input_name, "r");
    auto size = ext4_fsize(f);
    ext4_fclose(f);
    return size;
}


SharpExt4::ExtFileSystem^ SharpExt4::ExtFileSystem::Open(SharpExt4::Partition^ volume)
{
    auto fs = gcnew ExtFileSystem();

    struct ext4_bcache* bc = nullptr;
    fs->bd->part_offset = volume->Offset;
    fs->bd->part_size = volume->Size;

    auto r = ext4_device_register(fs->bd, bc ? bc : 0, "ext4_fs");
    if (r != EOK) {
        printf("ext4_device_register: rc = %d\n", r);
        return nullptr;
    }

    auto input_name = (char*)Marshal::StringToHGlobalAnsi(fs->mountPoint).ToPointer();
    r = ext4_mount("ext4_fs", input_name, false);
    if (r != EOK) {
        printf("ext4_mount: rc = %d\n", r);
        return nullptr;
    }
    return fs;
}

String^ SharpExt4::ExtFileSystem::ToString()
{
    return Name;
}

SharpExt4::ExtFileSystem::~ExtFileSystem()
{
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
    String^ query = /*"^" + */Regex::Escape(pattern)->Replace("\\*", ".*")->Replace("\\.", ".*")/* + "$"*/;
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

String^ SharpExt4::ExtFileSystem::GetFileFromPath(String^ path)
{
    auto trimmed = path->Trim('/');

    int index = trimmed->LastIndexOf('/');
    if (index < 0)
    {
        return trimmed; // No directory, just a file name
    }

    return trimmed->Substring(index + 1);
}

String^ SharpExt4::ExtFileSystem::GetDirectoryFromPath(String^ path)
{
    auto trimmed = path->TrimEnd('/');

    int index = trimmed->LastIndexOf('/');
    if (index < 0)
    {
        return String::Empty; // No directory, just a file name
    }

    return trimmed->Substring(0, index);
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
        //printf("  %s%s\n", entry_to_str(de->inode_type), sss);
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
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_dir_mk(input_name);
}

void SharpExt4::ExtFileSystem::CopyFile(String^ sourceFile, String^ destinationFile, bool overwrite)
{
    throw gcnew System::NotImplementedException();
}

void SharpExt4::ExtFileSystem::DeleteDirectory(String^ path)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_dir_rm(input_name);
}


void SharpExt4::ExtFileSystem::DeleteFile(String^ path)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_fremove(input_name);
}

bool SharpExt4::ExtFileSystem::DirectoryExists(String^ path)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_dir* d = nullptr;
    ext4_dir_open(d, input_name);
    if (d == nullptr)
    {
        return false;
    }
    ext4_dir_close(d);
    return true;
}

bool SharpExt4::ExtFileSystem::FileExists(String^ path)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_file f = { 0 };
    if (ext4_fopen(&f, input_name, "rb") == EOK)
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
    return FileAttributes();
}

void SharpExt4::ExtFileSystem::SetAttributes(String^ path, FileAttributes newValue)
{
    throw gcnew System::NotImplementedException();
}

DateTime^ SharpExt4::ExtFileSystem::GetCreationTime(String^ path)
{
    throw gcnew System::NotImplementedException();
    // TODO: insert return statement here
}

void SharpExt4::ExtFileSystem::SetCreationTime(String^ path, DateTime^ newTime)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer(); 
    ext4_file_set_ctime(input_name, newTime->ToFileTime());
}

DateTime^ SharpExt4::ExtFileSystem::GetLastAccessTime(String^ path)
{
    throw gcnew System::NotImplementedException();
    // TODO: insert return statement here
}

void SharpExt4::ExtFileSystem::SetLastAccessTime(String^ path, DateTime^ newTime)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_file_set_atime(input_name, newTime->ToFileTime());
}

DateTime^ SharpExt4::ExtFileSystem::GetLastWriteTime(String^ path)
{
    throw gcnew System::NotImplementedException();
    // TODO: insert return statement here
}

void SharpExt4::ExtFileSystem::SetLastWriteTime(String^ path, DateTime^ newTime)
{
    auto newPath = CombinePaths(mountPoint, path);
    auto input_name = (char*)Marshal::StringToHGlobalAnsi(newPath).ToPointer();
    ext4_file_set_mtime(input_name, newTime->ToFileTime());
}

