// -*- C++ -*-
/* This is the bullet class definition file.
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
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
	Bullet(const int f = -1, const int c = -1, const int s = -1);

	///
	Bullet(string const &);

	///
	void setCharacter(const int);
	///
	void setFont(const int);
	///
	void setSize(const int);
	///
	void setText(string const &);
	///
	int getCharacter() const;
	///
	int getFont() const;
	///
	int getSize() const;
	///
	string getText() const;
	///
	string getText();
	///
	char const * c_str();
	///
	Bullet & operator = (const Bullet &);
	///
	friend bool operator == (const Bullet &, const Bullet &);
	///
	friend bool operator != (const Bullet & b1, const Bullet & b2)
		{
			return !(b1 == b2);
		}
	

protected:
#ifdef ENABLE_ASSERTIONS
	void testInvariant() const
		{
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
	void generateText();
	///
	static string bulletSize(const short &);
	///
	static string bulletEntry(const short &, const short &);

	///
	short font;
	///
	short character;
	///
	short size;
	
	// size, character and font are array indices to access 
	// the predefined arrays of LaTeX equivalent strings.

	/** flag indicates if user has control of text (1)
	  or if I can use it to generate strings (0)
	  or have already (-1)
	  */
	short user_text; 

	//NOTE: Arranging these four shorts above to be together
	//      like this should ensure they are in a single cache line
 
	/** text may contain a user-defined LaTeX symbol command
	  or one generated internally from the font, character
	  and size settings.
	  */
	string text;
};


/*----------------Inline Bullet Member Functions------------------*/

inline Bullet::Bullet(string const & t) 
  :  font(MIN), character(MIN), size(MIN), user_text(1), text(t)
{
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline void Bullet::setCharacter(const int c)
{
	if (c < MIN || c >= CHARMAX) {
		character = MIN;
	}
	else {
		character = c;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline void Bullet::setFont(const int f)
{
	if (f < MIN || f >= FONTMAX) {
		font = MIN;
	}
	else {
		font = f;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline void Bullet::setSize(const int s)
{
	if (s < MIN || s >= SIZEMAX) {
		size = MIN;
	}
	else {
		size = s;
	}
	user_text = 0;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline void Bullet::setText(string const & t)
{
	font = character = size = MIN;
	user_text = 1;
	text = t;
#ifdef ENABLE_ASSERTIONS
	testInvariant();
#endif
}


inline int Bullet::getCharacter() const
{
	return character;
}


inline int Bullet::getFont() const
{
	return font;
}


inline int Bullet::getSize() const
{
	return size;
}


inline string Bullet::getText() const
{
	return text;
}


inline Bullet & Bullet::operator = (const Bullet & b)
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


inline char const * Bullet::c_str()
{
	return this->getText().c_str();
}


/*-----------------End Bullet Member Functions-----------------*/

/** The four LaTeX itemize environment default bullets
  */
static Bullet const ITEMIZE_DEFAULTS[4] = { Bullet( 0, 8 ),//"\\(\\bullet\\)"
                                            Bullet( 0, 0 ),//"\\normalfont\\bfseries{--}"
                                            Bullet( 0, 6 ),//"\\(\\ast\\)"
                                            Bullet( 0, 10 ) };//"\\(\\cdot\\)"
#endif /* BULLET_H_ */
