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
		static ExtDisk^ Open(String^ path);

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
