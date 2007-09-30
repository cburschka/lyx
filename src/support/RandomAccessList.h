// -*- C++ -*-
/**
 * \file RandomAccessList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#ifndef RANDOM_ACESS_LIST_H
#define RANDOM_ACESS_LIST_H

#include <vector>
#include <list>


namespace lyx {

#define USE_OLD_ITERATOR 1

/// Random Access List.
/**
This templatized class provide a std::vector like interface to a
standard std::list underneath. An important property is that it
keeps the std::list::iterator interface. A typical use would be:

	typedef RandomAccessList<some_class> MyContainer;

Then you can use MyContainer as if it was a standard
std::vector<some_class> for operator[] access and as if it was a
standard std::list for iterator access. The main difference with
std::vector is that insertion of elements is much less costly. Compared
to a standard list alone, there is of course a small overhead because
the class always keeps its internal vector of iterator (it_vector_) up
to date.
*/
template <class T>
class RandomAccessList {
public:
	// types
	typedef std::list<T> Container;
	typedef typename Container::reference reference;
	typedef typename Container::const_reference const_reference;
#if USE_OLD_ITERATOR
	// iterator (below)
	typedef typename Container::iterator iterator;
	// const_iterator (below)
	typedef typename Container::const_iterator const_iterator;
#else
	// wip
#endif
	typedef typename Container::size_type size_type;
	typedef typename Container::difference_type difference_type;
	typedef typename Container::value_type value_type;
	typedef typename Container::allocator_type allocator_type;
	typedef typename Container::pointer pointer;
	typedef typename Container::const_pointer const_pointer;
	// reverse_iterator
	// const_reverse_iterator

	typedef std::vector<typename Container::iterator> IterCont;

	// construct/copy/destroy

	RandomAccessList()
	{}

	// RandomAccessList(size_type n T const & value = T())

	template<class InputIterator>
	RandomAccessList(InputIterator first, InputIterator last)
	{
		assign(first, last);
	}



	RandomAccessList(RandomAccessList const & x)
	{
		assign(x.begin(), x.end());
	}

	// ~RandomAccessList()

	///
	RandomAccessList & operator=(RandomAccessList const & x)
	{
		assign(x.begin(), x.end());
		return *this;
	}

	template<class InputIterator>
	void assign(InputIterator first, InputIterator last)
	{
		container_.assign(first, last);
		recreateVector();
	}


	// void assign(size_type n, T const & u);

	// iterators

	iterator begin()
	{
		return container_.begin();
	}

	const_iterator begin() const
	{
		return container_.begin();
	}

	iterator end()
	{
		return container_.end();
	}

	const_iterator end() const
	{
		return container_.end();
	}

	// reverse_iterator rbegin();
	// const_reverse_iterator rbegin() const;
	// reverse_iterator rend();
	// const_reverse_iterator rend() const;

	// capacity
	size_type size() const
	{
		return iterCont_.size();
	}

	size_type max_size() const
	{
		return iterCont_.max_size();
	}

	// void resize(size_type sz,  T c = T());

	size_type capacity() const
	{
		return iterCont_.capacity();
	}

	bool empty() const
	{
		return container_.empty();
	}

	// void reserve(size_type n);

	// element access

	reference operator[](size_type pos)
	{
		return *iterCont_[pos];
	}

	///
	const_reference operator[](size_type pos) const
	{
		return *iterCont_[pos];
	}

	reference at(size_type pos)
	{
		return *iterCont_.at(pos);
	}

	const_reference at(size_type pos) const
	{
		return *iterCont_.at(pos);
	}

	reference front()
	{
		return container_.front();
	}

	const_reference front() const
	{
		return container_.front();
	}

	reference back()
	{
		return container_.back();
	}

	const_reference back() const
	{
		return container_.back();
	}

	// modifiers

	void push_back(T const & x)
	{
		typename Container::iterator it =
			container_.insert(container_.end(), x);
		iterCont_.push_back(it);
	}

	void pop_back()
	{
		container_.pop_back();
		iterCont_.pop_back();
	}

	iterator insert(iterator position, T const & x)
	{
		typename Container::iterator it =
			container_.insert(position, x);
		recreateVector();
		return it;
	}

	// void insert(iterator position, size_type n, T const & x);

	template<class InputIterator>
	void insert(iterator position,
		    InputIterator first, InputIterator last)
	{
		container_.insert(position, first, last);
		recreateVector();
	}

	iterator erase(iterator position)
	{
		typename Container::iterator it =
			container_.erase(position);
		recreateVector();
		return it;
	}

	iterator erase(iterator first, iterator last)
	{
		typename Container::iterator it =
			container_.erase(first, last);
		recreateVector();
		return it;
	}

	void swap(RandomAccessList & x)
	{
		std::swap(container_, x.container_);
		std::swap(iterCont_, x.iterCont_);
	}

	void clear()
	{
		container_.clear();
		iterCont_.clear();
	}

private:
	void recreateVector()
	{
		iterCont_.clear();
		typename Container::iterator beg = container_.begin();
		typename Container::iterator end = container_.end();
		for (; beg != end; ++beg)
			iterCont_.push_back(beg);
	}

	/// Our container.
	Container container_;
	/// Our container of iterators.
	IterCont iterCont_;
};


} // namespace lyx

#endif
