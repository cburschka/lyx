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
		Paragraph * ptr;
	};
	///
	ParagraphList();
	///
	ParagraphList(ParagraphList const &);
	///
	ParagraphList & operator=(ParagraphList const &);
	///
	iterator insert(iterator it, Paragraph * par);
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
	iterator begin() const;
	///
	iterator end();
	///
	iterator end() const;
	///
	void push_back(Paragraph *);
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

#endif

#endif
