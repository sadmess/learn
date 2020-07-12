#include "allocator.h"
#include "iterator.h"
#include "base_function.h"
#include "alog.h"
#include "exception.h"

namespace MySTL
{
	template <class T> struct list_node_base;
	template <class T> struct list_node;

	template <class T>
	struct node_traits
	{
		typedef list_node_base<T>* base_ptr;
		typedef list_node<T>* node_ptr;
	};

	template <class T>
	struct list_node_base
	{
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;

		base_ptr prev;
		base_ptr next;

		list_node_base() = default;

		node_ptr as_node() {
			return static_cast<node_ptr>(this);
		}

		void unlink()
		{
			prev = next = this;
		}

		base_ptr self() {
			return static_cast<base_ptr>(this);
		}
	};

	template <class T>
	struct list_node : public list_node_base<T>
	{
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;

		T value;

		list_node() = default;
		list_node(const T& v) : value(v){}
		list_node(T&& v):value(std::move(v)){}

		base_ptr as_base() {
			return static_cast<base_ptr>(this);
		}

		node_ptr self() {
			return static_cast<node_ptr>(this);
		}
	};

	template <class T>
	struct list_iterator :public MySTL::iterator<MySTL::bidirectional_iterator_tag,T>
	{
		typedef T                                 value_type;
		typedef T*		                          pointer;
		typedef T&                                reference;
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;
		typedef list_iterator<T>                  self;
	
		base_ptr node_;

		list_iterator() = default;
		list_iterator(base_ptr x):node_(x){}
		list_iterator(node_ptr x):node_(x){}
		list_iterator(const list_iterator& rhs):node_(rhs.node_){}

		reference operator*() const {
			return node_->as_node()->value;
		}

		pointer operator->() const {
			return &(operator*());
		}

		self& operator++() {
			node_ = node_->next;
			return *this;
		}

		self operator++(int) {
			self tmp = *this;
			node_ = node_->next;
			return tmp;
		}

		self& operator--()
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->prev;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			node_ = node_->prev;
			return tmp;
		}

		bool operator==(const self& rhs) const {
			return node_ == rhs.node_;
		}

