/*
 * Copyright (c) 2013 Grzegorz Kostka (kostka.grzegorz@gmail.com)
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

#include "../lwext4/include/ext4_config.h"
#include "../lwext4/include/ext4_blockdev.h"
#include "../lwext4/include/ext4_errno.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>

 /**@brief   Default filename.*/
//static const char* fname = "ext2";

/**@brief   IO block size.*/
#define EXT4_IORAW_BSIZE 512

/**@brief   Image file descriptor.*/
//static HANDLE dev_file;

/**********************BLOCKDEV INTERFACE**************************************/
static int io_raw_open(struct ext4_blockdev* bdev);
static int io_raw_bread(struct ext4_blockdev* bdev, void* buf, uint64_t blk_id,
	uint32_t blk_cnt);
static int io_raw_bwrite(struct ext4_blockdev* bdev, const void* buf,
	uint64_t blk_id, uint32_t blk_cnt);
static int io_raw_close(struct ext4_blockdev* bdev);

/******************************************************************************/
//EXT4_BLOCKDEV_STATIC_INSTANCE(_filedev, EXT4_IORAW_BSIZE, 0, io_raw_open,
//			      io_raw_bread, io_raw_bwrite, io_raw_close, 0, 0);
//
//static uint8_t _filedev_ph_bbuf[512] = { 0 };
//static struct ext4_blockdev_iface __name_filedev_iface =
//{
//	io_raw_open,
//	io_raw_bread,
//	io_raw_bwrite,
//	io_raw_close,
//	nullptr,
//	nullptr,
//	512,
//	0,
//	_filedev_ph_bbuf
//};

//static struct ext4_blockdev _filedev = { &__name_filedev_iface,0,0 };

/******************************************************************************/
static int io_raw_open(struct ext4_blockdev* bdev)
{
	char path[64];
	DISK_GEOMETRY pdg = { 0 };
	uint64_t disk_size;
	BOOL bResult = FALSE;
	DWORD junk;
	LARGE_INTEGER fileSize;

	//try to open as a normal file
	bdev->bdif->dev_file =
		CreateFileA(bdev->bdif->fname, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);

	if (bdev->bdif->dev_file != INVALID_HANDLE_VALUE) {

		bResult = GetFileSizeEx(bdev->bdif->dev_file, &fileSize);

		if (bResult == FALSE) {
			CloseHandle(bdev->bdif->dev_file);
			return EIO;
		}

		disk_size = fileSize.QuadPart;

		bdev->bdif->ph_scnt = 1;
		bdev->bdif->ph_tcnt = 1;
		bdev->bdif->ph_bcnt = disk_size / bdev->bdif->ph_bsize;

		bdev->part_offset = 0;
		bdev->part_size = disk_size;

		return EOK;
	}

	sprintf(path, "\\\\.\\%s", bdev->bdif->fname);

	bdev->bdif->dev_file =
		CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);

	if (bdev->bdif->dev_file == INVALID_HANDLE_VALUE) {
		return EIO;
	}

	bResult =
		DeviceIoControl(bdev->bdif->dev_file, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			&pdg, sizeof(pdg), &junk, (LPOVERLAPPED)NULL);

	if (bResult == FALSE) {
		CloseHandle(bdev->bdif->dev_file);
		return EIO;
	}

	disk_size = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
		(ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;

	bdev->bdif->ph_bsize = pdg.BytesPerSector;
	bdev->bdif->ph_scnt = pdg.SectorsPerTrack;
	bdev->bdif->ph_tcnt = pdg.TracksPerCylinder;
	bdev->bdif->ph_bcnt = disk_size / pdg.BytesPerSector;

	bdev->part_offset = 0;
	bdev->part_size = disk_size;

	return EOK;
}

/******************************************************************************/

static int io_raw_bread(struct ext4_blockdev* bdev, void* buf, uint64_t blk_id,
	uint32_t blk_cnt)
{
	long hipart = blk_id >> (32 - 9);
	long lopart = blk_id << 9;
	long err;

	SetLastError(0);
	lopart = SetFilePointer(bdev->bdif->dev_file, lopart, &hipart, FILE_BEGIN);

	if (lopart == -1 && NO_ERROR != (err = GetLastError())) {
		return EIO;
	}

	DWORD n;

	if (!ReadFile(bdev->bdif->dev_file, buf, blk_cnt * 512, &n, NULL)) {
		err = GetLastError();
		return EIO;
	}
	return EOK;
}

/******************************************************************************/
static int io_raw_bwrite(struct ext4_blockdev* bdev, const void* buf,
	uint64_t blk_id, uint32_t blk_cnt)
{
	long hipart = blk_id >> (32 - 9);
	long lopart = blk_id << 9;
	long err;

	SetLastError(0);
	lopart = SetFilePointer(bdev->bdif->dev_file, lopart, &hipart, FILE_BEGIN);

	if (lopart == -1 && NO_ERROR != (err = GetLastError())) {
		return EIO;
	}

	DWORD n;

	if (!WriteFile(bdev->bdif->dev_file, buf, blk_cnt * 512, &n, NULL)) {
		err = GetLastError();
		return EIO;
	}
	return EOK;
}

/******************************************************************************/
static int io_raw_close(struct ext4_blockdev* bdev)
{
	CloseHandle(bdev->bdif->dev_file);
	return EOK;
}

/******************************************************************************/
struct ext4_blockdev* ext4_io_raw_dev_get(const char* fname) {
	//return &_filedev;
	ext4_blockdev_iface *bdi = new ext4_blockdev_iface{
			io_raw_open,
			io_raw_bread,
			io_raw_bwrite,
			io_raw_close,
			nullptr,
			nullptr,
			512,
			0};
	memcpy(bdi->fname, fname, strlen(fname));
	return new ext4_blockdev{ bdi,0,0 };
}
/******************************************************************************/
//void ext4_io_raw_filename(const char* n) { fname = n; }

/******************************************************************************/
#endif
