// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 1996 Matthias Ettrich
 *           and the LyX Team.
 *
 *======================================================*/

// This one is heavily based on the string class in The C++
// Programming Language by Bjarne Stroustrup

#ifndef _LSTRING_H_
#define _LSTRING_H_ 

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h> // needed at least for compiler that do not
		    // understand 'explicit' (JMarc)
#ifdef _AIX
// AIX has strange ideas about where definitions like strcasecmp
// should go (JMarc)
#include <strings.h>
#else
#include <string.h>
#endif

/** A string class for LyX
  
  This is a permanent String class. It was supposed to be superseeded
  with QString when we switch to the Qt library, but now it's so developed
  that it is as good as theirs (or maybe even better :-)

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

  When you use the #.copy()# method, LString calls "#new []#", so you have to
  release the memory with #delete[]#. Don't preallocate memory.

  When you want to copy an LString, just do
  
	#LString a, b = "String";#
	#a = b;	// That's it!#

  not something like
  
  	#LString a, b = "String";#
  	#a = b.copy(); // This leaks.#
  
  The class automatically handles deep copying when required.
*/
class LString {
public:
	/**@name Constructors and Deconstructors */
	//@{
	/// #LString x;#
	LString();
	/// #LString x(LString ...)#
	LString(LString const &);
	/// #LString x("abc")#
	LString(char const*);
	/// #LString x('a')# 
	explicit LString(char const);
	// not all C++ compilers understands explicit as of now

	///
	~LString();
	//@}

	/**@name Operators */
	//@{
	///
	LString& operator=(LString const &);
	///
	LString& operator=(char const *);
	///
	LString& operator=(char);

	///
	char& operator[](int);
#ifndef const
	///
	char const& operator[](int) const;
#endif
	///
	LString& operator+=(LString const &);
	///
	LString& operator+=(char const*);
	///
	LString& operator+=(char);
	///
	LString& operator+=(int);
	///
	LString& operator+=(long);
	//@}

	/**@name Methods */
	//@{
	///  to comply with the STL strings
	bool empty() const;
	///
	//bool isEmpty() const;
	///
	//bool notEmpty() const;
	///
	int length() const;

	// should be renamed to erase() (STL)
        /** equivalent to  *this = empty. But usable outside LString
            implementation */
	LString &clean();

	/// This should be renamed to c_str()
	char const* c_str() const;

	// we should make the caller supply the storage to copy into. (STL)
	/** This one returns a verbatim copy. Only temporary.
	  The caller should call delete [] when done with the string
	  */
	char * copy() const;

	///
	bool contains(char const *) const;

	// Remove and replace (STL)
	/// Truncate to substring. I.e. #"abcdef".substring(2,4)="cde"#
	LString& substring(int i1, int i2);
	    
	/** Splits the string by the first delim.
	  Splits the string by the first appearance of delim.
	    The leading string up to delim is returned in piece (not including
	    delim), while the original string is cut from after the delimiter.
	    Example:
	    #s1=""; s2="a;bc".split(s1, ';') -> s1=="a"; s2 == "bc";#
	*/
	LString& split(LString& piece, char delim);
	/// Same as split but does not return a piece
	LString& split(char delim);
        /// Same as split but uses the last delim.
	LString& rsplit(LString& piece, char delim);
	
	/** Extracts a token from this string at the nth delim.
	    Doesn't modify the original string. Similar to strtok.
	    Example:
	    #"a;bc;d".token(';', 1) == "bc";#
	    #"a;bc;d".token(';', 2) == "d";#
	*/
	LString token(char delim, int n=0) const;

	/** Search a token in this string using the delim.
	    Doesn't modify the original string. Returns -1 in case of
	    failure. 
	    Example:
	    #"a;bc;d".tokenPos(';', "bc") == 1;#
	    #"a;bc;d".token(';', "d") == 2;#
	*/
	int tokenPos(char delim, LString const &tok);

	/** Strips characters off the end of a string.
	    #"abccc".strip('c') = "ab".#
	*/
	LString& strip(char const c = ' ');

