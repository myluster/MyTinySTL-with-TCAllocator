#pragma once
#include "type_traits.h"
//byte��һ��C++17�ı�׼����Ϊԭʼ�ڴ��ֽڵı�ʾ�������ʹ��void*��char*
//ʹ��void*��ʹ�����г���Ŵ�����static_cast<char*>(ptr)
//�� reinterpret_cast<unsigned char*>(ptr)
//ʮ���������
//��Ϊ�˷��㣬����ʵ��byte
namespace mystl
{
	//����һ��ǿ����ö�٣��ײ�����Ϊ unsigned char���������������ַ�����
	enum class byte:unsigned char{};

	//֧��λ����
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
		// static_assert ���ڱ���ʱ��� IntegralType �Ƿ�Ϊ���ͣ��������Ͱ�ȫ
		static_assert(mystl::is_integral<IntegralType>::value, "Shift amount must be an integral type.");
		return static_cast<byte>(static_cast<unsigned char>(b) << shift);
	}

	template<typename IntegralType>
	constexpr byte operator>>(byte b, IntegralType shift) noexcept {
		static_assert(mystl::is_integral<IntegralType>::value, "Shift amount must be an integral type.");
		return static_cast<byte>(static_cast<unsigned char>(b) >> shift);
	}

	//���ϸ�ֵ������
	constexpr byte& operator|=(byte& l, byte r) noexcept {
		l = l | r; // ��������� operator|
		return l;
	}

	constexpr byte& operator&=(byte& l, byte r) noexcept {
		l = l & r; // ��������� operator&
		return l;
	}

	constexpr byte& operator^=(byte& l, byte r) noexcept {
		l = l ^ r; // ��������� operator^
		return l;
	}

	template<typename IntegralType>
	constexpr IntegralType to_integer(byte b) noexcept {
		static_assert(mystl::is_integral<IntegralType>::value, "Target type must be an integral type.");
		return static_cast<IntegralType>(static_cast<unsigned char>(b));
	}
}