/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 * ======================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __GNUG__
#pragma implementation "lyxstring.h"
#endif

#include "lyxstring.h"
#include <cstdlib>
#include <cctype>
#include <algorithm>

#include "LAssert.h"

using std::min;

// Reference count has been checked, empty_rep removed and
// introduced again in a similar guise. Where is empty_rep _really_
// needed?

// Insertion and replace is implemented, as far as I can see everything
// works, but could perhaps be done smarter.

// All the different find functions need a good look over.
// I have so far not tested them extensively and would be
// happy if others took the time to have a peek.

#ifdef WITH_WARNINGS
#warning temporarily here for debugging purposes only
#endif
lyxstring::size_type lyxstring::size() const
{ 
	return rep->sz;
}


//--------------------------------------------------------------------------
// lyxstringInvariant
#ifdef DEVEL_VERSION

/** Testing of the lyxstring invariant
 * By creating an object that tests the lyxstring invariant during its
 * construction *and* its deconstruction we greatly simplify our code.
 * Calling TestlyxstringInvariant() upon entry to an lyxstring method 
 * will test the invariant upon entry to the code.  If the Asserts fail
 * then we know from the stack trace that the corruption occurred *before*
 * entry to this method.  We can also be sure it didn't happen in any of
 * the tested lyxstring methods.  It is therefore likely to be due to some
 * other external force.
 * Several lyxstring methods have multiple exit points which would otherwise
 * require us to insert a separate test before each return.  But since we
 * created an object its destructor will be called upon exit (any exit!).
 * We thus get testing at both start and end of a method with one line of
 * code at the head of a method.  More importantly,  we get good testing
 * everytime we run the code.
 * NOTE:  just because we test the invariant doesn't mean we can forget
 * about testing pre and post conditions specific to any given method.
 * This test simply proves that the lyxstring/Srep is in a valid state it
 * does *not* prove that the method did what it was supposed to.
 */
class lyxstringInvariant
{
public:
	lyxstringInvariant(lyxstring const *);
	~lyxstringInvariant();
private:
	void helper() const;
	lyxstring const * object;
};

// To test if this scheme works "as advertised" uncomment the printf's in
// the constructor and destructor below and then uncomment the printf and the
// call to TestlyxstringInvariant() in lyxstring::operator=(char const *).
// The correct output when LyX has been recompiled and run is:
//     lyxstringInvariant constructor
//     lyxstring::operator=(char const *)
//     lyxstringInvariant constructor
//     lyxstringInvariant destructor completed
//     lyxstringInvariant destructor completed
// NOTE: The easiest way to catch this snippet of the output is to wait for
//       the splash screen to disappear and then open and close Help->Credits
//
lyxstringInvariant::lyxstringInvariant(lyxstring const * ls) : object(ls)
{
//	printf("lyxstringInvariant constructor\n");
	helper();
}

lyxstringInvariant::~lyxstringInvariant()
{
	helper();
//	printf("lyxstringInvariant destructor completed\n");
}

void lyxstringInvariant::helper() const
{
	// Some of these tests might look pointless but they are
	// all part of the invariant and if we want to make sure
	// we have a bullet proof implementation then we need to
	// test every last little thing we *know* should be true.
	// I may have missed a test or two, so feel free to fill
	// in the gaps.  ARRae.
	// NOTE:  Don't put TestlyxstringInvariant() in any of the
	// lyxstring methods used below otherwise you'll get an
	// infinite recursion and a crash.
	Assert(object);
	Assert(object->rep);
	Assert(object->rep->s);    // s is never 0
	Assert(object->rep->res);  // always some space allocated
	Assert(object->size() <= object->rep->res);
	Assert(object->rep->ref >= 1);  // its in use so it must be referenced
	Assert(object->rep->ref < (1 << 8*sizeof(object->rep->ref)) - 1);
	// if it does ever == then we should be generating a new copy
	// and starting again.  (Is char always 8-bits?)
}
#define TestlyxstringInvariant(s) lyxstringInvariant lyxstring_invariant(s);
#else
#define TestlyxstringInvariant(s)
#endif //DEVEL_VERSION
//-------------------------------------------------------------------------

///////////////////////////////////////
// Constructors and Deconstructors.
///////////////////////////////////////


