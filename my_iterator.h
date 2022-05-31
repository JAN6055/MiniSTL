//
// Created by JAN on 2022/5/29.
//

#ifndef MYSTL__MY_ITERATOR_H_
#define MYSTL__MY_ITERATOR_H_
#include <cstddef>

namespace jan{
	//����ʶ����������͵�һ��
	struct input_iterator{ };
	struct output_iterator{ };
	struct forward_iterator : public input_iterator { };
	struct bidirectional_iterator : public forward_iterator{ };
	struct random_access_iterator : public bidirectional_iterator { };

	/**
	 * Ϊ�˷�ֹ���п����ĵ�������©��һЩtypedef�����Լ̳����������
	 * @tparam Category  ����������
	 * @tparam T		 ��������ָ���Ԫ������
	 * @tparam Distance  ����������֮��ľ�������
	 * @tparam Pointer   ������ָ���Ԫ�ص�ָ������
	 * @tparam Reference ������ָ���Ԫ�ص���������
	 */
	template <typename Category,
		typename T,
		typename Distance 	= ptrdiff_t,
		typename Pointer 	= T *,
		typename Reference  = T &>
	class iterator
	{
		typedef Category	iterator_category;
		typedef T			value_type;
		typedef Distance	difference_type;
		typedef Pointer		pointer;
		typedef Reference	reference;
	};

	template <typename Iterator>
	struct iterator_traits
	{
		typedef typename Iterator::iterator_category	iterator_category;
		typedef typename Iterator::value_type			value_type;
		typedef typename Iterator::difference_type		difference_type;
		typedef typename Iterator::pointer				pointer;
		typedef typename Iterator::reference			reference;
	};

	template<typename T>
	struct iterator_traits<T*>
	{
		typedef random_access_iterator	iterator_category;
		typedef T						value_type;
		typedef ptrdiff_t 				difference_type;
		typedef T*						pointer;
		typedef T&						reference;
	};

	template<typename T>
	struct iterator_traits<const T*>
	{
		typedef random_access_iterator	iterator_category;
		typedef T						value_type;
		typedef ptrdiff_t 				difference_type;
		typedef const T*				pointer;
		typedef const T&				reference;
	};

	template <typename Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
	iterator_category(const Iterator & iterator)
	{

		return iterator_traits<Iterator>::iterator_category ();
	}

	template <typename Iterator>
	inline typename iterator_traits<Iterator>::difference_type
	distance_type(const Iterator & iterator)
	{
		return static_cast<typename iterator_traits<Iterator>::difference_type>(0);
	}

	template <typename Iterator>
	inline typename iterator_traits<Iterator>::value_type
	value_type(const Iterator & iterator)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type>(0);
	}

	//distanceϵ�к���
	template <typename InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	_distance(InputIterator first, InputIterator last, input_iterator)
	{
		typename iterator_traits<InputIterator>::difference_type count = 0;
		while (first != last)
		{
			++first;
			++count;
		}
		return count;
	}

	template <typename RandomIterator>
	inline typename iterator_traits<RandomIterator>::difference_type
	_distance(const RandomIterator & first, const RandomIterator & last, random_access_iterator)
	{
		return last - first;
	}

	template <typename InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	distance(const InputIterator & first, const InputIterator & last)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category category;
		return _distance(first,last,category());
	}

	//advanceϵ�к���
	template <typename InputIterator, typename Distance>
	inline void _advance(InputIterator & i, Distance n, input_iterator)
	{
		while (n--)
			++i;
	}
	template <typename RandomIterator, typename Distance>
	inline void _advance(RandomIterator & i, Distance n, random_access_iterator)
	{
		i += n;
	}

	template <typename BidirectionalIterator, typename Distance>
	inline void _advance(BidirectionalIterator & i, Distance n, bidirectional_iterator)
	{
		if(n > 0)
			while (n--) ++i;
		else
			while (n++) --i;
	}

	template <typename InputIterator, typename Distance>
	inline void advance(InputIterator & i, Distance n)
	{
		_advance(i,n, iterator_category(i));
	}
}
#endif//MYSTL__MY_ITERATOR_H_
