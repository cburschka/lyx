/**
 * \file trivstring.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/trivstring.h"
#include "support/docstring.h"

#include <algorithm>

using namespace std;

namespace lyx {

template trivial_string<char>::trivial_string(trivial_string const &);
template trivial_string<char_type>::trivial_string(trivial_string const &);
template<typename Char>
trivial_string<Char>::trivial_string(trivial_string const & that) : size_(that.size_)
{
	if (use_sso())
		copy(that.data_sso(), that.data_sso() + size_ + 1, data_sso());
	else if (size_ > 0) {
		data_ = new Char[size_ + 1];
		copy(that.data_, that.data_ + size_ + 1, data_);
	}
	// else Happens only for really big Char types
}


template trivial_string<char>::trivial_string(string const &);
template trivial_string<char_type>::trivial_string(docstring const &);
template<typename Char>
trivial_string<Char>::trivial_string(
		basic_string<Char, char_traits<Char>, allocator<Char> > const & that)
	: size_(that.length())
{
	if (use_sso()) {
		copy(that.begin(), that.end(), data_sso());
		data_sso()[size_] = '\0';
	} else if (size_ > 0) {
		data_ = new Char[size_ + 1];
		copy(that.begin(), that.end(), data_);
		data_[size_] = '\0';
	}
	// else Happens only for really big Char types
}


template trivial_string<char> &
trivial_string<char>::operator=(trivial_string const &);
template trivial_string<char_type> &
trivial_string<char_type>::operator=(trivial_string const &);
template<typename Char>
trivial_string<Char> & trivial_string<Char>::operator=(trivial_string const & that)
{
	if (&that == this)
		return *this;
	if (!use_sso())
		delete[] data_;
	size_ = that.size_;
	if (use_sso())
		copy(that.data_sso(), that.data_sso() + size_ + 1, data_sso());
	else if (size_ > 0) {
		data_ = new Char[size_ + 1];
		copy(that.data_, that.data_ + size_ + 1, data_);
	} else {
		// Happens only for really big Char types
		data_ = 0;
	}
	return *this;
}


template trivial_string<char> &
trivial_string<char>::operator=(string const &);
template trivial_string<char_type> &
trivial_string<char_type>::operator=(docstring const &);
template<typename Char>
trivial_string<Char> &
trivial_string<Char>::operator=(basic_string<Char, char_traits<Char>, allocator<Char> > const & that)
{
	if (!use_sso())
		delete[] data_;
	size_ = that.size();
	if (use_sso()) {
		copy(that.begin(), that.end(), data_sso());
		data_sso()[size_] = '\0';
	} else if (size_ > 0) {
		data_ = new Char[size_ + 1];
		copy(that.begin(), that.end(), data_);
	} else {
		// Happens only for really big Char types
		data_ = 0;
	}
	return *this;
}


template void
trivial_string<char>::swap(trivial_string<char> &);
template void
trivial_string<char_type>::swap(trivial_string<char_type> &);
template<typename Char>
void trivial_string<Char>::swap(trivial_string & that)
{
	size_t const sizetmp = that.size_;
	that.size_ = size_;
	size_ = sizetmp;
	Char * const datatmp = that.data_;
	that.data_ = data_;
	data_ = datatmp;
}


template<typename Char>
int trivial_string<Char>::compare(trivial_string const & other) const
{
	size_t const lsize = this->length();
	size_t const rsize = other.length();
	size_t const len = min(lsize, rsize);
	int r = char_traits<Char>::compare(c_str(), other.c_str(), len);
	if (r == 0) {
		if (lsize > rsize)
			r = 1;
		else if (lsize < rsize)
			r = -1;
	}
	return r;
}


template string trivial_string<char>::str() const;
template docstring trivial_string<char_type>::str() const;
template<typename Char>
basic_string<Char, char_traits<Char>, allocator<Char> >
trivial_string<Char>::str() const
{
	if (use_sso())
		return basic_string<Char, char_traits<Char>, allocator<Char> >(
				data_sso(), size_);
	if (size_ > 0)
		return basic_string<Char, char_traits<Char>, allocator<Char> >(
				data_, size_);
	// Happens only for really big Char types
	return basic_string<Char, char_traits<Char>, allocator<Char> >();
}


template char const * trivial_string<char>::c_str() const;
template char_type const * trivial_string<char_type>::c_str() const;
template<typename Char> Char const * trivial_string<Char>::c_str() const
{
	if (use_sso())
		return data_sso();
	if (size_ > 0)
		return data_;
	// Happens only for really big Char types
	static const Char empty_char = '\0';
	return &empty_char;
}


template bool operator<(trivial_string<char> const &,
                        trivial_string<char> const &);
template bool operator<(trivial_string<char_type> const &,
                        trivial_string<char_type> const &);
template <typename Char>
bool operator<(trivial_string<Char> const & lhs, trivial_string<Char> const &rhs)
{
	return lhs.compare(rhs) < 0;
}

} // namespace lyx
