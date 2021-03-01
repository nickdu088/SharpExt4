#pragma once
#include <stdint.h>

namespace SharpExt4 {
	public ref class Partition sealed
	{
	public:
		property uint64_t Offset;
		property uint64_t Size;
		Partition();
	};
}

