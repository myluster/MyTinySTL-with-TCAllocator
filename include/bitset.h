#pragma once
#include <cstddef>//size_t
#include <string>//to_string
#include "type_traits.h"
#include "utils.h"
namespace mystl
{
	//����ÿ�����ڴ�����ֵ�λ��
	static constexpr size_t BITS_PER_WORD = sizeof(unsigned long long) * 8;// 1 byte = 8 bits

	template<size_t N>
	class bitset
	{
	public:
		using value_type = bool;
		using size_type = size_t;

		//������
		class reference {
			friend class bitset;
		private:
			unsigned long long* m_word_ptr;//ָ��λ���ڵ�word
			size_type m_pos_in_word;//λ��word�е�λ��

			//˽�й��캯����ֻ����bitset�ഴ��
			constexpr reference(unsigned long long* word_ptr,size_type pos_in_word)noexcept
				:m_word_ptr(word_ptr),m_pos_in_word(pos_in_word){ }
		public:
			// ��ֵ�����
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
				return *this = static_cast<bool>(rhs); // ת��Ϊ bool �ٸ�ֵ
			}

			// ��ʽת��Ϊ bool (���ڶ�����)
			constexpr operator bool() const noexcept {
				return (*m_word_ptr & (1ULL << m_pos_in_word)) != 0;
			}

			// ��תλ
			bool flip() noexcept {
				*m_word_ptr ^= (1ULL << m_pos_in_word);
				return static_cast<bool>(*this); // ���ط�ת���ֵ
			}
		};


		//���캯��
		constexpr bitset()noexcept
		{
			mystl::memset(m_words, 0, sizeof(m_words));
		}

		//��unsigned long long ���죨���ҽ���N<=BITS_PER_WORDʱ��Ч��
		template<
			typename =typename mystl::enable_if<N<=BITS_PER_WORD>::type>
		constexpr explicit bitset(unsigned long long val)noexcept
		{
			mystl::memset(m_words, 0, sizeof(m_words));
			m_words[0] = val;
		}

		//���ַ�������Ƚϸ��ӾͲ�ʵ����

		//λ����
		//����ָ��λΪ1
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

		// ��������λΪ 1
		bitset& set() {
			mystl::memset(m_words, 0xFF, sizeof(m_words));
			// ������� N λ�Ķ���λ����� N ���� BITS_PER_WORD ����������
			if (N % BITS_PER_WORD != 0) {
				m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return *this;
		}

		// ���ָ��λΪ 0
		bitset& reset(size_type pos) {
			assert(pos < N && "bitset::reset position out of bounds");
			m_words[pos / BITS_PER_WORD] &= ~(1ULL << (pos % BITS_PER_WORD));
			return *this;
		}

		// �������λΪ 0
		bitset& reset() {
			mystl::memset(m_words, 0, sizeof(m_words));
			return *this;
		}

		// ��תָ��λ
		bitset& flip(size_type pos) {
			assert(pos < N && "bitset::flip position out of bounds");
			m_words[pos / BITS_PER_WORD] ^= (1ULL << (pos % BITS_PER_WORD));
			return *this;
		}

		// ��ת����λ
		bitset& flip() {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				m_words[i] = ~m_words[i];
			}
			// ������� N λ�Ķ���λ����� N ���� BITS_PER_WORD ����������
			if (N % BITS_PER_WORD != 0) {
				m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return *this;
		}


		// ����λ
		// ����ָ��λ�Ƿ�Ϊ 1
		bool test(size_type pos) const {
			assert(pos < N && "bitset::test position out of bounds");
			return (m_words[pos / BITS_PER_WORD] & (1ULL << (pos % BITS_PER_WORD))) != 0;
		}

		// ����ָ��λ�����ش������
		reference operator[](size_type pos)noexcept {
			assert(pos < N && "bitset::operator[] position out of bounds");
			return reference(&m_words[pos / BITS_PER_WORD], pos % BITS_PER_WORD);
		}

		bool operator[](size_type pos) const noexcept {
			return test(pos);
		}

		// �������λ�Ƿ�Ϊ 1
		bool all() const {
			// ���ǰ NUM_WORDS-1 �����Ƿ�Ϊȫ1
			for (size_type i = 0; i < NUM_WORDS - 1; ++i) {
				if (m_words[i] != static_cast<unsigned long long>(-1)) { // static_cast<unsigned long long>(-1) ��ȫ1
					return false;
				}
			}
			// ������һ���֣�����N����BITS_PER_WORD�������������
			if (N % BITS_PER_WORD != 0) {
				return (m_words[NUM_WORDS - 1] == ((1ULL << (N % BITS_PER_WORD)) - 1));
			}
			else {
				return (m_words[NUM_WORDS - 1] == static_cast<unsigned long long>(-1));
			}
		}

		// ����Ƿ����κ�λΪ 1
		bool any() const {
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (m_words[i] != 0) {
					return true;
				}
			}
			return false;
		}

		// ����Ƿ�����λ��Ϊ 0
		bool none() const {
			return !any();
		}

		// �������õ�λΪ1������
		size_type count() const {
			size_type total_set_bits = 0;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				total_set_bits += popcount(m_words[i]);
			}
			return total_set_bits;
		}

		// ����λ���е�λ��
		size_type size() const noexcept {
			return N;
		}

		// ��������� (λ�߼������)
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

		// ��λ����� (����)
		bitset operator<<(size_type pos) const {
			if (pos >= N) return bitset(); // ��λ������Χ�����Ϊȫ0
			bitset res;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (i * BITS_PER_WORD + pos < N) { // ȷ����������λ�� N
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
			// ������� N ��λ
			if (N % BITS_PER_WORD != 0) {
				res.m_words[NUM_WORDS - 1] &= ((1ULL << (N % BITS_PER_WORD)) - 1);
			}
			return res;
		}

		bitset& operator<<=(size_type pos) {
			*this = (*this) << pos;
			return *this;
		}

		// ��λ����� (����)
		bitset operator>>(size_type pos) const {
			if (pos >= N) return bitset(); // ��λ������Χ�����Ϊȫ0
			bitset res;
			for (size_type i = 0; i < NUM_WORDS; ++i) {
				if (i >= pos / BITS_PER_WORD) { // ȷ������ȷ���ֿ�ʼ����
					size_type word_shift = pos / BITS_PER_WORD;
					size_type bit_shift = pos % BITS_PER_WORD;

					if (i - word_shift >= 0) { // ȷ��Ŀ������Ч
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

		//���Ϊ�ַ���
		std::string to_string(char zero = '0', char one = '1') const {
			std::string s(N, zero);
			for (size_type i = 0; i < N; ++i) {
				if (test(i)) {
					s[N - 1 - i] = one; // �Ӹ�λ����λ����ַ���
				}
			}
			return s;
		}

	private:
		//����洢�����unsigned long long ������
		static constexpr size_type NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;
		unsigned long long m_words[NUM_WORDS];

		//����һ��unsigned long long��λΪ1������������count()����
		size_type popcount(unsigned long long n)const
		{
			size_type count = 0;
			while (n > 0)
			{
				n &= (n - 1);//������ұߵ�1
				count++;
			}
			return count;
		}
	};

	// ȫ��λ�߼������
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

	// �Ƚ������
	template <size_t N>
	bool operator==(const bitset<N>& lhs, const bitset<N>& rhs) {
		for (size_t i = 0; i < bitset<N>::NUM_WORDS; ++i) {
			if (lhs.m_words[i] != rhs.m_words[i]) { // ֱ�ӷ���˽�г�Ա����Ҫ friend ����ͬһ�����ռ����ػ�
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