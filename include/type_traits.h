#pragma once
// ���ͷ�ļ�������ȡ������Ϣ
//������C++���ͱ���еĳ��ü��ɡ���traits ��̼���
//���ã�ͨ������ģ�����Ծ���������������Ƶ��Ի�ȡ�����ͻ�ĳ�����ԡ�
//Ŀ��1����Ի�õ����ͺ�ĳ����������ʵ�ֺ��������Ż��Դﵽ���ߵ����ܡ�
//Ŀ��2��ͨ����ȡ����������Ϊģ�庯���ķ������ͣ����㺯��ʵ�֡�

#include<type_traits>//���в��䣬��Ϊ���Թ��ڷ�������Щ���ӣ����Բ�һһʵ��
#include <cstddef> //std::nullptr_t
namespace mystl
{
    // �����������κγ�Ա���࣬���ڸ���ʵ��type_traitsģ��
    template<typename T, T v>
    struct integral_constant {
        using value_type = T;//���ͳ�Ա
        using type = integral_constant<T, v>;//�����Ա
        static constexpr T value = v;//ֵ��Ա
    };

    using _true_type = integral_constant<bool, true>;
    using _false_type = integral_constant<bool, false>;
    /*
    Ϊʲô��ֱ��ʹ��bool
    ��Ϊ��ʹ�õ�ʱ��������true����false�������ص�ͬһ������
    ���������û����������������ں����ڽ���if�ж�
    */

    template<typename T,typename U>
	struct is_same :public _false_type {};

	template<typename T>
	struct is_same<T, T> : public _true_type {};

    //is_void
	template<typename T>
	struct is_void : public _false_type {};

	template<>
	struct is_void<void> : public _true_type {};


    //is_function
	// �ж�һ�������Ƿ�Ϊ��������
    namespace detail {
        //Ĭ�ϲ��Ǻ���
        template<typename T>
        struct is_function_impl :public _false_type {};

        //ƥ����ͨ�������ͣ��޲�����
		template<typename R>
		struct is_function_impl<R()> :public _true_type {};

		//ƥ����ͨ�������ͣ���������
        template<typename R, typename... Args>
        struct is_function_impl<R(Args...)> :public _true_type {};

        // ��Ա�������ͣ��޲������� const/volatile
        template <typename Ret, typename ClassType>
        struct is_function_impl<Ret(ClassType::*)()> : public _true_type {};

        // ��Ա�������ͣ����������� const/volatile
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...)> : public _true_type {};

