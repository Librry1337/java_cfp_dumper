#pragma once
#include <cstdint>
// Created with ReClass.NET 1.2 by KN4CK3R

class ClassFileStream
{
public:
	char pad_0000[8]; //0x0000
	std::uint8_t* _buffer_start; //0x0008
	std::uint8_t* _buffer_end; //0x0010
	std::uint8_t* _current; //0x0018
	const char* _source; //0x0020
	char pad_0028[24]; //0x0028
}; //Size: 0x0040