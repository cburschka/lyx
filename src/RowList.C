#include <config.h>

#include "RowList.h"

#include "lyxrow.h"

////////// The RowList::iterator

RowList::iterator::iterator()
	: ptr(0)
{}


RowList::iterator::iterator(Row * p)
	: ptr(p)
{}


RowList::iterator::reference
RowList::iterator::operator*()
{
	return *ptr;
}


RowList::iterator::pointer
RowList::iterator::operator->()
{
	return ptr;
}


RowList::iterator &
RowList::iterator::operator++()
{
	ptr = ptr->next();
	return *this;
}


RowList::iterator
RowList::iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}


RowList::iterator &
RowList::iterator::operator--()
{
	ptr = ptr->previous();
	return *this;
}


RowList::iterator
RowList::iterator::operator--(int)
{
	iterator tmp = *this;
	--*this;
	return tmp;
}


bool operator==(RowList::iterator const & i1,
		RowList::iterator const & i2)
{
	return &(*const_cast<RowList::iterator&>(i1))
	    == &(*const_cast<RowList::iterator&>(i2));
}


bool operator!=(RowList::iterator const & i1,
		RowList::iterator const & i2)
{
	return !(i1 == i2);
}


////////// The RowList::const_iterator

RowList::const_iterator::const_iterator()
	: ptr(0)
{}


RowList::const_iterator::const_iterator(Row * p)
	: ptr(p)
{}


RowList::const_iterator::const_reference
RowList::const_iterator::operator*()
{
	return *ptr;
}


RowList::const_iterator::const_pointer
RowList::const_iterator::operator->()
{
	return ptr;
}


RowList::const_iterator &
RowList::const_iterator::operator++()
{
	ptr = ptr->next();
	return *this;
}


RowList::const_iterator
RowList::const_iterator::operator++(int)
{
	const_iterator tmp = *this;
	++*this;
	return tmp;
}


RowList::const_iterator &
RowList::const_iterator::operator--()
{
	ptr = ptr->previous();
	return *this;
}


RowList::const_iterator
RowList::const_iterator::operator--(int)
{
	const_iterator tmp = *this;
	--*this;
	return tmp;
}


bool operator==(RowList::const_iterator const & i1,
		RowList::const_iterator const & i2)
{
	return &(*const_cast<RowList::const_iterator&>(i1))
	    == &(*const_cast<RowList::const_iterator&>(i2));
}


bool operator!=(RowList::const_iterator const & i1,
		RowList::const_iterator const & i2)
{
	return !(i1 == i2);
}


////////// The RowList proper
RowList::RowList()
	: rowlist(0)
{}


RowList::iterator
RowList::insert(RowList::iterator it, Row * row)
{
	if (rowlist == 0) {
		rowlist = row;
	} else if (it != end()) {
		Row * prev = it->previous();
		row->next(&*it);
		row->previous(prev);
		if (prev)
			prev->next(row);
		else
			rowlist = row;
		it->previous(row);
	} else {
		// Find last par.
		Row * last = rowlist;
		while (last->next())
			last = last->next();
		last->next(row);
		row->previous(last);
	}
	return iterator(row);
}


void RowList::clear()
{
	while (rowlist) {
		Row * tmp = rowlist->next();
		delete rowlist;
		rowlist = tmp;
	}
}


void RowList::erase(RowList::iterator it)
{
	Row * prev = it->previous();
	Row * next = it->next();

	if (prev)
		prev->next(next);
	else
		rowlist = next;

	if (next)
		next->previous(prev);

	delete &*it;
}


RowList::iterator RowList::begin()
{
	return iterator(rowlist);
}


RowList::const_iterator RowList::begin() const
{
	return const_iterator(rowlist);
}


RowList::iterator RowList::end()
{
	return iterator();
}


RowList::const_iterator RowList::end() const
{
	return const_iterator();
}


Row const & RowList::front() const
{
	return *rowlist;
}


Row & RowList::front()
{
	return *rowlist;
}


Row const & RowList::back() const
{
	Row * tmp = rowlist;
	while (tmp->next())
		tmp = tmp->next();
	return *tmp;
}


Row & RowList::back()
{
	Row * tmp = rowlist;
	while (tmp->next())
		tmp = tmp->next();
	return *tmp;
}


void RowList::push_back(Row * p)
{
	if (!rowlist) {
		rowlist = p;
		return;
	}

	Row * pos = rowlist;
	while (pos->next())
		pos = pos->next();
	pos->next(p);
	p->previous(pos);
}


int RowList::size() const
{
	// When we switch to a std::container this will be O(1)
	// instead of O(n). (Lgb)
	Row * tmp = rowlist;
	int c = 0;
	while (tmp) {
		++c;
		tmp = tmp->next();
	}
	return c;
}


bool RowList::empty() const
{
	return rowlist == 0;
}
