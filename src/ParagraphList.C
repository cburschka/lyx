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
	return &(*const_cast<ParagraphList::iterator&>(i1)) == &(*const_cast<ParagraphList::iterator&>(i2));
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


void ParagraphList::clear()
{
	while (parlist) {
		Paragraph * tmp = parlist->next();
		delete parlist;
		parlist = tmp;
	}
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


void ParagraphList::set(Paragraph * p)
{
	parlist = p;
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
