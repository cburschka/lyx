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
	MathedRowStruct()
		: asc_(0), desc_(0), y_(0), numbered_(true)
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
	int  getTab(unsigned int i) const;
	/// 
	void setLabel(string const & l);
	///
	void setNumbered(bool nf);
	///
	void setTab(unsigned int i, int t);
protected:
	/// verticals 
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


 
class MathedRowContainer {
private:
	///
	struct iterator {
		///
		iterator() : st_(0), pos_(0) {}
		///
		explicit iterator(MathedRowContainer * m) : st_(m), pos_(0) {}
		/// "better" conversion to bool, static_cast doens't help?
		operator void *() const
			{ return (void *)(st_ && pos_ < st_->data_.size()); }
		///
		MathedRowStruct & operator*() { Assert(st_); return st_->data_[pos_]; }
		///
		MathedRowStruct * operator->() { Assert(st_); return &st_->data_[pos_]; }
		///
		MathedRowStruct const * operator->() const { Assert(st_); return &st_->data_[pos_]; }
		///
		void operator++() { Assert(st_); ++pos_; }
		///
		bool is_last() const { Assert(st_); return pos_ == st_->data_.size() - 1; }
		///
		bool operator==(const iterator & it) const
			{ return st_ == it.st_ && pos_ == it.pos_; }

	//private:
		///
		friend class MathedRowContainer;

		/// pointer to the container to which we belong
		MathedRowContainer * st_;
		/// position in this container, e.g. row number
		unsigned int pos_;
	};

public:
	/// 
	iterator begin() { return iterator(this); }
	///
	bool empty() const { return data_.size() == 0; }

	/// insert item before 'it'
	void insert(iterator const & it) {
		Assert(it.st_ == this);
		data_.insert(data_.begin() + it.pos_, MathedRowStruct());
	}
			
	/// erase item pointed to by 'it'
	void erase(iterator & it) {
		Assert(it.st_ == this);
		data_.erase(data_.begin() + it.pos_);
	}

	/// access to last item
	MathedRowStruct & back() {
		Assert(data_.size());
		return data_.back();
	}
	
	/// append empty element
	void push_back() {
		data_.push_back(MathedRowStruct());
	}
	

	///
	std::vector<MathedRowStruct> data_;

private:
	// currently unimplemented just to make sure it's not used
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
int MathedRowStruct::getTab(unsigned int i) const
{
	return i < widths_.size() ? widths_[i] : 0;
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
void MathedRowStruct::setTab(unsigned int i, int t)
{
	if (i >= widths_.size())
		widths_.resize(i + 2);	
	widths_[i] = t;
}
#endif
