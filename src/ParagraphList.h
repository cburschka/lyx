// -*- C++ -*-

#ifndef PARAGRAPH_LIST_H
#define PARAGRAPH_LIST_H

#define NO_STD_LIST 1

#ifndef NO_STD_LIST

#include "paragraph.h"

#include <list>

typedef std::list<Paragraph> ParagraphList;

#else

class Paragraph;

#include <iterator>
#include <utility>

///
class ParagraphList {
public:
	///
	class iterator {
	public:
		friend class ParagraphList;
		///
		typedef std::bidirectional_iterator_tag iterator_category;
		///
		typedef Paragraph * value_type;
		///
		typedef ptrdiff_t difference_type;
		///
		typedef Paragraph * pointer;
		///
		typedef Paragraph & reference;
		///
		iterator();
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
		iterator(value_type);
		///
		Paragraph * ptr;
	};
	///
	class const_iterator {
	public:
		friend class ParagraphList;
		///
		typedef std::bidirectional_iterator_tag iterator_category;
		///
		typedef Paragraph * value_type;
		///
		typedef ptrdiff_t difference_type;
		///
		typedef Paragraph const * const_pointer;
		///
		typedef Paragraph const & const_reference;
		///
		const_iterator();
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
		const_iterator(value_type);
		///
		Paragraph * ptr;
	};
	///
	ParagraphList();
	///
	ParagraphList(ParagraphList const &);
	///
	ParagraphList & operator=(ParagraphList const &);
	///
	iterator insert(iterator it, Paragraph const & par);
	///
	void insert(iterator pos, iterator beg, iterator end);
	///
	void assign(iterator beg, iterator end);
	///
	void splice(iterator pos, ParagraphList & pl);
	///
	void clear();
	///
	iterator erase(iterator it);
	///
	iterator erase(iterator first, iterator last);
	///
	iterator begin();
	///
	const_iterator begin() const;
	///
	iterator end();
	///
	const_iterator end() const;
	///
	void push_back(Paragraph const &);
	///
	Paragraph const & front() const;
	///
	Paragraph & front();
	///
	Paragraph const & back() const;
	///
	Paragraph & back();
	///
	int size() const;
	///
	bool empty() const;
private:
	///
	Paragraph * parlist;
};



typedef std::pair<ParagraphList::iterator, int> PitPosPair;



///
bool operator==(ParagraphList::iterator const & i1,
		ParagraphList::iterator const & i2);
///
bool operator!=(ParagraphList::iterator const & i1,
		ParagraphList::iterator const & i2);

///
bool operator==(ParagraphList::const_iterator const & i1,
		ParagraphList::const_iterator const & i2);
///
bool operator!=(ParagraphList::const_iterator const & i1,
		ParagraphList::const_iterator const & i2);

#endif

#endif
