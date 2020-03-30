#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>
#include <cmath>
#include <cstdlib>

namespace sjtu { 

inline double max(double __x, double __y) {
	return __x > __y ? __x : __y;
}

template<class T>
class deque {
private:
	size_t __size;
	static constexpr double __lower_size  = 9.98;
	static constexpr double __split_ratio = 2.98;
	static constexpr double __merge_ratio = 0.52;
	static constexpr double __block_ratio = 1.98;

	double __split_size() {
		return max(__lower_size, __split_ratio * std::sqrt(__size));
	}
	double __merge_size() {
		return __merge_ratio * std::sqrt(__size);
	}
	double __block_size() {
		return max(__lower_size, __block_ratio * std::sqrt(__size));
	}
private:
	struct node {
		node *prev, *next;
		T *value; // use pointer to judge whether the node is head or tail

		node() : prev(nullptr), next(nullptr), value(nullptr) {}

		~node() {
			delete value;
		}
	};

	struct list {
		list *prev, *next;
		node *head, *tail; // elements in the block
		size_t size;

		list() : head(new node), tail(new node), size(0), prev(nullptr), next(nullptr) {
			head->next = tail;
			tail->prev = head;
		}

		void clear() {
			node *__pos = head->next;
			while (__pos != tail) {
				node *__next = __pos->next;
				delete __pos;
				__pos = __next;
			}
			size = 0;
		}

		void __split(size_t pos) {
			node *new_tail = head;
			for (size_t i = 0; i < pos; ++i)
				new_tail = new_tail->next;
			node *new_head = new_tail->next;
			list *new_list = new list;
			new_list->next = next;
			next->prev = new_list;
			new_list->prev = this;
			next = new_list;

			new_list->tail->prev = tail->prev;
			tail->prev->next = new_list->tail;
			new_list->head->next = new_head;
			new_head->prev = new_list->head;
			tail->prev = new_tail;
			new_tail->next = tail;
			new_list->size = size - pos;
			size = pos;	
		}

		void __merge() {
			node *old_tail = tail, *old_head = next->head;
			old_tail->prev->next = old_head->next;
			old_head->next->prev = old_tail->prev;
			delete old_tail;
			delete old_head;
			tail = next->tail;
			size = size + next->size;
			list *old_list = next;
			old_list->next->prev = this;
			next = old_list->next;
			delete old_list;
		}

		static list *__copy_list(list *other) {
			list *new_list = new list;
			new_list->size = other->size;
			for (node *copied_node = other->head->next; copied_node != other->tail; copied_node = copied_node->next) {
				node *new_node = new node;
				new_node->value = new T(*(copied_node->value));
				new_list->tail->prev->next = new_node;
				new_node->prev = new_list->tail->prev;
				new_node->next = new_list->tail;
				new_list->tail->prev = new_node;
			}
			return new_list;
		}

		// static node *real_next(node *other_node, list *other_list) {
		// 	return other_node->next == other_list->tail ? other_list->next->head->next : other_node->next;
		// }
	};
private:
	list *head, *tail;

public:
	class iterator;
	class const_iterator;

	deque() : head(new list), tail(new list), __size(0) {
		head->next = tail;
		tail->prev = head;
	}
	deque(const deque &other) : head(new list), tail(new list), __size(other.__size) {
		head->next = tail;
		tail->prev = head;
		for (list *copied_list = other.head->next; copied_list != other.tail; copied_list = copied_list->next) {
			list *new_list = list::__copy_list(copied_list);
			tail->prev->next = new_list;
			new_list->prev = tail->prev;
			tail->prev = new_list;
			new_list->next = tail;
		}
	}

	~deque() {
		clear();
		delete head->head;
		delete head->tail;
		delete head;
		delete tail->head;
		delete tail->tail;
		delete tail;
	}

