/**
 * \file InsetMathHull.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathArray.h"
#include "InsetMathChar.h"
#include "InsetMathColor.h"
#include "MathData.h"
#include "InsetMathDelim.h"
#include "MathExtern.h"
#include "MathFactory.h"
#include "InsetMathHull.h"
#include "MathStream.h"
#include "MathParser.h"
#include "InsetMathSpace.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "InsetMathRef.h"

#include "bufferview_funcs.h"
#include "Text.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "FuncStatus.h"
#include "Color.h"
#include "LaTeXFeatures.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "sgml.h"
#include "TextPainter.h"
#include "Undo.h"

#include "insets/RenderPreview.h"
#include "insets/InsetLabel.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"

#include "frontends/Painter.h"

#include "support/lyxlib.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <sstream>


namespace lyx {

using cap::grabAndEraseSelection;
using support::bformat;
using support::subst;

using std::endl;
using std::max;
using std::ostream;
using std::auto_ptr;
using std::istringstream;
using std::ostringstream;
using std::pair;
using std::swap;
using std::vector;


namespace {

	int getCols(HullType type)
	{
		switch (type) {
			case hullEqnArray:
				return 3;
			case hullAlign:
			case hullFlAlign:
			case hullAlignAt:
			case hullXAlignAt:
			case hullXXAlignAt:
				return 2;
			default:
				return 1;
		}
	}


	// returns position of first relation operator in the array
	// used for "intelligent splitting"
	size_t firstRelOp(MathData const & ar)
	{
		for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
			if ((*it)->isRelOp())
				return it - ar.begin();
		return ar.size();
	}


	char const * star(bool numbered)
	{
		return numbered ? "" : "*";
	}


} // end anon namespace


HullType hullType(docstring const & s)
{
	if (s == "none")      return hullNone;
	if (s == "simple")    return hullSimple;
	if (s == "equation")  return hullEquation;
	if (s == "eqnarray")  return hullEqnArray;
	if (s == "align")     return hullAlign;
	if (s == "alignat")   return hullAlignAt;
	if (s == "xalignat")  return hullXAlignAt;
	if (s == "xxalignat") return hullXXAlignAt;
	if (s == "multline")  return hullMultline;
	if (s == "gather")    return hullGather;
	if (s == "flalign")   return hullFlAlign;
	lyxerr << "unknown hull type '" << to_utf8(s) << "'" << endl;
	return HullType(-1);
}


docstring hullName(HullType type)
{
	switch (type) {
		case hullNone:       return from_ascii("none");
		case hullSimple:     return from_ascii("simple");
		case hullEquation:   return from_ascii("equation");
		case hullEqnArray:   return from_ascii("eqnarray");
		case hullAlign:      return from_ascii("align");
		case hullAlignAt:    return from_ascii("alignat");
		case hullXAlignAt:   return from_ascii("xalignat");
		case hullXXAlignAt:  return from_ascii("xxalignat");
		case hullMultline:   return from_ascii("multline");
		case hullGather:     return from_ascii("gather");
		case hullFlAlign:    return from_ascii("flalign");
		default:
			lyxerr << "unknown hull type '" << type << "'" << endl;
			return from_ascii("none");
	}
}


InsetMathHull::InsetMathHull()
	: InsetMathGrid(1, 1), type_(hullNone), nonum_(1), label_(1),
	  preview_(new RenderPreview(this))
{
	//lyxerr << "sizeof InsetMath: " << sizeof(InsetMath) << endl;
	//lyxerr << "sizeof MetricsInfo: " << sizeof(MetricsInfo) << endl;
	//lyxerr << "sizeof InsetMathChar: " << sizeof(InsetMathChar) << endl;
	//lyxerr << "sizeof Font: " << sizeof(Font) << endl;
	initMath();
	setDefaults();
}


InsetMathHull::InsetMathHull(HullType type)
	: InsetMathGrid(getCols(type), 1), type_(type), nonum_(1), label_(1),
	  preview_(new RenderPreview(this))
{
	initMath();
	setDefaults();
}


InsetMathHull::InsetMathHull(InsetMathHull const & other)
	: InsetMathGrid(other),
	  type_(other.type_), nonum_(other.nonum_), label_(other.label_),
	  preview_(new RenderPreview(*other.preview_, this))
{}


InsetMathHull::~InsetMathHull()
{}


auto_ptr<Inset> InsetMathHull::doClone() const
{
	return auto_ptr<Inset>(new InsetMathHull(*this));
}


InsetMathHull & InsetMathHull::operator=(InsetMathHull const & other)
{
	if (this == &other)
		return *this;
	*static_cast<InsetMathGrid*>(this) = InsetMathGrid(other);
	type_  = other.type_;
	nonum_ = other.nonum_;
	label_ = other.label_;
	preview_.reset(new RenderPreview(*other.preview_, this));

	return *this;
}


Inset * InsetMathHull::editXY(Cursor & cur, int x, int y)
{
	if (use_preview_) {
		edit(cur, true);
		return this;
	}
	return InsetMathNest::editXY(cur, x, y);
}


InsetMath::mode_type InsetMathHull::currentMode() const
{
	if (type_ == hullNone)
		return UNDECIDED_MODE;
	// definitely math mode ...
	return MATH_MODE;
}


bool InsetMathHull::idxFirst(Cursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathHull::idxLast(Cursor & cur) const
{
	cur.idx() = nargs() - 1;
	cur.pos() = cur.lastpos();
	return true;
}


char InsetMathHull::defaultColAlign(col_type col)
{
	if (type_ == hullEqnArray)
		return "rcl"[col];
	if (type_ == hullGather)
		return 'c';
	if (type_ >= hullAlign)
		return "rl"[col & 1];
	return 'c';
}


int InsetMathHull::defaultColSpace(col_type col)
{
	if (type_ == hullAlign || type_ == hullAlignAt)
		return 0;
	if (type_ == hullXAlignAt)
		return (col & 1) ? 20 : 0;
	if (type_ == hullXXAlignAt || type_ == hullFlAlign)
		return (col & 1) ? 40 : 0;
	return 0;
}


docstring InsetMathHull::standardFont() const
{
	return from_ascii(type_ == hullNone ? "lyxnochange" : "mathnormal");
}


bool InsetMathHull::previewState(BufferView * bv) const
{
	if (!editing(bv) && RenderPreview::status() == LyXRC::PREVIEW_ON) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(*bv->buffer());
		return pimage && pimage->image();
	}
	return false;
}


bool InsetMathHull::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (previewState(mi.base.bv)) {
		preview_->metrics(mi, dim);
		// insert a one pixel gap in front of the formula
		dim.wid += 1;
		if (display())
			dim.des += displayMargin();
		if (dim_ == dim)
			return false;
		dim_ = dim;
		return true;
	}

	FontSetChanger dummy1(mi.base, standardFont());
	StyleChanger dummy2(mi.base, display() ? LM_ST_DISPLAY : LM_ST_TEXT);

	// let the cells adjust themselves
	InsetMathGrid::metrics(mi, dim);

	if (display()) {
		dim.asc += displayMargin();
		dim.des += displayMargin();
	}

	if (numberedType()) {
		FontSetChanger dummy(mi.base, from_ascii("mathbf"));
		int l = 0;
		for (row_type row = 0; row < nrows(); ++row)
			l = max(l, mathed_string_width(mi.base.font, nicelabel(row)));

		if (l)
			dim.wid += 30 + l;
	}

	// make it at least as high as the current font
	int asc = 0;
	int des = 0;
	math_font_max_dim(mi.base.font, asc, des);
	dim.asc = max(dim.asc, asc);
	dim.des = max(dim.des, des);

	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathHull::draw(PainterInfo & pi, int x, int y) const
{
	use_preview_ = previewState(pi.base.bv);

	// background of mathed under focus is not painted because
	// selection at the top level of nested inset is difficult to handle.
	if (!editing(pi.base.bv))
		pi.pain.fillRectangle(x + 1, y - ascent() + 1, width() - 2,
				ascent() + descent() - 1, Color::mathbg);

	if (use_preview_) {
		// one pixel gap in front
		preview_->draw(pi, x + 1, y);
		setPosCache(pi, x, y);
		return;
	}

	FontSetChanger dummy1(pi.base, standardFont());
	StyleChanger dummy2(pi.base, display() ? LM_ST_DISPLAY : LM_ST_TEXT);
	InsetMathGrid::draw(pi, x + 1, y);

	if (numberedType()) {
		int const xx = x + colinfo_.back().offset_ + colinfo_.back().width_ + 20;
		for (row_type row = 0; row < nrows(); ++row) {
			int const yy = y + rowinfo_[row].offset_;
			FontSetChanger dummy(pi.base, from_ascii("mathrm"));
			docstring const nl = nicelabel(row);
			pi.draw(xx, yy, nl);
		}
	}
	setPosCache(pi, x, y);
}


void InsetMathHull::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	if (display()) {
		InsetMathGrid::metricsT(mi, dim);
	} else {
		odocstringstream os;
		WriteStream wi(os, false, true);
		write(wi);
		dim.wid = os.str().size();
		dim.asc = 1;
		dim.des = 0;
	}
}


void InsetMathHull::drawT(TextPainter & pain, int x, int y) const
{
	if (display()) {
		InsetMathGrid::drawT(pain, x, y);
	} else {
		odocstringstream os;
		WriteStream wi(os, false, true);
		write(wi);
		pain.draw(x, y, os.str().c_str());
	}
}


namespace {

docstring const latex_string(InsetMathHull const & inset)
{
	odocstringstream ls;
	WriteStream wi(ls, false, false);
	inset.write(wi);
	return ls.str();
}

} // namespace anon


void InsetMathHull::addPreview(graphics::PreviewLoader & ploader) const
{
	if (RenderPreview::status() == LyXRC::PREVIEW_ON) {
		docstring const snippet = latex_string(*this);
		preview_->addPreview(snippet, ploader);
	}
}


bool InsetMathHull::notifyCursorLeaves(Cursor & cur)
{
	if (RenderPreview::status() == LyXRC::PREVIEW_ON) {
		Buffer const & buffer = cur.buffer();
		docstring const snippet = latex_string(*this);
		preview_->addPreview(snippet, buffer);
		preview_->startLoading(buffer);
		cur.updateFlags(Update::Force);
	}
	return false;
}


docstring InsetMathHull::label(row_type row) const
{
	BOOST_ASSERT(row < nrows());
	return label_[row];
}


void InsetMathHull::label(row_type row, docstring const & label)
{
	//lyxerr << "setting label '" << label << "' for row " << row << endl;
	if (label.empty()) {
		label_[row].clear();
	} else
		label_[row] = label;
}


void InsetMathHull::numbered(row_type row, bool num)
{
	nonum_[row] = !num;
	if (nonum_[row])
		label_[row].clear();
}


bool InsetMathHull::numbered(row_type row) const
{
	return !nonum_[row];
}


bool InsetMathHull::ams() const
{
	return
		type_ == hullAlign ||
		type_ == hullFlAlign ||
		type_ == hullMultline ||
		type_ == hullGather ||
		type_ == hullAlignAt ||
		type_ == hullXAlignAt ||
		type_ == hullXXAlignAt;
}


Inset::DisplayType InsetMathHull::display() const
{
	return (type_ != hullSimple && type_ != hullNone) ? AlignCenter : Inline;
}


void InsetMathHull::getLabelList(Buffer const &, vector<docstring> & labels) const
{
	for (row_type row = 0; row < nrows(); ++row)
		if (!label_[row].empty() && nonum_[row] != 1)
			labels.push_back(label_[row]);
}


bool InsetMathHull::numberedType() const
{
	if (type_ == hullNone)
		return false;
	if (type_ == hullSimple)
		return false;
	if (type_ == hullXXAlignAt)
		return false;
	for (row_type row = 0; row < nrows(); ++row)
		if (!nonum_[row])
			return true;
	return false;
}


void InsetMathHull::validate(LaTeXFeatures & features) const
{
	if (ams())
		features.require("amsmath");


	// Validation is necessary only if not using AMS math.
	// To be safe, we will always run mathedvalidate.
	//if (features.amsstyle)
	//  return;

	//features.binom      = true;

	InsetMathGrid::validate(features);
}


void InsetMathHull::header_write(WriteStream & os) const
{
	bool n = numberedType();

	switch(type_) {
	case hullNone:
		break;

	case hullSimple:
		os << '$';
		if (cell(0).empty())
			os << ' ';
		break;

	case hullEquation:
		if (n)
			os << "\\begin{equation" << star(n) << "}\n";
		else
			os << "\\[\n";
		break;

	case hullEqnArray:
	case hullAlign:
	case hullFlAlign:
	case hullGather:
	case hullMultline:
		os << "\\begin{" << hullName(type_) << star(n) << "}\n";
		break;

	case hullAlignAt:
	case hullXAlignAt:
		os << "\\begin{" << hullName(type_) << star(n) << '}'
		  << '{' << static_cast<unsigned int>((ncols() + 1)/2) << "}\n";
		break;

	case hullXXAlignAt:
		os << "\\begin{" << hullName(type_) << '}'
		  << '{' << static_cast<unsigned int>((ncols() + 1)/2) << "}\n";
		break;

	default:
		os << "\\begin{unknown" << star(n) << '}';
		break;
	}
}


void InsetMathHull::footer_write(WriteStream & os) const
{
	bool n = numberedType();

	switch(type_) {
	case hullNone:
		os << "\n";
		break;

	case hullSimple:
		os << '$';
		break;

	case hullEquation:
		if (n)
			os << "\\end{equation" << star(n) << "}\n";
		else
			os << "\\]\n";
		break;

	case hullEqnArray:
	case hullAlign:
	case hullFlAlign:
	case hullAlignAt:
	case hullXAlignAt:
	case hullGather:
	case hullMultline:
		os << "\\end{" << hullName(type_) << star(n) << "}\n";
		break;

	case hullXXAlignAt:
		os << "\\end{" << hullName(type_) << "}\n";
		break;

	default:
		os << "\\end{unknown" << star(n) << '}';
		break;
	}
}


bool InsetMathHull::rowChangeOK() const
{
	return
		type_ == hullEqnArray || type_ == hullAlign ||
		type_ == hullFlAlign || type_ == hullAlignAt ||
		type_ == hullXAlignAt || type_ == hullXXAlignAt ||
		type_ == hullGather || type_ == hullMultline;
}


bool InsetMathHull::colChangeOK() const
{
	return
		type_ == hullAlign || type_ == hullFlAlign ||type_ == hullAlignAt ||
		type_ == hullXAlignAt || type_ == hullXXAlignAt;
}


void InsetMathHull::addRow(row_type row)
{
	if (!rowChangeOK())
		return;

	bool numbered = numberedType();
	docstring label;
	if (type_ == hullMultline) {
		if (row + 1 == nrows()) {
			nonum_[row] = true;
			label = label_[row];
		} else
			numbered = false;
	}

	nonum_.insert(nonum_.begin() + row + 1, !numbered);
	label_.insert(label_.begin() + row + 1, label);
	InsetMathGrid::addRow(row);
}


void InsetMathHull::swapRow(row_type row)
{
	if (nrows() <= 1)
		return;
	if (row + 1 == nrows())
		--row;
	swap(nonum_[row], nonum_[row + 1]);
	swap(label_[row], label_[row + 1]);
	InsetMathGrid::swapRow(row);
}


void InsetMathHull::delRow(row_type row)
{
	if (nrows() <= 1 || !rowChangeOK())
		return;
	if (row + 1 == nrows() && type_ == hullMultline) {
		swap(nonum_[row - 1], nonum_[row]);
		swap(label_[row - 1], label_[row]);
		InsetMathGrid::delRow(row);
		return;
	}
	InsetMathGrid::delRow(row);
	// The last dummy row has no number info nor a label.
	// Test nrows() + 1 because we have already erased the row.
	if (row == nrows() + 1)
		row--;
	nonum_.erase(nonum_.begin() + row);
	label_.erase(label_.begin() + row);
}


void InsetMathHull::addCol(col_type col)
{
	if (!colChangeOK())
		return;
	InsetMathGrid::addCol(col);
}


void InsetMathHull::delCol(col_type col)
{
	if (ncols() <= 1 || !colChangeOK())
		return;
	InsetMathGrid::delCol(col);
}


docstring InsetMathHull::nicelabel(row_type row) const
{
	if (nonum_[row])
		return docstring();
	if (label_[row].empty())
		return from_ascii("(#)");
	return '(' + label_[row] + from_ascii(", #)");
}


void InsetMathHull::glueall()
{
	MathData ar;
	for (idx_type i = 0; i < nargs(); ++i)
		ar.append(cell(i));
	*this = InsetMathHull(hullSimple);
	cell(0) = ar;
	setDefaults();
}


void InsetMathHull::splitTo2Cols()
{
	BOOST_ASSERT(ncols() == 1);
	InsetMathGrid::addCol(1);
	for (row_type row = 0; row < nrows(); ++row) {
		idx_type const i = 2 * row;
		pos_type pos = firstRelOp(cell(i));
		cell(i + 1) = MathData(cell(i).begin() + pos, cell(i).end());
		cell(i).erase(pos, cell(i).size());
	}
}


void InsetMathHull::splitTo3Cols()
{
	BOOST_ASSERT(ncols() < 3);
	if (ncols() < 2)
		splitTo2Cols();
	InsetMathGrid::addCol(2);
	for (row_type row = 0; row < nrows(); ++row) {
		idx_type const i = 3 * row + 1;
		if (cell(i).size()) {
			cell(i + 1) = MathData(cell(i).begin() + 1, cell(i).end());
			cell(i).erase(1, cell(i).size());
		}
	}
}


void InsetMathHull::changeCols(col_type cols)
{
	if (ncols() == cols)
		return;
	else if (ncols() < cols) {
		// split columns
		if (cols < 3)
			splitTo2Cols();
		else {
			splitTo3Cols();
			while (ncols() < cols)
				InsetMathGrid::addCol(ncols());
		}
		return;
	}

	// combine columns
	for (row_type row = 0; row < nrows(); ++row) {
		idx_type const i = row * ncols();
		for (col_type col = cols; col < ncols(); ++col) {
			cell(i + cols - 1).append(cell(i + col));
		}
	}
	// delete columns
	while (ncols() > cols) {
		InsetMathGrid::delCol(ncols() - 1);
	}
}


HullType InsetMathHull::getType() const
{
	return type_;
}


void InsetMathHull::setType(HullType type)
{
	type_ = type;
	setDefaults();
}


void InsetMathHull::mutate(HullType newtype)
{
	//lyxerr << "mutating from '" << type_ << "' to '" << newtype << "'" << endl;

	// we try to move along the chain
	// none <-> simple <-> equation <-> eqnarray -> *align* -> multline, gather -+
	//                                     ^                                     |
	//                                     +-------------------------------------+
	// we use eqnarray as intermediate type for mutations that are not
	// directly supported because it handles labels and numbering for
	// "down mutation".

	if (newtype == type_) {
		// done
	}

	else if (newtype < hullNone) {
		// unknown type
		dump();
	}

	else if (type_ == hullNone) {
		setType(hullSimple);
		numbered(0, false);
		mutate(newtype);
	}

	else if (type_ == hullSimple) {
		if (newtype == hullNone) {
			setType(hullNone);
			numbered(0, false);
		} else {
			setType(hullEquation);
			numbered(0, false);
			mutate(newtype);
		}
	}

	else if (type_ == hullEquation) {
		if (newtype < type_) {
			setType(hullSimple);
			numbered(0, false);
			mutate(newtype);
		} else if (newtype == hullEqnArray) {
			// split it "nicely" on the first relop
			splitTo3Cols();
			setType(hullEqnArray);
		} else if (newtype == hullMultline || newtype == hullGather) {
			setType(newtype);
		} else {
			// split it "nicely"
			splitTo2Cols();
			setType(hullAlign);
			mutate(newtype);
		}
	}

	else if (type_ == hullEqnArray) {
		if (newtype < type_) {
			// set correct (no)numbering
			bool allnonum = true;
			for (row_type row = 0; row < nrows(); ++row)
				if (!nonum_[row])
					allnonum = false;

			// set first non-empty label
			docstring label;
			for (row_type row = 0; row < nrows(); ++row) {
				if (!label_[row].empty()) {
					label = label_[row];
					break;
				}
			}

			glueall();
			nonum_[0] = allnonum;
			label_[0] = label;
			mutate(newtype);
		} else { // align & Co.
			changeCols(2);
			setType(hullAlign);
			mutate(newtype);
		}
	}

	else if (type_ ==  hullAlign || type_ == hullAlignAt ||
		 type_ == hullXAlignAt || type_ == hullFlAlign) {
		if (newtype < hullAlign) {
			changeCols(3);
			setType(hullEqnArray);
			mutate(newtype);
		} else if (newtype == hullGather || newtype == hullMultline) {
			changeCols(1);
			setType(newtype);
		} else if (newtype ==   hullXXAlignAt) {
			for (row_type row = 0; row < nrows(); ++row)
				numbered(row, false);
			setType(newtype);
		} else {
			setType(newtype);
		}
	}

	else if (type_ == hullXXAlignAt) {
		for (row_type row = 0; row < nrows(); ++row)
			numbered(row, false);
		if (newtype < hullAlign) {
			changeCols(3);
			setType(hullEqnArray);
			mutate(newtype);
		} else if (newtype == hullGather || newtype == hullMultline) {
			changeCols(1);
			setType(newtype);
		} else {
			setType(newtype);
		}
	}

	else if (type_ == hullMultline || type_ == hullGather) {
		if (newtype == hullGather || newtype == hullMultline)
			setType(newtype);
		else if (newtype == hullAlign || newtype == hullFlAlign  ||
			 newtype == hullAlignAt || newtype == hullXAlignAt) {
			splitTo2Cols();
			setType(newtype);
		} else if (newtype ==   hullXXAlignAt) {
			splitTo2Cols();
			for (row_type row = 0; row < nrows(); ++row)
				numbered(row, false);
			setType(newtype);
		} else {
			splitTo3Cols();
			setType(hullEqnArray);
			mutate(newtype);
		}
	}

	else {
		lyxerr << "mutation from '" << to_utf8(hullName(type_))
		       << "' to '" << to_utf8(hullName(newtype))
		       << "' not implemented" << endl;
	}
}


docstring InsetMathHull::eolString(row_type row, bool emptyline, bool fragile) const
{
	docstring res;
	if (numberedType()) {
		if (!label_[row].empty() && !nonum_[row])
			res += "\\label{" + label_[row] + '}';
		if (nonum_[row] && (type_ != hullMultline))
			res += "\\nonumber ";
	}
	return res + InsetMathGrid::eolString(row, emptyline, fragile);
}


void InsetMathHull::write(WriteStream & os) const
{
	header_write(os);
	InsetMathGrid::write(os);
	footer_write(os);
}


void InsetMathHull::normalize(NormalStream & os) const
{
	os << "[formula " << hullName(type_) << ' ';
	InsetMathGrid::normalize(os);
	os << "] ";
}


void InsetMathHull::mathmlize(MathStream & os) const
{
	InsetMathGrid::mathmlize(os);
}


void InsetMathHull::infoize(odocstream & os) const
{
	os << "Type: " << hullName(type_);
}


void InsetMathHull::check() const
{
	BOOST_ASSERT(nonum_.size() == nrows());
	BOOST_ASSERT(label_.size() == nrows());
}


void InsetMathHull::doExtern(Cursor & cur, FuncRequest & func)
{
	docstring dlang;
	docstring extra;
	idocstringstream iss(func.argument());
	iss >> dlang >> extra;
	if (extra.empty())
		extra = from_ascii("noextra");
	std::string const lang = to_ascii(dlang);

#ifdef WITH_WARNINGS
#warning temporarily disabled
	//if (cur.selection()) {
	//	MathData ar;
	//	selGet(cur.ar);
	//	lyxerr << "use selection: " << ar << endl;
	//	insert(pipeThroughExtern(lang, extra, ar));
	//	return;
	//}
#endif

	MathData eq;
	eq.push_back(MathAtom(new InsetMathChar('=')));

	// go to first item in line
	cur.idx() -= cur.idx() % ncols();
	cur.pos() = 0;

	if (getType() == hullSimple) {
		size_type pos = cur.cell().find_last(eq);
		MathData ar;
		if (cur.inMathed() && cur.selection()) {
			asArray(grabAndEraseSelection(cur), ar);
		} else if (pos == cur.cell().size()) {
			ar = cur.cell();
			lyxerr << "use whole cell: " << ar << endl;
		} else {
			ar = MathData(cur.cell().begin() + pos + 1, cur.cell().end());
			lyxerr << "use partial cell form pos: " << pos << endl;
		}
		cur.cell().append(eq);
		cur.cell().append(pipeThroughExtern(lang, extra, ar));
		cur.pos() = cur.lastpos();
		return;
	}

	if (getType() == hullEquation) {
		lyxerr << "use equation inset" << endl;
		mutate(hullEqnArray);
		MathData & ar = cur.cell();
		lyxerr << "use cell: " << ar << endl;
		++cur.idx();
		cur.cell() = eq;
		++cur.idx();
		cur.cell() = pipeThroughExtern(lang, extra, ar);
		// move to end of line
		cur.pos() = cur.lastpos();
		return;
	}

	{
		lyxerr << "use eqnarray" << endl;
		cur.idx() += 2 - cur.idx() % ncols();
		cur.pos() = 0;
		MathData ar = cur.cell();
		lyxerr << "use cell: " << ar << endl;
#ifdef WITH_WARNINGS
#warning temporarily disabled
#endif
		addRow(cur.row());
		++cur.idx();
		++cur.idx();
		cur.cell() = eq;
		++cur.idx();
		cur.cell() = pipeThroughExtern(lang, extra, ar);
		cur.pos() = cur.lastpos();
	}
}


void InsetMathHull::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "action: " << cmd.action << endl;
	switch (cmd.action) {

	case LFUN_FINISHED_LEFT:
	case LFUN_FINISHED_RIGHT:
		//lyxerr << "action: " << cmd.action << endl;
		InsetMathGrid::doDispatch(cur, cmd);
		notifyCursorLeaves(cur);
		cur.undispatched();
		break;

	case LFUN_BREAK_PARAGRAPH:
		// just swallow this
		break;

	case LFUN_BREAK_LINE:
		// some magic for the common case
		if (type_ == hullSimple || type_ == hullEquation) {
			recordUndoInset(cur);
			bool const align =
				cur.bv().buffer()->params().use_amsmath == BufferParams::package_on;
			mutate(align ? hullAlign : hullEqnArray);
			cur.idx() = nrows() * ncols() - 1;
			cur.pos() = cur.lastpos();
		}
		InsetMathGrid::doDispatch(cur, cmd);
		break;

	case LFUN_MATH_NUMBER: {
		//lyxerr << "toggling all numbers" << endl;
		recordUndoInset(cur);
		bool old = numberedType();
		if (type_ == hullMultline)
			numbered(nrows() - 1, !old);
		else
			for (row_type row = 0; row < nrows(); ++row)
				numbered(row, !old);
		
		cur.message(old ? _("No number") : _("Number"));
		break;
	}

	case LFUN_MATH_NONUMBER: {
			recordUndoInset(cur);
			row_type r = (type_ == hullMultline) ? nrows() - 1 : cur.row();
			bool old = numbered(r);
			cur.message(old ? _("No number") : _("Number"));
			numbered(r, !old);
		break;
	}

	case LFUN_LABEL_INSERT: {
		recordUndoInset(cur);
		row_type r = (type_ == hullMultline) ? nrows() - 1 : cur.row();
		docstring old_label = label(r);
		docstring const default_label = from_ascii(
			(lyxrc.label_init_length >= 0) ? "eq:" : "");
		if (old_label.empty())
			old_label = default_label;

		InsetCommandParams p("label");
		p["name"] = cmd.argument().empty() ? old_label : cmd.argument();
		std::string const data = InsetCommandMailer::params2string("label", p);

		if (cmd.argument().empty())
			cur.bv().showInsetDialog("label", data, 0);
		else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_WORD_DELETE_FORWARD:
	case LFUN_CHAR_DELETE_FORWARD:
		if (col(cur.idx()) + 1 == ncols()
		    && cur.pos() == cur.lastpos()) {
			if (!label(row(cur.idx())).empty()) {
				recordUndoInset(cur);
				label(row(cur.idx()), docstring());
			} else if (numbered(row(cur.idx()))) {
				recordUndoInset(cur);
				numbered(row(cur.idx()), false);
			} else {
				InsetMathGrid::doDispatch(cur, cmd);
				return;
			}
		} else {
			InsetMathGrid::doDispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_INSET_INSERT: {
		//lyxerr << "arg: " << to_utf8(cmd.argument()) << endl;
		std::string const name = cmd.getArg(0);
		if (name == "label") {
			InsetCommandParams p("label");
			InsetCommandMailer::string2params(name, to_utf8(cmd.argument()), p);
			docstring str = p["name"];
			recordUndoInset(cur);
			row_type const r = (type_ == hullMultline) ? nrows() - 1 : cur.row();
			str = support::trim(str);
			if (!str.empty())
				numbered(r, true);
			docstring old = label(r);
			if (str != old) {
				cur.bv().buffer()->changeRefsIfUnique(old, str,
							Inset::REF_CODE);
				label(r, str);
			}
			break;
		}
		InsetMathGrid::doDispatch(cur, cmd);
		return;
	}

	case LFUN_MATH_EXTERN:
		recordUndoInset(cur);
		doExtern(cur, cmd);
		break;

	case LFUN_MATH_MUTATE: {
		recordUndoInset(cur);
		row_type row = cur.row();
		col_type col = cur.col();
		mutate(hullType(cmd.argument()));
		cur.idx() = row * ncols() + col;
		if (cur.idx() > cur.lastidx()) {
			cur.idx() = cur.lastidx();
			cur.pos() = cur.lastpos();
		}
		if (cur.pos() > cur.lastpos())
			cur.pos() = cur.lastpos();

		// FIXME: find some more clever handling of the selection,
		// i.e. preserve it.
		cur.clearSelection();
		//cur.dispatched(FINISHED);
		break;
	}

	case LFUN_MATH_DISPLAY: {
		recordUndoInset(cur);
		mutate(type_ == hullSimple ? hullEquation : hullSimple);
		cur.idx() = 0;
		cur.pos() = cur.lastpos();
		//cur.dispatched(FINISHED);
		break;
	}

	default:
		InsetMathGrid::doDispatch(cur, cmd);
		break;
	}
}


bool InsetMathHull::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_FINISHED_LEFT:
	case LFUN_FINISHED_RIGHT:
	case LFUN_UP:
	case LFUN_DOWN:
	case LFUN_BREAK_LINE:
	case LFUN_MATH_EXTERN:
	case LFUN_MATH_MUTATE:
	case LFUN_MATH_DISPLAY:
		// we handle these
		status.enabled(true);
		return true;
	case LFUN_MATH_NUMBER:
		// FIXME: what is the right test, this or the one of
		// LABEL_INSERT?
		status.enabled(display());
		status.setOnOff(numberedType());
		return true;
	case LFUN_MATH_NONUMBER: {
		// FIXME: what is the right test, this or the one of
		// LABEL_INSERT?
		bool const enable = (type_ == hullMultline) ?
			(nrows() - 1 == cur.row()) : display();
		row_type const r = (type_ == hullMultline) ? nrows() - 1 : cur.row();
		status.enabled(enable);
		status.setOnOff(numbered(r));
		return true;
	}
	case LFUN_LABEL_INSERT:
		status.enabled(type_ != hullSimple);
		return true;
	case LFUN_INSET_INSERT:
		if (cmd.getArg(0) == "label") {
			status.enabled(type_ != hullSimple);
			return true;
		}
		return InsetMathGrid::getStatus(cur, cmd, status);
	case LFUN_TABULAR_FEATURE: {
		istringstream is(to_utf8(cmd.argument()));
		std::string s;
		is >> s;
		if (!rowChangeOK()
		    && (s == "append-row"
			|| s == "delete-row"
			|| s == "copy-row")) {
			status.message(bformat(
				from_utf8(N_("Can't change number of rows in '%1$s'")),
				hullName(type_)));
			status.enabled(false);
			return true;
		}
		if (!colChangeOK()
		    && (s == "append-column"
			|| s == "delete-column"
			|| s == "copy-column")) {
			status.message(bformat(
				from_utf8(N_("Can't change number of columns in '%1$s'")),
				hullName(type_)));
			status.enabled(false);
			return true;
		}
		if ((type_ == hullSimple
		  || type_ == hullEquation
		  || type_ == hullNone) &&
		    (s == "add-hline-above" || s == "add-hline-below")) {
			status.message(bformat(
				from_utf8(N_("Can't add horizontal grid lines in '%1$s'")),
				hullName(type_)));
			status.enabled(false);
			return true;
		}
		if (s == "add-vline-left" || s == "add-vline-right") {
			status.message(bformat(
				from_utf8(N_("Can't add vertical grid lines in '%1$s'")),
				hullName(type_)));
			status.enabled(false);
			return true;
		}
		if (s == "valign-top" || s == "valign-middle"
		 || s == "valign-bottom" || s == "align-left"
		 || s == "align-center" || s == "align-right") {
			status.enabled(false);
			return true;
		}
		return InsetMathGrid::getStatus(cur, cmd, status);
	}
	default:
		return InsetMathGrid::getStatus(cur, cmd, status);
	}

	// This cannot really happen, but inserted to shut-up gcc
	return InsetMathGrid::getStatus(cur, cmd, status);
}


/////////////////////////////////////////////////////////////////////



// simply scrap this function if you want
void InsetMathHull::mutateToText()
{
#if 0
	// translate to latex
	ostringstream os;
	latex(NULL, os, false, false);
	string str = os.str();

	// insert this text
	Text * lt = view_->cursor().innerText();
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit)
		view_->getIntl()->getTransManager().TranslateAndInsert(*cit, lt);

	// remove ourselves
	//dispatch(LFUN_ESCAPE);
#endif
}


void InsetMathHull::handleFont(Cursor & cur, docstring const & arg,
	docstring const & font)
{
	// this whole function is a hack and won't work for incremental font
	// changes...
	recordUndo(cur);
	if (cur.inset().asInsetMath()->name() == font)
		cur.handleFont(to_utf8(font));
	else {
		cur.handleNest(createInsetMath(font));
		cur.insert(arg);
	}
}


void InsetMathHull::handleFont2(Cursor & cur, docstring const & arg)
{
	recordUndo(cur);
	Font font;
	bool b;
	bv_funcs::string2font(to_utf8(arg), font, b);
	if (font.color() != Color::inherit) {
		MathAtom at = MathAtom(new InsetMathColor(true, font.color()));
		cur.handleNest(at, 0);
	}
}


void InsetMathHull::edit(Cursor & cur, bool left)
{
	cur.push(*this);
	left ? idxFirst(cur) : idxLast(cur);
	// The inset formula dimension is not necessarily the same as the
	// one of the instant preview image, so we have to indicate to the
	// BufferView that a metrics update is needed.
	cur.updateFlags(Update::Force);
}


docstring const InsetMathHull::editMessage() const
{
	return _("Math editor mode");
}


void InsetMathHull::revealCodes(Cursor & cur) const
{
	if (!cur.inMathed())
		return;
	odocstringstream os;
	cur.info(os);
	cur.message(os.str());
/*
	// write something to the minibuffer
	// translate to latex
	cur.markInsert(bv);
	ostringstream os;
	write(NULL, os);
	string str = os.str();
	cur.markErase(bv);
	string::size_type pos = 0;
	string res;
	for (string::iterator it = str.begin(); it != str.end(); ++it) {
		if (*it == '\n')
			res += ' ';
		else if (*it == '\0') {
			res += "  -X-  ";
			pos = it - str.begin();
		}
		else
			res += *it;
	}
	if (pos > 30)
		res = res.substr(pos - 30);
	if (res.size() > 60)
		res = res.substr(0, 60);
	cur.message(res);
*/
}


