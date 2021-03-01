#pragma once
using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Text::RegularExpressions;

#include "Partition.h"
#include "../lwext4/include/ext4_blockdev.h"
#include "ExtDirEntry.h"

namespace SharpExt4 {
	ref class ExtFileStream;

	public ref class ExtFileSystem sealed
	{
	private:
		static String^ mountPoint = "/mnt/";
		struct ext4_blockdev* bd;
		ExtFileSystem();
		void DoSearch(List<String^>^ results, String^ path, Regex^ regex, bool subFolders, bool dirs, bool files);
		//Regex^ ConvertWildcardsToRegEx(String^ pattern);
		String^ GetFileFromPath(String^ path);
		String^ GetDirectoryFromPath(String^ path);
		List<ExtDirEntry^>^ GetDirectory(String^ path);

	public:
		void CreateDirectory(String^ path);
		void CopyFile(String^ sourceFile, String^ destinationFile, bool overwrite);
		void DeleteDirectory(String^ path);
		void DeleteFile(String^ path);
		bool DirectoryExists(String^ path);
		bool FileExists(String^ path);
		array<String^>^ GetDirectories(String^ path, String^ searchPattern, SearchOption searchOption); 
		array<String^>^ GetFiles(String^ path, String^ searchPattern, SearchOption searchOption);
		void MoveDirectory(String^ sourceDirectoryName, String^ destinationDirectoryName);
		void MoveFile(String^ sourceName, String^ destinationName, bool overwrite);
		SharpExt4::ExtFileStream^ OpenFile(String^ path, FileMode mode, FileAccess access);
		FileAttributes GetAttributes(String^ path);
		void SetAttributes(String^ path, FileAttributes newValue);
		DateTime^ GetCreationTime(String^ path);
		void SetCreationTime(String^ path, DateTime^ newTime);
		DateTime^ GetLastAccessTime(String^ path);
		void SetLastAccessTime(String^ path, DateTime^ newTime);
		DateTime^ GetLastWriteTime(String^ path);
		void SetLastWriteTime(String^ path, DateTime^ newTime);
		uint64_t GetFileLength(String^ path);

		static ExtFileSystem^ Open(SharpExt4::Partition^ volume);

		property String^ Name { String^ get(); }
		property String^ Description { String^ get(); }
		property String^ VolumeLabel { String^ get(); }
		
		String^ ToString() override;
		property bool CanWrite { bool get(); }
		~ExtFileSystem();
		static property String^ MountPoint { String^ get(); }
	};
	Regex^ ConvertWildcardsToRegEx(String^ pattern);
	String^ CombinePaths(String^ a, String^ b);
}