lyxstring::Srep::Srep(lyxstring::size_type nsz, const value_type * p)
{
// can be called with p==0 by lyxstring::assign(const value_type *, size_type)

	sz = nsz;
	ref = 1;
	res = sz + xtra;
	s = new value_type[res + 1]; // add space for terminator
	if (p && sz) {
		// if sz = 0 nothing gets copied and we have an error
		memcpy(s, p, sz);
	} else {
		// possibly allows for large but empty string
		sz = 0;  // this line should be redundant
		s[0] = '\0';
	}
}


lyxstring::Srep::Srep(lyxstring::size_type nsz, value_type ch)
{
	sz = nsz;
	ref = 1;
	res = sz + xtra;
	s = new value_type[res + 1]; // add space for terminator
	memset(s, ch, sz);
	if (!ch) {
		// if ch == '\0' strlen(lyxstring.c_str()) == 0 so sz = 0
		// allows for large but empty string
		sz = 0;
	}
}
	

void lyxstring::Srep::assign(lyxstring::size_type nsz, const value_type * p)
{
// can be called with p==0 by lyxstring::assign(const value_type *, size_type)

	if (res < nsz) {
		delete[] s;
		sz = nsz;
		res = sz + xtra;
		s = new value_type[res + 1]; // add space for terminator
	} else {
		sz = nsz;
	}
	if (p && sz) {
		// if sz = 0 nothing gets copied and we have an error
		memcpy(s, p, sz);
	} else {
		// stops segfaults
		sz = 0;  // this line should be redundant
		s[0] = '\0';
	}
}


void lyxstring::Srep::assign(lyxstring::size_type nsz, value_type ch)
{
	sz = nsz;
	if (res < nsz) {
		delete[] s;
		res = sz + xtra;
		s = new value_type[res + 1]; // add space for terminator
	}
	memset(s, ch, sz);
	if (!ch) {
		// if ch == '\0' strlen(lyxstring.c_str()) == 0 so sz = 0
		// allows for a large empty string
		sz = 0;
	}
}


void lyxstring::Srep::append(lyxstring::size_type asz, const value_type * p)
{
	register unsigned int const len = sz + asz;
	if (res < len) {
		res = len + xtra;
		value_type * tmp = new value_type[res + 1];
		memcpy(tmp, s, sz);
		memcpy(tmp + sz, p, asz);
		sz += asz;
		delete[] s;
		s = tmp;
	} else {
		memcpy(s + sz, p, asz);
		sz += asz;
	}
}


void lyxstring::Srep::push_back(value_type c)
{
	s[sz] = c; // it is always room to put a value_type at the end
	++sz;
	if (res < sz) {
		res = sz + xtra;
		value_type * tmp = new value_type[res + 1];
		memcpy(tmp, s, sz);
		delete[] s;
		s = tmp;
	}
}


void lyxstring::Srep::insert(lyxstring::size_type pos, const value_type * p,
			   lyxstring::size_type n)
{
	Assert(pos <= sz);
	if (res < n + sz) {
		res = sz + n + xtra;
		value_type * tmp = new value_type[res + 1];
		memcpy(tmp, s, pos);
		memcpy(tmp + pos, p, n);
		memcpy(tmp + pos + n, & s[pos], sz - pos);
		sz += n;
		delete[] s;
		s = tmp;
	} else {
		memmove(s + pos + n, & s[pos], sz - pos);
		memcpy(s + pos, p, n);
		sz += n;
	}
}


void lyxstring::Srep::resize(size_type n, value_type c)
{
	Assert(n < npos);
	// This resets sz to res_arg
	res = min(n, npos - 2); // We keep no xtra when we resize
	value_type * tmp = new value_type[res + 1];
	memcpy(tmp, s, min(sz, res));
	if (res > sz)
		memset(tmp + sz, c, res - sz);
	delete[] s;
	sz = res;
	s = tmp;
}


void lyxstring::Srep::reserve(lyxstring::size_type res_arg)
{
	// This keeps the old sz, but
	// increases res with res_arg
	res += res_arg;
	value_type * tmp = new value_type[res + 1];
	memcpy(tmp, s, sz);
	delete[] s;
	s = tmp;
}


