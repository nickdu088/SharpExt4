#pragma once

#include <tchar.h>
#include <stdint.h>
#include <Windows.h>
#include "Geometry.h"
#include "../lwext4/include/ext4.h"
#include "../lwext4/include/ext4_blockdev.h"
#include "../lwext4/include/ext4_mbr.h"
#include "Partition.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

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

namespace SharpExt4 {
	public ref class ExtDisk sealed : IDisposable
	{
	private:
		struct ext4_mbr_bdevs* bdevs;
		struct ext4_blockdev* bd;

		uint64_t capacity;
		Geometry^ geometry;
		String^ diskPath;
		ExtDisk(String^ diskPath);
		IList<Partition^>^ partitions;

	public:
		static ExtDisk^ Open(int DiskNumber);
		static ExtDisk^ Open(String^ imagePath);

		property uint64_t Capacity
		{
			uint64_t get();
		}

		property Geometry^ Geometry
		{
			SharpExt4::Geometry^ get();
		}

		property IList<Partition^>^ Parititions
		{
			IList<Partition^>^ get();
		}

		array<Byte>^ GetMasterBootRecord();
		~ExtDisk();
	};
}
