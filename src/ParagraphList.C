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
	ptr = ptr->next();
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
	ptr = ptr->previous();
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
	if (it != end()) {
		Paragraph * prev = it->previous();
		par->next(&*it);
		par->previous(prev);
		prev->next(par);
		it->previous(par);
	} else if (parlist == 0) {
		parlist = par;
	} else {
		// Find last par.
		Paragraph * last = parlist;
		while (last->next())
			last = last->next();
		last->next(par);
		par->previous(last);
	}
	return iterator(par);
}


void ParagraphList::clear()
{
	while (parlist) {
		Paragraph * tmp = parlist->next();
		delete parlist;
		parlist = tmp;
	}
}


void ParagraphList::erase(ParagraphList::iterator it)
{
	Paragraph * prev = it->previous();
	Paragraph * next = it->next();

	if (prev)
		prev->next(next);
	if (next)
		next->previous(prev);

	delete &*it;
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
	while (tmp->next())
		tmp = tmp->next();
	return *tmp;
}


Paragraph & ParagraphList::back()
{
	Paragraph * tmp = parlist;
	while (tmp->next())
		tmp = tmp->next();
	return *tmp;
}


void ParagraphList::set(Paragraph * p)
{
	parlist = p;
}


void ParagraphList::push_back(Paragraph * p)
{
	if (!parlist) {
		parlist = p;
		return;
	}

	Paragraph * pos = parlist;
	while (pos->next())
		pos = pos->next();
	pos->next(p);
	p->previous(pos);
}


int ParagraphList::size() const
{
	// When we switch to a std::container this will be O(1)
	// instead of O(n). (Lgb)
	Paragraph * tmp = parlist;
	int c = 0;
	while (tmp) {
		++c;
		tmp = tmp->next();
	}
	return c;
}


bool ParagraphList::empty() const
{
	return parlist == 0;
}
