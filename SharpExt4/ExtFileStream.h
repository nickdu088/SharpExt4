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
	public:
		ExtFileStream(ExtFileSystem^ fs, String^ path, FileMode mode, FileAccess access);
		~ExtFileStream();
		property bool CanSeek { bool get() override; };
		property bool CanRead {			bool get() override;		};
		property bool CanWrite {			bool get() override;		};
		property int64_t Length {			int64_t get() override;		};
		property int64_t Position {			int64_t get() override; void set(int64_t) override;		};

		int Read(array<uint8_t>^ array, int offset, int count) override;
		void Write(array<uint8_t>^ array, int offset, int count) override;
		void Close() override;
		int64_t Seek(int64_t offset, SeekOrigin origin) override;
		void Flush() override;
		void SetLength(int64_t value) override;
	};
}

