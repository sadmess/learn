#ifndef MYTINYSTL_FUNCTIONAL_H_
#define MYTINYSTL_FUNCTIONAL_H_
#include <cstring>
#include "allocator.h"
#include "iterator.h"

namespace MySTL 
{
	template <class Tp>
	void swap(Tp& lhs, Tp& rhs) {

		auto tmp(std::move(lhs));
		lhs = std::move(rhs);
		rhs = std::move(tmp);
	}

	template <class ForwardIter1, class ForwardIter2>
	ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2) {
		for (; first1 != last1; first1++, (void)first2++) {
			MySTL::swap(*first1, *first2);
		}
		return first2;
	}

	template <class Tp, size_t N>
	void swap(Tp(&a)[N], Tp(&b)[N])
	{
		MySTL::swap_range(a, a + N, b);  //����ʹ�����ã���ʹ�����ÿ��������ʱ����ǳ��������ʱʵ���˻�Ϊָ�룬Nֵ�޷�����
	}

	template <class T>
	const T& max(const T& lhs, const T& rhs)
	{
		return lhs < rhs ? rhs : lhs;
	}

	template <class T,class Compare>
	const T& max(const T& lhs, const T& rhs, Compare comp) {
		return comp(lhs, rhs) ? rhs : lhs;
	}

	template <class T>
	const T& min(const T& lhs, const T& rhs)
	{
		return rhs < lhs ? rhs : lhs;
	}

	template <class T, class Compare>
	const T& min(const T& lhs, const T& rhs, Compare comp)
	{
		return comp(rhs, lhs) ? rhs : lhs;
	}

// iter_swap
// ��������������ָ����Ե�
	template <class FIter1, class FIter2>
	void iter_swap(FIter1 lhs, FIter2 rhs)
	{
		MySTL::swap(*lhs, *rhs);
	}


// copy
// �� [first, last)�����ڵ�Ԫ�ؿ����� [result, result + (last - first))��
	template <class InputIter, class OutputIter>
	OutputIter
		unchecked_copy(InputIter first, InputIter last, OutputIter result)
	{
		for (; first != last; ++first, ++result)
		{
			*result = *first;
		}
		return result;
	}

	template <class Tp, class Up>
	typename std::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
		std::is_trivially_copy_assignable<Up>::value,
		Up*>::type
		unchecked_copy(Tp* first, Tp* last, Up* result)
	{
		const auto n = static_cast<size_t>(last - first);
		if (n != 0)
			std::memmove(result, first, n * sizeof(Up));
		return result + n;
	}

	template <class InputIter, class OutputIter>
	OutputIter copy(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_copy(first, last, result);
	}

// copy_n
// �� [first, first + n)�����ϵ�Ԫ�ؿ����� [result, result + n)��

	template <class InputIter, class Size, class OutputIter>
	OutputIter
		unchecked_copy_n(InputIter first, Size n, OutputIter result, MySTL::input_iterator_tag)
	{
		for (; n > 0; --n, ++first, ++result)
		{
			*result = *first;
		}
		return result;
	}

	template <class RandomIter, class Size, class OutputIter>
	OutputIter
		unchecked_copy_n(RandomIter first, Size n, OutputIter result,
			MySTL::random_access_iterator_tag)
	{
		auto last = first + n;
		return MySTL::copy(first, last, result);
	}

	template <class InputIter, class Size, class OutputIter>
	OutputIter
		copy_n(InputIter first, Size n, OutputIter result)
	{
		return unchecked_copy_n(first, n, result, iterator_category(first));
	}

