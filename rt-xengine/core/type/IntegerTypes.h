#pragma once

#include <cstdint>

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef uint8	byte;
typedef uint32	uint;

#define BIT(x) (1 << x)
#define CHECK_BIT(var,pos) ((var) & (1<<(m_cursorPosition)))
