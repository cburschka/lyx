#include <config.h>

#include "ParagraphList.h"

#include "paragraph.h"

#ifdef NO_STD_LIST

////////// The ParagraphList::iterator

ParagraphList::iterator::iterator()
	: ptr(0)
{}


ParagraphList::iterator::iterator(Paragraph * p)
	: ptr(p)
{}


ParagraphList::iterator::reference
ParagraphList::iterator::operator*()
{
	return *ptr;
}


ParagraphList::iterator::pointer
ParagraphList::iterator::operator->()
{
	return ptr;
}


ParagraphList::iterator &
ParagraphList::iterator::operator++()
{
	ptr = ptr->next_par_;
	return *this;
}


ParagraphList::iterator
ParagraphList::iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}


ParagraphList::iterator &
ParagraphList::iterator::operator--()
{
	ptr = ptr->prev_par_;
	return *this;
}


ParagraphList::iterator
ParagraphList::iterator::operator--(int)
{
	iterator tmp = *this;
	--*this;
	return tmp;
}


bool operator==(ParagraphList::iterator const & i1,
		ParagraphList::iterator const & i2)
{
	return &(*const_cast<ParagraphList::iterator&>(i1))
	    == &(*const_cast<ParagraphList::iterator&>(i2));
}


bool operator!=(ParagraphList::iterator const & i1,
		ParagraphList::iterator const & i2)
{
	return !(i1 == i2);
}

//////////
////////// The ParagraphList proper
//////////

ParagraphList::ParagraphList()
	: parlist(0)
{}


ParagraphList::ParagraphList(ParagraphList const & pl)
	: parlist(0)
{
	// Deep copy.
	ParagraphList::iterator it = pl.begin();
	ParagraphList::iterator end = pl.end();
	for (; it != end; ++it) {
		push_back(*it);
	}
}


ParagraphList & ParagraphList::operator=(ParagraphList const & rhs)
{
	ParagraphList tmp(rhs);
	std::swap(parlist, tmp.parlist);
	return *this;
}


ParagraphList::iterator
ParagraphList::insert(ParagraphList::iterator it, Paragraph const & p)
{
	Paragraph * par = new Paragraph(p);

	if (it != end()) {
		Paragraph * prev = it->prev_par_;
		par->next_par_ = &*it;
		par->prev_par_ = prev;
		prev->next_par_ = par;
		it->prev_par_= par;
	} else if (parlist == 0) {
		parlist = par;
	} else {
		// Find last par.
		Paragraph * last = parlist;
		while (last->next_par_)
			last = last->next_par_;
		last->next_par_ = par;
		par->prev_par_ = last;
	}
	return iterator(par);
}



void ParagraphList::insert(iterator pos, iterator beg, iterator end)
{
	for (; beg != end; ++beg) {
		insert(pos, *beg);
	}
}


void ParagraphList::assign(iterator beg, iterator end)
{
	clear();
	for (; beg != end; ++beg) {
		push_back(*beg);
	}
}


void ParagraphList::splice(iterator pos, ParagraphList & pl)
{
	if (pl.parlist == 0)
		return;

	Paragraph * first = pl.parlist;
	Paragraph * last = first;
	while (last->next_par_)
		last = last->next_par_;

	if (pos == end()) {
		if (parlist == 0) {
			parlist = first;
		} else {
			Paragraph * last_par = &back();
			last_par->next_par_ = first;
			first->prev_par_ = last_par;
		}
	} else if (pos == begin()) {
		last->next_par_ = parlist;
		parlist->prev_par_ = last;
		parlist = first;
	} else {
		Paragraph * pos_par = &*pos;
		Paragraph * before_pos = pos_par->prev_par_;

		before_pos->next_par_ = first;
		first->prev_par_ = before_pos;
		last->next_par_ = pos_par;
		pos_par->prev_par_ = last;
	}
	pl.parlist = 0;
}


void ParagraphList::clear()
{
	while (parlist) {
		Paragraph * tmp = parlist->next_par_;
		delete parlist;
		parlist = tmp;
	}
}


ParagraphList::iterator ParagraphList::erase(ParagraphList::iterator it)
{
	Paragraph * prev = it->prev_par_;
	Paragraph * next = it->next_par_;

	if (prev)
		prev->next_par_ = next;
	else
		parlist = next;

	if (next)
		next->prev_par_ = prev;

	delete &*it;
	return next;
}


ParagraphList::iterator ParagraphList::erase(ParagraphList::iterator first,
			      ParagraphList::iterator last)
{
	while (first != last) {
		erase(first++);
	}
	return last;
}


ParagraphList::iterator ParagraphList::begin()
{
	return iterator(parlist);
}


ParagraphList::iterator ParagraphList::begin() const
{
	return iterator(parlist);
}


ParagraphList::iterator ParagraphList::end()
{
	return iterator();
}


ParagraphList::iterator ParagraphList::end() const
{
	return iterator();
}


Paragraph const & ParagraphList::front() const
{
	return *parlist;
}


Paragraph & ParagraphList::front()
{
	return *parlist;
}


Paragraph const & ParagraphList::back() const
{
	Paragraph * tmp = parlist;
	while (tmp->next_par_)
		tmp = tmp->next_par_;
	return *tmp;
}


Paragraph & ParagraphList::back()
{
	Paragraph * tmp = parlist;
	while (tmp->next_par_)
		tmp = tmp->next_par_;
	return *tmp;
}


void ParagraphList::push_back(Paragraph const & pr)
{
	Paragraph * p = new Paragraph(pr);

	if (!parlist) {
		parlist = p;
		return;
	}

	Paragraph * pos = parlist;
	while (pos->next_par_)
		pos = pos->next_par_;
	pos->next_par_ = p;
	p->prev_par_ = pos;
}


int ParagraphList::size() const
{
	// When we switch to a std::container this will be O(1)
	// instead of O(n). (Lgb)
	Paragraph * tmp = parlist;
	int c = 0;
	while (tmp) {
		++c;
		tmp = tmp->next_par_;
	}
	return c;
}


bool ParagraphList::empty() const
{
	return parlist == 0;
}

#endif
