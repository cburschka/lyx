// -*- C++ -*-
#ifndef MATH_ROWST_H
#define MATH_ROWST_H

#include <vector>

/** The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */
struct MathedRowSt
{
	///
	typedef std::vector<int> Widths;
	
	///
	explicit
	MathedRowSt(int n)
		: asc_(0), desc_(0), y_(0), widths_(n + 1, 0),
		  numbered_(true), next_(0)
		{}
	/// Should be const but...
	MathedRowSt * getNext() const  { return next_; }
	/// ...we couldn't use this.
	void setNext(MathedRowSt * n) { next_ = n; }
	///
	string const & getLabel() const { return label_; }
	///
	bool isNumbered() const { return numbered_; }
	///
	int  getBaseline() const { return y_; }
	///
	void setBaseline(int b) { y_ = b; }
	///
	int ascent() const { return asc_; }
	///
	int descent() const { return desc_; }
	///
	void ascent(int a) { asc_ = a; }
	///
	void descent(int d) { desc_ = d; }
	///
	int  getTab(int i) const { return widths_[i]; }
	/// 
	void setLabel(string const & l) { label_ = l; }
	///
	void setNumbered(bool nf) { numbered_ = nf; }
	///
	void setTab(int i, int t) { widths_[i] = t; }
private:
	/// Vericals 
	int asc_;
	int desc_;
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
#endif
