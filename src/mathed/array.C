
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "array.h"

// Is this still needed? (Lgb)
static inline
void * my_memcpy(void * ps_in, void const * pt_in, size_t n)
{
	char * ps = static_cast<char *>(ps_in);
	char const * pt = static_cast<char const *>(pt_in);
	while (n--) *ps++ = *pt++;
	return ps_in;
}


MathedArray::MathedArray()
	: bf_(1, 0), last_(0)
{}


MathedArray::iterator MathedArray::begin() 
{
	return bf_.begin();
}


MathedArray::iterator MathedArray::end() 
{
	return bf_.end();
}


MathedArray::const_iterator MathedArray::begin() const
{
	return bf_.begin();
}


MathedArray::const_iterator MathedArray::end() const
{
	return bf_.end();
}


int MathedArray::empty() const
{
	return (last_ == 0);
}
   

int MathedArray::last() const
{
	return last_;
}


void MathedArray::last(int l)
{
	last_ = l;
}


void MathedArray::need_size(int needed)
{
	if (needed >= static_cast<int>(bf_.size()))
		resize(needed);
}


void MathedArray::resize(int newsize)
{
	// still a bit smelly...
	++newsize;
	bf_.resize(newsize + 1);
	if (last_ >= newsize)
		last_ = newsize - 1;
	bf_[last_] = 0;
}


void MathedArray::move(int p, int shift)
{
	if (p <= last_) {
		need_size(last_ + shift);
		memmove(&bf_[p + shift], &bf_[p], last_ - p);
		last_ += shift;
		bf_[last_] = 0;
	}
}


#if 0
void MathedArray::insert(MathedArray::iterator pos,
			 MathedArray::const_iterator beg,
			 MathedArray::const_iterator end)
{
	bf_.insert(pos, beg, end);
	last_ = bf_.size() - 1;
}
#else
void MathedArray::mergeF(MathedArray * a, int p, int dx)
{
	my_memcpy(&bf_[p], &a->bf_[0], dx);
}
#endif


void MathedArray::raw_pointer_copy(MathedInset ** p, int pos) const
{
	my_memcpy(p, &bf_[pos], sizeof(MathedInset*));
}


#if 0
void MathedArray::insertInset(int pos, MathedInset * p, int type)
{
	//bf_.insert(pos, type);
	InsetTable tmp(pos, p);
	insetList_.push_back(tmp);
}


MathedInset * MathedArray::getInset(int pos) 
{
	InsetList::const_iterator cit = insetList_.begin();
	InsetList::const_iterator end = insetList_.end();
	for (; cit != end; ++cit) {
		if ((*cit).pos == pos)
			return (*cit).inset;
	}
	// not found
	return 0;
	// We would really like to throw an exception instead... (Lgb)
	// throw inset_not_found();
}

#else
void MathedArray::raw_pointer_insert(void * p, int pos, int len)
{
	my_memcpy(&bf_[pos], &p, len);
}
#endif


void MathedArray::strange_copy(MathedArray * dest, int dpos,
				int spos, int len)
{
	my_memcpy(&dest->bf_[dpos], &bf_[spos], len);
}


byte MathedArray::operator[](int i) const
{
	return bf_[i];
}


byte & MathedArray::operator[](int i)
{
	return bf_[i];
}
