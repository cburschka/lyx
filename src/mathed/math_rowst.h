// -*- C++ -*-
#ifndef MATH_ROWST_H
#define MATH_ROWST_H

#include <vector>
#include "support/LAssert.h"

/** The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */

class MathedRowStruct
{
public:
	///
	typedef std::vector<int> Widths;
	
	///
	explicit
	MathedRowStruct(int n)
		: asc_(0), desc_(0), y_(0), widths_(n + 1, 0),
		  numbered_(true)
		{}
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
	///
	friend class MathedRowSt;
protected:
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
};


class MathedRowContainer;

class MathedRowSt : public MathedRowStruct {
public:
	///
	explicit MathedRowSt(int n)
			: MathedRowStruct(n), next_(0)
		{}
//private:
	///
	MathedRowSt * next_;
	///
	friend class MathedRowContainer;
};


// The idea is to change this  MathedRowContainer  to mimic the behaviour
// of std::list<MathedRowStruct> in several small steps.  In the end it
// could be replaced by such a list and MathedRowSt can go as well. 
 
struct MathedRowContainer {
	///
	struct iterator {
		///
		iterator() : st_(0) {}
		///
		explicit iterator(MathedRowSt * st) : st_(st) {}
		///
		explicit iterator(MathedRowContainer * m) : st_(m->data_) {}
		/// "better" conversion to bool
		operator void *() const { return st_; }
		///
		MathedRowStruct & operator*() { Assert(st_); return *st_; }
		///
		MathedRowStruct * operator->() { return st_; }
		///
		MathedRowStruct const * operator->() const { return st_; }
		///
		void operator++() { Assert(st_); st_ = st_->next_; }
		///
		bool is_last() const { Assert(st_); return st_->next_ == 0; }
		///
		bool operator==(const iterator & it) const { return st_ == it.st_; }

	//private:
		///
		MathedRowSt * st_;
	};

	///
	MathedRowContainer() : data_(0) {}

	///
	iterator begin() { return iterator(this); }
	///
	bool empty() const { return data_ == 0; }

	/// insert 'item' before 'iterator'
	void insert(iterator const & it, MathedRowSt const & item) {
		MathedRowSt * r = new MathedRowSt(item);
		if (data_ == it.st_)
			data_ = r;
		else {
			MathedRowSt * pos = data_;
			if (pos->next_ == it.st_)
				pos->next_ = r;
		}
		r->next_  = it.st_;
	}
			
	/// insert 'item' after 'iterator'
	void insert_after(iterator & it, MathedRowSt const & item) {
		MathedRowSt * r = new MathedRowSt(item);
		if (it) {
			r->next_ = it.st_->next_;
			it.st_->next_ = r;
		} else {
			it.st_ = r;
			r->next_ = 0;
		}
	}

	///
	MathedRowSt * data_;

private:
	// currently unimplemented just to make sure it's not used
	MathedRowContainer(MathedRowContainer const &); // unimplemented
	void operator=(MathedRowContainer const &); // unimplemented
};



inline
string const & MathedRowStruct::getLabel() const
{
	return label_;
}


inline
bool MathedRowStruct::isNumbered() const
{
	return numbered_;
}


inline
int MathedRowStruct::getBaseline() const
{
	return y_;
}


inline
void MathedRowStruct::setBaseline(int b)
{
	y_ = b;
}


inline
int MathedRowStruct::ascent() const
{
	return asc_;
}


inline
int MathedRowStruct::descent() const
{
	return desc_;
}


inline
void MathedRowStruct::ascent(int a)
{
	asc_ = a;
}


inline
void MathedRowStruct::descent(int d)
{
	desc_ = d;
}


inline
int MathedRowStruct::getTab(int i) const
{
	return widths_[i];
}


inline
void MathedRowStruct::setLabel(string const & l)
{
	label_ = l;
}


inline
void MathedRowStruct::setNumbered(bool nf)
{
	numbered_ = nf;
}


inline
void MathedRowStruct::setTab(int i, int t)
{
	widths_[i] = t;
}
#endif
