#pragma once

#include <stdint.h>
#include "EntryType.h"

using namespace System;

namespace SharpExt4 {
	public ref class ExtDirEntry sealed
	{
	private:
		String^ name;
		uint64_t length;
		EntryType type;
	public:
		property String^ Name {String^ get() { return name; }; private:	void set(String^ value) { name = value; }; }
		property uint64_t Length { uint64_t get() { return length; }; private:	void set(uint64_t value) { length = value; }; }
		property EntryType Type { EntryType get() { return type; }; private:	void set(EntryType value) { type = value; }; }
		ExtDirEntry(String^ name, uint64_t length, EntryType type);
	};
}

