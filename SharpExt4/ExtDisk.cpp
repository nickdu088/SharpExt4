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

#include "pch.h"
#include "ExtDisk.h"
#include "io_raw.h"

using namespace System::IO;

/// <summary>
/// Constructor for ExtDisk
/// </summary>
/// <param name="diskPath">Linux disk path</param>
SharpExt4::ExtDisk::ExtDisk(String^ diskPath)
{
	diskPath = diskPath;
	auto input_name = (char*)Marshal::StringToHGlobalAnsi(diskPath).ToPointer();
	//ext4_io_raw_filename(input_name);
	bd = ext4_io_raw_dev_get(input_name);
}

/// <summary>
/// To provide Linux disk image file access
/// </summary>
/// <param name="imagePath">Linux disk image file name</param>
/// <returns></returns>
SharpExt4::ExtDisk^ SharpExt4::ExtDisk::Open(String^ imagePath)
{
	if (File::Exists(imagePath))
	{
		auto disk = gcnew ExtDisk(imagePath);
		disk->bdevs = new ext4_mbr_bdevs();

		auto r = ext4_mbr_scan(disk->bd, disk->bdevs);
		if (r == EOK)
		{
			disk->capacity = disk->bd->part_offset;
			disk->geometry = gcnew SharpExt4::Geometry(disk->bd->part_size, disk->bd->bdif->ph_tcnt, disk->bd->bdif->ph_scnt, disk->bd->bdif->ph_bsize);

			disk->partitions = gcnew List<Partition^>();

			for (auto par : disk->bdevs->partitions)
			{
				Partition^ partition = gcnew Partition();
				partition->Offset = par.part_offset;
				partition->Size = par.part_size;
				disk->partitions->Add(partition);
			}
			return disk;
		}
		throw gcnew IOException("Could not read disk MBR.");
	}
	throw gcnew FileNotFoundException("Could not find file '" + imagePath +"'.");
}

/// <summary>
/// Get Linux disk MBR
/// </summary>
/// <returns>MBR sector</returns>
array<Byte>^ SharpExt4::ExtDisk::GetMasterBootRecord()
{
	auto r = ext4_mbr_scan(bd, bdevs);
	if (r == EOK) 
	{
		array<Byte>^ mbr = gcnew array<Byte>(512);
		Marshal::Copy((IntPtr)bd->bdif->ph_bbuf, mbr, 0, 512);
		return mbr;
	}
	throw gcnew IOException("Could not read disk MBR.");
}

/// <summary>
/// Destructor of ExtDisk
/// </summary>
SharpExt4::ExtDisk::~ExtDisk()
{
	ext4_block_fini(bd);
	delete bdevs;
}

/// <summary>
/// To provide physical Linux disk access
/// </summary>
/// <param name="DiskNumber">Physical Disk Number</param>
/// <returns>ExtDisk</returns>
SharpExt4::ExtDisk^ SharpExt4::ExtDisk::Open(int DiskNumber)
{
	auto disk = gcnew ExtDisk(String::Format("PhysicalDrive{0}", DiskNumber));
	disk->bdevs = new ext4_mbr_bdevs();
	
	auto r = ext4_mbr_scan(disk->bd, disk->bdevs);
	if (r == EOK)
	{
		disk->capacity = disk->bd->part_offset;
		disk->geometry = gcnew SharpExt4::Geometry(disk->bd->part_size, disk->bd->bdif->ph_tcnt, disk->bd->bdif->ph_scnt, disk->bd->bdif->ph_bsize);

		disk->partitions = gcnew List<Partition^>();

		for (auto par : disk->bdevs->partitions)
		{
			Partition^ partition = gcnew Partition();
			partition->Offset = par.part_offset;
			partition->Size = par.part_size;
			disk->partitions->Add(partition);
		}
		return disk;
	}
	throw gcnew IOException("Could not read disk MBR.");
}

/// <summary>
/// Linux disk capacity
/// </summary>
uint64_t SharpExt4::ExtDisk::Capacity::get()
{
	return capacity;
}

/// <summary>
/// Linux disk geometry
/// </summary>
SharpExt4::Geometry^ SharpExt4::ExtDisk::Geometry::get()
{
	return geometry;
}

/// <summary>
/// Linux disk partitions
/// </summary>
IList<SharpExt4::Partition^>^ SharpExt4::ExtDisk::Partitions::get()
{
	return partitions;
}