//
//  ByteOrder.h
//  Square
//
//  Endianness-aware, alignment-safe reads/writes of scalar values from/to raw
//  byte buffers. Use these instead of dereferencing a reinterpret_cast pointer:
//  the memcpy has no alignment requirement and the byte swap is compiled away on
//  little-endian hosts, so on the common path it is a single load/store.
//
#pragma once
#include <cstring>
#include <cstddef>
#include <type_traits>

namespace Square
{
	// True at compile time on the platforms Square targets (x86 / ARM-LE / Apple /
	// consoles). Binary formats in the engine are canonical little-endian.
	constexpr bool host_is_little_endian =
	#if defined(_WIN32)
		true;
	#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
		(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
	#else
		true;
	#endif

	// Reverse the bytes of a trivially-copyable scalar in place.
	template<typename T>
	inline void byte_swap(T& value)
	{
		unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
		for (std::size_t i = 0; i < sizeof(T) / 2; ++i)
		{
			const unsigned char tmp  = bytes[i];
			bytes[i]                 = bytes[sizeof(T) - 1 - i];
			bytes[sizeof(T) - 1 - i] = tmp;
		}
	}

	// Read a little-endian scalar from raw bytes into an aligned value.
	template<typename T>
	inline T read_le(const void* source)
	{
		static_assert(std::is_trivially_copyable<T>::value, "read_le requires a trivially copyable type");
		T value;
		std::memcpy(&value, source, sizeof(T));
		if constexpr (!host_is_little_endian) byte_swap(value);
		return value;
	}

	// Write a scalar to raw bytes in little-endian order.
	template<typename T>
	inline void write_le(void* destination, T value)
	{
		static_assert(std::is_trivially_copyable<T>::value, "write_le requires a trivially copyable type");
		if constexpr (!host_is_little_endian) byte_swap(value);
		std::memcpy(destination, &value, sizeof(T));
	}
}
