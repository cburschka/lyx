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
	iterator begin() const;
	///
	iterator end();
	///
	iterator end() const;
	///
	void set(Row *);
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

#endif