void lyxstring::Srep::replace(lyxstring::size_type i, lyxstring::size_type n,
			    value_type const * p, size_type n2)
{
// can be called with p=0 and n2=0
	Assert(i < sz && ((!p && !n2) || p));
	n = min(sz - i, n);
	sz -= n;
	if (res >= n2 + sz) {
		memmove(s + i + n2, &s[i + n], sz - i);
		memcpy(s + i, p, n2);
		sz += n2;
	} else {
		res = sz + n2 + xtra;
		value_type * tmp = new value_type[res + 1];
		memcpy(tmp, s, i);
		memcpy(tmp + i, p, n2);
		memcpy(tmp + i + n2, &s[i + n], sz - i);
		delete[] s;
		s = tmp;
		sz += n2; 
	}
}


lyxstring::size_type const lyxstring::npos = static_cast<lyxstring::size_type>(-1);

lyxstring::lyxstring()
{
	static Srep empty_rep(0, "");
	++empty_rep.ref;
	rep = &empty_rep;
}


lyxstring::lyxstring(lyxstring const & x, size_type pos, size_type n)
{
	Assert(pos < x.rep->sz || pos == 0);
	if (pos == 0 && n >= x.length()) { // this is the default
		x.rep->ref++;
		rep = x.rep;
	} else {
		rep = new Srep(min(n, x.rep->sz - pos), &(x.rep->s[pos]));
	}
}


lyxstring::lyxstring(value_type const * s, size_type n)
{
	Assert(s); // we don't allow null pointers
	static Srep empty_rep(0, "");
	if (*s && n) { // s is not empty string and n > 0
		rep = new Srep(min(strlen(s), n), s);
	} else {
		++empty_rep.ref;
		rep = &empty_rep;
	}
}


lyxstring::lyxstring(value_type const * s)
{
	Assert(s); // we don't allow null pointers
	static Srep empty_rep(0, "");
	if (*s) { // s is not empty string
		rep = new Srep(strlen(s), s);
	} else {
		++empty_rep.ref;
		rep = &empty_rep;
	}
}


lyxstring::lyxstring(size_type n, value_type c)
{
	rep = new Srep(n, c);
}


lyxstring::lyxstring(iterator first, iterator last)
{
	rep = new Srep(last - first, first);
}


///////////////////////
// Iterators
///////////////////////

lyxstring::iterator lyxstring::begin()
{
	return rep->s;
}


lyxstring::const_iterator lyxstring::begin() const
{
	return rep->s;
}


lyxstring::iterator lyxstring::end()
{
	return rep->s + rep->sz;
}


lyxstring::const_iterator lyxstring::end() const
{
	return rep->s + rep->sz;
}

#if 0
reverse_iterator lyxstring::rbegin()
{
	return reverse_iterator( end() );
}


const_reverse_iterator lyxstring::rbegin() const
{
	return const_reverse_iterator( end() );
}


reverse_iterator lyxstring::rend()
{
	return reverse_iterator( begin() );
}


const_reverse_iterator lyxstring::rend() const
{
	return const_reverse_iterator( begin() );
}
#endif

///////////////////////
// Size and Capacity
///////////////////////

void lyxstring::resize(size_type n, value_type c)
{
	TestlyxstringInvariant(this);

	// This resets sz to res_arg
	rep = rep->get_own_copy();
	rep->resize(n, c);
}


lyxstring::size_type lyxstring::capacity() const
{
	return rep->res;
}


void lyxstring::reserve(size_type res_arg)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->reserve(res_arg);
}


////////////////
// Assignment
////////////////

lyxstring & lyxstring::operator=(lyxstring const & x)
{
	TestlyxstringInvariant(this);

	return assign(x);
}


lyxstring & lyxstring::operator=(value_type const * s)
{
	Assert(s);	
	TestlyxstringInvariant(this);
//	printf("lyxstring::operator=(value_type const *)\n");

	return assign(s);
}


lyxstring & lyxstring::operator=(value_type c)
{
	TestlyxstringInvariant(this);

	value_type s[1];
	s[0] = c;
	if (rep->ref == 1) // recycle rep
		rep->assign(1, s);
	else {
		rep->ref--;
		rep = new Srep(1, s);
	}
	return *this;
}


lyxstring & lyxstring::assign(lyxstring const & x)
{
	TestlyxstringInvariant(this);

	x.rep->ref++; // protect against ``st = st''
	if (--rep->ref == 0) delete rep;
	rep = x.rep; // share representation
	return *this;
}
	

