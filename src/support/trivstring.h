// -*- C++ -*-
/**
 * \file trivstring.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_TRIVSTRING_H
#define LYX_TRIVSTRING_H

#include "support/strfwd.h"

#ifdef STD_STRING_USES_COW
#include <cstdlib>

namespace lyx {

/**
 * Trivial string class with almost no features.
 * The public interface is a subset of the std::basic_string interface.
 * The only important feature is that any read-only access does not need
 * synchronization between multiple threads, i.e. it is thread-safe without
 * locking.
 * Therefore you can safely use a const trivial_string object in multiple
 * threads at the same time. This is not the case for std::basic_string in some
 * STL implementations (e. g. GNU libcstd++, see bug 9336 and
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=21334.
 * This class should not be used for anything else than providing thread-safety.
 * It should be removed as soon as LyX requires C++11, and all supported STL
 * implementations provide a C++11 conformant std::basic_string.
 *
 * If you change anything in this class please ensure that the unit test
 * tests/check_trivstring.cpp still tests 100% of the public interface.
 */
template <typename Char> class trivial_string
{
public:
	/// Construct an empty string
	trivial_string() : size_(0), data_(0) {}
	/// Construct a string from a copy of \p that
	trivial_string(trivial_string const & that);
	/// Construct a string from a copy of \p that
	trivial_string(std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> > const & that);
	///
	~trivial_string() { if (!use_sso()) delete[] data_; }
	/// Assign a copy of \p that
	trivial_string & operator=(trivial_string const & that);
	/// Assign a copy of \p that
	trivial_string & operator=(std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> > const & that);
	/// Exchange contents with contents of \p that
	void swap(trivial_string & that);
	/// The length of the string, excluding the final 0 character
	size_t length() const { return size_; }
	/// Is this string empty?
	bool empty() const { return size_ == 0; }
	/// Is this string ordered before, at the same position or after \p other?
	int compare(trivial_string const & other) const;
	/// Create a copy as std::basic_string
	operator std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> >() const;
	/// Return a C-compatible string, terminated by a 0 character.
	/// This is never a copy and only valid for the life time of the trivial_string instance.
	Char const * c_str() const;
private:
	/**
	 * Whether short string optimization is used.
	 * Short string optimization is a technique where no additional memory
	 * needs to be allocated to store the string contents.
	 * Instead, the memory which would be used to store the pointer to the
	 * character buffer is reinterpreted to be a Char * buffer.
	 * On most 64 bit systems and with Char == char this allows to store
	 * strings of up to 7 characters without allocating additional memory.
	 */
	bool use_sso() const { return (size_ + 1) * sizeof(Char) <= sizeof(Char *); }
	/// The character storage if sso is used
	Char       * data_sso()       { return reinterpret_cast<Char *      >(&data_); }
	/// The character storage if sso is used
	Char const * data_sso() const { return reinterpret_cast<Char const *>(&data_); }
	/// The length of the string, excluding the final 0 character
	size_t size_;
	/// The character storage
	Char * data_;
};


/// Comparison operator (needed for std::set etc)
template <typename Char> bool operator<(trivial_string<Char> const & lhs, trivial_string<Char> const & rhs);


/// Equality operator
template <typename Char> bool operator==(trivial_string<Char> const & lhs, trivial_string<Char> const & rhs);
template <typename Char> bool operator==(trivial_string<Char> const & lhs, Char const * rhs);
template <typename Char> bool operator==(Char const * lhs, trivial_string<Char> const & rhs);


/// Stream output operator
template <typename Char>
std::basic_ostream<Char, std::char_traits<Char> > &
operator<<(std::basic_ostream<Char, std::char_traits<Char> > &, trivial_string<Char> const &);

} // namespace lyx
#else
#include <string>
#endif
#endif