	deque &operator=(const deque &other) {
		if (this == &other) return *this;
		clear();
		__size = other.__size;
		for (list *copied_list = other.head->next; copied_list != other.tail; copied_list = copied_list->next) {
			list *new_list = list::__copy_list(copied_list);
			tail->prev->next = new_list;
			new_list->prev = tail->prev;
			tail->prev = new_list;
			new_list->next = tail;
		}
		return *this;
	}
	/**
	 * access specified element with bounds checking
	 * throw index_out_of_bound if out of bound.
	 */
	T &at(const size_t &pos) {
	 	return operator[](pos);
	} 
	const T &at(const size_t &pos) const {
		return operator[](pos);
	} 
	T &operator[](const size_t &pos) {
		if (pos < 0 || pos >= __size) throw index_out_of_bound();
		list *now_list = head->next;
		size_t now_pos = 0;
		while (now_pos + now_list->size <= pos) {
			now_pos += now_list->size;
			now_list = now_list->next;
		}
		node *now_node = now_list->head->next;
		while (now_pos < pos) {
			++now_pos;
			now_node = now_node->next;
		}
		return *(now_node->value);
	}
	const T &operator[](const size_t &pos) const {
		if (pos < 0 || pos >= __size) throw index_out_of_bound();
		list *now_list = head->next;
		size_t now_pos = 0;
		while (now_pos + now_list->size <= pos) {
			now_pos += now_list->size;
			now_list = now_list->next;
		}
		node *now_node = now_list->head->next;
		while (now_pos < pos) {
			++now_pos;
			now_node = now_node->next;
		}
		return *(now_node->value);
	}
	/**
	 * access the first element
	 * throw container_is_empty when the container is empty.
	 */
	const T &front() const {
		if (__size == 0) throw container_is_empty();
		return *(head->next->head->next->value);
	}
	/**
	 * access the last element
	 * throw container_is_empty when the container is empty.
	 */ 
	const T &back() const {
		if (__size == 0) throw container_is_empty();
		return *(tail->prev->tail->prev->value);
	}
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		return iterator(this, head->next, head->next->head->next);
	}
	const_iterator cbegin() const {
		return const_iterator(this, head->next, head->next->head->next);
	}
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		return iterator(this, tail, tail->head->next);
	}
	const_iterator cend() const {
		return const_iterator(this, tail, tail->head->next);
	}
	/**
	 * checks whether the container is empty.
	 */
	bool empty() const {
		return __size == 0;
	}
	/**
	 * returns the number of elements
	 */
	size_t size() const {
		return __size;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		list *__pos__ = head->next;
        while (__pos__ != tail) {
            list *__next__ = __pos__->next;
            __pos__->clear();
            delete __pos__->head;
            delete __pos__->tail;
            delete __pos__;
            __pos__ = __next__;
        }
        head->next = tail;
        tail->prev = head;
        __size = 0;
	}
	/**
	 * inserts elements at the specified locat on in the container.
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	iterator insert(iterator pos, const T &value) {
		if (pos.__deque != this) throw invalid_iterator();
		if (pos == end()) {
			push_back(value);
			return --end();
		}
		++__size;
		list *old_list = pos.__list;
		++old_list->size;
		node *pos_node = pos.__node;
		node *new_node = new node;
		new_node->value = new T(value);
		new_node->prev = pos_node->prev;
		pos_node->prev->next = new_node;
		new_node->next = pos_node;
		pos_node->prev = new_node;
		if (old_list->size > __split_size())
			old_list->__split(old_list->size >> 1);
		for (node *p = old_list->head->next; p != old_list->tail; p = p->next)
			if (p == new_node)
				return iterator(this, old_list, new_node);
		return iterator(this, old_list->next, new_node);
	}
	/**
	 * removes specified element at pos.
	 * removes the element at pos.
	 * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	iterator erase(iterator pos) {
		if (pos.__deque != this || pos.__node == nullptr || pos.__node->value == nullptr) throw invalid_iterator();
		--__size;
		list *old_list = pos.__list;
		--old_list->size;
		node *pos_node = pos.__node;
		if (old_list->size == 0) {
			iterator new_iterator(this, old_list->next, old_list->next->head->next);
			old_list->clear();
			delete old_list->head;
			delete old_list->tail;
			old_list->prev->next = old_list->next;
			old_list->next->prev = old_list->prev;
			delete old_list;
			return new_iterator;
		}
		bool next_in_next_list = pos_node->next == old_list->tail;
		node *next_node = next_in_next_list ? old_list->next->head->next : pos_node->next;
		pos_node->next->prev = pos_node->prev;
		pos_node->prev->next = pos_node->next;
		delete pos_node;
		if (old_list->prev != head && old_list->size + old_list->prev->size < __merge_size()) {
			list *new_list = old_list->prev;
			new_list->__merge();
			if (next_in_next_list)
				return iterator(this, new_list->next, next_node);
			else
				return iterator(this, new_list, next_node);
		} else if (old_list->next != tail && old_list->size + old_list->next->size < __merge_size()) {
			old_list->__merge();
			return iterator(this, old_list, next_node);
		} else {
			if (next_in_next_list)
				return iterator(this, old_list->next, next_node);
			else
				return iterator(this, old_list, next_node);
		}
	}
	/**
	 * adds an element to the end
	 */
	void push_back(const T &value) {
		++__size;
		if (__size == 1 || tail->prev->size > __block_size()) {
			list *new_list = new list;
			node *new_node = new node;
			new_node->value = new T(value);
			new_list->head->next = new_node;
			new_node->prev = new_list->head;
			new_list->tail->prev = new_node;
			new_node->next = new_list->tail;
			tail->prev->next = new_list;
			new_list->prev = tail->prev;
			tail->prev = new_list;
			new_list->next = tail;
			new_list->size = 1;
		} else {
			list *old_list = tail->prev;
			node *new_node = new node;
			new_node->value = new T(value);
			++old_list->size;
			old_list->tail->prev->next = new_node;
			new_node->prev = old_list->tail->prev;
			old_list->tail->prev = new_node;
			new_node->next = old_list->tail;
		}
	}
	/**
	 * removes the last element
	 *     throw when the container is empty.
	 */
	void pop_back() {
		if (__size == 0) throw container_is_empty();
		--__size;
		--tail->prev->size;
		if (tail->prev->size == 0) {
			list *old_list = tail->prev;
			delete old_list->tail->prev;
			delete old_list->head;
			delete old_list->tail;
			old_list->prev->next = tail;
			tail->prev = old_list->prev;
			delete old_list;
		} else {
			node *old_node = tail->prev->tail->prev;
			old_node->prev->next = old_node->next;
			old_node->next->prev = old_node->prev;
			delete old_node;
		}
	}
	/**
	 * inserts an element to the beginning.
	 */
	void push_front(const T &value) {
		++__size;
		if (__size == 1 || head->next->size > __block_size()) {
			list *new_list = new list;
			node *new_node = new node;
			new_node->value = new T(value);
			new_list->head->next = new_node;
			new_node->prev = new_list->head;
			new_list->tail->prev = new_node;
			new_node->next = new_list->tail;
			head->next->prev = new_list;
			new_list->next = head->next;
			head->next = new_list;
			new_list->prev = head;
			new_list->size = 1;
		} else {
			list *old_list = head->next;
			node *new_node = new node;
			new_node->value = new T(value);
			++old_list->size;
			old_list->head->next->prev = new_node;
			new_node->next = old_list->head->next;
			old_list->head->next = new_node;
			new_node->prev = old_list->head;
		}
	}
	/**
	 * removes the first element.
	 *     throw when the container is empty.
	 */
	void pop_front() {
		if (__size == 0) throw container_is_empty();
		--__size;
		--head->next->size;
		if (head->next->size == 0) {
			list *old_list = head->next;
			delete old_list->head->next;
			delete old_list->head;
			delete old_list->tail;
			old_list->next->prev = head;
			head->next = old_list->next;
			delete old_list;
		} else {
			node *old_node = head->next->head->next;
			old_node->prev->next = old_node->next;
			old_node->next->prev = old_node->prev;
			delete old_node;
		}
	}

