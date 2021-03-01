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
#include <stdint.h>
using namespace System;

namespace SharpExt4 {
	public ref class Geometry sealed
	{
	private:
		int _cylinders;
		int _headsPerCylinder;
		int _sectorsPerTrack;
		int _bytesPerSector;
	public:
		Geometry(int cylinders, int headsPerCylinder, int sectorsPerTrack);
		Geometry(int cylinders, int headsPerCylinder, int sectorsPerTrack, int bytesPerSector);
		Geometry(uint64_t capacity, int headsPerCylinder, int sectorsPerTrack, int bytesPerSector);
		
		property int Cylinders { int get() { return _cylinders; } }
		property int HeadsPerCylinder {int get() { return _headsPerCylinder; }}
		property  int SectorsPerTrack {int get() { return _sectorsPerTrack; }}
		property  int BytesPerSector { int get() { return _bytesPerSector; }}
		property uint64_t TotalSectors {uint64_t get() { return (uint64_t)Cylinders * (uint64_t)HeadsPerCylinder * (uint64_t)SectorsPerTrack; }}
		property uint64_t Capacity { uint64_t get() { return TotalSectors * (uint64_t)BytesPerSector; }}
		
		String^ ToString() override;

	};
}

