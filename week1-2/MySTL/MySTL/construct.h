#include <new>

#include "iterator.h"

namespace MySTL
{

    // construct �������

    template <class Ty>
    void construct(Ty* ptr)
    {
        ::new ((void*)ptr) Ty();   //���������������ڴ�
    }

    template <class Ty1, class Ty2>
    void construct(Ty1* ptr, const Ty2& value)
    {
        ::new ((void*)ptr) Ty1(value);
    }

    template <class Ty, class... Args>
    void construct(Ty* ptr, Args&&... args)
    {
        ::new ((void*)ptr) Ty(std::forward<Args>(args)...);  
    }

    // destroy ����������

    template <class Ty>
    void destroy_one(Ty*, std::true_type) {}

    template <class Ty>
    void destroy_one(Ty* pointer, std::false_type)
    {
        if (pointer != nullptr)
        {
            pointer->~Ty();
        }
    }

    template <class ForwardIter>
    void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}  

    template <class ForwardIter>
    void destroy_cat(ForwardIter first, ForwardIter last, std::false_type)
    {
        for (; first != last; ++first)
            destroy(&*first);
    }

    template <class Ty>
    void destroy(Ty* pointer)
    {
        destroy_one(pointer, std::is_trivially_destructible<Ty>{});
    }

    template <class ForwardIter>
    void destroy(ForwardIter first, ForwardIter last)
    {
        destroy_cat(first, last, std::is_trivially_destructible<
            typename iterator_traits<ForwardIter>::value_type>{});//��������ȡ���ͣ����жϴ������ޱ�Ҫ�������˴���һ���򵥵�ģ���Ƶ�
    }

} // namespace MySTL