Inset::Code InsetMathHull::lyxCode() const
{
	return MATH_CODE;
}


/////////////////////////////////////////////////////////////////////


#if 0
bool InsetMathHull::searchForward(BufferView * bv, string const & str,
				     bool, bool)
{
#ifdef WITH_WARNINGS
#warning completely broken
#endif
	static InsetMathHull * lastformula = 0;
	static CursorBase current = DocIterator(ibegin(nucleus()));
	static MathData ar;
	static string laststr;

	if (lastformula != this || laststr != str) {
		//lyxerr << "reset lastformula to " << this << endl;
		lastformula = this;
		laststr = str;
		current	= ibegin(nucleus());
		ar.clear();
		mathed_parse_cell(ar, str);
	} else {
		increment(current);
	}
	//lyxerr << "searching '" << str << "' in " << this << ar << endl;

	for (DocIterator it = current; it != iend(nucleus()); increment(it)) {
		CursorSlice & top = it.back();
		MathData const & a = top.asInsetMath()->cell(top.idx_);
		if (a.matchpart(ar, top.pos_)) {
			bv->cursor().setSelection(it, ar.size());
			current = it;
			top.pos_ += ar.size();
			bv->update();
			return true;
		}
	}

	//lyxerr << "not found!" << endl;
	lastformula = 0;
	return false;
}
#endif


