// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

// This one is heavily based on the string class in The C++
// Programming Language by Bjarne Stroustrup

// This class is supposed to be functionaly equivalent to a
// standard conformant string. This mean among others that we
// are useing the same requirements. Before you change anything
// in this file consult me and/or the standard to discover the
// right behavior.

#ifndef LYXSTRING_H
#define LYXSTRING_H 

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#if 0
#include <iterator>
#endif

#include <cstring> // for size_t

/** A string class for LyX
  
  This is a permanent String class. It is modeled closely after the C++ STL
  string class. In comparison with STL string lyxstring lack support for
  reverse iterators and allocators, also char_traits is not used. In most
  other senses it is written to be  a drop in replacement for STL string
  (or as a transition tool). So documentation for STL string should be
  valid for lyxstring too.

  Notes for usage:

  When you declare an lyxstring, it is initially empty. There is no need to
  do things like #lyxstring a= "";#, especially not in constructors.

  If you want to use a default empty lyxstring as a parameter, use
  
  	#void foo(lyxstring par = lyxstring());	// Correct#
  
  rather than

  	#void foo(lyxstring par = "");	// WRONG!#
  	#void foo(lyxstring par = 0);	// WRONG!#

  (The last one is only wrong because some compilers can't handle it.)

  Methods that take an index as parameter all follow this rule: Valid indexes
  go from 0 to length()-1.
  \begin{tabular}{rl}
  Correct: & #foo.substring(0, length()-1);# \\
  Wrong:   & #bar.substring(0, length());#
  \end{tabular}
  
  It is important that you declare lyxstring as const if possible, because
  some methods are much more efficient in const versions.
  
  If you want to check whether a string is empty, do

  	#if (foo.empty()) something right#

  rather than something along the lines of

  	#if (!foo) completely wrong#

When you want to copy an lyxstring, just do
  
	#lyxstring a, b = "String";#
	#a = b;	// That's it!#

  not something like
  
  	#lyxstring a, b = "String";#
  	#a = b.copy(); // This leaks. // and does not work either. #
  
  The class automatically handles deep copying when required.
*/
class lyxstring {
public:
	/**@name Typedefs */
	//@{
	
	///
	typedef char value_type;

	///
	typedef value_type * pointer;

	///
	typedef value_type & reference;

	///
	typedef value_type const & const_reference;

	///
	typedef size_t size_type;

	///
	typedef int difference_type;

	///
	typedef value_type * iterator;
	///
	typedef const value_type * const_iterator;
#if 0
	///
	typedef reverse_iterator<iterator, value_type, reference>
	reverse_iterator;
	///
	typedef reverse_iterator<const_iterator, const value_type,
		const_reference> const_reverse_iterator;

#endif
	//@}

	///
	iterator begin();
	///
	const_iterator begin() const;
	///
	iterator end();
	///
	const_iterator end() const;
#if 0
	///
	reverse_iterator rbegin();
	///
	const_reverse_iterator rbegin() const;
	///
	reverse_iterator rend();
	///
	const_reverse_iterator rend() const;
#endif
	/**@name Constructors and Deconstructors. lyxstring has the same
	  constructors as STL, except the one using iterators, one other
	  difference is that lyxstring, do not allow the optional allocator
	  parameter. */
	//@{
	
	/// "all characters" marker
	static const size_type npos;

	/// #lyxstring x;# 
	lyxstring();
	
	/// #lyxstring x(lyxstring ...)# 
	lyxstring(lyxstring const &, size_type pos = 0, size_type n = npos);
	
	/// #lyxstring x("abc", 2) -> "ab"#
	lyxstring(value_type const *, size_type n);
	
	/// #lyxstring x("abc")#
	lyxstring(value_type const *);
	
	/// lyxstring(5, 'n') -> "nnnnn"
	lyxstring(size_type n, value_type c);

#if 1
	///
	lyxstring(const_iterator first, const_iterator last);
#else
	///
	template<class InputIterator>
	lyxstring::lyxstring(InputIterator begin, InputIterator end) {
		while (begin != end) {
			push_back((*begin));
			++begin;
		}
	}
#endif
	///
	~lyxstring();

	//@}

	
	/**@name Size and Capacity */
	//@{
	
	/// number of characters
	size_type size() const;

	/// largest possible string
	size_type max_size() const { return npos - 1; }

	///
	size_type length() const { return size(); }

	///
	bool empty() const { return size() == 0; }

	///
	void resize(size_type n, value_type c);

	///
	void resize(size_type n) { resize(n, ' '); }

	/// size of the memory (in number of elements) allocated.
	size_type capacity() const;

	///
	void reserve(size_type res_arg = 0);
	
	//@}

	/**@name Assignment */
	//@{

	///
	lyxstring & operator=(lyxstring const &);
	
	///
	lyxstring & operator=(value_type const *);
	
	///
	lyxstring & operator=(value_type);

	///
	lyxstring & assign(lyxstring const &);
	
	///
	lyxstring & assign(lyxstring const &, size_type pos, size_type n);
	
	///
	lyxstring & assign(value_type const * p, size_type n);
	
	///
	lyxstring & assign(value_type const * p);
	