lyxstring & lyxstring::assign(lyxstring const & x, size_type pos, size_type n)
{
	TestlyxstringInvariant(this);

	return assign(x.substr(pos, n));
}
	

lyxstring & lyxstring::assign(value_type const * s, size_type n)
{
	Assert(s);
	TestlyxstringInvariant(this);

	n = min(strlen(s), n);
	if (rep->ref == 1) // recycle rep
		rep->assign(n, s);
	else {
		rep->ref--;
		rep = new Srep(n, s);
	}
	return *this;
}
	

lyxstring & lyxstring::assign(value_type const * s)
{
	Assert(s);
	TestlyxstringInvariant(this);

	return assign(s, strlen(s));
}


lyxstring & lyxstring::assign(size_type n, value_type ch)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->assign(n, ch);
	return *this;
}


lyxstring & lyxstring::assign(iterator first, iterator last)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->assign(last - first, first);
	return *this;
}


////////////////////
// Element Access
////////////////////

lyxstring::const_reference lyxstring::operator[](size_type pos) const
{
	Assert(pos < rep->sz);

	return rep->s[pos];
}


lyxstring::reference lyxstring::operator[](size_type pos)
{
	Assert(pos < rep->sz);
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	return rep->s[pos];
}


lyxstring::const_reference lyxstring::at(size_type n) const
{
	Assert(n < rep->sz);
	
	return rep->s[n];
}


lyxstring::reference lyxstring::at(size_type n)
{
	Assert(n < rep->sz);
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	return rep->s[n];
}


/////////////
// Insert
/////////////

lyxstring & lyxstring::operator+=(lyxstring const & x)
{
	TestlyxstringInvariant(this);

	return append(x);
}


lyxstring & lyxstring::operator+=(value_type const * x)
{
	Assert(x);
	TestlyxstringInvariant(this);

	return append(x);
}


lyxstring & lyxstring::operator+=(value_type c)
{
	TestlyxstringInvariant(this);

	push_back(c);
	return *this;
}


void lyxstring::push_back(value_type c)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->push_back(c);
}


lyxstring & lyxstring::append(lyxstring const & x)
{
	TestlyxstringInvariant(this);

	if (x.empty()) return *this;
	rep = rep->get_own_copy();
	rep->append(x.length(), x.rep->s);
	return *this;
}


lyxstring & lyxstring::append(lyxstring const & x, size_type pos, size_type n)
{
	TestlyxstringInvariant(this);

	return append(x.substr(pos, n));
}


lyxstring & lyxstring::append(value_type const * p, size_type n)
{
	Assert(p);
	TestlyxstringInvariant(this);

	if (!*p || !n) return *this;
	rep = rep->get_own_copy();
	rep->append(min(n, strlen(p)), p);
	return *this;
}


lyxstring & lyxstring::append(value_type const * p)
{
	Assert(p);
	TestlyxstringInvariant(this);

	if (!*p) return *this;
	rep = rep->get_own_copy();
	rep->append(strlen(p), p);
	return *this;
}


lyxstring & lyxstring::append(size_type n, value_type c)
{
	TestlyxstringInvariant(this);

	value_type * tmp = new value_type[n];
	memset(tmp, c, n);
	rep = rep->get_own_copy();
	rep->assign(n, tmp);
	delete[] tmp;
	return *this;
}


lyxstring & lyxstring::append(iterator first, iterator last)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->append(last - first, first);
	return *this;
}

// insert value_typeacters before (*this)[pos]

lyxstring & lyxstring::insert(size_type pos, lyxstring const & x)
{
	TestlyxstringInvariant(this);

	return insert(pos, x, 0, x.rep->sz);
}


lyxstring & lyxstring::insert(size_type pos, lyxstring const & x,
			  size_type pos2, size_type n)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->insert(pos, &(x.rep->s[pos2]), min(n, x.rep->sz));
	return *this;
}


lyxstring & lyxstring::insert(size_type pos, value_type const * p, size_type n)
{
	Assert(p);
	TestlyxstringInvariant(this);

	if (*p && n) {
		// insert nothing and you change nothing
		rep = rep->get_own_copy();
		rep->insert(pos, p, min(n, strlen(p)));
	}
	return *this;
}


