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
 * ======================================================*/

#ifndef BULLET_H
#define BULLET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#ifdef DEBUG_AS_DEFAULT
#include <cassert>
#endif

///
class Bullet {
public:
	///
	Bullet(const int f = -1, const int c = -1, const int s = -1);

	///
	Bullet(const string &);

	///
	~Bullet();

	///
	void setCharacter(const int);
	///
	void setFont(const int);
	///
	void setSize(const int);
	///
	void setText(const string &);
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
#ifdef DEBUG_AS_DEFAULT
	void testInvariant() const
		{
			assert(font >= MIN);
			assert(font < FONTMAX);
			assert(character >= MIN);
			assert(character < CHARMAX);
			assert(size >= MIN);
			assert(size < SIZEMAX);
			assert(user_text >= -1);
			assert(user_text <= 1);
			// now some relational/operational tests
			if (user_text == 1) {
				assert(font == -1 && (character == -1 && size == -1));
				//        assert(!text.empty()); // this isn't necessarily an error
			}
			//      else if (user_text == -1) {
			//        assert(!text.empty()); // this also isn't necessarily an error
			//      }
			//      else {
			//        // user_text == 0
			//        assert(text.empty()); // not usually true
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
	static const string & bulletSize(const short &);
	///
	static const string & bulletEntry(const short &, const short &);

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

inline Bullet::Bullet(const string & t) 
  :  font(MIN), character(MIN), size(MIN), user_text(1), text(t)
{
#ifdef DEBUG_AS_DEFAULT
	testInvariant();
#endif
}


inline Bullet::~Bullet()
{}


inline void Bullet::setCharacter(const int c)
{
	if (c < MIN || c >= CHARMAX) {
		character = MIN;
	}
	else {
		character = c;
	}
	user_text = 0;
#ifdef DEBUG_AS_DEFAULT
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
#ifdef DEBUG_AS_DEFAULT
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
#ifdef DEBUG_AS_DEFAULT
	testInvariant();
#endif
}


inline void Bullet::setText(const string & t)
{
	font = character = size = MIN;
	user_text = 1;
	text = t;
#ifdef DEBUG_AS_DEFAULT
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
#ifdef DEBUG_AS_DEFAULT
	b.testInvariant();
#endif
	font = b.font;
	character = b.character;
	size = b.size;
	user_text = b.user_text;
	text = b.text;
#ifdef DEBUG_AS_DEFAULT
    this->testInvariant();
#endif
	return *this;
}


inline const char * Bullet::c_str()
{
	return this->getText().c_str();
}


/*-----------------End Bullet Member Functions-----------------*/

/** The four LaTeX itemize environment default bullets
  */
static Bullet const ITEMIZE_DEFAULTS[4] = { Bullet( 0, 8 ),//"\\(\\bullet\\)"
                                            Bullet( 0, 0 ),//"\\normalfont\\bfseries{--}"
                                            Bullet( 0, 6 ),//"\\(\\ast\\)"
                                            Bullet( 0,10 ) };//"\\(\\cdot\\)"
#endif /* BULLET_H_ */