public:
	class iterator {
		friend class const_iterator;
		friend iterator deque::insert(iterator, const T &);
		friend iterator deque::erase(iterator);
	public:
		typedef T value;
		typedef T * pointer;
		typedef T & reference;
		typedef std::ptrdiff_t difference_type;
	private:
		deque *__deque;
		list *__list;
		node *__node;
	public:
		explicit iterator(deque *__deque__ = nullptr, list *__list__ = nullptr, node *__node__ = nullptr) :
			__deque(__deque__), __list(__list__), __node(__node__) {}
		iterator(const iterator &other) = default;
		explicit iterator(const const_iterator &other) :
			__deque(other.__deque), __list(other.__list), __node(other.__node) {}

		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, the behaviour is **undefined**.
		 * as well as operator-
		 */
		iterator operator+(const difference_type &n) const {
			if (n < 0) return operator-(-n);
			iterator new_iterator(*this);
			difference_type ptr_diff = 0;
			while (ptr_diff < n && new_iterator.__node->next != new_iterator.__list->tail) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->next;
			}
			if (ptr_diff == n) return new_iterator;
			++ptr_diff;
			new_iterator.__list = new_iterator.__list->next;
			while (ptr_diff + new_iterator.__list->size <= n && new_iterator.__list->size != 0) {
				ptr_diff += new_iterator.__list->size;
				new_iterator.__list = new_iterator.__list->next;
			}
			if (ptr_diff != n && new_iterator.__list->size == 0) throw invalid_iterator();
			new_iterator.__node = new_iterator.__list->head->next;
			while (ptr_diff < n) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->next;
			}
			return new_iterator;
		}
		iterator operator-(const difference_type &n) const {
			if (n < 0) return operator+(-n);
			iterator new_iterator(*this);
			difference_type ptr_diff = 0;
			while (ptr_diff < n && new_iterator.__node->prev != new_iterator.__list->head) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->prev;
			}
			if (ptr_diff == n) return new_iterator;
			++ptr_diff;
			new_iterator.__list = new_iterator.__list->prev;
			while (ptr_diff + new_iterator.__list->size <= n && new_iterator.__list->size != 0) {
				ptr_diff += new_iterator.__list->size;
				new_iterator.__list = new_iterator.__list->prev;
			}
			if (ptr_diff != n && new_iterator.__list->size == 0) throw invalid_iterator();
			new_iterator.__node = new_iterator.__list->tail->prev;
			while (ptr_diff < n) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->prev;
			}
			return new_iterator;
		}
	private:
		size_t ptr_diff_t() const {
			size_t ptr = 0;
			for (node *ptr_node = __node; ptr_node->prev != __list->head; ptr_node = ptr_node->prev)
				++ptr;
			for (list *ptr_list = __list; ptr_list->prev != __deque->head; ptr_list = ptr_list->prev)
				ptr += ptr_list->prev->size;
			return ptr;
		}
	public:
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		int operator-(const iterator &other) const {
			if (__deque != other.__deque) throw invalid_iterator();
			return ptr_diff_t() - other.ptr_diff_t();
		}
		iterator& operator+=(const difference_type &n) {
			iterator result = operator+(n);
			__list = result.__list;
			__node = result.__node;
			return *this;
		}
		iterator& operator-=(const difference_type &n) {
			iterator result = operator-(n);
			__list = result.__list;
			__node = result.__node;
			return *this;
		}

		iterator operator++(int) {
			if (*this == __deque->end()) throw invalid_iterator();
			auto backup = *this;
			__node = __node->next;
			if (__node == __list->tail) {
				__list = __list->next;
				__node = __list->head->next;
			}
			return backup;
		}
		iterator& operator++() {
			if (*this == __deque->end()) throw invalid_iterator();
			__node = __node->next;
			if (__node == __list->tail) {
				__list = __list->next;
				__node = __list->head->next;
			}
			return *this;
		}
		iterator operator--(int) {
			if (*this == __deque->begin()) throw invalid_iterator();
			auto backup = *this;
			__node = __node->prev;
			if (__node == __list->head) {
				__list = __list->prev;
				__node = __list->tail->prev;
			}
			return backup;
		}
		iterator& operator--() {
			if (*this == __deque->begin()) throw invalid_iterator();
			__node = __node->prev;
			if (__node == __list->head) {
				__list = __list->prev;
				__node = __list->tail->prev;
			}
			return *this;
		}
		
		reference operator*() const {
			if (__node == nullptr || __node->value == nullptr) throw invalid_iterator();
			return *__node->value;
		}
		pointer operator->() const noexcept {
			if (__node == nullptr || __node->value == nullptr) throw invalid_iterator();
			return __node->value;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &other) const {
			return __node == other.__node;
		}
		bool operator==(const const_iterator &other) const {
			return __node == other.__node;
		}
		bool operator!=(const iterator &other) const {
			return __node != other.__node;
		}
		bool operator!=(const const_iterator &other) const {
			return __node != other.__node;
		}
	};
	class const_iterator {
		friend class iterator;
	public:
		typedef const T value;
		typedef const T * pointer;
		typedef const T & reference;
		typedef std::ptrdiff_t difference_type;
	private:
		const deque *__deque;
		const list *__list;
		const node *__node;
	public:
		explicit const_iterator(const deque *__deque__ = nullptr, const list *__list__ = nullptr, const node *__node__ = nullptr) :
			__deque(__deque__), __list(__list__), __node(__node__) {}
		const_iterator(const const_iterator &other) = default;
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, the behaviour is **undefined**.
		 * as well as operator-
		 */
		const_iterator operator+(const difference_type &n) const {
			if (n < 0) return operator-(-n);
			const_iterator new_iterator(*this);
			difference_type ptr_diff = 0;
			while (ptr_diff < n && new_iterator.__node->next != new_iterator.__list->tail) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->next;
			}
			if (ptr_diff == n) return new_iterator;
			++ptr_diff;
			new_iterator.__list = new_iterator.__list->next;
			while (ptr_diff + new_iterator.__list->size <= n && new_iterator.__list->size != 0) {
				ptr_diff += new_iterator.__list->size;
				new_iterator.__list = new_iterator.__list->next;
			}
			if (ptr_diff != n && new_iterator.__list->size == 0) throw invalid_iterator();
			new_iterator.__node = new_iterator.__list->head->next;
			while (ptr_diff < n) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->next;
			}
			return new_iterator;
		}
		const_iterator operator-(const difference_type &n) const {
			if (n < 0) return operator+(-n);
			const_iterator new_iterator(*this);
			difference_type ptr_diff = 0;
			while (ptr_diff < n && new_iterator.__node->prev != new_iterator.__list->head) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->prev;
			}
			if (ptr_diff == n) return new_iterator;
			++ptr_diff;
			new_iterator.__list = new_iterator.__list->prev;
			while (ptr_diff + new_iterator.__list->size <= n && new_iterator.__list->size != 0) {
				ptr_diff += new_iterator.__list->size;
				new_iterator.__list = new_iterator.__list->prev;
			}
			if (ptr_diff != n && new_iterator.__list->size == 0) throw invalid_iterator();
			new_iterator.__node = new_iterator.__list->tail->prev;
			while (ptr_diff < n) {
				++ptr_diff;
				new_iterator.__node = new_iterator.__node->prev;
			}
			return new_iterator;
		}
	private:
		size_t ptr_diff_t() const {
			size_t ptr = 0;
			for (auto ptr_node = __node; ptr_node->prev != __list->head; ptr_node = ptr_node->prev)
				++ptr;
			for (auto ptr_list = __list; ptr_list->prev != __deque->head; ptr_list = ptr_list->prev)
				ptr += ptr_list->prev->size;
			return ptr;
		}
	public:
		int operator-(const const_iterator &other) const {
			if (__deque != other.__deque) throw invalid_iterator();
			return ptr_diff_t() - other.ptr_diff_t();
		}
		const_iterator& operator+=(const difference_type &n) {
			const_iterator result = operator+(n);
			__list = result.__list;
			__node = result.__node;
			return *this;
		}
		const_iterator& operator-=(const difference_type &n) {
			const_iterator result = operator-(n);
			__list = result.__list;
			__node = result.__node;
			return *this;
		}

		const_iterator operator++(int) {
			if (*this == __deque->cend()) throw invalid_iterator();
			auto backup = *this;
			__node = __node->next;
			if (__node == __list->tail) {
				__list = __list->next;
				__node = __list->head->next;
			}
			return backup;
		}
		const_iterator& operator++() {
			if (*this == __deque->cend()) throw invalid_iterator();
			__node = __node->next;
			if (__node == __list->tail) {
				__list = __list->next;
				__node = __list->head->next;
			}
			return *this;
		}
		const_iterator operator--(int) {
			if (*this == __deque->cbegin()) throw invalid_iterator();
			auto backup = *this;
			__node = __node->prev;
			if (__node == __list->head) {
				__list = __list->prev;
				__node = __list->tail->prev;
			}
			return backup;
		}
		const_iterator& operator--() {
			if (*this == __deque->cbegin()) throw invalid_iterator();
			__node = __node->prev;
			if (__node == __list->head) {
				__list = __list->prev;
				__node = __list->tail->prev;
			}
			return *this;
		}
		
		reference operator*() const {
			if (__node == nullptr || __node->value == nullptr) throw invalid_iterator();
			return *__node->value;
		}
		pointer operator->() const noexcept {
			if (__node == nullptr || __node->value == nullptr) throw invalid_iterator();
			return __node->value;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &other) const {
			return __node == other.__node;
		}
		bool operator==(const const_iterator &other) const {
			return __node == other.__node;
		}
		bool operator!=(const iterator &other) const {
			return __node != other.__node;
		}
		bool operator!=(const const_iterator &other) const {
			return __node != other.__node;
		}
	};
};

}

#endif
