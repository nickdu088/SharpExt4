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
