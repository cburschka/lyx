// -*- C++ -*-
/*
 *  File:        array.h
 *  Purpose:     A general purpose resizable array.  
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *
 *  Dependencies: None (almost)
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra
 *                 1997  The LyX Team!
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <string.h>

#ifndef byte
#define byte unsigned char
#endif

/*@Doc: A resizable array
  Why is it called "LyXArrayBase" if it is generic? (Lgb)
  Initially I thought it could be the base class for both mathed's
  and LyX' kernels data buffer. (Ale)
 
  */
class LyxArrayBase  {
public:
	///
	enum {
		///
		ARRAY_SIZE = 256,
		///
		ARRAY_STEP = 16,
		///
		ARRAY_MIN_SIZE = 4
	};

	///
	LyxArrayBase(int size= ARRAY_STEP);
	///
	LyxArrayBase(const LyxArrayBase&);
	///
	~LyxArrayBase();
   
	
	///
	int Empty() { return (last == 0); }
   
	///
	int Last() { return last; }
   
	/// Fills with 0 the entire array and set last to 0
	void Init();     
   
	/// Make the allocated memory fit the needed size
	void Fit();     

	/// Remove dx elements from position pos. Don't changes the size
	void Remove(int pos, int dx);   

	/// Merge dx elements from array a at pos. Changes the size if necessary.
	void Merge(LyxArrayBase *a, int pos, int dx); 

	/// Same as Merge but doesn't changes the size (dangerous)
	void MergeF(LyxArrayBase *a, int pos, int dx); 

	/// Copy dx byts from an array at position pos
	void Copy(void *, int pos, int dx); 

	/// Constructs a new array with dx elements starting at pos 
	LyxArrayBase* Extract(int pos, int dx); 

	/// Insert a character at position pos
	void Insert(int pos, byte);

	/// Insert a string of lenght dx at position pos
	void Insert(int pos, byte *, int dx);

	/// Constructs a new array with dx elements starting at pos 
	byte operator[](const int);

	/// Constructs a new array with dx elements starting at pos 
	LyxArrayBase& operator= (const LyxArrayBase&); 

protected:
	///
	void Resize(int newsize);
	///
	bool Move(int p, int shift);

	/// Buffer
	byte *bf;
	/// Last position inserted.
	int last;
	/// Max size of the array.
	int maxsize;
private:
	///
	friend class MathedIter;
};
   


/************************ Inline functions *****************************/

inline
void LyxArrayBase::Init()
{
	memset(bf, 0, maxsize);
	last = 0;
}

inline // Hmmm, Hp-UX's CC can't handle this inline. Asger.
void LyxArrayBase::Resize(int newsize)
{
	if (newsize<ARRAY_MIN_SIZE)
		newsize = ARRAY_MIN_SIZE;
	newsize += ARRAY_STEP - (newsize % ARRAY_STEP);
	byte *nwbf = new byte[newsize];
	if (last >= newsize) last = newsize-1;
	maxsize = newsize;
	memcpy(nwbf, bf, last);
	delete[] bf;
	bf = nwbf;
	bf[last] = 0;
}

inline
LyxArrayBase::LyxArrayBase(int size) 
{
	maxsize = (size<ARRAY_MIN_SIZE) ? ARRAY_MIN_SIZE: size;
	bf = new byte[maxsize]; // this leaks
	Init();
}

inline   
LyxArrayBase::~LyxArrayBase() 
{
	delete[] bf;
}

inline
LyxArrayBase::LyxArrayBase(const LyxArrayBase& a) 
{
	maxsize = a.maxsize;
	bf = new byte[maxsize];
	memcpy(&bf[0], &a.bf[0], maxsize);
	last = a.last;
}

inline
LyxArrayBase& LyxArrayBase::operator= (const LyxArrayBase& a)
{
	if (this != &a) {
		Resize(a.maxsize);
		memcpy(&bf[0], &a.bf[0], maxsize);
	}
	return *this;
}

inline   
bool LyxArrayBase::Move(int p, int shift) 
{
	bool result = false;
	if (p<= last) {
		if (last+shift>= maxsize) { 
		    Resize(last + shift);
		}
		memmove(&bf[p+shift], &bf[p], last-p);
		last += shift;
		bf[last] = 0;
		result = true;
	}
	return result;
}

inline
void LyxArrayBase::Fit()
{
	Resize(last);
}

inline
void LyxArrayBase::Remove(int pos, int dx)
{
	Move(pos+dx, -dx);
}    

inline
void LyxArrayBase::Merge(LyxArrayBase *a, int p, int dx)
{
	Move(p, dx);
	memcpy(&bf[p], &a->bf[0], dx);
}
 
inline
void LyxArrayBase::MergeF(LyxArrayBase *a, int p, int dx)
{
	memcpy(&bf[p], &a->bf[0], dx);
}
 
inline
void LyxArrayBase::Copy(void *a, int p, int dx)
{
	memcpy(&bf[p], a, dx);
}

inline
LyxArrayBase *LyxArrayBase::Extract(int, int dx)
{
	LyxArrayBase *a = new LyxArrayBase(dx);
	a->Merge(this, 0, dx);
	return a;
}
 
inline
byte LyxArrayBase::operator[](const int i)
{
	return bf[i];
}


inline
void LyxArrayBase::Insert(int pos, byte c)
{
	if (pos<0) pos = last;
	if (pos>= maxsize) 
		Resize(maxsize+ARRAY_STEP);
	bf[pos] = c;
	if (pos>= last)
		last = pos+1;
}
