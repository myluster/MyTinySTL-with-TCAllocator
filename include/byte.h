#pragma once
#include "type_traits.h"
//byte是一个C++17的标准，作为原始内存字节的表示，代替掉使用void*、char*
//使用void*会使代码中充斥着大量的static_cast<char*>(ptr)
//或 reinterpret_cast<unsigned char*>(ptr)
//十分难以理解
//故为了方便，我们实现byte
namespace mystl
{
	//它是一个强类型枚举，底层类型为 unsigned char，不具有算术或字符语义
	enum class byte:unsigned char{};

	//支持位操作
	constexpr byte operator|(byte l, byte r)noexcept
	{
		return static_cast<byte>(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
	}

	constexpr byte operator&(byte l, byte r) noexcept
	{
		return static_cast<byte>(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
	}

	constexpr byte operator^(byte l, byte r) noexcept
	{
		return static_cast<byte>(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
	}

	template<typename IntegralType>
	constexpr byte operator<<(byte b, IntegralType shift) noexcept {
		// static_assert 用于编译时检查 IntegralType 是否为整型，增加类型安全
		static_assert(mystl::is_integral<IntegralType>::value, "Shift amount must be an integral type.");
		return static_cast<byte>(static_cast<unsigned char>(b) << shift);
	}

	template<typename IntegralType>
	constexpr byte operator>>(byte b, IntegralType shift) noexcept {
		static_assert(mystl::is_integral<IntegralType>::value, "Shift amount must be an integral type.");
		return static_cast<byte>(static_cast<unsigned char>(b) >> shift);
	}

	//复合赋值操作符
	constexpr byte& operator|=(byte& l, byte r) noexcept {
		l = l | r; // 调用上面的 operator|
		return l;
	}

	constexpr byte& operator&=(byte& l, byte r) noexcept {
		l = l & r; // 调用上面的 operator&
		return l;
	}

	constexpr byte& operator^=(byte& l, byte r) noexcept {
		l = l ^ r; // 调用上面的 operator^
		return l;
	}

	template<typename IntegralType>
	constexpr IntegralType to_integer(byte b) noexcept {
		static_assert(mystl::is_integral<IntegralType>::value, "Target type must be an integral type.");
		return static_cast<IntegralType>(static_cast<unsigned char>(b));
	}
}