		bool operator!=(const self& rhs) const {
			return node_ != rhs.node_;
		}
	};

	template <class T>
	struct list_const_iterator :public iterator<bidirectional_iterator_tag,T>
	{
		typedef T                                 value_type;
		typedef const T* pointer;
		typedef const T& reference;
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;
		typedef list_const_iterator<T>            self;
	
		base_ptr node_;

		list_const_iterator() = default;
		list_const_iterator(base_ptr x):node_(x) {}
		list_const_iterator(node_ptr x):node_(x->as_base()) {}
		list_const_iterator(const list_iterator<T> & rhs):node_(rhs.node_) {}
		list_const_iterator(const list_const_iterator & rhs):node_(rhs.node_) {}

		reference operator*() const {
			return node_->as_node()->value;
		}

		pointer operator->() const {
			return &(operator*());
		}

		self& operator++() {
			node_ = node_->next;
			return *this;
		}

		self operator++(int) {
			self tmp = *this;
			node_ = node_->next;
			return tmp;
		}

		self& operator--()
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->prev;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			node_ = node_->prev;
			return tmp;
		}

		bool operator==(const self& rhs) const { 
			return node_ == rhs.node_; 
		}
		bool operator!=(const self& rhs) const { 
			return node_ != rhs.node_; 
		}
	};
	
	template <class T>
	class list {
	public:
		typedef MySTL::allocator<T>                      allocator_type;
		typedef MySTL::allocator<T>                      data_allocator;
		typedef MySTL::allocator<list_node_base<T>>      base_allocator;
		typedef MySTL::allocator<list_node<T>>           node_allocator;

		typedef typename allocator_type::value_type      value_type;
		typedef typename allocator_type::pointer         pointer;
		typedef typename allocator_type::const_pointer   const_pointer;
		typedef typename allocator_type::reference       reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename allocator_type::size_type       size_type;
		typedef typename allocator_type::difference_type difference_type;

		typedef list_iterator<T>                         iterator;
		typedef list_const_iterator<T>                   const_iterator;
		typedef MySTL::reverse_iterator<iterator>        reverse_iterator;
		typedef MySTL::reverse_iterator<const_iterator>  const_reverse_iterator;

		typedef typename node_traits<T>::base_ptr        base_ptr;
		typedef typename node_traits<T>::node_ptr        node_ptr;

	private:
		base_ptr node_;
		size_type size_;
	public:
		list() {
			fill_init(0, value_type());
		}

		explicit list(size_type n) {
			fill_init(n, value_type());
		}
		list(size_type n, const T& value) {
			fill_init(n, value);
		}

		template <class Iter, typename std::enable_if<
			MySTL::is_input_iterator<Iter>::value, int>::type = 0>
			list(Iter first, Iter last) {
			copy_init(first, last);
		}

		list(std::initializer_list<T> ilist) {
			copy_init(ilist.begin(), ilist.end());
		}

		list(const list& rhs) {
			copy_init(rhs.cbegin(), rhs.cend());
		}

		list(list&& rhs) noexcept
			:node_(rhs.node_), size_(rhs.size_) {
			rhs.node_ = nullptr;
			rhs.size_ = 0;
		}

		list& operator=(const list& rhs) {
			if (this != &rhs) {
				assign(rhs.begin(), rhs.end());
			}
			return *this;
		}
		list& operator=(list&& rhs) noexcept
		{
			clear();
			splice(end(), rhs);
			return *this;
		}
		list& operator=(std::initializer_list<T> ilist) {
			list tmp(ilist.begin(), ilist.end());
			swap(tmp);
			return *this;
		}

		~list() {
			if (node_) {
				clear();
				base_allocator::deallocate(node_);
				node_ = nullptr;
				size_ = 0;
			}
		}

	public:
		iterator               begin()         noexcept
		{
			return node_->next;
		}
		const_iterator         begin()   const noexcept
		{
			return node_->next;
		}
		iterator               end()           noexcept
		{
			return node_;
		}
		const_iterator         end()     const noexcept
		{
			return node_;
		}

		reverse_iterator       rbegin()        noexcept
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator rbegin()  const noexcept
		{
			return reverse_iterator(end());
		}
		reverse_iterator       rend()          noexcept
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator rend()    const noexcept
		{
			return reverse_iterator(begin());
		}

		const_iterator         cbegin()  const noexcept
		{
			return begin();
		}
		const_iterator         cend()    const noexcept
		{
			return end();
		}
		const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}
		const_reverse_iterator crend()   const noexcept
		{
			return rend();
		}

		bool empty() const noexcept {
			return node_->next == node_;
		}

		size_type size() const noexcept {
			return size_;
		}

		size_type max_size() const noexcept
		{
			return static_cast<size_type>(-1);
		}

		const_reference front() const {
			return *begin();
		}

		reference       back()
		{
			return *(--end());
		}

		const_reference back()  const
		{
			return *(--end());
		}

		void assign(size_type n, const value_type& value) {
			fill_assign(n, value);
		}

		template <class Iter,typename std::enable_if<
			MySTL::is_input_iterator<Iter>::value,int>::type=0>
			void assign(Iter first, Iter last) {
		}

		void assign(std::initializer_list<T> ilist) {
			copy_assign(ilist.begin(), ilist.end());
		}

		template <class ...Args>
		void emplace_front(Args&& ...args) {
			auto link_node = create_node(std::forward<Args>(args)...);
			link_nodes_at_front(link_node->as_base(), link_node->as_base());
			++size_;
		}

		template <class ...Args>
		void emplace_back(Args&& ...args) {
			auto link_node = create_node(std::forward<Args>(args)...);
			link_nodes_at_back(link_node->as_base(), link_node->as_base());
			++size_;
		}

		template <class ...Args>
		iterator emplace(const_iterator pos, Args&& ...args)
		{
			auto link_node = create_node(std::forward<Args>(args)...);
			link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
			++size_;
			return iterator(link_node);
		}

		iterator insert(const_iterator pos, const value_type& value) {
			auto link_node = create_node(value);
			++size_;
			return link_iter_node(pos, link_node->as_base());
		}

		iterator insert(const_iterator pos, value_type&& value) {
			auto link_node = create_node(std::move(value));
			++size_;
			return link_iter_node(pos, link_node->as_base());
		}

		iterator insert(const_iterator pos, size_type n, const value_type& value) {
			return fill_insert(pos, n, value);
		}

		template <class Iter, typename std::enable_if<
			MySTL::is_input_iterator<Iter>::value, int>::type = 0>
			iterator insert(const_iterator pos, Iter first, Iter last) {
			size_type n = MySTL::distance(first, last);
			return copy_insert(pos, n, first);
		}


		void push_front(const value_type& value) {
			emplace_front(value);
		}

		void push_front(value_type&& value) {
			emplace_front(std::move(value));
		}

		void push_back(const value_type& value) {
			emplace_back(value);
		}

		void push_back(value_type&& value) {
			emplace_back(std::move(value));
		}

		void pop_front() {
			auto n = node_->next;
			unlink_nodes(n, n);
			destroy_node(n->as_node());
			--size_;
		}

		void pop_back() {
			auto n = node_->prev;
			unlink_nodes(n, n);
			destroy_node(n->as_node());
			--size_;
		}

		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);

		void clear();

		void resize(size_type new_size) { 
			resize(new_size, value_type()); 
		}
		void resize(size_type new_size, const value_type& value);
		void swap(list& rhs) noexcept {
			
			MySTL::swap(node_, rhs.node_);
			MySTL::swap(size_, rhs.size_);
		}

		void splice(const_iterator pos, list& other);
		void splice(const_iterator pos, list& other, const_iterator it);
		void splice(const_iterator pos, list& other, const_iterator first, const_iterator last);

		void remove(const value_type& value)
		{
			remove_if([&](const value_type& v) {return v == value; });
		}

		template <class UnaryPredicate>
		void remove_if(UnaryPredicate pred);

		void unique() {
			unique(std::equal_to<T>());
		}
		template <class BinaryPredicate>
		void unique(BinaryPredicate pred);
		void merge(list& x)
		{
			merge(x, std::less<T>());
		}
		template <class Compare>
		void merge(list& x, Compare comp);

		void sort()
		{
			list_sort(begin(), end(), size(), std::less<T>());
		}
		template <class Compared>
		void sort(Compared comp)
		{
			list_sort(begin(), end(), size(), comp);
		}

		void reverse();

		private:
			// helper functions

			// create / destroy node
			template <class ...Args>
			node_ptr create_node(Args&& ...agrs);
			void     destroy_node(node_ptr p);

			// initialize
			void      fill_init(size_type n, const value_type& value);
			template <class Iter>
			void      copy_init(Iter first, Iter last);

			// link / unlink
			iterator  link_iter_node(const_iterator pos, base_ptr node);
			void      link_nodes(base_ptr p, base_ptr first, base_ptr last);
			void      link_nodes_at_front(base_ptr first, base_ptr last);
			void      link_nodes_at_back(base_ptr first, base_ptr last);
			void      unlink_nodes(base_ptr f, base_ptr l);

			// assign
			void      fill_assign(size_type n, const value_type& value);
			template <class Iter>
			void      copy_assign(Iter first, Iter last);

			// insert
			iterator  fill_insert(const_iterator pos, size_type n, const value_type& value);
			template <class Iter>
			iterator  copy_insert(const_iterator pos, size_type n, Iter first);

			// sort
			template <class Compared>
			iterator  list_sort(iterator first, iterator last, size_type n, Compared comp);
	};

	template <class T>
	typename list<T>::iterator
	list<T>::erase(const_iterator pos) {
		auto n = pos.node_;
		auto next = n->next;
		unlink_nodes(n, n);
		destroy_node(n->as_node());
		--size_;
		return iterator(next);
	}

	template <class T>
	typename list<T>::iterator
	list<T>::erase(const_iterator first, const_iterator last) {
		if (first != last) {
			unlink_nodes(first.node_, last.node_->prev);
			while (first != last) {
				auto cur = first.node_;
				++first;
				destroy_node(cur->as_node());
				--size_;
			}
		}
		return iterator(last.node_);
	}

	template <class T>
	void list<T>::clear() {
		if (size_ != 0)
		{
			auto cur = node_->next;
			for (base_ptr next = cur->next; cur != node_; cur = next, next = cur->next)
			{
				destroy_node(cur->as_node());
			}
			node_->unlink();
			size_ = 0;
		}
	}

	template <class T>
	void list<T>::resize(size_type new_size, const value_type& value) {
		auto i = begin();
		size_type len = 0;
		while (i != end() && len < new_size)
		{
			++i;
			++len;
		}
		if (len == new_size)
		{
			erase(i, node_);
		}
		else
		{
			insert(node_, new_size - len, value);
		}
	}

	template <class T>
	void list<T>::splice(const_iterator pos, list& x) {
		MYSTL_DEBUG(this != &x);
		if (!x.empty())
		{
			auto f = x.node_->next;
			auto l = x.node_->prev;

			x.unlink_nodes(f, l);
			link_nodes(pos.node_, f, l);

			size_ += x.size_;
			x.size_ = 0;
		}
	}

	template <class T>
	void list<T>::splice(const_iterator pos, list& x, const_iterator it)
	{
		MYSTL_DEBUG(this != &x);
		if (pos.node_ != it.node_ && pos.node_ != it.node_->next)
		{

			auto f = it.node_;

			x.unlink_nodes(f, f);
			link_nodes(pos.node_, f, f);

			++size_;
			--x.size_;
		}
	}

	template <class T>
	void list<T>::splice(const_iterator pos, list& x, const_iterator first, const_iterator last)
	{
		MYSTL_DEBUG(this != &x);
		if (first != last && this != &x)
		{
			size_type n = MySTL::distance(first, last);
			auto f = first.node_;
			auto l = last.node_->prev;

			x.unlink_nodes(f, l);
			link_nodes(pos.node_, f, l);

			size_ += n;
			x.size_ -= n;
		}
	}

	template <class T>
	template <class UnaryPredicate>
	void list<T>::remove_if(UnaryPredicate pred)
	{
		auto f = begin();
		auto l = end();
		for (auto next = f; f != l; f = next)
		{
			++next;
			if (pred(*f))
			{
				erase(f);
			}
		}
	}

	template <class T>
	template <class BinaryPredicate>
	void list<T>::unique(BinaryPredicate pred)
	{
		auto i = begin();
		auto e = end();
		auto j = i;
		++j;
		while (j != e)
		{
			if (pred(*i, *j))
			{
				erase(j);
			}
			else
			{
				i = j;
			}
			j = i;
			++j;
		}
	}

	template <class T>
	template <class Compare>
	void list<T>::merge(list& x, Compare comp)
	{
		MYSTL_DEBUG(this != &x);
		if (this != &x)
		{

			auto f1 = begin();
			auto l1 = end();
			auto f2 = x.begin();
			auto l2 = x.end();

			while (f1 != l1 && f2 != l2)
			{
				if (comp(*f2, *f1))
				{
					// 使 comp 为 true 的一段区间
					auto next = f2;
					++next;
					for (; next != l2 && comp(*next, *f1); ++next)
						;
					auto f = f2.node_;
					auto l = next.node_->prev;
					f2 = next;

					// link node
					x.unlink_nodes(f, l);
					link_nodes(f1.node_, f, l);
					++f1;
				}
				else
				{
					++f1;
				}
			}
			// 连接剩余部分
			if (f2 != l2)
			{
				auto f = f2.node_;
				auto l = l2.node_->prev;
				x.unlink_nodes(f, l);
				link_nodes(l1.node_, f, l);
			}

			size_ += x.size_;
			x.size_ = 0;
		}
	}

	template <class T>
	void list<T>::reverse()
	{
		if (size_ <= 1)
		{
			return;
		}
		auto i = begin();
		auto f = begin();
		auto e = end();
		int k = 0;
		while (i.node_ != e.node_)
		{
			MySTL::swap(i.node_->prev, i.node_->next);
			i.node_ = i.node_->prev;
		}
		MySTL::swap(e.node_->prev, e.node_->next);
	}
	// helper function

	template <class T>
	template <class ...Args>
	typename list<T>::node_ptr
		list<T>::create_node(Args&& ...args)
	{
		node_ptr p = node_allocator::allocate(1);
		try
		{
			data_allocator::construct(MySTL::address_of(p->value), std::forward<Args>(args)...);
			p->prev = nullptr;
			p->next = nullptr;
		}
		catch (...)
		{
			node_allocator::deallocate(p);
			throw;
		}
		return p;
	}

	template <class T>
	void list<T>::destroy_node(node_ptr p)
	{
		data_allocator::destroy(MySTL::address_of(p->value));
		node_allocator::deallocate(p);
	}

	template <class T>
	void list<T>::fill_init(size_type n, const value_type& value)
	{
		node_ = base_allocator::allocate(1);
		node_->unlink();
		size_ = n;
		try
		{
			for (; n > 0; --n)
			{
				auto node = create_node(value);
				link_nodes_at_back(node->as_base(), node->as_base());
			}
		}
		catch (...)
		{
			clear();
			base_allocator::deallocate(node_);
			node_ = nullptr;
			throw;
		}
	}

	template <class T>
	template <class Iter>
	void list<T>::copy_init(Iter first, Iter last)
	{
		node_ = base_allocator::allocate(1);
		node_->unlink();
		size_type n = MySTL::distance(first, last);
		size_ = n;
		try
		{
			for (; n > 0; --n, ++first)
			{
				auto node = create_node(*first);
				link_nodes_at_back(node->as_base(), node->as_base());
			}
		}
		catch (...)
		{
			clear();
			base_allocator::deallocate(node_);
			node_ = nullptr;
			throw;
		}
	}

	template <class T>
	typename list<T>::iterator
		list<T>::link_iter_node(const_iterator pos, base_ptr link_node)
	{
		if (pos == node_->next)
		{
			link_nodes_at_front(link_node, link_node);
		}
		else if (pos == node_)
		{
			link_nodes_at_back(link_node, link_node);
		}
		else
		{
			link_nodes(pos.node_, link_node, link_node);
		}
		return iterator(link_node);
	}

	// 在 pos 处连接 [first, last] 的结点
	template <class T>
	void list<T>::link_nodes(base_ptr pos, base_ptr first, base_ptr last)
	{
		pos->prev->next = first;
		first->prev = pos->prev;
		pos->prev = last;
		last->next = pos;
	}

	// 在头部连接 [first, last] 结点
	template <class T>
	void list<T>::link_nodes_at_front(base_ptr first, base_ptr last)
	{
		first->prev = node_;
		last->next = node_->next;
		last->next->prev = last;
		node_->next = first;
	}

	// 在尾部连接 [first, last] 结点
	template <class T>
	void list<T>::link_nodes_at_back(base_ptr first, base_ptr last)
	{
		last->next = node_;
		first->prev = node_->prev;
		first->prev->next = first;
		node_->prev = last;
	}

	// 容器与 [first, last] 结点断开连接
	template <class T>
	void list<T>::unlink_nodes(base_ptr first, base_ptr last)
	{
		first->prev->next = last->next;
		last->next->prev = first->prev;
	}

	// 用 n 个元素为容器赋值
	template <class T>
	void list<T>::fill_assign(size_type n, const value_type& value)
	{
		auto i = begin();
		auto e = end();
		for (; n > 0 && i != e; --n, ++i)
		{
			*i = value;
		}
		if (n > 0)
		{
			insert(e, n, value);
		}
		else
		{
			erase(i, e);
		}
	}

	// 复制[f2, l2)为容器赋值
	template <class T>
	template <class Iter>
	void list<T>::copy_assign(Iter f2, Iter l2)
	{
		auto f1 = begin();
		auto l1 = end();
		for (; f1 != l1 && f2 != l2; ++f1, ++f2)
		{
			*f1 = *f2;
		}
		if (f2 == l2)
		{
			erase(f1, l1);
		}
		else
		{
			insert(l1, f2, l2);
		}
	}

	// 在 pos 处插入 n 个元素
	template <class T>
	typename list<T>::iterator
		list<T>::fill_insert(const_iterator pos, size_type n, const value_type& value)
	{
		iterator r(pos.node_);
		if (n != 0)
		{
			const auto add_size = n;
			auto node = create_node(value);
			node->prev = nullptr;
			r = iterator(node);
			iterator end = r;
			try
			{
				// 前面已经创建了一个节点，还需 n - 1 个
				for (--n; n > 0; --n, ++end)
				{
					auto next = create_node(value);
					end.node_->next = next->as_base();  // link node
					next->prev = end.node_;
				}
				size_ += add_size;
			}
			catch (...)
			{
				auto enode = end.node_;
				while (true)
				{
					auto prev = enode->prev;
					destroy_node(enode->as_node());
					if (prev == nullptr)
						break;
					enode = prev;
				}
				throw;
			}
			link_nodes(pos.node_, r.node_, end.node_);
		}
		return r;
	}

	// 在 pos 处插入 [first, last) 的元素
	template <class T>
	template <class Iter>
	typename list<T>::iterator
		list<T>::copy_insert(const_iterator pos, size_type n, Iter first)
	{
		iterator r(pos.node_);
		if (n != 0)
		{
			const auto add_size = n;
			auto node = create_node(*first);
			node->prev = nullptr;
			r = iterator(node);
			iterator end = r;
			try
			{
				for (--n, ++first; n > 0; --n, ++first, ++end)
				{
					auto next = create_node(*first);
					end.node_->next = next->as_base();  // link node
					next->prev = end.node_;
				}
				size_ += add_size;
			}
			catch (...)
			{
				auto enode = end.node_;
				while (true)
				{
					auto prev = enode->prev;
					destroy_node(enode->as_node());
					if (prev == nullptr)
						break;
					enode = prev;
				}
				throw;
			}
			link_nodes(pos.node_, r.node_, end.node_);
		}
		return r;
	}

	// 对 list 进行归并排序，返回一个迭代器指向区间最小元素的位置
	template <class T>
	template <class Compared>
	typename list<T>::iterator
		list<T>::list_sort(iterator f1, iterator l2, size_type n, Compared comp)
	{
		if (n < 2)
			return f1;

		if (n == 2)
		{
			if (comp(*--l2, *f1))
			{
				auto ln = l2.node_;
				unlink_nodes(ln, ln);
				link_nodes(f1.node_, ln, ln);
				return l2;
			}
			return f1;
		}

		auto n2 = n / 2;
		auto l1 = f1;
		MySTL::advance(l1, n2);
		auto result = f1 = list_sort(f1, l1, n2, comp);  // 前半段的最小位置
		auto f2 = l1 = list_sort(l1, l2, n - n2, comp);  // 后半段的最小位置

		// 把较小的一段区间移到前面
		if (comp(*f2, *f1))
		{
			auto m = f2;
			++m;
			for (; m != l2 && comp(*m, *f1); ++m)
				;
			auto f = f2.node_;
			auto l = m.node_->prev;
			result = f2;
			l1 = f2 = m;
			unlink_nodes(f, l);
			m = f1;
			++m;
			link_nodes(f1.node_, f, l);
			f1 = m;
		}
		else
		{
			++f1;
		}

		// 合并两段有序区间
		while (f1 != l1 && f2 != l2)
		{
			if (comp(*f2, *f1))
			{
				auto m = f2;
				++m;
				for (; m != l2 && comp(*m, *f1); ++m)
					;
				auto f = f2.node_;
				auto l = m.node_->prev;
				if (l1 == f2)
					l1 = m;
				f2 = m;
				unlink_nodes(f, l);
				m = f1;
				++m;
				link_nodes(f1.node_, f, l);
				f1 = m;
			}
			else
			{
				++f1;
			}
		}
		return result;
	}

	// 重载比较操作符
	template <class T>
	bool operator==(const list<T>& lhs, const list<T>& rhs)
	{
		auto f1 = lhs.cbegin();
		auto f2 = rhs.cbegin();
		auto l1 = lhs.cend();
		auto l2 = rhs.cend();
		for (; f1 != l1 && f2 != l2 && *f1 == *f2; ++f1, ++f2)
			;
		return f1 == l1 && f2 == l2;
	}

	template <class T>
	bool operator<(const list<T>& lhs, const list<T>& rhs)
	{
		return MySTL::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
	}

	template <class T>
	bool operator!=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T>
	bool operator>(const list<T>& lhs, const list<T>& rhs)
	{
		return rhs < lhs;
	}

	template <class T>
	bool operator<=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T>
	bool operator>=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(lhs < rhs);
	}

	// 重载 MySTL 的 swap
	template <class T>
	void swap(list<T>& lhs, list<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}
}