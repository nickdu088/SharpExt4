#include "pch.h"

#include "ExtDisk.h"
#include "io_raw.h"

using namespace System::IO;

SharpExt4::ExtDisk::ExtDisk(String^ diskPath)
{
	diskPath = diskPath;
	auto input_name = (char*)Marshal::StringToHGlobalAnsi(diskPath).ToPointer();
	ext4_io_raw_filename(input_name);
	bd = ext4_io_raw_dev_get();
}

SharpExt4::ExtDisk^ SharpExt4::ExtDisk::Open(String^ path)
{
	if (File::Exists(path))
	{
		auto disk = gcnew ExtDisk(path);

		disk->bdevs = new ext4_mbr_bdevs();

		auto r = ext4_mbr_scan(disk->bd, disk->bdevs);
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
	return nullptr;
}

array<Byte>^ SharpExt4::ExtDisk::GetMasterBootRecord()
{
	auto r = ext4_mbr_scan(bd, bdevs);
	if (r != EOK) {
		return nullptr;
	}
	array<Byte>^ mbr = gcnew array<Byte>(512);
	Marshal::Copy((IntPtr)bd->bdif->ph_bbuf, mbr, 0, 512);
	return mbr;
}

SharpExt4::ExtDisk::~ExtDisk()
{
	bd->bdif->close(bd);
}

SharpExt4::ExtDisk^ SharpExt4::ExtDisk::Open(int DiskNumber)
{
	auto disk = gcnew ExtDisk(String::Format("PhysicalDrive{0}", DiskNumber));
	disk->bdevs = new ext4_mbr_bdevs();
	
	auto r = ext4_mbr_scan(disk->bd, disk->bdevs);
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

uint64_t SharpExt4::ExtDisk::Capacity::get()
{
	return capacity;
}

SharpExt4::Geometry^ SharpExt4::ExtDisk::Geometry::get()
{
	return geometry;
}

IList<SharpExt4::Partition^>^ SharpExt4::ExtDisk::Parititions::get()
{
	return partitions;
}