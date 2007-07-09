/**
 * \file math_hullinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_charinset.h"
#include "math_colorinset.h"
#include "math_data.h"
#include "math_extern.h"
#include "math_factory.h"
#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "FuncStatus.h"
#include "LColor.h"
#include "LaTeXFeatures.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "outputparams.h"
#include "sgml.h"
#include "textpainter.h"
#include "undo.h"

#include "insets/render_preview.h"
#include "insets/insetlabel.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <sstream>

using lyx::cap::grabAndEraseSelection;
using lyx::support::bformat;
using lyx::support::subst;

using std::endl;
using std::max;
using std::string;
using std::ostream;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::swap;
using std::vector;


namespace {

	int getCols(string const & type)
	{
		if (type == "eqnarray")
			return 3;
		if (type == "align")
			return 2;
		if (type == "flalign")
			return 2;
		if (type == "alignat")
			return 2;
		if (type == "xalignat")
			return 2;
		if (type == "xxalignat")
			return 2;
		return 1;
	}


	// returns position of first relation operator in the array
	// used for "intelligent splitting"
	size_t firstRelOp(MathArray const & ar)
	{
		for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
			if ((*it)->isRelOp())
				return it - ar.begin();
		return ar.size();
	}


	char const * star(bool numbered)
	{
		return numbered ? "" : "*";
	}


	int typecode(string const & s)
	{
		if (s == "none")      return 0;
		if (s == "simple")    return 1;
		if (s == "equation")  return 2;
		if (s == "eqnarray")  return 3;
		if (s == "align")     return 4;
		if (s == "alignat")   return 5;
		if (s == "xalignat")  return 6;
		if (s == "xxalignat") return 7;
		if (s == "multline")  return 8;
		if (s == "gather")    return 9;
		if (s == "flalign")   return 10;
		lyxerr << "unknown hull type '" << s << "'" << endl;
		return -1;
	}

	bool smaller(string const & s, string const & t)
	{
		return typecode(s) < typecode(t);
	}


} // end anon namespace



MathHullInset::MathHullInset()
	: MathGridInset(1, 1), type_("none"), nonum_(1), label_(1),
	  preview_(new RenderPreview(this))
{
	//lyxerr << "sizeof MathInset: " << sizeof(MathInset) << endl;
	//lyxerr << "sizeof MetricsInfo: " << sizeof(MetricsInfo) << endl;
	//lyxerr << "sizeof MathCharInset: " << sizeof(MathCharInset) << endl;
	//lyxerr << "sizeof LyXFont: " << sizeof(LyXFont) << endl;
	initMath();
	setDefaults();
}


MathHullInset::MathHullInset(string const & type)
	: MathGridInset(getCols(type), 1), type_(type), nonum_(1), label_(1),
	  preview_(new RenderPreview(this))
{
	initMath();
	setDefaults();
}


MathHullInset::MathHullInset(MathHullInset const & other)
	: MathGridInset(other),
	  type_(other.type_), nonum_(other.nonum_), label_(other.label_),
	  preview_(new RenderPreview(*other.preview_, this))
{}


MathHullInset::~MathHullInset()
{}


auto_ptr<InsetBase> MathHullInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathHullInset(*this));
}


MathHullInset & MathHullInset::operator=(MathHullInset const & other)
{
	if (this == &other)
		return *this;
	*static_cast<MathGridInset*>(this) = MathGridInset(other);
	type_  = other.type_;
	nonum_ = other.nonum_;
	label_ = other.label_;
	preview_.reset(new RenderPreview(*other.preview_, this));

	return *this;
}


InsetBase * MathHullInset::editXY(LCursor & cur, int x, int y)
{
	if (use_preview_) {
		edit(cur, true);
		return this;
	}
	return MathNestInset::editXY(cur, x, y); 
}


MathInset::mode_type MathHullInset::currentMode() const
{
	if (type_ == "none")
		return UNDECIDED_MODE;
	// definitely math mode ...
	return MATH_MODE;
}


bool MathHullInset::idxFirst(LCursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool MathHullInset::idxLast(LCursor & cur) const
{
	cur.idx() = nargs() - 1;
	cur.pos() = cur.lastpos();
	return true;
}


char MathHullInset::defaultColAlign(col_type col)
{
	if (type_ == "eqnarray")
		return "rcl"[col];
	if (typecode(type_) >= typecode("align"))
		return "rl"[col & 1];
	return 'c';
}


int MathHullInset::defaultColSpace(col_type col)
{
	if (type_ == "align" || type_ == "alignat")
		return 0;
	if (type_ == "xalignat")
		return (col & 1) ? 20 : 0;
	if (type_ == "xxalignat" || type_ == "flalign")
		return (col & 1) ? 40 : 0;
	return 0;
}


char const * MathHullInset::standardFont() const
{
	return type_ == "none" ? "lyxnochange" : "mathnormal";
}


bool MathHullInset::previewState(BufferView * bv) const
{
	if (!editing(bv) && RenderPreview::status() == LyXRC::PREVIEW_ON) {
		lyx::graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(*bv->buffer());
		return pimage && pimage->image();
	}
	return false;
}


void MathHullInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (previewState(mi.base.bv)) {
		preview_->metrics(mi, dim);
		// insert a one pixel gap in front of the formula
		dim.wid += 1;
		if (display())
			dim.des += 12;
		dim_ = dim;
		return;
	}

	FontSetChanger dummy1(mi.base, standardFont());
	StyleChanger dummy2(mi.base, display() ? LM_ST_DISPLAY : LM_ST_TEXT);

	// let the cells adjust themselves
	MathGridInset::metrics(mi, dim);

	if (display()) {
		dim.asc += 12;
		dim.des += 12;
	}

	if (numberedType()) {
		FontSetChanger dummy(mi.base, "mathbf");
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

	dim_ = dim;
}


void MathHullInset::draw(PainterInfo & pi, int x, int y) const
{
	use_preview_ = previewState(pi.base.bv);

	if (use_preview_) {
		// one pixel gap in front
		preview_->draw(pi, x + 1, y);
		setPosCache(pi, x, y);
		return;
	}

	FontSetChanger dummy1(pi.base, standardFont());
	StyleChanger dummy2(pi.base, display() ? LM_ST_DISPLAY : LM_ST_TEXT);
	MathGridInset::draw(pi, x + 1, y);

	if (numberedType()) {
		int const xx = x + colinfo_.back().offset_ + colinfo_.back().width_ + 20;
		for (row_type row = 0; row < nrows(); ++row) {
			int const yy = y + rowinfo_[row].offset_;
			FontSetChanger dummy(pi.base, "mathrm");
			pi.draw(xx, yy, nicelabel(row));
		}
	}
	setPosCache(pi, x, y);
}


void MathHullInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	if (display()) {
		MathGridInset::metricsT(mi, dim);
	} else {
		ostringstream os;
		WriteStream wi(os, false, true);
		write(wi);
		dim.wid = os.str().size();
		dim.asc = 1;
		dim.des = 0;
	}
}


void MathHullInset::drawT(TextPainter & pain, int x, int y) const
{
	if (display()) {
		MathGridInset::drawT(pain, x, y);
	} else {
		ostringstream os;
		WriteStream wi(os, false, true);
		write(wi);
		pain.draw(x, y, os.str().c_str());
	}
}


namespace {

string const latex_string(MathHullInset const & inset)
{
	ostringstream ls;
	WriteStream wi(ls, false, false);
	inset.write(wi);
	return ls.str();
}

} // namespace anon


void MathHullInset::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	if (RenderPreview::status() == LyXRC::PREVIEW_ON) {
		string const snippet = latex_string(*this);
		preview_->addPreview(snippet, ploader);
	}
}


bool MathHullInset::notifyCursorLeaves(LCursor & cur)
{
	if (RenderPreview::status() == LyXRC::PREVIEW_ON) {
		Buffer const & buffer = cur.buffer();
		string const snippet = latex_string(*this);
		preview_->addPreview(snippet, buffer);
		preview_->startLoading(buffer);
	}
	return false;
}


string MathHullInset::label(row_type row) const
{
	BOOST_ASSERT(row < nrows());
	return label_[row];
}


void MathHullInset::label(row_type row, string const & label)
{
	//lyxerr << "setting label '" << label << "' for row " << row << endl;
	label_[row] = label;
}


void MathHullInset::numbered(row_type row, bool num)
{
	nonum_[row] = !num;
	if (nonum_[row])
		label_[row].clear();
}


bool MathHullInset::numbered(row_type row) const
{
	return !nonum_[row];
}


bool MathHullInset::ams() const
{
	return
		type_ == "align" ||
		type_ == "flalign" ||
		type_ == "multline" ||
		type_ == "gather" ||
		type_ == "alignat" ||
		type_ == "xalignat" ||
		type_ == "xxalignat";
}


bool MathHullInset::display() const
{
	return type_ != "simple" && type_ != "none";
}


void MathHullInset::getLabelList(Buffer const &, vector<string> & labels) const
{
	for (row_type row = 0; row < nrows(); ++row)
		if (!label_[row].empty() && nonum_[row] != 1)
			labels.push_back(label_[row]);
}


bool MathHullInset::numberedType() const
{
	if (type_ == "none")
		return false;
	if (type_ == "simple")
		return false;
	if (type_ == "xxalignat")
		return false;
	for (row_type row = 0; row < nrows(); ++row)
		if (!nonum_[row])
			return true;
	return false;
}


void MathHullInset::validate(LaTeXFeatures & features) const
{
	if (ams())
		features.require("amsmath");


	// Validation is necessary only if not using AMS math.
	// To be safe, we will always run mathedvalidate.
	//if (features.amsstyle)
	//  return;

	features.require("boldsymbol");
	//features.binom      = true;

	MathGridInset::validate(features);
}


void MathHullInset::header_write(WriteStream & os) const
{
	bool n = numberedType();

	if (type_ == "none")
		;

	else if (type_ == "simple") {
		os << '$';
		if (cell(0).empty())
			os << ' ';
	}

	else if (type_ == "equation") {
		if (n)
			os << "\\begin{equation" << star(n) << "}\n";
		else
			os << "\\[\n";
	}

	else if (type_ == "eqnarray" || type_ == "align" || type_ == "flalign"
		 || type_ == "gather" || type_ == "multline")
			os << "\\begin{" << type_ << star(n) << "}\n";

	else if (type_ == "alignat" || type_ == "xalignat")
		os << "\\begin{" << type_ << star(n) << '}'
		  << '{' << static_cast<unsigned int>((ncols() + 1)/2) << "}\n";

	else if (type_ == "xxalignat")
		os << "\\begin{" << type_ << '}'
		  << '{' << static_cast<unsigned int>((ncols() + 1)/2) << "}\n";

	else
		os << "\\begin{unknown" << star(n) << '}';
}


void MathHullInset::footer_write(WriteStream & os) const
{
	bool n = numberedType();

	if (type_ == "none")
		os << "\n";

	else if (type_ == "simple")
		os << '$';

	else if (type_ == "equation")
		if (n)
			os << "\\end{equation" << star(n) << "}\n";
		else
			os << "\\]\n";

	else if (type_ == "eqnarray" || type_ == "align" || type_ == "flalign"
		 || type_ == "alignat" || type_ == "xalignat"
		 || type_ == "gather" || type_ == "multline")
		os << "\\end{" << type_ << star(n) << "}\n";

	else if (type_ == "xxalignat")
		os << "\\end{" << type_ << "}\n";

	else
		os << "\\end{unknown" << star(n) << '}';
}


bool MathHullInset::rowChangeOK() const
{
	return
		type_ == "eqnarray" || type_ == "align" ||
		type_ == "flalign" || type_ == "alignat" ||
		type_ == "xalignat" || type_ == "xxalignat" ||
		type_ == "gather" || type_ == "multline";
}


bool MathHullInset::colChangeOK() const
{
	return
		type_ == "align" || type_ == "flalign" ||type_ == "alignat" ||
		type_ == "xalignat" || type_ == "xxalignat";
}


void MathHullInset::addRow(row_type row)
{
	if (!rowChangeOK())
		return;
	nonum_.insert(nonum_.begin() + row + 1, !numberedType());
	label_.insert(label_.begin() + row + 1, string());
	MathGridInset::addRow(row);
}


void MathHullInset::swapRow(row_type row)
{
	if (nrows() <= 1)
		return;
	if (row + 1 == nrows())
		--row;
	swap(nonum_[row], nonum_[row + 1]);
	swap(label_[row], label_[row + 1]);
	MathGridInset::swapRow(row);
}


void MathHullInset::delRow(row_type row)
{
	if (nrows() <= 1 || !rowChangeOK())
		return;
	MathGridInset::delRow(row);
	// The last dummy row has no number info nor a label.
	// Test nrows() + 1 because we have already erased the row.
	if (row == nrows() + 1)
		row--;
	nonum_.erase(nonum_.begin() + row);
	label_.erase(label_.begin() + row);
}


void MathHullInset::addCol(col_type col)
{
	if (!colChangeOK())
		return;
	MathGridInset::addCol(col);
}


void MathHullInset::delCol(col_type col)
{
	if (ncols() <= 1 || !colChangeOK())
		return;
	MathGridInset::delCol(col);
}


string MathHullInset::nicelabel(row_type row) const
{
	if (nonum_[row])
		return string();
	if (label_[row].empty())
		return string("(#)");
	return '(' + label_[row] + ')';
}


void MathHullInset::glueall()
{
	MathArray ar;
	for (idx_type i = 0; i < nargs(); ++i)
		ar.append(cell(i));
	*this = MathHullInset("simple");
	cell(0) = ar;
	setDefaults();
}


void MathHullInset::splitTo2Cols()
{
	BOOST_ASSERT(ncols() == 1);
	MathGridInset::addCol(1);
	for (row_type row = 0; row < nrows(); ++row) {
		idx_type const i = 2 * row;
		pos_type pos = firstRelOp(cell(i));
		cell(i + 1) = MathArray(cell(i).begin() + pos, cell(i).end());
		cell(i).erase(pos, cell(i).size());
	}
}


void MathHullInset::splitTo3Cols()
{
	BOOST_ASSERT(ncols() < 3);
	if (ncols() < 2)
		splitTo2Cols();
	MathGridInset::addCol(1);
	for (row_type row = 0; row < nrows(); ++row) {
		idx_type const i = 3 * row + 1;
		if (cell(i).size()) {
			cell(i + 1) = MathArray(cell(i).begin() + 1, cell(i).end());
			cell(i).erase(1, cell(i).size());
		}
	}
}


void MathHullInset::changeCols(col_type cols)
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
				MathGridInset::addCol(ncols() - 1);
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
		MathGridInset::delCol(ncols() - 1);
	}
}


string const & MathHullInset::getType() const
{
	return type_;
}


void MathHullInset::setType(string const & type)
{
	type_ = type;
	setDefaults();
}



void MathHullInset::mutate(string const & newtype)
{
	//lyxerr << "mutating from '" << type_ << "' to '" << newtype << "'" << endl;

	// we try to move along the chain
	// none <-> simple <-> equation <-> eqnarray -> *align* -> multline, gather -+
	//                                     ^                                     |
	//                                     +-------------------------------------+
	// we use eqnarray as intermediate type for mutations that are not
	// directly supported because it handles labels and numbering for
	// "down mutation".

	if (newtype == "dump") {
		dump();
	}

	else if (newtype == type_) {
		// done
	}

	else if (typecode(newtype) < 0) {
		// unknown type
	}

	else if (type_ == "none") {
		setType("simple");
		numbered(0, false);
		mutate(newtype);
	}

	else if (type_ == "simple") {
		if (newtype == "none") {
			setType("none");
			numbered(0, false);
		} else {
			setType("equation");
			numbered(0, false);
			mutate(newtype);
		}
	}

	else if (type_ == "equation") {
		if (smaller(newtype, type_)) {
			setType("simple");
			numbered(0, false);
			mutate(newtype);
		} else if (newtype == "eqnarray") {
			// split it "nicely" on the first relop
			splitTo3Cols();
			setType("eqnarray");
		} else if (newtype == "multline" || newtype == "gather") {
			setType(newtype);
		} else {
			// split it "nicely"
			splitTo2Cols();
			setType("align");
			mutate(newtype);
		}
	}

	else if (type_ == "eqnarray") {
		if (smaller(newtype, type_)) {
			// set correct (no)numbering
			bool allnonum = true;
			for (row_type row = 0; row < nrows(); ++row)
				if (!nonum_[row])
					allnonum = false;

			// set first non-empty label
			string label;
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
			setType("align");
			mutate(newtype);
		}
	}

	else if (type_ ==  "align"   || type_ == "alignat" ||
	         type_ == "xalignat" || type_ == "flalign") {
		if (smaller(newtype, "align")) {
			changeCols(3);
			setType("eqnarray");
			mutate(newtype);
		} else if (newtype == "gather" || newtype == "multline") {
			changeCols(1);
			setType(newtype);
		} else if (newtype ==   "xxalignat") {
			for (row_type row = 0; row < nrows(); ++row)
				numbered(row, false);
			setType(newtype);
		} else {
			setType(newtype);
		}
	}

	else if (type_ == "xxalignat") {
		for (row_type row = 0; row < nrows(); ++row)
			numbered(row, false);
		if (smaller(newtype, "align")) {
			changeCols(3);
			setType("eqnarray");
			mutate(newtype);
		} else if (newtype == "gather" || newtype == "multline") {
			changeCols(1);
			setType(newtype);
		} else {
			setType(newtype);
		}
	}

	else if (type_ == "multline" || type_ == "gather") {
		if (newtype == "gather" || newtype == "multline")
			setType(newtype);
		else if (newtype ==   "align"   || newtype == "flalign"  ||
		         newtype ==   "alignat" || newtype == "xalignat") {
			splitTo2Cols();
			setType(newtype);
		} else if (newtype ==   "xxalignat") {
			splitTo2Cols();
			for (row_type row = 0; row < nrows(); ++row)
				numbered(row, false);
			setType(newtype);
		} else {
			splitTo3Cols();
			setType("eqnarray");
			mutate(newtype);
		}
	}

	else {
		lyxerr << "mutation from '" << type_
		       << "' to '" << newtype << "' not implemented" << endl;
	}
}


string MathHullInset::eolString(row_type row, bool emptyline, bool fragile) const
{
	string res;
	if (numberedType()) {
		if (!label_[row].empty() && !nonum_[row])
			res += "\\label{" + label_[row] + '}';
		if (nonum_[row] && (type_ != "multline"))
			res += "\\nonumber ";
	}
	return res + MathGridInset::eolString(row, emptyline, fragile);
}


void MathHullInset::write(WriteStream & os) const
{
	header_write(os);
	MathGridInset::write(os);
	footer_write(os);
}


void MathHullInset::normalize(NormalStream & os) const
{
	os << "[formula " << type_ << ' ';
	MathGridInset::normalize(os);
	os << "] ";
}


void MathHullInset::mathmlize(MathMLStream & os) const
{
	MathGridInset::mathmlize(os);
}


void MathHullInset::infoize(ostream & os) const
{
	os << "Type: " << type_;
}


void MathHullInset::check() const
{
	BOOST_ASSERT(nonum_.size() == nrows());
	BOOST_ASSERT(label_.size() == nrows());
}


void MathHullInset::doExtern(LCursor & cur, FuncRequest & func)
{
	string lang;
	string extra;
	istringstream iss(func.argument);
	iss >> lang >> extra;
	if (extra.empty())
		extra = "noextra";

#ifdef WITH_WARNINGS
#warning temporarily disabled
	//if (cur.selection()) {
	//	MathArray ar;
	//	selGet(cur.ar);
	//	lyxerr << "use selection: " << ar << endl;
	//	insert(pipeThroughExtern(lang, extra, ar));
	//	return;
	//}
#endif

	MathArray eq;
	eq.push_back(MathAtom(new MathCharInset('=')));

	// go to first item in line
	cur.idx() -= cur.idx() % ncols();
	cur.pos() = 0;

	if (getType() == "simple") {
		size_type pos = cur.cell().find_last(eq);
		MathArray ar;
		if (cur.inMathed() && cur.selection()) {
			asArray(grabAndEraseSelection(cur), ar);
		} else if (pos == cur.cell().size()) {
			ar = cur.cell();
			lyxerr << "use whole cell: " << ar << endl;
		} else {
			ar = MathArray(cur.cell().begin() + pos + 1, cur.cell().end());
			lyxerr << "use partial cell form pos: " << pos << endl;
		}
		cur.cell().append(eq);
		cur.cell().append(pipeThroughExtern(lang, extra, ar));
		cur.pos() = cur.lastpos();
		return;
	}

	if (getType() == "equation") {
		lyxerr << "use equation inset" << endl;
		mutate("eqnarray");
		MathArray & ar = cur.cell();
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
		MathArray ar = cur.cell();
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


void MathHullInset::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "action: " << cmd.action << endl;
	switch (cmd.action) {

	case LFUN_FINISHED_LEFT:
	case LFUN_FINISHED_RIGHT:
	case LFUN_FINISHED_UP:
	case LFUN_FINISHED_DOWN:
		//lyxerr << "action: " << cmd.action << endl;
		MathGridInset::doDispatch(cur, cmd);
		notifyCursorLeaves(cur);
		cur.undispatched();
		break;

	case LFUN_BREAKPARAGRAPH:
		// just swallow this
		break;

	case LFUN_BREAKLINE:
		// some magic for the common case
		if (type_ == "simple" || type_ == "equation") {
			recordUndoInset(cur);
			bool const align =
				cur.bv().buffer()->params().use_amsmath == BufferParams::AMS_ON;
			mutate(align ? "align" : "eqnarray");
			cur.idx() = 0;
			cur.pos() = cur.lastpos();
		}
		MathGridInset::doDispatch(cur, cmd);
		break;

	case LFUN_MATH_NUMBER:
		//lyxerr << "toggling all numbers" << endl;
		if (display()) {
			recordUndoInset(cur);
			bool old = numberedType();
			if (type_ == "multline")
				numbered(nrows() - 1, !old);
			else
				for (row_type row = 0; row < nrows(); ++row)
					numbered(row, !old);
			cur.message(old ? _("No number") : _("Number"));
		}
		break;

	case LFUN_MATH_NONUMBER:
		if (display()) {
			recordUndoInset(cur);
			row_type r = (type_ == "multline") ? nrows() - 1 : cur.row();
			bool old = numbered(r);
			cur.message(old ? _("No number") : _("Number"));
			numbered(r, !old);
		}
		break;

	case LFUN_INSERT_LABEL: {
		recordUndoInset(cur);
		row_type r = (type_ == "multline") ? nrows() - 1 : cur.row();
		string old_label = label(r);
		string const default_label =
			(lyxrc.label_init_length >= 0) ? "eq:" : "";
		if (old_label.empty())
			old_label = default_label;
		string const contents = cmd.argument.empty() ?
			old_label : cmd.argument;

		InsetCommandParams p("label", contents);
		string const data = InsetCommandMailer::params2string("label", p);

		if (cmd.argument.empty()) {
			cur.bv().owner()->getDialogs().show("label", data, 0);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_INSET_INSERT: {
		//lyxerr << "arg: " << cmd.argument << endl;
		string const name = cmd.getArg(0);
		if (name == "label") {
			InsetCommandParams p;
			InsetCommandMailer::string2params(name, cmd.argument, p);
			string str = p.getContents();
			recordUndoInset(cur);
			row_type const r = (type_ == "multline") ? nrows() - 1 : cur.row();
			str = lyx::support::trim(str);
			if (!str.empty())
				numbered(r, true);
			string old = label(r);
			if (str != old) {
				cur.bv().buffer()->changeRefsIfUnique(old, str, InsetBase::REF_CODE);
				label(r, str);
			}
			break;
		}
		MathGridInset::doDispatch(cur, cmd);
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
		mutate(cmd.argument);
		cur.idx() = row * ncols() + col;
		if (cur.idx() > cur.lastidx()) {
			cur.idx() = cur.lastidx();
			cur.pos() = cur.lastpos();
		}
		if (cur.pos() > cur.lastpos())
			cur.pos() = cur.lastpos();
		//cur.dispatched(FINISHED);
		break;
	}

	case LFUN_MATH_DISPLAY: {
		recordUndoInset(cur);
		mutate(type_ == "simple" ? "equation" : "simple");
		cur.idx() = 0;
		cur.pos() = cur.lastpos();
		//cur.dispatched(FINISHED);
		break;
	}

	default:
		MathGridInset::doDispatch(cur, cmd);
		break;
	}
}


bool MathHullInset::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_FINISHED_LEFT:
	case LFUN_FINISHED_RIGHT:
	case LFUN_FINISHED_UP:
	case LFUN_FINISHED_DOWN:
		status.enabled(true);
		return true;
	case LFUN_BREAKLINE:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_EXTERN:
	case LFUN_MATH_MUTATE:
	case LFUN_MATH_DISPLAY:
		// we handle these
		status.enabled(true);
		return true;
	case LFUN_INSERT_LABEL:
		status.enabled(type_ != "simple");
		return true;
	case LFUN_INSET_INSERT:
		if (cmd.getArg(0) == "label") {
			status.enabled(type_ != "simple");
			return true;
		}
		return MathGridInset::getStatus(cur, cmd, status);
	case LFUN_TABULAR_FEATURE: {
		istringstream is(cmd.argument);
		string s;
		is >> s;
		if (!rowChangeOK()
		    && (s == "append-row"
			|| s == "delete-row"
			|| s == "copy-row")) {
			status.message(bformat(
				N_("Can't change number of rows in '%1$s'"),
				type_));
			status.enabled(false);
			return true;
		}
		if (!colChangeOK()
		    && (s == "append-column"
			|| s == "delete-column"
			|| s == "copy-column")) {
			status.message(bformat(
				N_("Can't change number of columns in '%1$s'"),
				type_));
			status.enabled(false);
			return true;
		}
		if ((type_ == "simple"
		  || type_ == "equation"
		  || type_ == "none") &&
		    (s == "add-hline-above" || s == "add-hline-below")) {
			status.message(bformat(
				N_("Can't add horizontal grid lines in '%1$s'"),
				type_));
			status.enabled(false);
			return true;
		}
		if (s == "add-vline-left" || s == "add-vline-right") {
			status.message(bformat(
				N_("Can't add vertical grid lines in '%1$s'"),
				type_));
			status.enabled(false);
			return true;
		}
		if (s == "valign-top" || s == "valign-middle"
		 || s == "valign-bottom" || s == "align-left"
		 || s == "align-center" || s == "align-right") {
			status.enabled(false);
			return true;
		}
		return MathGridInset::getStatus(cur, cmd, status);
	}
	default:
		return MathGridInset::getStatus(cur, cmd, status);
	}

	// This cannot really happen, but inserted to shut-up gcc
	return MathGridInset::getStatus(cur, cmd, status);
}


/////////////////////////////////////////////////////////////////////

#include "math_arrayinset.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_parser.h"
#include "math_spaceinset.h"
#include "ref_inset.h"

#include "bufferview_funcs.h"
#include "lyxtext.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

#include "support/lyxlib.h"


// simply scrap this function if you want
void MathHullInset::mutateToText()
{
#if 0
	// translate to latex
	ostringstream os;
	latex(NULL, os, false, false);
	string str = os.str();

	// insert this text
	LyXText * lt = view_->getLyXText();
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit)
		view_->owner()->getIntl()->getTransManager().TranslateAndInsert(*cit, lt);

	// remove ourselves
	//view_->owner()->dispatch(LFUN_ESCAPE);
#endif
}


void MathHullInset::handleFont(LCursor & cur, string const & arg,
	string const & font)
{
	// this whole function is a hack and won't work for incremental font
	// changes...
	recordUndo(cur);
	if (cur.inset().asMathInset()->name() == font)
		cur.handleFont(font);
	else {
		cur.handleNest(createMathInset(font));
		cur.insert(arg);
	}
}


void MathHullInset::handleFont2(LCursor & cur, string const & arg)
{
	recordUndo(cur);
	LyXFont font;
	bool b;
	bv_funcs::string2font(arg, font, b);
	if (font.color() != LColor::inherit) {
		MathAtom at = MathAtom(new MathColorInset(true, font.color()));
		cur.handleNest(at, 0);
	}
}


void MathHullInset::edit(LCursor & cur, bool left)
{
	cur.push(*this);
	left ? idxFirst(cur) : idxLast(cur);
}


string const MathHullInset::editMessage() const
{
	return _("Math editor mode");
}


void MathHullInset::revealCodes(LCursor & cur) const
{
	if (!cur.inMathed())
		return;
	ostringstream os;
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


InsetBase::Code MathHullInset::lyxCode() const
{
	return MATH_CODE;
}


/////////////////////////////////////////////////////////////////////


#if 0
bool MathHullInset::searchForward(BufferView * bv, string const & str,
				     bool, bool)
{
#ifdef WITH_WARNINGS
#warning completely broken
#endif
	static MathHullInset * lastformula = 0;
	static CursorBase current = DocIterator(ibegin(nucleus()));
	static MathArray ar;
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
		MathArray const & a = top.asMathInset()->cell(top.idx_);
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


void MathHullInset::write(Buffer const &, std::ostream & os) const
{
	WriteStream wi(os, false, false);
	os << "Formula ";
	write(wi);
}


void MathHullInset::read(Buffer const &, LyXLex & lex)
{
	MathAtom at;
	mathed_parse_normal(at, lex);
	operator=(*at->asHullInset());
}


int MathHullInset::plaintext(Buffer const &, ostream & os,
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
		WriteStream wi(os, false, true);
		wi << cell(0);
		return wi.line();
	}
}


int MathHullInset::linuxdoc(Buffer const & buf, ostream & os,
			   OutputParams const & runparams) const
{
	return docbook(buf, os, runparams);
}


int MathHullInset::docbook(Buffer const & buf, ostream & os,
			  OutputParams const & runparams) const
{
	MathMLStream ms(os);
	int res = 0;
	string name;
	if (getType() == "simple")
		name= "inlineequation";
	else
		name = "informalequation";

	string bname = name;
	if (!label(0).empty())
		bname += " id=\"" + sgml::cleanID(buf, runparams, label(0)) + "\"";
	ms << MTag(bname.c_str());

	ostringstream ls;
	if (runparams.flavor == OutputParams::XML) {
		ms << MTag("alt role=\"tex\" ");
		// Workaround for db2latex: db2latex always includes equations with
		// \ensuremath{} or \begin{display}\end{display}
		// so we strip LyX' math environment
		WriteStream wi(ls, false, false);
		MathGridInset::write(wi);
		ms << subst(subst(ls.str(), "&", "&amp;"), "<", "&lt;");
		ms << ETag("alt");
		ms << MTag("math");
		MathGridInset::mathmlize(ms);
		ms << ETag("math");
	} else {
		ms << MTag("alt role=\"tex\"");
		res = latex(buf, ls, runparams);
		ms << subst(subst(ls.str(), "&", "&amp;"), "<", "&lt;");
		ms << ETag("alt");
	}

	ms <<  "<graphic fileref=\"eqn/";
	if ( !label(0).empty())
		ms << sgml::cleanID(buf, runparams, label(0));
	else
		ms << sgml::uniqueID("anon");

	if (runparams.flavor == OutputParams::XML)
		ms << "\"/>";
	else
		ms << "\">";

	ms << ETag(name.c_str());
	return ms.line() + res;
}


int MathHullInset::textString(Buffer const & buf, ostream & os,
		       OutputParams const & op) const
{
	return plaintext(buf, os, op);
}
