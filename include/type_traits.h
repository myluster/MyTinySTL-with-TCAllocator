#pragma once
// 这个头文件用于提取类型信息
//利用了C++泛型编程中的常用技巧——traits 编程技法
//作用：通过函数模板来对具体对象或变量进行推导以获取其类型或某种特性。
//目的1：针对获得的类型和某种特性来对实现函数进行优化以达到更高的性能。
//目的2：通过提取出的类型作为模板函数的返回类型，方便函数实现。

#include<type_traits>//进行补充，因为特性过于繁多且有些复杂，所以不一一实现
#include <cstddef> //std::nullptr_t
namespace mystl
{
    // 两个不包含任何成员的类，用于辅助实现type_traits模板
    template<typename T, T v>
    struct integral_constant {
        using value_type = T;//类型成员
        using type = integral_constant<T, v>;//自身成员
        static constexpr T value = v;//值成员
    };

    using _true_type = integral_constant<bool, true>;
    using _false_type = integral_constant<bool, false>;
    /*
    为什么不直接使用bool
    因为在使用的时候无论是true还是false都会重载到同一个函数
    而这样设置会是两个函数，不在函数内进行if判断
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
	// 判断一个类型是否为函数类型
    namespace detail {
        //默认不是函数
        template<typename T>
        struct is_function_impl :public _false_type {};

        //匹配普通函数类型（无参数）
		template<typename R>
		struct is_function_impl<R()> :public _true_type {};

		//匹配普通函数类型（带参数）
        template<typename R, typename... Args>
        struct is_function_impl<R(Args...)> :public _true_type {};

        // 成员函数类型：无参数，非 const/volatile
        template <typename Ret, typename ClassType>
        struct is_function_impl<Ret(ClassType::*)()> : public _true_type {};

        // 成员函数类型：带参数，非 const/volatile
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...)> : public _true_type {};

        // 成员函数类型：const 限定
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...) const> : public _true_type {};

        // 成员函数类型：volatile 限定
        template <typename Ret, typename ClassType, typename... Args>
        struct is_function_impl<Ret(ClassType::*)(Args...) volatile> : public _true_type {};

        // 成员函数类型：const volatile 限定
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
    struct is_array<T[N]> : public _true_type {}; // 匹配固定长度的数组

    template<typename T>
    struct is_array<T[]> : public _true_type {}; // 匹配任意长度的数组


    //判断一个类型From是否可以隐式转换为另一个类型To
    //实现真正的is_convertible函数非常复杂，需要编译器的支持
    //给出模拟简单的隐式转换的实现方法
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


	//检查类型T是否有默认构造函数
    //注意这里我们给出的模拟实现的并不能识别出private和protect的构造函数
    /*
    namespace detail {
        template <typename T>
        struct is_default_constructible_test {
            template <typename U, typename = decltype(U())>
            static _true_type test(int) { return _true_type(); } // 需要添加空定义
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


    //检查类型T是否可拷贝构造
    template <typename T>
    struct is_copy_constructible : public std::conditional<
        std::is_copy_constructible<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //检查类型T是否可用给定的参数Args...进行构造
    template <typename T, typename... Args>
    struct is_constructible : public std::conditional<
        std::is_constructible<T, Args...>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //检查类型T是否为左值引用类型
    template <typename T>
    struct is_lvalue_reference : public std::conditional<
        std::is_lvalue_reference<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };



    //检查类型T是否具有平凡的拷贝赋值运算符
    template <typename T>
    struct is_trivially_copy_assignable : public std::conditional<
        std::is_trivially_copy_assignable<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //检查类型T是否具有平凡的移动赋值运算符
    template <typename T>
    struct is_trivially_move_assignable : public std::conditional<
        std::is_trivially_move_assignable<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };


    //检查类型T是否为整型
    template <typename T>
    struct is_integral : public std::conditional<
        std::is_integral<T>::value,
        mystl::_true_type,
        mystl::_false_type
    >::type {
    };

    //根据一个编译时条件，有条件地启用或禁用模板的某个特化、函数重载或类成员
    template <bool B, class T = void>
    struct enable_if {};

    template <class T>
    struct enable_if<true, T> { typedef T type; };



    /*基本的type_traits模板
    我们先将所有性质都设为false_type，也就是说给所有使用该模板的类型一个最保守的值
    接着使用模板特化的方法，为已知为true_type的类型提供特化版本
    */
    template<typename type>
    struct type_traits {
        //指定它拥有的以下特性为的值为_true_type
        //使用例：if(type_trait<T>::istribvial_default_constructor)
        using has_trivial_default_constructor = _false_type;
        using has_trivial_copy_constructor = _false_type;
        using has_trivial_assignment_operator = _false_type;
        using has_trivial_destructor = _false_type;
        using is_POD_type = _false_type;
        //c11弃用了POD，使用is_trivial和is_standard_layout，但是其依赖于编译器的内在函数，我们很难实现
    };


