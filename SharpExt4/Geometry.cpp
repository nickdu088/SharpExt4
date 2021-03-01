#include "pch.h"
#include "Geometry.h"

SharpExt4::Geometry::Geometry(int cylinders, int headsPerCylinder, int sectorsPerTrack)
{
    _cylinders = cylinders;
    _headsPerCylinder = headsPerCylinder;
    _sectorsPerTrack = sectorsPerTrack;
    _bytesPerSector = 512;
}

SharpExt4::Geometry::Geometry(int cylinders, int headsPerCylinder, int sectorsPerTrack, int bytesPerSector)
{
    _cylinders = cylinders;
    _headsPerCylinder = headsPerCylinder;
    _sectorsPerTrack = sectorsPerTrack;
    _bytesPerSector = bytesPerSector;
}

SharpExt4::Geometry::Geometry(uint64_t capacity, int headsPerCylinder, int sectorsPerTrack, int bytesPerSector)
{
    _cylinders = (int)(capacity / (headsPerCylinder * (uint64_t)sectorsPerTrack * bytesPerSector));
    _headsPerCylinder = headsPerCylinder;
    _sectorsPerTrack = sectorsPerTrack;
    _bytesPerSector = bytesPerSector;
}

String^ SharpExt4::Geometry::ToString()
{
    if (_bytesPerSector == 512)
    {
        return gcnew String("(" + _cylinders + "/" + _headsPerCylinder + "/" + _sectorsPerTrack + ")");
    }
    else
    {
        return gcnew String("(" + _cylinders + "/" + _headsPerCylinder + "/" + _sectorsPerTrack + ":" + _bytesPerSector + ")");
    }
}