	///
	lyxstring & assign(size_type n, value_type c);

#if 1
	///
	lyxstring & assign(const_iterator first, const_iterator last);
#else
	///
	template<class InputIterator>
	lyxstring & assign(InputIterator begin, InputIterator end) {
		clear;
		while (begin != end) {
			push_back((*begin));
			++begin;
		}
	}
#endif
	//@}

	/**@name Element Access. Since lyxstring does not use exceptions,
	  an abort is called on out-of-range access to at(). */
	
	/// unchecked access
	const_reference operator[](size_type) const;
	
	/// unchecked access
	reference operator[](size_type);
	
	/// checked access
	const_reference at(size_type) const;
	
	/// checked access
	reference at(size_type);

	//@}

	/**@name Insert */
	//@{
	
	// add characters after (*this)[length()-1]:
	
	///
	lyxstring & operator+=(lyxstring const &);
	
	///
	lyxstring & operator+=(value_type const *);
	
	///
	lyxstring & operator+=(value_type);

	///
	void push_back(value_type);

	///
	lyxstring & append(lyxstring const &);

	///
	lyxstring & append(lyxstring const &, size_type pos, size_type n);

	///
	lyxstring & append(value_type const *, size_type n);

	///
	lyxstring & append(value_type const *);

	///
	lyxstring & append(size_type n, value_type);

#if 1
	///
	lyxstring & append(iterator first, iterator last);
#else
	///
	template<class InputIterator>
	lyxstring & append(InputIterator begin, InputIterator end) {
		while (begin != end) {
			push_back((*begin));
			++begin;
		}
		return *this;
	}
#endif
	// insert characters before (*this)[pos]:

	///
	lyxstring & insert(size_type pos, lyxstring const &);
	
	///
	lyxstring & insert(size_type pos, lyxstring const &,
			size_type pos2, size_type n);
	
	///
	lyxstring & insert(size_type pos, value_type const * p,
			 size_type n);

	///
	lyxstring & insert(size_type pos, value_type const * p);

	///
	lyxstring & insert(size_type pos, size_type n, value_type c);

	// insert characters before p

	///
	iterator insert(iterator p, value_type c);

	///
	void insert(iterator p, size_type n , value_type c);

#if 1
	///
	void insert(iterator p, iterator first, iterator last);
#else
	///
	template<class InputIterator>
	void insert(iterator p, InputIterator begin, InputIterator end) {
		iterator it;
		while (begin != end) {
			it = insert(p, (*begin));
			++begin;
		}
	}
#endif
	
	//@}

	/**@name Find */
	//@{

	///
	size_type find(lyxstring const &, size_type i = 0) const;
	
	///
	size_type find(value_type const * p,
			 size_type i, size_type n) const;
	
	///
	size_type find(value_type const * p, size_type i = 0) const;
	
	///
	size_type find(value_type c, size_type i = 0) const;

	///
	size_type rfind(lyxstring const &, size_type i = npos) const;
	
	///
	size_type rfind(value_type const * p, size_type i, size_type n) const;
	
	///
	size_type rfind(value_type const * p, size_type i = npos) const;
	
	///
	size_type rfind(value_type c, size_type i = npos) const;

	///
	size_type find_first_of(lyxstring const &, size_type i = 0) const;
	
	///
	size_type find_first_of(value_type const * p, size_type i,
				size_type n) const;
	
	///
	size_type find_first_of(value_type const * p, size_type i = 0) const;
	
	///
	size_type find_first_of(value_type c, size_type i = 0) const;

	///
	size_type find_last_of(lyxstring const &, size_type i = npos) const;
	
	///
	size_type find_last_of(value_type const * p, size_type i,
			       size_type n) const;
	
	///
	size_type find_last_of(value_type const * p, size_type i = npos) const;
	
	///
	size_type find_last_of(value_type c, size_type i = npos) const;

	///
	size_type find_first_not_of(lyxstring const &, size_type i = 0) const;
	
	///
	size_type find_first_not_of(value_type const * p, size_type i,
				    size_type n) const;
	
	///
	size_type find_first_not_of(value_type const * p,
				    size_type i = 0) const;
	
	///
	size_type find_first_not_of(value_type c, size_type i = 0) const;

	///
	size_type find_last_not_of(lyxstring const &,
				   size_type i = npos) const;
	
	///
	size_type find_last_not_of(value_type const * p, size_type i,
				   size_type n) const;
	
	///
	size_type find_last_not_of(value_type const * p,
				   size_type i = npos) const;
	
	///
	size_type find_last_not_of(value_type c, size_type i = npos) const;

	//*}

	
	/**@name Replace */
	//@{

	// replace [(*this)[i], (*this)[i+n]] with other characters:

	///
	lyxstring & replace(size_type i, size_type n, lyxstring const & str);

	///
	lyxstring & replace(size_type i, size_type n, lyxstring const & s,
			  size_type i2, size_type n2);

	///
	lyxstring & replace(size_type i, size_type n, value_type const * p,
			  size_type n2);

	///
	lyxstring & replace(size_type i, size_type n, value_type const * p);

