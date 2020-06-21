#pragma once

#include <cinttypes>
#include <utility>

#pragma pack(push,1)
struct FileDataBlock
{
	uint32_t index;
	std::pair<uint32_t/*sec*/, uint32_t/*msec*/> timestamp;
	uint32_t value;
	uint8_t status;
};
#pragma pack(pop)
