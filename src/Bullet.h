// -*- C++ -*-
/* This is the bullet class definition file.
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 1997-1999
 *           Allan Rae
 * ====================================================== */

#ifndef BULLET_H
#define BULLET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "support/LAssert.h"

///
class Bullet {
public:
	///
	Bullet(int f = -1, int c = -1, int s = -1);

	///
	explicit Bullet(string const &);

	///
	void setCharacter(int);
	///
	void setFont(int);
	///
	void setSize(int);
	///
	void setText(string const &);
	///
	int getCharacter() const;
	///
	int getFont() const;
	///
	int getSize() const;
	///
	string const & getText() const;
	///
	Bullet & operator=(Bullet const &);
	///
	friend bool operator==(Bullet const &, Bullet const &);
protected:
#ifdef ENABLE_ASSERTIONS
	///
	void testInvariant() const {
		Assert(font >= MIN);
		Assert(font < FONTMAX);
		Assert(character >= MIN);
		Assert(character < CHARMAX);
		Assert(size >= MIN);
		Assert(size < SIZEMAX);
		Assert(user_text >= -1);
		Assert(user_text <= 1);
		// now some relational/operational tests
		if (user_text == 1) {
			Assert(font == -1 && (character == -1 && size == -1));
		       	//        Assert(!text.empty()); // this isn't necessarily an error
		}
		//      else if (user_text == -1) {
		//        Assert(!text.empty()); // this also isn't necessarily an error
		//      }
		//      else {
		//        // user_text == 0
		//        Assert(text.empty()); // not usually true
		//      }
	}
#endif
private:
	/**
	   This enum makes adding additional panels or changing panel sizes
	   easier. Since you only need change these values for all tests to
	   be correct for the new values.
	   
	   Note: MAX means the size of the array so to test you need:
	   (x < MAX)  *not* (x <= MAX)
	*/
	enum {
		///
		MIN = -1,
		///
		FONTMAX = 6,
		///
		CHARMAX = 36,
		///
		SIZEMAX = 10
	};
	
	///
	void generateText() const;
	///
	static string const bulletSize(short int);
	///
	static string const bulletEntry(short int, short int);
	
	///
	int font;
	///
	int character;
	///
	int size;
	
	// size, character and font are array indices to access 
	// the predefined arrays of LaTeX equivalent strings.
	
	/** flag indicates if user has control of text (1)
	    or if I can use it to generate strings (0)
	    or have already (-1)
	*/
	mutable short user_text; 
	
	//NOTE: Arranging these four shorts above to be together
	//      like this should ensure they are in a single cache line
	
	/** text may contain a user-defined LaTeX symbol command
	    or one generated internally from the font, character
	    and size settings.
	*/
	mutable string text;
};


/*----------------Inline Bullet Member Functions------------------*/

inline
Bullet::Bullet(string const & t) 
	:  font(MIN), character(MIN), size(MIN), user_text(1), text(t)
{
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline
void Bullet::setCharacter(int c)
{
	if (c < MIN || c >= CHARMAX) {
		character = MIN;
	} else {
		character = c;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline
void Bullet::setFont(int f)
{
	if (f < MIN || f >= FONTMAX) {
		font = MIN;
	} else {
		font = f;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline
void Bullet::setSize(int s)
{
	if (s < MIN || s >= SIZEMAX) {
		size = MIN;
	} else {
		size = s;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline
void Bullet::setText(string const & t)
{
	font = character = size = MIN;
	user_text = 1;
	text = t;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline
int Bullet::getCharacter() const
{
	return character;
}


inline
int Bullet::getFont() const
{
	return font;
}


inline
int Bullet::getSize() const
{
	return size;
}


inline
Bullet & Bullet::operator=(Bullet const & b)
{
#ifdef ENABLE_ASSERTIONS
	b.testInvariant();
#endif
	font = b.font;
	character = b.character;
	size = b.size;
	user_text = b.user_text;
	text = b.text;
#ifdef ENABLE_ASSERTIONS
	this->testInvariant();
#endif
	return *this;
}

/*-----------------End Bullet Member Functions-----------------*/

inline
bool operator!=(Bullet const & b1, Bullet const & b2)
{
	return !(b1 == b2);
}

///
extern
Bullet const ITEMIZE_DEFAULTS[];

#endif /* BULLET_H_ */
