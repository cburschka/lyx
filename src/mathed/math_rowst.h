// -*- C++ -*-
#ifndef MATH_ROWST_H
#define MATH_ROWST_H

#include <vector>

/** The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */
class MathedRowSt
{
public:
	///
	typedef std::vector<int> Widths;
	
	///
	explicit
	MathedRowSt(int n)
		: asc_(0), desc_(0), y_(0), widths_(n + 1, 0),
		  numbered_(true), next_(0)
		{}
	/// Should be const but...
	MathedRowSt * getNext() const;
	/// ...we couldn't use this.
	void setNext(MathedRowSt * n);
	///
	string const & getLabel() const;
	///
	bool isNumbered() const;
	///
	int  getBaseline() const;
	///
	void setBaseline(int b);
	///
	int ascent() const;
	///
	int descent() const;
	///
	void ascent(int a);
	///
	void descent(int d);
	///
	int  getTab(int i) const;
	/// 
	void setLabel(string const & l);
	///
	void setNumbered(bool nf);
	///
	void setTab(int i, int t);
private:
	/// Vericals 
	int asc_;
	///
	int desc_;
	///
	int y_;
	/// widths 
	Widths widths_;
	/// 
	string label_;
	///
	bool numbered_;
	///
	MathedRowSt * next_;
};


inline
MathedRowSt * MathedRowSt::getNext() const
{
	return next_;
}


inline
void MathedRowSt::setNext(MathedRowSt * n)
{
	next_ = n;
}


inline
string const & MathedRowSt::getLabel() const
{
	return label_;
}


inline
bool MathedRowSt::isNumbered() const
{
	return numbered_;
}


inline
int MathedRowSt::getBaseline() const
{
	return y_;
}


inline
void MathedRowSt::setBaseline(int b)
{
	y_ = b;
}


inline
int MathedRowSt::ascent() const
{
	return asc_;
}


inline
int MathedRowSt::descent() const
{
	return desc_;
}


inline
void MathedRowSt::ascent(int a)
{
	asc_ = a;
}


inline
void MathedRowSt::descent(int d)
{
	desc_ = d;
}


inline
int MathedRowSt::getTab(int i) const
{
	return widths_[i];
}


inline
void MathedRowSt::setLabel(string const & l)
{
	label_ = l;
}


inline
void MathedRowSt::setNumbered(bool nf)
{
	numbered_ = nf;
}


inline
void MathedRowSt::setTab(int i, int t)
{
	widths_[i] = t;
}
#endif
