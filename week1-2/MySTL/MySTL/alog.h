

#ifndef MYTINYSTL_ALGO_H_
#define MYTINYSTL_ALGO_H_
#include <cstring>

#include "iterator.h"
#include "base_function.h"
// ���ذ汾ʹ�ú������� comp ����Ƚϲ���
namespace MySTL
{
    /*****************************************************************************************/
// lexicographical_compare
// ���ֵ������ж��������н��бȽϣ�����ĳ��λ�÷��ֵ�һ�鲻���Ԫ��ʱ�������м��������
// (1)�����һ���е�Ԫ�ؽ�С������ true �����򷵻� false
// (2)������� last1 ����δ���� last2 ���� true
// (3)������� last2 ����δ���� last1 ���� false
// (4)���ͬʱ���� last1 �� last2 ���� false
/*****************************************************************************************/
    template <class InputIter1, class InputIter2>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
        InputIter2 first2, InputIter2 last2)
    {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (*first1 < *first2)
                return true;
            if (*first2 < *first1)
                return false;
        }
        return first1 == last1 && first2 != last2;
    }

    // ���ذ汾ʹ�ú������� comp ����Ƚϲ���
    template <class InputIter1, class InputIter2, class Compred>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
        InputIter2 first2, InputIter2 last2, Compred comp)
    {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (comp(*first1, *first2))
                return true;
            if (comp(*first2, *first1))
                return false;
        }
        return first1 == last1 && first2 != last2;
    }

    // ��� const unsigned char* ���ػ��汾
    bool lexicographical_compare(const unsigned char* first1,
        const unsigned char* last1,
        const unsigned char* first2,
        const unsigned char* last2)
    {
        const auto len1 = last1 - first1;
        const auto len2 = last2 - first2;
        // �ȱȽ���ͬ���ȵĲ���
        const auto result = std::memcmp(first1, first2, MySTL::min(len1, len2)); //����˵memcmpЧ�ʸ���
        // ����ȣ����Ƚϳ��ıȽϴ�
        return result != 0 ? result < 0 : len1 < len2;
    }
}
#endif