// copy_backward
// �� [first, last)�����ڵ�Ԫ�ؿ����� [result - (last - first), result)��

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
			BidirectionalIter2 result)
	{
		while (first != last)
			*--result = *--last;
		return result;
	}

	template <class Tp, class Up>
	typename std::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
		std::is_trivially_copy_assignable<Up>::value,
		Up*>::type
		unchecked_copy_backward(Tp* first, Tp* last, Up* result)
	{
		const auto n = static_cast<size_t>(last - first);
		if (n != 0)
		{
			result -= n;
			std::memmove(result, first, n * sizeof(Up));
		}
		return result;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
	{
		return unchecked_copy_backward(first, last, result);
	}
// copy_if
// ��[first, last)������һԪ���� unary_pred ��Ԫ�ؿ������� result Ϊ��ʼ��λ����
	template <class InputIter, class OutputIter, class UnaryPredicate>
	OutputIter
		copy_if(InputIter first, InputIter last, OutputIter result, UnaryPredicate unary_pred)
	{
		for (; first != last; ++first)
		{
			if (unary_pred(*first))
				*result++ = *first;
		}
		return result;
	}
// move
// �� [first, last)�����ڵ�Ԫ���ƶ��� [result, result + (last - first))��
	template <class InputIter, class OutputIter>
	OutputIter
		unchecked_move(InputIter first, InputIter last, OutputIter result)
	{
		for (; first != last; ++first, ++result)
		{
			*result = std::move(*first);
		}
		return result;
	}

	template <class Tp, class Up>
	typename std::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
		std::is_trivially_move_assignable<Up>::value,
		Up*>::type
		unchecked_move(Tp* first, Tp* last, Up* result)
	{
		const size_t n = static_cast<size_t>(last - first);
		if (n != 0)
			std::memmove(result, first, n * sizeof(Up));
		return result + n;
	}

	template <class InputIter, class OutputIter>
	OutputIter move(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_move(first, last, result);
	}
// move_backward
// �� [first, last)�����ڵ�Ԫ���ƶ��� [result - (last - first), result)��
	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
			BidirectionalIter2 result)
	{
		while (first != last)
			*--result = MySTL::move(*--last);
		return result;
	}

	template <class Tp, class Up>
	typename std::enable_if<
		std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
		std::is_trivially_move_assignable<Up>::value,
		Up*>::type
		unchecked_move_backward(Tp* first, Tp* last, Up* result)
	{
		const size_t n = static_cast<size_t>(last - first);
		if (n != 0)
		{
			result -= n;
			std::memmove(result, first, n * sizeof(Up));
		}
		return result;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
	{
		return unchecked_move_backward(first, last, result);
	}
// equal
// �Ƚϵ�һ������ [first, last)�����ϵ�Ԫ��ֵ�Ƿ�͵ڶ��������
/*****************************************************************************************/
	template <class InputIter1, class InputIter2>
	bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			if (*first1 != *first2)
				return false;
		}
		return true;
	}

	template <class InputIter1, class InputIter2, class Compared>
	bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp)
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			if (!comp(*first1, *first2))
				return false;
		}
		return true;
	}
// fill_n
// �� first λ�ÿ�ʼ��� n ��ֵ
	template <class OutputIter, class Size, class T>
	OutputIter fill_n(OutputIter first, Size n, const T& value)
	{
		for (; n > 0; --n, ++first)
		{
			*first = value;
		}
		return first;
	}

	template <class ForwardIter, class T>
	void fill_cat(ForwardIter first, ForwardIter last, const T& value,
		MySTL::forward_iterator_tag)
	{
		for (; first != last; ++first)
		{
			*first = value;
		}
	}

	template <class RandomIter, class T>
	void fill_cat(RandomIter first, RandomIter last, const T& value,
		MySTL::random_access_iterator_tag)
	{
		fill_n(first, last - first, value);
	}

	template <class ForwardIter, class T>
	void fill(ForwardIter first, ForwardIter last, const T& value)
	{
		fill_cat(first, last, value, iterator_category(first));
	}
// uninitialized_copy
// �� [first, last) �ϵ����ݸ��Ƶ��� result Ϊ��ʼ���Ŀռ䣬���ظ��ƽ�����λ��
	template <class InputIter, class ForwardIter>
	ForwardIter
		unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::true_type)
	{
		return MySTL::copy(first, last, result);
	}

	template <class InputIter, class ForwardIter>
	ForwardIter
		unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::false_type)
	{
		auto cur = result;
		try
		{
			for (; first != last; ++first, ++cur)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			for (; result != cur; ++result)
				MySTL::destroy(result);
		}
		return cur;
	}

	template <class InputIter, class ForwardIter>
	ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result)
	{
		return MySTL::unchecked_uninit_copy(first, last, result,
			std::is_trivially_copy_assignable<
			typename iterator_traits<ForwardIter>::
			value_type>{});
	}

// uninitialized_copy_n
// �� [first, first + n) �ϵ����ݸ��Ƶ��� result Ϊ��ʼ���Ŀռ䣬���ظ��ƽ�����λ��
	template <class InputIter, class Size, class ForwardIter>
	ForwardIter
		unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::true_type)
	{
		return MySTL::copy_n(first, n, result);
	}

	template <class InputIter, class Size, class ForwardIter>
	ForwardIter
		unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::false_type)
	{
		auto cur = result;
		try
		{
			for (; n > 0; --n, ++cur, ++first)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			for (; result != cur; ++result)
				MySTL::destroy(&*result);
		}
		return cur;
	}

	template <class InputIter, class Size, class ForwardIter>
	ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result)
	{
		return MySTL::unchecked_uninit_copy_n(first, n, result,
			std::is_trivially_copy_assignable<
			typename iterator_traits<InputIter>::
			value_type>{});
	}