void InsetMathHull::write(Buffer const &, std::ostream & os) const
{
	odocstringstream oss;
	WriteStream wi(oss, false, false);
	oss << "Formula ";
	write(wi);
	os << to_utf8(oss.str());
}


void InsetMathHull::read(Buffer const &, Lexer & lex)
{
	MathAtom at;
	mathed_parse_normal(at, lex);
	operator=(*at->asHullInset());
}


int InsetMathHull::plaintext(Buffer const &, odocstream & os,
			     OutputParams const &) const
{
	if (0 && display()) {
		Dimension dim;
		TextMetricsInfo mi;
		metricsT(mi, dim);
		TextPainter tpain(dim.width(), dim.height());
		drawT(tpain, 0, dim.ascent());
		tpain.show(os, 3);
		// reset metrics cache to "real" values
		//metrics();
		return tpain.textheight();
	} else {
		odocstringstream oss;
		WriteStream wi(oss, false, true);
		wi << cell(0);

		docstring const str = oss.str();
		os << str;
		return str.size();
	}
}


int InsetMathHull::docbook(Buffer const & buf, odocstream & os,
			   OutputParams const & runparams) const
{
	MathStream ms(os);
	int res = 0;
	docstring name;
	if (getType() == hullSimple)
		name = from_ascii("inlineequation");
	else
		name = from_ascii("informalequation");

	docstring bname = name;
	if (!label(0).empty())
		bname += " id='" + sgml::cleanID(buf, runparams, label(0)) + "'";
	ms << MTag(bname);

	odocstringstream ls;
	if (runparams.flavor == OutputParams::XML) {
		ms << MTag(from_ascii("alt role='tex' "));
		// Workaround for db2latex: db2latex always includes equations with
		// \ensuremath{} or \begin{display}\end{display}
		// so we strip LyX' math environment
		WriteStream wi(ls, false, false);
		InsetMathGrid::write(wi);
		ms << from_utf8(subst(subst(to_utf8(ls.str()), "&", "&amp;"), "<", "&lt;"));
		ms << ETag(from_ascii("alt"));
		ms << MTag(from_ascii("math"));
		ms << ETag(from_ascii("alt"));
		ms << MTag(from_ascii("math"));
		InsetMathGrid::mathmlize(ms);
		ms << ETag(from_ascii("math"));
	} else {
		ms << MTag(from_ascii("alt role='tex'"));
		res = latex(buf, ls, runparams);
		ms << from_utf8(subst(subst(to_utf8(ls.str()), "&", "&amp;"), "<", "&lt;"));
		ms << ETag(from_ascii("alt"));
	}

	ms << from_ascii("<graphic fileref=\"eqn/");
	if (!label(0).empty())
		ms << sgml::cleanID(buf, runparams, label(0));
	else
		ms << sgml::uniqueID(from_ascii("anon"));

	if (runparams.flavor == OutputParams::XML)
		ms << from_ascii("\"/>");
	else
		ms << from_ascii("\">");

	ms << ETag(name);
	return ms.line() + res;
}


void InsetMathHull::textString(Buffer const & buf, odocstream & os) const
{
	plaintext(buf, os, OutputParams(0));
}


} // namespace lyx
