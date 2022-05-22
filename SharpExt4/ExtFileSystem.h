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

#pragma once
using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Text::RegularExpressions;

#include "Partition.h"
#include "ExtDirEntry.h"
#include "ExtDisk.h"

namespace SharpExt4 {
	ref class ExtFileStream;

	public ref class ExtFileSystem sealed
	{
	private:
		String^ mountPoint = "/";
		char* devName = nullptr;
		static DateTime TheEpoch = DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind::Utc);
		SharpExt4::ExtDisk^ disk = nullptr;
		ExtFileSystem(ExtDisk^ disk);
		void DoSearch(List<String^>^ results, String^ path, Regex^ regex, bool subFolders, bool dirs, bool files);
		List<ExtDirEntry^>^ GetDirectory(String^ path);

	public:
#pragma region File related API
		void CopyFile(String^ sourceFile, String^ destinationFile, bool overwrite);
		void RenameFile(String^ sourceFileName, String^ destFileName);
		void DeleteFile(String^ path);
		bool FileExists(String^ path);
		String^ ReadSymLink(String^ path);
		array<String^>^ GetFiles(String^ path, String^ searchPattern, SearchOption searchOption);
		SharpExt4::ExtFileStream^ OpenFile(String^ path, FileMode mode, FileAccess access);
#pragma endregion

#pragma region Directory related API
		void CreateDirectory(String^ path);
		void DeleteDirectory(String^ path);
		bool DirectoryExists(String^ path);
		array<String^>^ GetDirectories(String^ path, String^ searchPattern, SearchOption searchOption); 
		void MoveDirectory(String^ sourceDirectoryName, String^ destinationDirectoryName);
#pragma endregion

#pragma region Common API
		DateTime^ GetCreationTime(String^ path);
		void SetCreationTime(String^ path, DateTime newTime);
		DateTime GetLastAccessTime(String^ path);
		void SetLastAccessTime(String^ path, DateTime newTime);
		DateTime GetLastWriteTime(String^ path);
		void SetLastWriteTime(String^ path, DateTime newTime);
		uint64_t GetFileLength(String^ path);
		void CreateSymLink(String^ target, String^ path);
		void CreateHardLink(String^ target, String^ path);
		uint32_t GetMode(String^ path);
		void SetMode(String^ path, uint32_t mode);
		Tuple<uint32_t, uint32_t>^ GetOwner(String^ path);
		void SetOwner(String^ path, uint32_t uid, uint32_t gid);
		void Truncate(String^ path, uint64_t size);
		static ExtFileSystem^ Open(SharpExt4::ExtDisk^ disk, SharpExt4::Partition^ partition);
#pragma endregion

		property String^ Name { String^ get(); }
		property String^ Description { String^ get(); }
		property String^ VolumeLabel { String^ get(); }
		property bool CanWrite { bool get(); }
		property String^ MountPoint { String^ get(); }

		String^ ToString() override;
		~ExtFileSystem();
	};

	Regex^ ConvertWildcardsToRegEx(String^ pattern);
	String^ CombinePaths(String^ a, String^ b);
}