        // ��Ա�������ͣ�const �޶�
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...) const> : public _true_type {};

        // ��Ա�������ͣ�volatile �޶�
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...) volatile> : public _true_type {};

        // ��Ա�������ͣ�const volatile �޶�
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...) const volatile> : public _true_type {};
    }

	template<typename T>
	struct is_function : public detail::is_function_impl<T> {};


    //is_pair
    template<class T1, class T2>
    struct pair;

    template<class T>
    struct is_pair :_false_type {};

    template<class T1, class T2>
    struct is_pair<pair<T1, T2>> :_true_type {};

    //is_array
    template<typename T>
    struct is_array : public _false_type {};

    template<typename T, std::size_t N>
    struct is_array<T[N]> : public _true_type {}; // ƥ��̶����ȵ�����

    template<typename T>
    struct is_array<T[]> : public _true_type {}; // ƥ�����ⳤ�ȵ�����


    //�ж�һ������From�Ƿ������ʽת��Ϊ��һ������To
    //ʵ��������is_convertible�����ǳ����ӣ���Ҫ��������֧��
    //����ģ��򵥵���ʽת����ʵ�ַ���
    /*
    namespace detail {
        template <typename From, typename To>
        struct is_convertible_impl {
            template <typename T> static void test_sink(T) {}
            template <typename F> static mystl::_true_type test(decltype(test_sink<To>(std::declval<F>()))*){}
            template <typename F> static mystl::_false_type test(...);

            static const bool value = mystl::is_same<decltype(test<From>(nullptr)), mystl::_true_type>::value;
        };
    }
    template <typename From, typename To>
    struct is_convertible : public detail::is_convertible_impl<From, To> {};
    */
    template <typename From, typename To>
    struct is_convertible : public std::conditional<
        std::is_convertible<From, To>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


	//�������T�Ƿ���Ĭ�Ϲ��캯��
    //ע���������Ǹ�����ģ��ʵ�ֵĲ�����ʶ���private��protect�Ĺ��캯��
    /*
    namespace detail {
        template <typename T>
        struct is_default_constructible_test {
            template <typename U, typename = decltype(U())>
            static _true_type test(int) { return _true_type(); } // ��Ҫ��ӿն���
            template <typename U>
            static _false_type test(...) { return _false_type(); }
        };
    }
    template <typename T>
    struct is_default_constructible : public detail::is_default_constructible_test<T>::type {};
    */
    template <typename T>
    struct is_default_constructible : public std::conditional<
        std::is_default_constructible<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //�������T�Ƿ�ɿ�������
    template <typename T>
    struct is_copy_constructible : public std::conditional<
        std::is_copy_constructible<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //�������T�Ƿ���ø����Ĳ���Args...���й���
    template <typename T, typename... Args>
    struct is_constructible : public std::conditional<
        std::is_constructible<T, Args...>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //�������T�Ƿ�Ϊ��ֵ��������
    template <typename T>
    struct is_lvalue_reference : public std::conditional<
        std::is_lvalue_reference<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };



    //�������T�Ƿ����ƽ���Ŀ�����ֵ�����
    template <typename T>
    struct is_trivially_copy_assignable : public std::conditional<
        std::is_trivially_copy_assignable<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //�������T�Ƿ����ƽ�����ƶ���ֵ�����
    template <typename T>
    struct is_trivially_move_assignable : public std::conditional<
        std::is_trivially_move_assignable<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //�������T�Ƿ�Ϊ����
    template <typename T>
    struct is_integral : public std::conditional<
        std::is_integral<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };

    //����һ������ʱ�����������������û����ģ���ĳ���ػ����������ػ����Ա
    template <bool B, class T = void>
    struct enable_if {};

    template <class T>
    struct enable_if<true, T> { typedef T type; };



    /*������type_traitsģ��
    �����Ƚ��������ʶ���Ϊfalse_type��Ҳ����˵������ʹ�ø�ģ�������һ����ص�ֵ
    ����ʹ��ģ���ػ��ķ�����Ϊ��֪Ϊtrue_type�������ṩ�ػ��汾
    */
    template<typename type>
    struct type_traits {
        //ָ����ӵ�е���������Ϊ��ֵΪ_true_type
        //ʹ������if(type_trait<T>::istribvial_default_constructor)
        using has_trivial_default_constructor = _false_type;
        using has_trivial_copy_constructor = _false_type;
        using has_trivial_assignment_operator = _false_type;
        using has_trivial_destructor = _false_type;
        using is_POD_type = _false_type;
        //c11������POD��ʹ��is_trivial��is_standard_layout�������������ڱ����������ں��������Ǻ���ʵ��
    };


    // ������ͨ����ʱ�̳� false_type ,��ʱ�侲̬��Ա value Ϊ false
    template<typename T>
    struct is_const : public _false_type {};

    // ƫ�ػ�ʹ�ý��� const ����ʱ�̳� true_type����ʱ�侲̬��Ա value Ϊ true
    template<typename T>
    struct is_const<const T> : public _true_type {};

    // ���ָ���ƫ�ػ��汾
    // ָ�뱾�����һ�������ƽ�����ͣ���type�޹�
    // ָ��Ĺ��졢��������ֵ������������������ֵ���ڴ��ַ���Ĳ�������������κ��û������߼�
    template<typename type>
    struct type_traits<type*> {
        using has_trivial_default_constructor = _true_type;
        using has_trivial_copy_constructor = _true_type;
        using has_trivial_assignment_operator = _true_type;
        using has_trivial_destructor = _true_type;
        using is_POD_type = _true_type;
    };

	//���std::nullptr_t���ػ�
    template<>
    struct type_traits<std::nullptr_t> {
        using has_trivial_default_constructor = _true_type;
        using has_trivial_copy_constructor = _true_type;
        using has_trivial_assignment_operator = _true_type;
        using has_trivial_destructor = _true_type;
        using is_POD_type = _true_type;
	};


    //��Ը����������͵��ػ��汾
    //���ú�������Щ�ػ����Լ����ظ�����
