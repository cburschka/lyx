#include <config.h>

#include "math_rowst.h"
#include "support/LAssert.h"


//
// MathedRowContainer
//

MathedRowStruct::MathedRowStruct()
	: asc_(0), desc_(0), y_(0), numbered_(true)
{}

string const & MathedRowStruct::getLabel() const
{
	return label_;
}

bool MathedRowStruct::isNumbered() const
{
	return numbered_;
}

int MathedRowStruct::getBaseline() const
{
	return y_;
}

void MathedRowStruct::setBaseline(int b)
{
	y_ = b;
}

int MathedRowStruct::ascent() const
{
	return asc_;
}

int MathedRowStruct::descent() const
{
	return desc_;
}

void MathedRowStruct::ascent(int a)
{
	asc_ = a;
}

void MathedRowStruct::descent(int d)
{
	desc_ = d;
}

int MathedRowStruct::getTab(unsigned int i) const
{
	return i < widths_.size() ? widths_[i] : 0;
}

void MathedRowStruct::setLabel(string const & l)
{
	label_ = l;
}

void MathedRowStruct::setNumbered(bool nf)
{
	numbered_ = nf;
}

void MathedRowStruct::setTab(unsigned int i, int t)
{
	if (i >= widths_.size())
		widths_.resize(i + 2);	
	widths_[i] = t;
}



//
// MathedRowContainer
//


MathedRowContainer::iterator MathedRowContainer::begin()
{
	return iterator(this);
}

MathedRowContainer::iterator MathedRowContainer::end()
{
	iterator it(this);
	it.pos_ = data_.size();
	return it;
}

bool MathedRowContainer::empty() const
{
	return data_.size() == 0;
}

void MathedRowContainer::insert(iterator const & it)
{
	lyx::Assert(it.st_ == this);
	data_.insert(data_.begin() + it.pos_, MathedRowStruct());
}
		
void MathedRowContainer::erase(iterator & it)
{
	lyx::Assert(it.st_ == this);
	data_.erase(data_.begin() + it.pos_);
}

MathedRowStruct & MathedRowContainer::back()
{
	lyx::Assert(data_.size());
	return data_.back();
}

void MathedRowContainer::push_back()
{
	data_.push_back(MathedRowStruct());
}


MathedRowContainer::size_type MathedRowContainer::size() const
{
	return data_.size();
}



//
// MathedRowContainer::iterator
//

MathedRowContainer::iterator::iterator()
	: st_(0), pos_(0)
{}

MathedRowContainer::iterator::iterator(MathedRowContainer * m)
	: st_(m), pos_(0)
{}

MathedRowContainer::iterator::operator void *() const
{
	return (void *)(st_ && pos_ < st_->size());
}

MathedRowStruct * MathedRowContainer::iterator::operator->()
{
	lyx::Assert(st_);
	return &st_->data_[pos_];
}

MathedRowStruct const * MathedRowContainer::iterator::operator->() const
{
	lyx::Assert(st_);
	return &st_->data_[pos_];
}

void MathedRowContainer::iterator::operator++()
{
	lyx::Assert(st_);
	++pos_;
}

bool MathedRowContainer::iterator::is_last() const
{
	lyx::Assert(st_);
	return pos_ == st_->size() - 1;
}

bool MathedRowContainer::iterator::operator==(const iterator & it) const
{
	return st_ == it.st_ && pos_ == it.pos_;
}


