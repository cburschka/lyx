// -*- C++ -*-

#ifndef ROW_LIST_H
#define ROW_LIST_H

#include <iterator>

class Row;

///
class RowList {
public:
	///
	class iterator {
	public:
		///
		typedef std::bidirectional_iterator_tag iterator_category;
		///
		typedef Row * value_type;
		///
		typedef ptrdiff_t difference_type;
		///
		typedef Row * pointer;
		///
		typedef Row & reference;
		///
		iterator();
		///
		iterator(value_type);
		///
		reference operator*();
		///
		pointer operator->();
		///
		iterator & operator++();
		///
		iterator operator++(int);
		///
		iterator & operator--();
		///
		iterator operator--(int);
	private:
		///
		Row * ptr;
	};
	///
	class const_iterator {
	public:
		///
		typedef std::bidirectional_iterator_tag iterator_category;
		///
		typedef Row * value_type;
		///
		typedef ptrdiff_t difference_type;
		///
		typedef Row const * const_pointer;
		///
		typedef Row const & const_reference;
		///
		const_iterator();
		///
		const_iterator(value_type);
		///
		const_reference operator*();
		///
		const_pointer operator->();
		///
		const_iterator & operator++();
		///
		const_iterator operator++(int);
		///
		const_iterator & operator--();
		///
		const_iterator operator--(int);
	private:
		///
		Row * ptr;
	};
	///
	RowList();
	///
	iterator insert(iterator it, Row * row);
	///
	void clear();
	///
	void erase(iterator it);
	///
	iterator begin();
	///
	const_iterator begin() const;
	///
	iterator end();
	///
	const_iterator end() const;
	///
	void push_back(Row *);
	///
	Row const & front() const;
	///
	Row & front();
	///
	Row const & back() const;
	///
	Row & back();
	///
	int size() const;
	///
	bool empty() const;
private:
	///
	Row * rowlist;
};

///
bool operator==(RowList::iterator const & i1,
		RowList::iterator const & i2);
///
bool operator!=(RowList::iterator const & i1,
		RowList::iterator const & i2);

///
bool operator==(RowList::const_iterator const & i1,
		RowList::const_iterator const & i2);
///
bool operator!=(RowList::const_iterator const & i1,
		RowList::const_iterator const & i2);

#endif
