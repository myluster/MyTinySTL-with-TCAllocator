#pragma once
#include <cstddef>//size_t
#include <string>//to_string
#include "type_traits.h"
#include "utils.h"
namespace mystl
{
	//定义每个用于储存的字的位数
	static constexpr size_t BITS_PER_WORD = sizeof(unsigned long long) * 8;// 1 byte = 8 bits

	template<size_t N>
	class bitset
	{
	public:
		using value_type = bool;
		using size_type = size_t;

		//代理类
		class reference {
			friend class bitset;
		private:
			unsigned long long* m_word_ptr;//指向位所在的word
			size_type m_pos_in_word;//位在word中的位置

			//私有构造函数，只能由bitset类创建
			constexpr reference(unsigned long long* word_ptr,size_type pos_in_word)noexcept
				:m_word_ptr(word_ptr),m_pos_in_word(pos_in_word){ }
		public:
			// 赋值运算符
			reference& operator=(bool x) noexcept {
				if (x) {
					*m_word_ptr |= (1ULL << m_pos_in_word);
				}
				else {
					*m_word_ptr &= ~(1ULL << m_pos_in_word);
				}
				return *this;
			}

			reference& operator=(const reference& rhs) noexcept {
				return *this = static_cast<bool>(rhs); // 转换为 bool 再赋值
			}

			// 隐式转换为 bool (用于读操作)
			constexpr operator bool() const noexcept {
				return (*m_word_ptr & (1ULL << m_pos_in_word)) != 0;
			}

			// 翻转位
			bool flip() noexcept {
				*m_word_ptr ^= (1ULL << m_pos_in_word);
				return static_cast<bool>(*this); // 返回翻转后的值
			}
		};


		//构造函数
		constexpr bitset()noexcept
		{
			mystl::memset(m_words, 0, sizeof(m_words));
		}

		//从unsigned long long 构造（当且仅当N<=BITS_PER_WORD时有效）
		template<
			typename =typename mystl::enable_if<N<=BITS_PER_WORD>::type>
		constexpr explicit bitset(unsigned long long val)noexcept
		{
			mystl::memset(m_words, 0, sizeof(m_words));
			m_words[0] = val;
		}

		//从字符串构造比较复杂就不实现了

		//位操作
		//设置指定位为1
		bitset& set(size_type pos, bool value = true)
		{
			assert(pos < N && "bitset::set position out of bounds");
			if (value)
			{
				m_words[pos / BITS_PER_WORD] |= (1ULL << (pos % BITS_PER_WORD));
			}
			else
			{
				m_words[pos / BITS_PER_WORD] &= ~(1ULL << (pos % BITS_PER_WORD));
			}
			return *this;
		}

		// 设置所有位为 1
		bitset& set() {
			mystl::memset(m_words, 0xFF, sizeof(m_words));
			// 清除超出 N 位的多余位（如果 N 不是 BITS_PER_WORD 的整数倍）
			if (N % BITS_PER_WORD != 0) {
				m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return *this;
		}

		// 清除指定位为 0
		bitset& reset(size_type pos) {
			assert(pos < N && "bitset::reset position out of bounds");
			m_words[pos / BITS_PER_WORD] &= ~(1ULL << (pos % BITS_PER_WORD));
			return *this;
		}

		// 清除所有位为 0
		bitset& reset() {
			mystl::memset(m_words, 0, sizeof(m_words));
			return *this;
		}

		// 翻转指定位
		bitset& flip(size_type pos) {
			assert(pos < N && "bitset::flip position out of bounds");
			m_words[pos / BITS_PER_WORD] ^= (1ULL << (pos % BITS_PER_WORD));
			return *this;
		}

		// 翻转所有位
		bitset& flip() {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				m_words[i] = ~m_words[i];
			}
			// 清除超出 N 位的多余位（如果 N 不是 BITS_PER_WORD 的整数倍）
			if (N % BITS_PER_WORD != 0) {
				m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return *this;
		}


		// 访问位
		// 测试指定位是否为 1
		bool test(size_type pos) const {
			assert(pos < N && "bitset::test position out of bounds");
			return (m_words[pos / BITS_PER_WORD] & (1ULL << (pos % BITS_PER_WORD))) != 0;
		}

		// 访问指定位（返回代理对象）
		reference operator[](size_type pos)noexcept {
			assert(pos < N && "bitset::operator[] position out of bounds");
			return reference(&m_words[pos / BITS_PER_WORD], pos % BITS_PER_WORD);
		}

		bool operator[](size_type pos) const noexcept {
			return test(pos);
		}

		// 检查所有位是否都为 1
		bool all() const {
			// 检查前 NUM_WORDS-1 个字是否都为全1
			for (size_type i = 0; i < NUM_WORDS - 1; ++i) {
				if (m_words[i] != static_cast<unsigned long long>(-1)) { // static_cast<unsigned long long>(-1) 是全1
					return false;
				}
			}
			// 检查最后一个字（处理N不是BITS_PER_WORD整数倍的情况）
			if (N % BITS_PER_WORD != 0) {
				return (m_words[NUM_WORDS - 1] == ((1ULL << (N % BITS_PER_WORD)) - 1));
			}
			else {
				return (m_words[NUM_WORDS - 1] == static_cast<unsigned long long>(-1));
			}
		}

		// 检查是否有任何位为 1
		bool any() const {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (m_words[i] != 0) {
					return true;
				}
			}
			return false;
		}

