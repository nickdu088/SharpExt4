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
#include "../lwext4/include/ext4.h"
#include "ExtFileSystem.h"

using namespace System;
using namespace System::IO;

namespace SharpExt4 {
	public ref class ExtFileStream : Stream
	{
	private:
		FileAccess access;
		FileMode mode;
		ExtFileSystem^ fs;
		ext4_file* file = nullptr;
		String^ path;

	public:
		ExtFileStream(ExtFileSystem^ fs, String^ path, FileMode mode, FileAccess access);
		~ExtFileStream();
		property bool CanSeek { bool get() override; };
		property bool CanRead {	bool get() override; };
		property bool CanWrite { bool get() override; };
		property int64_t Length { int64_t get() override; };
		property int64_t Position { int64_t get() override; void set(int64_t) override; };

		int Read(array<uint8_t>^ array, int offset, int count) override;
		void Write(array<uint8_t>^ array, int offset, int count) override;
		void Close() override;
		int64_t Seek(int64_t offset, SeekOrigin origin) override;
		void Flush() override;
		void SetLength(int64_t value) override;
	};
}