// uninitialized_fill
// �� [first, last) ���������Ԫ��ֵ

	template <class ForwardIter, class T>
	void
		unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::true_type)
	{
		MySTL::fill(first, last, value);
	}

	template <class ForwardIter, class T>
	void
		unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::false_type)
	{
		auto cur = first;
		try
		{
			for (; cur != last; ++cur)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			for (; first != cur; ++first)
				MySTL::destroy(first);
		}
	}

	template <class ForwardIter, class T>
	void  uninitialized_fill(ForwardIter first, ForwardIter last, const T& value)
	{
		MySTL::unchecked_uninit_fill(first, last, value,
			std::is_trivially_copy_assignable<
			typename iterator_traits<ForwardIter>::
			value_type>{});
	}
// uninitialized_fill_n
// �� first λ�ÿ�ʼ����� n ��Ԫ��ֵ��������������λ��
	template <class ForwardIter, class Size, class T>
	ForwardIter
		unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::true_type)
	{
		return MySTL::fill_n(first, n, value);
	}

	template <class ForwardIter, class Size, class T>
	ForwardIter
		unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::false_type)
	{
		auto cur = first;
		try
		{
			for (; n > 0; --n, ++cur)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			for (; first != cur; ++first)
				MySTL::destroy(&*first);
		}
		return cur;
	}

	template <class ForwardIter, class Size, class T>
	ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value)
	{
		return MySTL::unchecked_uninit_fill_n(first, n, value,
			std::is_trivially_copy_assignable<
			typename iterator_traits<ForwardIter>::
			value_type>{});
	}

// uninitialized_move
// ��[first, last)�ϵ������ƶ����� result Ϊ��ʼ���Ŀռ䣬�����ƶ�������λ��
	template <class InputIter, class ForwardIter>
	ForwardIter
		unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::true_type)
	{
		return MySTL::move(first, last, result);
	}

	template <class InputIter, class ForwardIter>
	ForwardIter
		unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type)
	{
		ForwardIter cur = result;
		try
		{
			for (; first != last; ++first, ++cur)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			MySTL::destroy(result, cur);
		}
		return cur;
	}

	template <class InputIter, class ForwardIter>
	ForwardIter uninitialized_move(InputIter first, InputIter last, ForwardIter result)
	{
		return MySTL::unchecked_uninit_move(first, last, result,
			std::is_trivially_move_assignable<
			typename iterator_traits<InputIter>::
			value_type>{});
	}

// uninitialized_move_n
// ��[first, first + n)�ϵ������ƶ����� result Ϊ��ʼ���Ŀռ䣬�����ƶ�������λ��
	template <class InputIter, class Size, class ForwardIter>
	ForwardIter
		unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type)
	{
		return MySTL::move(first, first + n, result);
	}

	template <class InputIter, class Size, class ForwardIter>
	ForwardIter
		unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::false_type)
	{
		auto cur = result;
		try
		{
			for (; n > 0; --n, ++first, ++cur)
			{
				MySTL::construct(cur, *first);
			}
		}
		catch (...)
		{
			for (; result != cur; ++result)
				MySTL::destroy(&*result);
			throw;
		}
		return cur;
	}

	template <class InputIter, class Size, class ForwardIter>
	ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result)
	{
		return MySTL::unchecked_uninit_move_n(first, n, result,
			std::is_trivially_move_assignable<
			typename iterator_traits<InputIter>::
			value_type>{});
	}

	template <class Tp>
	constexpr Tp* address_of(Tp& value) noexcept
	{
		return &value;
	}

// lower_bound
// ��[first, last)�в��ҵ�һ����С�� value ��Ԫ�ص�λ��
// ����һ����������ָ���ڷ�Χ�ڵ����������п��Բ���ָ��ֵ�����ƻ�����˳��ĵ�һ��λ��
	template <class ForwardIter, class T>
	ForwardIter
		lbound_dispatch(ForwardIter first, ForwardIter last,
			const T& value, forward_iterator_tag)
	{
		auto len = MySTL::distance(first, last);
		auto half = len;
		ForwardIter middle;
		while (len > 0)
		{
			half = len >> 1;
			middle = first;
			MySTL::advance(middle, half);
			if (*middle < value)
			{
				first = middle;
				++first;
				len = len - half - 1;
			}
			else
			{
				len = half;
			}
		}
		return first;
	}

	// lbound_dispatch �� random_access_iterator_tag �汾
	template <class RandomIter, class T>
	RandomIter
		lbound_dispatch(RandomIter first, RandomIter last,
			const T& value, random_access_iterator_tag)
	{
		auto len = last - first;
		auto half = len;
		RandomIter middle;
		while (len > 0)
		{
			half = len >> 1;
			middle = first + half;
			if (*middle < value)
			{
				first = middle + 1;
				len = len - half - 1;
			}
			else
			{
				len = half;
			}
		}
		return first;
	}

	template <class ForwardIter, class T>
	ForwardIter
		lower_bound(ForwardIter first, ForwardIter last, const T& value)
	{
		return MySTL::lbound_dispatch(first, last, value, iterator_category(first));
	}

}

#endif