		// 检查是否所有位都为 0
		bool none() const {
			return !any();
		}

		// 返回设置的位为1的数量
		size_type count() const {
			size_type total_set_bits = 0;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				total_set_bits += popcount(m_words[i]);
			}
			return total_set_bits;
		}

		// 返回位集中的位数
		size_type size() const noexcept {
			return N;
		}

		// 运算符重载 (位逻辑运算符)
		bitset operator~() const {
			bitset res = *this;
			res.flip();
			return res;
		}

		bitset& operator&=(const bitset& other) {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				m_words[i] &= other.m_words[i];
			}
			return *this;
		}

		bitset& operator|=(const bitset& other) {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				m_words[i] |= other.m_words[i];
			}
			return *this;
		}

		bitset& operator^=(const bitset& other) {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				m_words[i] ^= other.m_words[i];
			}
			return *this;
		}

		// 移位运算符 (左移)
		bitset operator<<(size_type pos) const {
			if (pos >= N) return bitset(); // 移位超出范围，结果为全0
			bitset res;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (i * BITS_PER_WORD + pos < N) { // 确保不超过总位数 N
					size_type word_shift = pos / BITS_PER_WORD;
					size_type bit_shift = pos % BITS_PER_WORD;

					if (i + word_shift < NUM_WORDS) {
						res.m_words[i + word_shift] |= (m_words[i] << bit_shift);
					}
					if (bit_shift > 0 && i + word_shift + 1 < NUM_WORDS) {
						res.m_words[i + word_shift + 1] |= (m_words[i] >> (BITS_PER_WORD - bit_shift));
					}
				}
			}
			// 清除超出 N 的位
			if (N % BITS_PER_WORD != 0) {
				res.m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return res;
		}

		bitset& operator<<=(size_type pos) {
			*this = (*this) << pos;
			return *this;
		}

		// 移位运算符 (右移)
		bitset operator>>(size_type pos) const {
			if (pos >= N) return bitset(); // 移位超出范围，结果为全0
			bitset res;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (i >= pos / BITS_PER_WORD) { // 确保从正确的字开始处理
					size_type word_shift = pos / BITS_PER_WORD;
					size_type bit_shift = pos % BITS_PER_WORD;

					if (i - word_shift >= 0) { // 确保目标字有效
						res.m_words[i - word_shift] |= (m_words[i] >> bit_shift);
					}
					if (bit_shift > 0 && i - word_shift - 1 >= 0) {
						res.m_words[i - word_shift - 1] |= (m_words[i] << (BITS_PER_WORD - bit_shift));
					}
				}
			}
			return res;
		}

		bitset& operator>>=(size_type pos) {
			*this = (*this) >> pos;
			return *this;
		}

		//输出为字符串
		std::string to_string(char zero = '0', char one = '1') const {
			std::string s(N, zero);
			for (size_type i = 0; i < N; ++i) {
				if (test(i)) {
					s[N - 1 - i] = one; // 从高位到低位填充字符串
				}
			}
			return s;
		}

	private:
		//计算存储所需的unsigned long long 的数量
		static constexpr size_type NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;
		unsigned long long m_words[NUM_WORDS];

		//计算一个unsigned long long中位为1的数量，用于count()函数
		size_type popcount(unsigned long long n)const
		{
			size_type count = 0;
			while (n > 0)
			{
				n &= (n - 1);//清除最右边的1
				count++;
			}
			return count;
		}
	};

	// 全局位逻辑运算符
	template <size_t N>
	bitset<N> operator&(const bitset<N>& lhs, const bitset<N>& rhs) {
		bitset<N> res = lhs;
		res &= rhs;
		return res;
	}

	template <size_t N>
	bitset<N> operator|(const bitset<N>& lhs, const bitset<N>& rhs) {
		bitset<N> res = lhs;
		res |= rhs;
		return res;
	}

	template <size_t N>
	bitset<N> operator^(const bitset<N>& lhs, const bitset<N>& rhs) {
		bitset<N> res = lhs;
		res ^= rhs;
		return res;
	}

	// 比较运算符
	template <size_t N>
	bool operator==(const bitset<N>& lhs, const bitset<N>& rhs) {
		for (size_t i = 0; i < bitset<N>::NUM_WORDS; ++i) {
			if (lhs.m_words[i] != rhs.m_words[i]) { // 直接访问私有成员，需要 friend 或在同一命名空间下特化
				return false;
			}
		}
		return true;
	}

	template <size_t N>
	bool operator!=(const bitset<N>& lhs, const bitset<N>& rhs) {
		return !(lhs == rhs);
	}
}