#ifndef DIMENSION_H
#define DIMENSION_H

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
	/// get height
	int height() const { return a + d; }
	/// get ascent
	int ascent() const { return a; }
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

#endif
