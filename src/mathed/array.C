
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "array.h"
#include "math_defs.h"


// Is this still needed? (Lgb)
static inline
void * my_memcpy(void * ps_in, void const * pt_in, size_t n)
{
	char * ps = static_cast<char *>(ps_in);
	char const * pt = static_cast<char const *>(pt_in);
	while (n--) *ps++ = *pt++;
	return ps_in;
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


int MathedArray::maxsize() const
{
	return maxsize_;
}


void MathedArray::resize(int newsize)
{
	if (newsize < ARRAY_MIN_SIZE)
		newsize = ARRAY_MIN_SIZE;
	newsize += ARRAY_STEP - (newsize % ARRAY_STEP);
	bf_.resize(newsize);
	if (last_ >= newsize) last_ = newsize - 1;
	maxsize_ = newsize;
	bf_[last_] = 0;
}


MathedArray::MathedArray(int size) 
{
	maxsize_ = (size < ARRAY_MIN_SIZE) ? ARRAY_MIN_SIZE : size;
	bf_.resize(maxsize_);
	last_ = 0;
}


void MathedArray::move(int p, int shift)
{
	if (p <= last_) {
		if (last_ + shift >= maxsize_) { 
		    resize(last_ + shift);
		}
		memmove(&bf_[p + shift], &bf_[p], last_ - p);
		last_ += shift;
		bf_[last_] = 0;
	}
}


void MathedArray::mergeF(MathedArray * a, int p, int dx)
{
	my_memcpy(&bf_[p], &a->bf_[0], dx);
}


void MathedArray::raw_pointer_copy(MathedInset ** p, int pos) const
{
	my_memcpy(p, &bf_[pos], sizeof(MathedInset*));
}


void MathedArray::raw_pointer_insert(void * p, int pos, int len)
{
	my_memcpy(&bf_[pos], &p, len);
}


void MathedArray::strange_copy(MathedArray * dest, int dpos,
				int spos, int len)
{
	my_memcpy(&dest[dpos], &bf_[spos], len);
}


byte MathedArray::operator[](int i) const
{
	return bf_[i];
}


byte & MathedArray::operator[](int i)
{
	return bf_[i];
}


void MathedArray::insert(int pos, byte c)
{
	if (pos < 0) pos = last_;
	if (pos >= maxsize_) 
		resize(maxsize_ + ARRAY_STEP);
	bf_[pos] = c;
	if (pos >= last_)
		last_ = pos + 1;
}