	/** Strips characters of the beginning of a string.
	  #"cccba".frontstrip('c') = "ba"#. */
	LString& frontStrip(char const c = ' ');
	
	/// Does the string start with this prefix?
	bool prefixIs(char const *) const;

	/// Does the string end with this char?
	bool suffixIs(char) const;

	/// Does the string end with this suffix?
	bool suffixIs(char const *) const;
	
	/// Substitute all "oldchar"s with "newchar"
	LString& subst(char oldchar, char newchar);

	/// Substitutes all instances of oldstr with newstr
	LString& subst(char const * oldstr, LString const & newstr);
 
	/** Compares a string and a (simple) regular expression
	  The only element allowed is "*" for any string of characters
	  */
	bool regexMatch(LString const & pattern) const;

	/// Lowercases a string
	LString& lowercase();

	/// Counts how many of character c there is in string
	int countChar(const char c) const;

	/// Position of the character c from the beggining
	int charPos(const char c) const;
	    
	//@}

	/**@name Friends */
	//@{
	
	///
	friend bool operator==(LString const &x, char const *s)
	{
		if (s == 0 || !(*s)) 
			return x.p->s[0] == '\0';
		else
			return strcmp(x.p->s, s) == 0;
	}
	
	///
	friend bool operator==(LString const &x, LString const &y)
	{
		return strcmp(x.p->s, y.p->s) == 0;
	}
	
	///
	friend bool operator!=(LString const &x, char const *s)
	{
		if (s == 0 || !(*s)) 
			return x.p->s[0] != '\0';
		else
			return strcmp(x.p->s, s) != 0;
	}
	
	///
	friend bool operator!=(LString const &x, LString const &y)
	{
		return strcmp(x.p->s, y.p->s) != 0;
	}
	//@}

protected:
private:
	/// A string representation
	struct srep {
		/// Length
		unsigned int l;
		/// Reference count (number of references - 1)
		unsigned short n;
		/// Extra space at end of allocated string
		unsigned short e;
		/// Data. At least 1 char for trailing null.
		char s[1];
		///
		srep() { n = 0; l = 0; e = 0; s[0] = '\0'; }
	};

	/** The empty srep is a local static in each function that
	    needs one. There is no "global" empty srep but LString
	    doesn't need one (no code actually relies upon a single
	    empty srep).
	    This overcomes *all* "static initialization" problems,
	    at maximum speed, with a small overhead of 6 local static
	    empty_reps.
	 */
// This is the only other option (or a variant of this anyway)
// I originally offered this style of empty_rep in 9709 but
// it was rejected for requiring too many function calls and pointer operations(!)
// and because another implementation was to be trialed (which has since
// apparently failed for some compilers). ARRae
//	static srep& empty_rep()
//		{
//			static srep mt;
//			return mt;
//		}

	/// A string is a pointer to it's representation
	srep *p;
	///
        void lose();
};


inline LString::LString(LString const &x)
{
	x.p->n++;
	p = x.p;
}


inline void LString::lose()
{
	if (p->n-- == 0) {
		delete[] (char*)p;
	}
}

inline LString::~LString()
{
   lose();
}


inline int LString::length() const
{
	return p->l;
}


inline bool LString::empty() const
{
	return p->l == 0;
}


inline int LString::countChar(const char c) const
{
	int n = 0;
	for (int i=0; i < length(); i++)
		if (operator[](i) == c) n++;
	return n;
}


inline LString operator+(LString const &x, LString const &y)
{
	LString str(x);
	str += y;
	return str;
}


inline LString operator+(LString const &x, char const &y)
{
	LString str(x);
	str += y;
	return str;
}


inline LString operator+(LString const &x, int const &y)
{
	LString str(x);
	str += y;
	return str;
}


inline LString operator+(LString const &x, long const &y)
{
	LString str(x);
	str += y;
	return str;
}

inline char const* LString::c_str() const
{
	return (char const*)p->s;
}


inline int LString::charPos(const char c) const
{
        for (int i=0; i < length(); i++) {
	        if (operator[](i) == c) return i;
	}
        return -1;
}
	 
#endif
