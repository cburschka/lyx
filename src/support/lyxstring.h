// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 *======================================================*/

// This one is heavily based on the string class in The C++
// Programming Language by Bjarne Stroustrup

#ifndef LYXSTRING_H
#define LYXSTRING_H 

#ifdef __GNUG__
#pragma interface
#endif

#ifdef HAVE_CONFIG_H
#include <config.h> // needed at least for compilers that do not
#endif              // understand `explicit' (JMarc)

#if 0
#include <iterator>
#endif

#include <cstring>
#include "LAssert.h"
/** A string class for LyX
  
  This is a permanent String class. It is modeled closely after the C++ STL
  string class. In comparison with STL string LString lack support for
  reverse iterators and allocators, in all other senses it is written to be
  a drop in replacement for STL string (or as a transition tool). So
  documentation for STL string should be valid for LString too.

  Notes for usage:

  When you declare an LString, it is initially empty. There is no need to
  do things like #LString a= "";#, especially not in constructors.

  If you want to use a default empty LString as a parameter, use
  
  	#void foo(LString par = LString());	// Correct#
  
  rather than

  	#void foo(LString par = "");	// WRONG!#
  	#void foo(LString par = 0);	// WRONG!#

  (The last one is only wrong because some compilers can't handle it.)

  Methods that take an index as parameter all follow this rule: Valid indexes
  go from 0 to length()-1.
  \begin{tabular}{rl}
  Correct: & #foo.substring(0, length()-1);# \\
  Wrong:   & #bar.substring(0, length());#
  \end{tabular}
  
  It is important that you declare LStrings as const if possible, because
  some methods are much more efficient in const versions.
  
  If you want to check whether a string is empty, do

  	#if (foo.empty()) something right#

  rather than something along the lines of

  	#if (!foo) completely wrong#

When you want to copy an LString, just do
  
	#LString a, b = "String";#
	#a = b;	// That's it!#

  not something like
  
  	#LString a, b = "String";#
  	#a = b.copy(); // This leaks.#
  
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
	typedef const reference const_reference;

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
	inline lyxstring();
	
	/// #lyxstring x(lyxstring ...)# 
	lyxstring(lyxstring const &, size_type pos = 0, size_type n = npos);
	
	/// #lyxstring x("abc", 2) -> "ab"#
	lyxstring(value_type const *, size_type n);
	
	/// #lyxstring x("abc")#
	lyxstring(value_type const *);
	
	/// lyxstring(5, 'n') -> "nnnnn"
	lyxstring(size_type n, value_type c);

	///
	lyxstring(iterator first, iterator last);
	
	///
	~lyxstring() { if (--rep->ref == 0) delete rep; }

	//@}

	
	/**@name Size and Capacity */
	//@{
	
	/// number of characters
	size_type size() const; // { return rep->sz; }

	/// largest possible string
	size_type max_size() const { return npos -1; }

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

	///
	lyxstring & assign(iterator first, iterator last);
	
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
	inline lyxstring & operator+=(value_type);

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

	///
	lyxstring & append(iterator first, iterator last);
	
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

	///
	void insert(iterator p, iterator first, iterator last);
	
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
	size_type find_first_of(value_type const * p, size_type i, size_type n) const;
	
	///
	size_type find_first_of(value_type const * p, size_type i = 0) const;
	
	///
	size_type find_first_of(value_type c, size_type i = 0) const;

	///
	size_type find_last_of(lyxstring const &, size_type i = npos) const;
	
	///
	size_type find_last_of(value_type const * p, size_type i, size_type n) const;
	
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
	size_type find_first_not_of(value_type const * p, size_type i = 0) const;
	
	///
	size_type find_first_not_of(value_type c, size_type i = 0) const;

	///
	size_type find_last_not_of(lyxstring const &, size_type i = npos) const;
	
	///
	size_type find_last_not_of(value_type const * p, size_type i,
				   size_type n) const;
	
	///
	size_type find_last_not_of(value_type const * p, size_type i = npos) const;
	
	///
	size_type find_last_not_of(value_type c, size_type i = npos) const;

	//*}

	
	/**@name Replace */
	//@{

	// replace [(*this)[i], (*this)[i+n]] with other characters:

	///
	lyxstring & replace(size_type i,size_type n, lyxstring const & str);

	///
	lyxstring & replace(size_type i,size_type n, lyxstring const & s,
			  size_type i2, size_type n2);

	///
	lyxstring & replace(size_type i,size_type n, value_type const * p,
			  size_type n2);

	///
	lyxstring & replace(size_type i,size_type n, value_type const * p);

	///
	lyxstring & replace(size_type i, size_type n, size_type n2, value_type c);

	///
	lyxstring & replace(iterator i, iterator i2, const lyxstring & str);

	///
	lyxstring & replace(iterator i, iterator i2,
			  value_type const * p, size_type n);

	///
	lyxstring & replace(iterator i, iterator i2, value_type const * p);

	///
	lyxstring & replace(iterator i, iterator i2, size_type n , value_type c);
	
	///
	lyxstring & replace(iterator i, iterator i2, iterator j, iterator j2);

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
	size_type copy(value_type * buf, size_type len, size_type pos = 0) const;

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
	///
	lyxstring & operator+(int);
	///
	lyxstring & operator=(int);
	
	/// A string representation
	struct Srep {
		///
		static lyxstring::size_type const xtra = 
					static_cast<lyxstring::size_type>(8);
		/// size
		lyxstring::size_type sz;
		/// Reference count
		unsigned short ref;
		/// The total amount of data reserved for this representaion
		lyxstring::size_type res;
		/// Data. At least 1 char for trailing null.
		lyxstring::value_type * s;

		///
		Srep(lyxstring::size_type nsz, const lyxstring::value_type * p);
		///
		Srep(lyxstring::size_type nsz, lyxstring::value_type ch);
		///
		~Srep() { delete[] s; }
		///
		Srep * get_own_copy()
		{
			if (ref == 1) return this;
			ref--;
			return new Srep(sz, s);
		}
		
		///
		void assign(lyxstring::size_type nsz, const lyxstring::value_type * p);
		///
		void assign(lyxstring::size_type nsz, lyxstring::value_type ch);
		///
		void append(lyxstring::size_type asz, const lyxstring::value_type * p);
		///
		void push_back(lyxstring::value_type c);
		///
		void insert(lyxstring::size_type pos,
			    const lyxstring::value_type * p,
			    lyxstring::size_type n);
		///
		void resize(lyxstring::size_type n, lyxstring::value_type c);
		///
		void reserve(lyxstring::size_type res_arg);
		///
		void replace(lyxstring::size_type i, lyxstring::size_type n,
			     lyxstring::value_type const * p, lyxstring::size_type n2);
	private:
		Srep(const Srep &);
		Srep & operator=(const Srep &);
	};

	/** The empty_rep is a local static in each function that
	    benefits from one. There is no "global" empty srep but lyxstring
	    doesn't need one (no code actually relies upon a single
	    empty srep).
	    This overcomes *all* "static initialization" problems,
	    at maximum speed, with a small overhead of a few local static
	    empty_reps.
	 */

	/// A string is a pointer to it's representation
	Srep * rep;

#ifdef DEVEL_VERSION
	/// lyxstringInvariant is used to test the lyxstring Invariant
	friend class lyxstringInvariant;
#endif //DEVEL_VERSION
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

class istream; class ostream;
istream & operator>>(istream &, lyxstring &);
ostream & operator<<(ostream &, lyxstring const &);
istream & getline(istream &, lyxstring &, lyxstring::value_type delim = '\n');

#endif
