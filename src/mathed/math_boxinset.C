#ifdef __GNUG__
#pragma implementation
#endif

#include "math_boxinset.h"
#include "support/LOstream.h"
#include "LColor.h"
#include "debug.h"
#include "Painter.h"
#include "math_cursor.h"
#include "insets/insettext.h"


MathBoxInset::MathBoxInset(string const & name)
	: MathDimInset(), name_(name), text_(new InsetText), buffer_(0)
{}


MathBoxInset::MathBoxInset(MathBoxInset const & m)
	:	MathDimInset(*this), name_(m.name_), text_(0), buffer_(m.buffer_)
{
	if (!m.buffer_)
		lyxerr << "no buffer\n";
	else
		text_ = static_cast<InsetText *>(m.text_->clone(*m.buffer_, false));
}


MathBoxInset::~MathBoxInset()
{
	delete text_;
}


MathInset * MathBoxInset::clone() const
{
	return new MathBoxInset(*this);
}


UpdatableInset * MathBoxInset::asHyperActiveInset() const
{
	return text_;
}


void MathBoxInset::write(MathWriteInfo & os) const
{
	os << "\\" << name_ << "{" << cell(0) << "}";
}


void MathBoxInset::writeNormal(std::ostream & os) const
{
	os << "[mbox ";
	//text_->write(buffer(), os);
	os << "] ";
}


void MathBoxInset::metrics(MathMetricsInfo const & st) const
{
	mi_ = st;
	if (text_ && mi_.view) {
		ascent_  = text_->ascent(mi_.view, mi_.font)  + 2;
		descent_ = text_->descent(mi_.view, mi_.font) + 2;
		width_   = text_->width(mi_.view, mi_.font)   + 4;
	} else {
		ascent_  = 10;
		descent_ = 0;
		width_   = 10;
	}
}


void MathBoxInset::draw(Painter & pain, int x, int y) const
{
	float fx = x + 2;
	if (text_ && mi_.view)
		text_->draw(mi_.view, mi_.font, y, fx, false);
	if (mathcursor && mathcursor->isInside(this))
		pain.rectangle(x, y - ascent(), xcell(0).width(), height(),
			LColor::mathframe);
}


void MathBoxInset::edit(BufferView * bv, int x, int y, unsigned int button)
{
	if (text_)
		text_->edit(bv, x, y, button);
}