lyxstring & lyxstring::insert(size_type pos, value_type const * p)
{
	Assert(p);
	TestlyxstringInvariant(this);

	if (*p) {
		// insert nothing and you change nothing
		rep = rep->get_own_copy();
		rep->insert(pos, p, strlen(p));
	}
	return *this;
}


lyxstring & lyxstring::insert(size_type pos, size_type n, value_type c)
{
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	value_type * tmp = new value_type[n];
	memset(tmp, c, n);
	rep->insert(pos, tmp, n);
	delete[] tmp;
	return *this;
}


lyxstring::iterator lyxstring::insert(iterator p, value_type c)
{
	TestlyxstringInvariant(this);

	// what iterator is this supposed to return??
	size_type tmp = p - begin();
	insert(p - begin(), 1, c);
	return begin() + tmp + 1; // ??
}


void lyxstring::insert(iterator p, size_type n , value_type c)
{
	TestlyxstringInvariant(this);

	insert(p - begin(), n , c);
}


void lyxstring::insert(iterator p, iterator first, iterator last)
{
	TestlyxstringInvariant(this);

	insert(p - begin(), first, last - first);
}
	

////////////////
// Find
////////////////
 
         // All the below find functions should be verified,
         // it is very likely that I have mixed up or interpreted
         // some of the parameters wrong, also some of the funcs can surely
         // be written more effectively.

lyxstring::size_type lyxstring::find(lyxstring const & a, size_type i) const
{
	if (!rep->sz) return npos;
	
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; rep->sz - t >= a.length(); ++t) {
		// search until (*this)[i] == a[0]
		if (rep->s[t] == a[0]) {
			// check if the rest of the value_types match
			bool equal = true;
			for (size_type j = 0; j < a.length(); ++j) {
				if (rep->s[t + j] != a[j]) {
					equal = false;
					break;
				}
			}
			if (equal) return t;
		}
	}
	return npos;
}


lyxstring::size_type lyxstring::find(value_type const * ptr, size_type i,
				 size_type n) const
{
	Assert(ptr);
	if (!rep->sz || !*ptr) return npos;
	
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	// What is "n" here? is it the number of value_types to use in ptr
	// or does "i" and "n" togeter form a substring to search
	// for ptr in? For now I will assume that "n" tells the length
	// of ptr. (Lgb)
	n = min(n, strlen(ptr));
	for (size_type t = i; length() - t >= n; ++t) {
		// search until (*this)[i] == a[0]
		if (rep->s[t] == ptr[0]) {
			// check if the rest of the value_types match
			bool equal = true;
			for (size_type j = 0; j < n; ++j) {
				if (rep->s[t + j] != ptr[j]) {
					equal = false;
					break;
				}
			}
			if (equal) return t;
		}
	}
	return npos;
}


lyxstring::size_type lyxstring::find(value_type const * s, size_type i) const
{
	Assert(s);
	if (!rep->sz) return npos;
	
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	if (!s || !*s) return npos;
	return find(s, i, strlen(s));
}


lyxstring::size_type lyxstring::find(value_type c, size_type i) const
{
	if (!rep->sz) return npos;

	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

        for (size_type t = 0; t + i < length(); ++t) {
	        if (rep->s[t + i] == c) return t + i;
	}
        return npos;
}


lyxstring::size_type lyxstring::rfind(lyxstring const & a, size_type i) const
{
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	do {
		if (a[a.length() - 1] == rep->s[ii]) {
			int t = length() - 2;
			size_type l = ii - 1;
			for (; t >= 0; --t, --l) {
				if (a[t] != rep->s[l]) break;
			}
			if (a[t] == rep->s[l]) return l;
		}
	} while(ii-- > 0);
	return npos;
}


lyxstring::size_type lyxstring::rfind(value_type const * ptr, size_type i,
				  size_type n) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);
	if (!*ptr) return npos;

	size_type ii = min(length() - 1, i);
	do {
		if (ptr[n - 1] == rep->s[ii]) {
			int t = n - 2;
			size_type l = ii - 1;
			for (; t >= 0; --t, --l) {
				if (ptr[t] != rep->s[l]) break;
			}
			if (ptr[t] == rep->s[l]) return l;
		}
	} while (ii-- > 0);
	return npos;
}