#define MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(TYPE) \
    template<> \
    struct type_traits<TYPE> { \
        using has_trivial_default_constructor = _true_type; \
        using has_trivial_copy_constructor = _true_type; \
        using has_trivial_assignment_operator = _true_type; \
        using has_trivial_destructor = _true_type; \
        using is_POD_type = _true_type; \
    }; \
    template<> \
    struct type_traits<const TYPE> { \
        using has_trivial_default_constructor = _true_type; \
        using has_trivial_copy_constructor = _true_type; \
        using has_trivial_assignment_operator = _true_type; \
        using has_trivial_destructor = _true_type; \
        using is_POD_type = _true_type; \
    };

    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(bool)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(char)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(unsigned char)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(signed char)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(wchar_t)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(short)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(unsigned short)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(int)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(unsigned int)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(long)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(unsigned long)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(long long)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(unsigned long long)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(float)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(double)
    MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS(long double)

#undef MYSTL_GENERIC_TRIVIAL_TYPE_TRAITS
    

    //remove_reference�Ƴ����͵��������η�
    template<class T>
    struct remove_reference
    {
        //Ĭ�ϴ������������
        using type = T;
    };

    //�ػ���ֵ����
    template<class T>
    struct remove_reference<T&>
    {
        using type = T;
    };

    //�ػ���ֵ����
    template <class T>
    struct remove_reference<T&&>
    {
        using type = T;
    };

    template<typename T>
    struct remove_const
    {
        //Ĭ�ϴ����const����
        using type = T;
	};

	//�ػ�const����
	template<typename T>
	struct remove_const<const T>
	{
		using type = T;
	};


    template<typename T>
    struct remove_volatile
    {
        //Ĭ�ϴ����volatile����
        using type = T;
	};

	//�ػ�volatile����
	template<typename T>
	struct remove_volatile<volatile T>
	{
		using type = T;
	};

	//remove_cv�Ƴ����͵�const��volatile���η�
    template<typename T>
    struct remove_cv
    {
        typedef T type;
    };

    // ƫ�ػ����Ƴ� const
    template<typename T>
    struct remove_cv<const T>
    {
        typedef T type; // ���� remove_cv<const int>::type �� int
    };

    // ƫ�ػ����Ƴ� volatile
    template<typename T>
    struct remove_cv<volatile T>
    {
        typedef T type; // ���� remove_cv<volatile int>::type �� int
    };

    // ƫ�ػ����Ƴ� const volatile (ע��˳�����Ӧ������������֮��)
    // ������������ƥ�����ػ��ģ����� const volatile T ��ƥ������汾
    template<typename T>
    struct remove_cv<const volatile T>
    {
        typedef T type; // ���� remove_cv<const volatile int>::type �� int
    };

	//���ڰ�ȫ�Ļ�ȡ����ĵ�ַ����ʹoperator&������
	template<typename T>
    inline T* addressof(T& arg) noexcept
    {
        //ʹ������ת�������ƹ�operator&
        return reinterpret_cast<T*>(//3.ת��T*
			&const_cast<char&>(//2.ȥ��const��volatile
                reinterpret_cast<const volatile char&>(arg)//1.���κ�ָ�������ת��Ϊ�������͵�ָ������ã������ײ�����Ʊ�ʾ
                )
            );
    }


    //Ϊ����T�����ֵ�����޶���
    namespace detail {
        template<typename T,bool IsFunction=mystl::is_function<T>::value,bool IsVoid=mystl::is_void<T>::value>
        struct add_rvalue_reference_impl {
            using type = T;//Ĭ��������� void, �������ͣ����Ѿ���ͨ�������۵�������������ͣ�
        };

		template<typename T>
        struct add_rvalue_reference_impl<T, false, false> {
            using type = T&&; // ���ڷǺ����ͷ�void���ͣ������ֵ����
		};

        template<typename T>
        struct add_rvalue_reference_impl<T&, false, false> {
            using type = T&; // ������ֵ���ã������۵�Ϊ��ֵ
        };

		template<typename T>
        struct add_rvalue_reference_impl<T&&, false, false> {
            using type = T&&; // ������ֵ���ã�������ֵ����
		};
    }

    template<typename T>
    struct add_rvalue_reference :public detail::add_rvalue_reference_impl<T> {};

    //declval
    //�ڲ�ʵ�ʹ������������»�ȡ������
    template<typename T>
    typename mystl::add_rvalue_reference<T>::type declval()noexcept;
}