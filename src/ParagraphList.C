#include <config.h>

#include "ParagraphList.h"

#include "paragraph.h"

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
#ifndef NO_NEXT
	ptr = ptr->next_;
#else
	ptr = ptr->next_par_;
#endif
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
#ifndef NO_NEXT
	ptr = ptr->previous_;
#else
	ptr = ptr->prev_par_;
#endif
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


////////// The ParagraphList proper
ParagraphList::ParagraphList()
	: parlist(0)
{}


ParagraphList::iterator
ParagraphList::insert(ParagraphList::iterator it, Paragraph * par)
{
#ifndef NO_NEXT
	if (it != end()) {
		Paragraph * prev = it->previous_;
		par->next_ = &*it;
		par->previous_ = prev;
		prev->next_ = par;
		it->previous_ = par;
	} else if (parlist == 0) {
		parlist = par;
	} else {
		// Find last par.
		Paragraph * last = parlist;
		while (last->next_)
			last = last->next_;
		last->next_ = par;
		par->previous_ = last;
	}
	return iterator(par);
#else
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
#endif
}


void ParagraphList::clear()
{
#ifndef NO_NEXT
	while (parlist) {
		Paragraph * tmp = parlist->next_;
		delete parlist;
		parlist = tmp;
	}
#else
	while (parlist) {
		Paragraph * tmp = parlist->next_par_;
		delete parlist;
		parlist = tmp;
	}
#endif
}


void ParagraphList::erase(ParagraphList::iterator it)
{
#ifndef NO_NEXT
	Paragraph * prev = it->previous_;
	Paragraph * next = it->next_;

	if (prev)
		prev->next_ = next;
	else
		parlist = next;

	if (next)
		next->previous_ = prev;

	it->previous_ = 0;
	it->next_ = 0;
	delete &*it;
#else
	Paragraph * prev = it->prev_par_;
	Paragraph * next = it->next_par_;

	if (prev)
		prev->next_par_ = next;
	else
		parlist = next;

	if (next)
		next->prev_par_ = prev;

	delete &*it;
#endif
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
#ifndef NO_NEXT
	Paragraph * tmp = parlist;
	while (tmp->next_)
		tmp = tmp->next_;
	return *tmp;
#else
	Paragraph * tmp = parlist;
	while (tmp->next_par_)
		tmp = tmp->next_par_;
	return *tmp;
#endif
}


Paragraph & ParagraphList::back()
{
#ifndef NO_NEXT
	Paragraph * tmp = parlist;
	while (tmp->next_)
		tmp = tmp->next_;
	return *tmp;
#else
	Paragraph * tmp = parlist;
	while (tmp->next_par_)
		tmp = tmp->next_par_;
	return *tmp;
#endif
}


void ParagraphList::set(Paragraph * p)
{
	parlist = p;
}


void ParagraphList::push_back(Paragraph * p)
{
#ifndef NO_NEXT
	if (!parlist) {
		parlist = p;
		return;
	}

	Paragraph * pos = parlist;
	while (pos->next_)
		pos = pos->next_;
	pos->next_ = p;
	p->previous_ = pos;
#else
	if (!parlist) {
		parlist = p;
		return;
	}

	Paragraph * pos = parlist;
	while (pos->next_par_)
		pos = pos->next_par_;
	pos->next_par_ = p;
	p->prev_par_ = pos;
#endif
}


int ParagraphList::size() const
{
#ifndef NO_NEXT
	// When we switch to a std::container this will be O(1)
	// instead of O(n). (Lgb)
	Paragraph * tmp = parlist;
	int c = 0;
	while (tmp) {
		++c;
		tmp = tmp->next_;
	}
	return c;
#else
	// When we switch to a std::container this will be O(1)
	// instead of O(n). (Lgb)
	Paragraph * tmp = parlist;
	int c = 0;
	while (tmp) {
		++c;
		tmp = tmp->next_par_;
	}
	return c;
#endif
}


bool ParagraphList::empty() const
{
	return parlist == 0;
}