    // 接受普通类型时继承 false_type ,此时其静态成员 value 为 false
    template<typename T>
    struct is_const : public _false_type {};

    // 偏特化使得接受 const 类型时继承 true_type，此时其静态成员 value 为 true
    template<typename T>
    struct is_const<const T> : public _true_type {};

    // 针对指针的偏特化版本
    // 指针本身就是一个特殊的平凡类型，与type无关
    // 指针的构造、拷贝、赋值、析构本质上是整数值（内存地址）的操作，无需调用任何用户定义逻辑
    template<typename type>
    struct type_traits<type*> {
        using has_trivial_default_constructor = _true_type;
        using has_trivial_copy_constructor = _true_type;
        using has_trivial_assignment_operator = _true_type;
        using has_trivial_destructor = _true_type;
        using is_POD_type = _true_type;
    };

	//针对std::nullptr_t的特化
    template<>
    struct type_traits<std::nullptr_t> {
        using has_trivial_default_constructor = _true_type;
        using has_trivial_copy_constructor = _true_type;
        using has_trivial_assignment_operator = _true_type;
        using has_trivial_destructor = _true_type;
        using is_POD_type = _true_type;
	};


    //针对各种算术整型的特化版本
    //利用宏生成这些特化，以减少重复代码
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
    

    //remove_reference移除类型的引用修饰符
    template<class T>
    struct remove_reference
    {
        //默认处理非引用类型
        using type = T;
    };

    //特化左值引用
    template<class T>
    struct remove_reference<T&>
    {
        using type = T;
    };

    //特化右值引用
    template <class T>
    struct remove_reference<T&&>
    {
        using type = T;
    };

    template<typename T>
    struct remove_const
    {
        //默认处理非const类型
        using type = T;
	};

	//特化const类型
	template<typename T>
	struct remove_const<const T>
	{
		using type = T;
	};


    template<typename T>
    struct remove_volatile
    {
        //默认处理非volatile类型
        using type = T;
	};

	//特化volatile类型
	template<typename T>
	struct remove_volatile<volatile T>
	{
		using type = T;
	};

	//remove_cv移除类型的const和volatile修饰符
    template<typename T>
    struct remove_cv
    {
        typedef T type;
    };

    // 偏特化：移除 const
    template<typename T>
    struct remove_cv<const T>
    {
        typedef T type; // 例如 remove_cv<const int>::type 是 int
    };

    // 偏特化：移除 volatile
    template<typename T>
    struct remove_cv<volatile T>
    {
        typedef T type; // 例如 remove_cv<volatile int>::type 是 int
    };

    // 偏特化：移除 const volatile (注意顺序，这个应该在上面两个之后)
    // 编译器会优先匹配最特化的，所以 const volatile T 会匹配这个版本
    template<typename T>
    struct remove_cv<const volatile T>
    {
        typedef T type; // 例如 remove_cv<const volatile int>::type 是 int
    };

	//用于安全的获取对象的地址，即使operator&被重载
	template<typename T>
    inline T* addressof(T& arg) noexcept
    {
        //使用类型转化技巧绕过operator&
        return reinterpret_cast<T*>(//3.转回T*
			&const_cast<char&>(//2.去除const、volatile
                reinterpret_cast<const volatile char&>(arg)//1.将任何指针或引用转换为其他类型的指针或引用，保留底层二进制表示
                )
            );
    }


    //为类型T添加右值引用限定符
    namespace detail {
        template<typename T,bool IsFunction=mystl::is_function<T>::value,bool IsVoid=mystl::is_void<T>::value>
        struct add_rvalue_reference_impl {
            using type = T;//默认情况（如 void, 函数类型，或已经能通过引用折叠处理的引用类型）
        };

		template<typename T>
        struct add_rvalue_reference_impl<T, false, false> {
            using type = T&&; // 对于非函数和非void类型，添加右值引用
		};

        template<typename T>
        struct add_rvalue_reference_impl<T&, false, false> {
            using type = T&; // 对于左值引用，引用折叠为左值
        };

		template<typename T>
        struct add_rvalue_reference_impl<T&&, false, false> {
            using type = T&&; // 对于右值引用，保持右值引用
		};
    }

    template<typename T>
    struct add_rvalue_reference :public detail::add_rvalue_reference_impl<T> {};

    //declval
    //在不实际构建对象的情况下获取其类型
    template<typename T>
    typename mystl::add_rvalue_reference<T>::type declval()noexcept;
}