#pragma once
#include <cstddef>  // For size_t, ptrdiff_t
#include <cstdlib>  // For malloc, free
#include <new>      // For std::bad_alloc

namespace mystl
{

    template <class T>
    class malloc_allocator
    {
    public:
        typedef T           value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef size_t      size_type;
        typedef ptrdiff_t   difference_type;

        // Rebind allocator to type U
        template <class U>
        struct rebind {
            typedef malloc_allocator<U> other;
        };

        static pointer allocate(size_type n)
        {
            pointer result = static_cast<pointer>(malloc(n * sizeof(T)));
            if (!result)
            {
                throw std::bad_alloc();
            }
            return result;
        }

        static void deallocate(pointer ptr, size_type /*n*/)
        {
            free(ptr);
        }
    };

}
