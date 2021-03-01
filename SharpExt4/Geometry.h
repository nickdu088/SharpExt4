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

