#ifndef DIMENSION_H
#define DIMENSION_H

#include <iosfwd>

class LyXFont;

class Dimension {
public:
	/// constructor
	Dimension() : w(0), a(0), d(0) {}
	/// initialize data
	Dimension(int ww, int aa, int dd) : w(ww), a(aa), d(dd) {}

	/// glue horizontally
	void operator+=(Dimension const & dim);
	/// set to empty box
	void clear() { w = a = d = 0; }
	/// set to empty box suitble for given font
	void clear(LyXFont const & font);
	/// get height
	int height() const { return a + d; }
	/// get ascent
	int ascent() const { return a; }
	/// get descent
	int descent() const { return d; }
	/// get width
	int width() const { return w; }

public:	
	/// width
	int w;
	/// ascent
	int a;
	/// descent
	int d;
};

std::ostream & operator<<(std::ostream & os, Dimension const & dim);

#endif