	///
	lyxstring & replace(size_type i, size_type n,
			    size_type n2, value_type c);

	///
	lyxstring & replace(iterator i, iterator i2, const lyxstring & str);

	///
	lyxstring & replace(iterator i, iterator i2,
			  value_type const * p, size_type n);

	///
	lyxstring & replace(iterator i, iterator i2, value_type const * p);

	///
	lyxstring & replace(iterator i, iterator i2,
			    size_type n , value_type c);

	///
	lyxstring & replace(iterator i, iterator i2, iterator j, iterator j2);

	///
	void swap(lyxstring & str);
	
	/// Erase n chars from position i.
	lyxstring & erase(size_type i = 0, size_type n = npos);

	///
	lyxstring & clear() {
		return erase(0, npos);
	}

	///
	iterator erase(iterator i);

	///
	iterator erase(iterator first, iterator last);

	//@}

	
	/**@name Conversion to C-style Strings */
	//@{
	
	/// 
	value_type const * c_str() const;

	/** Note that this is STL compilant, so you can not assume
	  that the returned array has a trailing '\0'. */
	value_type const * data() const;

	/** This one returns a verbatim copy. Not the trailing '\0'
	  The caller must provide a buffer with engough room.
	  */
	size_type copy(value_type * buf, size_type len,
		       size_type pos = 0) const;

	//@}

	
	/**@name Comparisons. Combined > and == */
	//@{
	
	///
	int compare(lyxstring const & str) const; 

	///
	int compare(value_type const * p) const;

	///
	int compare(size_type pos, size_type n, lyxstring const & str) const;

	///
	int compare(size_type pos, size_type n, lyxstring const & str,
		    size_type pos2, size_type n2) const;

	///
	int compare(size_type pos, size_type n, value_type const * p,
		    size_type n2 = npos) const;
	
	//@}

	
	
	/**@name Substrings */
	//@{

	///
	lyxstring substr(size_type i = 0, size_type n = npos) const;
	
	//@}

private:
	// These three operators can be used to discover erronous use of
	// ints and strings. However a conforming C++ compiler will flag
	// a lot of char operations as ambigous when they are compiled
	// in. Use them for debugging only (or perhaps not even then.)
	// Lgb.
	//
	//lyxstring & operator+(int);
	//
	//lyxstring & operator=(int);
	//
	//lyxstring & operator+=(int);

	/// Compare this with s. works with embedded '\0' chars also.
	int internal_compare(size_type pos, size_type n,
			     value_type const * s,
			     size_type slen, size_type n2) const;
		
	/// Forward declaration of the string representation
	struct Srep;
	// DEC cxx requires this.
	friend struct Srep;

	/// A string is a pointer to it's representation
	Srep * rep;

	/** Note: The empty_rep is a local static in each function that
	    benefits from one. There is no "global" empty srep but lyxstring
	    doesn't need one (no code actually relies upon a single
	    empty srep).
	    This overcomes *all* "static initialization" problems,
	    at maximum speed, with a small overhead of a few local static
	    empty_reps.
	*/

#ifdef ENABLE_ASSERTIONS
	/// lyxstringInvariant is used to test the lyxstring Invariant
	friend class lyxstringInvariant;
#endif
};

// The usual comparison operators ==, !=, >, <, >=, <= are
// provided for lyxstrings

bool operator==(lyxstring const &, lyxstring const &);
bool operator==(lyxstring::value_type const *, lyxstring const &);
bool operator==(lyxstring const &, lyxstring::value_type const *);


bool operator!=(lyxstring const &, lyxstring const &);
bool operator!=(lyxstring::value_type const *, lyxstring const &);
bool operator!=(lyxstring const &, lyxstring::value_type const *);


bool operator>(lyxstring const &, lyxstring const &);
bool operator>(lyxstring::value_type const *, lyxstring const &);
bool operator>(lyxstring const &, lyxstring::value_type const *);


bool operator<(lyxstring const &, lyxstring const &);
bool operator<(lyxstring::value_type const *, lyxstring const &);
bool operator<(lyxstring const &, lyxstring::value_type const *);


bool operator>=(lyxstring const &, lyxstring const &);
bool operator>=(lyxstring::value_type const *, lyxstring const &);
bool operator>=(lyxstring const &, lyxstring::value_type const *);


bool operator<=(lyxstring const &, lyxstring const &);
bool operator<=(lyxstring::value_type const *, lyxstring const &);
bool operator<=(lyxstring const &, lyxstring::value_type const *);


// Concatenation

lyxstring operator+(lyxstring const & a, lyxstring const & b);
lyxstring operator+(char const * a, lyxstring const & b);
lyxstring operator+(lyxstring::value_type a, lyxstring const & b);
lyxstring operator+(lyxstring const & a, lyxstring::value_type const * b);
lyxstring operator+(lyxstring const & a, lyxstring::value_type b);

void swap(lyxstring & s1, lyxstring & s2);

std::istream & operator>>(std::istream &, lyxstring &);
std::ostream & operator<<(std::ostream &, lyxstring const &);
std::istream & getline(std::istream &, lyxstring &, lyxstring::value_type delim = '\n');

#endif