lyxstring::size_type lyxstring::rfind(value_type const * ptr, size_type i) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);
	if (!*ptr) return npos;

	size_type ii = min(length() - 1, i);
	do {
		if (ptr[strlen(ptr) - 1] == rep->s[ii]) {
			int t = strlen(ptr) - 2;
			size_type l = ii - 1;
			for (; t >= 0; --t, --l) {
				if (ptr[t] != rep->s[l]) break;
			}
			if (ptr[t] == rep->s[l]) return l;
		}
	} while (ii-- > 0);
	return npos;
}


lyxstring::size_type lyxstring::rfind(value_type c, size_type i) const
{
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
        for (size_type t = ii; t != 0; --t) {
	        if (rep->s[t] == c) return t;
	}
        return npos;
}


lyxstring::size_type lyxstring::find_first_of(lyxstring const & a,
					  size_type i) const
{
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; t < length(); ++t) {
		if (a.find(rep->s[t]) != npos) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_of(value_type const * ptr, size_type i,
					  size_type n) const
{
	Assert(ptr && i < rep->sz);
	TestlyxstringInvariant(this);
	if (!n) return npos;

	for (size_type t = i; t < length(); ++t) {
		if(memchr(ptr, rep->s[t], n) != 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_of(value_type const * ptr,
					  size_type i) const
{
	Assert(ptr && i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; t < length(); ++t) {
		if (strchr(ptr, rep->s[t]) != 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_of(value_type c, size_type i) const
{
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; t < length(); ++t) {
		if (rep->s[t] == c) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_of(lyxstring const & a,
					 size_type i) const
{
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (a.find(rep->s[t]) != npos) return t;
	}
	return npos;
}

lyxstring::size_type lyxstring::find_last_of(value_type const * ptr, size_type i,
					 size_type n) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);
	if (!n) return npos;

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if(memchr(ptr, rep->s[t], n) != 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_of(value_type const * ptr,
					 size_type i) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (strchr(ptr, rep->s[t]) != 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_of(value_type c, size_type i) const
{
	TestlyxstringInvariant(this);

	if (!rep->sz) return npos;
	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (rep->s[t] == c) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_not_of(lyxstring const & a,
					      size_type i) const
{
	TestlyxstringInvariant(this);

	if (!rep->sz) return npos;
	Assert(i < rep->sz);
	for (size_type t = i; t < length(); ++t) {
		if (a.find(rep->s[t]) == npos) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_not_of(value_type const * ptr, size_type i,
					      size_type n) const
{
	Assert(ptr && i < rep->sz);
	TestlyxstringInvariant(this);

	if (!n) return (i < length()) ? i : npos;
	for (size_type t = i; t < length(); ++t) {
		if(memchr(ptr, rep->s[t], n) == 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_not_of(value_type const * ptr,
					      size_type i) const
{
	Assert(ptr && i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; t < length(); ++t) {
		if (strchr(ptr, rep->s[t]) == 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_first_not_of(value_type c, size_type i) const
{
	if (!rep->sz) return npos;
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	for (size_type t = i; t < length(); ++t) {
		if (rep->s[t] != c) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_not_of(lyxstring const & a,
					     size_type i) const
{
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (a.find(rep->s[t]) == npos) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_not_of(value_type const * ptr,
						 size_type i,
						 size_type n) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);

	if (!n) return npos;
	size_type ii = min(length() - 1, i);
	//if (!n) return (ii >= 0) ? ii : npos;
	for (int t = ii; t >= 0; --t) {
		if(memchr(ptr, rep->s[t], n) == 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_not_of(value_type const * ptr,
					     size_type i) const
{
	Assert(ptr);
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (strchr(ptr, rep->s[t]) == 0) return t;
	}
	return npos;
}


lyxstring::size_type lyxstring::find_last_not_of(value_type c, size_type i) const
{
	TestlyxstringInvariant(this);

	size_type ii = min(length() - 1, i);
	for (int t = ii; t >= 0; --t) {
		if (rep->s[t] != c) return t;
	}
	return npos;
}


/////////////////
// Replace
/////////////////

lyxstring & lyxstring::replace(size_type i, size_type n, lyxstring const & x)
{
	Assert(i < rep->sz || i == 0);
	TestlyxstringInvariant(this);

	return replace(i, n, x, 0, x.length());
}


lyxstring &  lyxstring::replace(size_type i,size_type n, lyxstring const & x,
			    size_type i2, size_type n2)
{
	Assert((i < rep->sz || i == 0) && (i2 < x.rep->sz || i2 == 0));
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->replace(i, min(n, rep->sz), &(x.rep->s[i2]), min(n2, x.rep->sz));
	return *this;
}


lyxstring & lyxstring::replace(size_type i, size_type n, value_type const * p,
			   size_type n2)
{
	Assert(p && i < rep->sz);
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	rep->replace(i, min(n, rep->sz), p, min(n2, strlen(p)));
	return *this;
}


lyxstring & lyxstring::replace(size_type i, size_type n, value_type const * p)
{
	Assert(p && i < rep->sz);
	TestlyxstringInvariant(this);

	return replace(i, min(n, rep->sz), p, (!p) ? 0 : strlen(p));
}


lyxstring & lyxstring::replace(size_type i, size_type n, size_type n2, value_type c)
{
	Assert(i < rep->sz);
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	value_type * tmp = new value_type[n2];
	memset(tmp, c, n2);
	rep->replace(i, min(n, rep->sz), tmp, n2);
	delete[] tmp;
	return *this;
}


lyxstring & lyxstring::replace(iterator i, iterator i2, const lyxstring & str)
{
	TestlyxstringInvariant(this);

	return replace(i - begin(), i2 - i, str); 
}


lyxstring & lyxstring::replace(iterator i, iterator i2,
			   value_type const * p, size_type n)
{
	Assert(p);
	TestlyxstringInvariant(this);

	return replace(i - begin(), i2 - i, p, n);
}


lyxstring & lyxstring::replace(iterator i, iterator i2, value_type const * p)
{
	Assert(p);
	TestlyxstringInvariant(this);

	return replace(i - begin(), i2 - i, p);
}


lyxstring & lyxstring::replace(iterator i, iterator i2, size_type n , value_type c)
{
	TestlyxstringInvariant(this);

	return replace(i - begin(), i2 - i, n, c);
}
	

lyxstring & lyxstring::replace(iterator i, iterator i2, iterator j, iterator j2)
{
	TestlyxstringInvariant(this);

	return replace(i - begin(), i2 - i, j, j2 - j);
}


lyxstring & lyxstring::erase(size_type i, size_type n)
{
	Assert(i < rep->sz || i == 0);
	TestlyxstringInvariant(this);

	rep = rep->get_own_copy();
	if (i == 0 && n >= rep->sz) {
		rep->sz = 0;
	} else {
		n = min(n, rep->sz - i);
		memmove(&(rep->s[i]), &(rep->s[i + n]), rep->sz - i - n);
		rep->sz -= n;
	}
	return *this;
}


lyxstring::iterator lyxstring::erase(iterator i)
{
	TestlyxstringInvariant(this);

	// what iterator is this supposed to return?
	// the iterator after the one erased
	erase(i - begin(), 1);
	return begin(); // BUG
}


lyxstring::iterator lyxstring::erase(iterator first, iterator last)
{
	TestlyxstringInvariant(this);

	erase(first - begin(), last - first);
	return begin(); // BUG
}


/////////////////////////////////////
// Conversion to C-style Strings
/////////////////////////////////////

lyxstring::value_type const * lyxstring::c_str() const
{
	rep->s[length()] = '\0';
	return rep->s;
}


lyxstring::value_type const * lyxstring::data() const
{
	return rep->s;
}


lyxstring::size_type lyxstring::copy(value_type * buf, size_type len, size_type pos) const
{
	Assert(buf);
	TestlyxstringInvariant(this);

	register int nn = min(len, length() - pos);
	memcpy(buf, &(rep->s[pos]), nn);
	return nn;
}


////////////////////
// Comparisons
////////////////////

// Compare funcs should be verified.
// Should we try to make them work with '\0' value_types?
// An STL string can usually contain '\0' value_types.

int lyxstring::compare(lyxstring const & str) const
{
	TestlyxstringInvariant(this);

	return compare(0, rep->sz, str.rep->s, str.rep->sz);
}


int lyxstring::compare(value_type const * s) const
{
	Assert(s);
	TestlyxstringInvariant(this);

	return compare(0, rep->sz, s, (!s) ? 0 : strlen(s));
}


int lyxstring::compare(size_type pos, size_type n, lyxstring const & str) const
{
	TestlyxstringInvariant(this);

	return compare(pos, n, str.rep->s, str.rep->sz);
}


int lyxstring::compare(size_type pos, size_type n, lyxstring const & str,
		     size_type pos2, size_type n2) const
{
	TestlyxstringInvariant(this);

	return compare(pos, n, str.rep->s + pos2, n2);
}


int lyxstring::compare(size_type pos, size_type n, value_type const * s,
		     size_type n2) const
{
	Assert(s && (pos < rep->sz || pos == 0));
	TestlyxstringInvariant(this);

	if ((rep->sz == 0 || n == 0) && (!*s || n2 == 0)) return 0;
	if (!*s) return 1;
	// since n > n2, min(n,n2) == 0, c == 0 (stops segfault also)

        // remember that n can very well be a lot larger than rep->sz
        // so we have to ensure that n is no larger than rep->sz
        n = min(n, rep->sz);
	n2 = min(n2, strlen(s));
        if (n == n2)
		return memcmp(&(rep->s[pos]), s, n);
	int c = memcmp(&(rep->s[pos]), s, min(n,n2));
	if (c)
		return c;
	if (n < n2)
		return -1;
	return 1;
}


/////////////////
// Substrings
/////////////////

// i = index, n = length
lyxstring lyxstring::substr(size_type i, size_type n) const
{
	Assert(i < rep->sz || i == 0);
	TestlyxstringInvariant(this);

	return lyxstring(*this, i, n);
}



/////////////////////////////////////////////
// String operators, non member functions
/////////////////////////////////////////////

bool operator==(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) == 0;
}


bool operator==(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) == 0;
}


bool operator==(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) == 0;
}


bool operator!=(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) != 0;
}


bool operator!=(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) != 0;
}


bool operator!=(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) != 0;
}


bool operator>(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) > 0;
}


bool operator>(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) < 0; // since we reverse the parameters
}


bool operator>(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) > 0;
}


bool operator<(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) < 0;
}


bool operator<(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) > 0; // since we reverse the parameters
}


bool operator<(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) < 0;
}


bool operator>=(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) >= 0;
}


bool operator>=(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) <= 0; // since we reverse the parameters
}


bool operator>=(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) >= 0;
}


bool operator<=(lyxstring const & a, lyxstring const & b)
{
	return a.compare(b) <= 0;
}


bool operator<=(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	return b.compare(a) >= 0; // since we reverse the parameters
}


bool operator<=(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	return a.compare(b) <= 0;
}


lyxstring operator+(lyxstring const & a, lyxstring const & b)
{
	lyxstring tmp(a);
	tmp += b;
	return tmp;
}


lyxstring operator+(lyxstring::value_type const * a, lyxstring const & b)
{
	Assert(a);
	lyxstring tmp(a);
	tmp += b;
	return tmp;
}


lyxstring operator+(lyxstring::value_type a, lyxstring const & b)
{
	lyxstring tmp;
	tmp += a;
	tmp += b;
	return tmp;
}


lyxstring operator+(lyxstring const & a, lyxstring::value_type const * b)
{
	Assert(b);
	lyxstring tmp(a);
	tmp += b;
	return tmp;
}


lyxstring operator+(lyxstring const & a, lyxstring::value_type b)
{
	lyxstring tmp(a);
	tmp += b;
	return tmp;
}

#include <iostream>

istream & operator>>(istream & is, lyxstring & s)
{
	// very bad solution
	char * nome = new char[1024];
	is >> nome;
	lyxstring tmp(nome);
	delete [] nome;
	if (!tmp.empty()) s = tmp;
	return is;
}

ostream & operator<<(ostream & o, lyxstring const & s)
{
	return o.write(s.data(), s.length());
}

istream & getline(istream & is, lyxstring & s,
		  lyxstring::value_type delim)
{
	// very bad solution
	char tmp;
	s.erase();
	while(is) {
		is >> tmp;
		if (tmp != delim) {
			s += tmp;
		} else {
			break;
		}
	}
	return is;
}
