#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "debug.h"
#include "frontends/Painter.h"
#include "textpainter.h"
#include "Lsstream.h"
#include "LaTeXFeatures.h"
#include "support/LAssert.h"

#include <vector>

using std::vector;
using std::max;
using std::endl;

namespace {

	int getCols(string const & type)
	{
		if (type == "eqnarray")
			return 3;
		if (type == "align")
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
	MathArray::size_type firstRelOp(MathArray const & ar)
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
		lyxerr << "unknown hull type '" << s << "'\n";
		return 0;
	}

	bool smaller(string const & s, string const & t)
	{
		return typecode(s) < typecode(t);
	}


} // end anon namespace


MathHullInset::MathHullInset()
	: MathGridInset(1, 1), type_("none"), nonum_(1), label_(1)
{
	setDefaults();
}


MathHullInset::MathHullInset(string const & type)
	: MathGridInset(getCols(type), 1), type_(type), nonum_(1), label_(1)
{
	setDefaults();
}


MathHullInset::MathHullInset(string const & type, MathGridInset const & grid)
	: MathGridInset(grid), type_(type), nonum_(1), label_(1)
{
	setDefaults();
}


MathHullInset::MathHullInset(string const & type, col_type cols)
	: MathGridInset(cols, 1), type_(type), nonum_(1), label_(1)
{
	setDefaults();
}


MathInset * MathHullInset::clone() const
{
	return new MathHullInset(*this);
}


bool MathHullInset::idxFirst(idx_type & idx, pos_type & pos) const
{
	idx = 0;
	pos = 0;
	return true;
}


bool MathHullInset::idxLast(idx_type & idx, pos_type & pos) const
{
	idx = nargs() - 1;
	pos = cell(idx).size();
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
	if (type_ == "xxalignat")
		return (col & 1) ? 40 : 0;
	return 0;
}


char const * MathHullInset::standardFont() const
{
	if (type_ == "none")
		return "lyxnochange";
	return "mathnormal";
}


void MathHullInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, standardFont());

	// let the cells adjust themselves
	MathGridInset::metrics(mi);

	if (display()) {
		dim_.a += 12;
		dim_.d += 12;
	}

	if (numberedType()) {
		MathFontSetChanger dummy(mi.base, "mathbf");
		int l = 0;
		for (row_type row = 0; row < nrows(); ++row)
			l = max(l, mathed_string_width(mi.base.font, nicelabel(row)));

		if (l)
			dim_.w += 30 + l;
	}

	// make it at least as high as the current font
	int asc = 0;
	int des = 0;
	math_font_max_dim(mi.base.font, asc, des);
	dim_.a = max(dim_.a,  asc);
	dim_.d = max(dim_.d, des);

	// for markers
	metricsMarkers2();
}


void MathHullInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, standardFont());
	MathGridInset::draw(pi, x + 1, y);

	if (numberedType()) {
		int const xx = x + colinfo_.back().offset_ + colinfo_.back().width_ + 20;
		for (row_type row = 0; row < nrows(); ++row) {
			int const yy = y + rowinfo_[row].offset_;
			MathFontSetChanger dummy(pi.base, "mathrm");
			drawStr(pi, pi.base.font, xx, yy, nicelabel(row));
		}
	}

	drawMarkers2(pi, x, y);
}


void MathHullInset::metricsT(TextMetricsInfo const & mi) const
{
	if (display()) {
		MathGridInset::metricsT(mi);
	} else {
		ostringstream os;
		WriteStream wi(os, false, true);
		write(wi);
		dim_.w = os.str().size();
		dim_.a = 1;
		dim_.d = 0;
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


string MathHullInset::label(row_type row) const
{
	row_type n = nrows();
	lyx::Assert(row < n);
	return label_[row];
}


void MathHullInset::label(row_type row, string const & label)
{
	lyxerr << "setting label '" << label << "' for row " << row << endl;
	label_[row] = label;
}


void MathHullInset::numbered(row_type row, bool num)
{
	nonum_[row] = !num;
}


bool MathHullInset::numbered(row_type row) const
{
	return !nonum_[row];
}


bool MathHullInset::ams() const
{
	return
		type_ == "align" ||
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


vector<string> MathHullInset::getLabelList() const
{
	vector<string> res;
	for (row_type row = 0; row < nrows(); ++row)
		if (!label_[row].empty() && nonum_[row] != 1)
			res.push_back(label_[row]);
	return res;
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

	MathNestInset::validate(features);
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

	else if (type_ == "eqnarray" || type_ == "align")
			os << "\\begin{" << type_ << star(n) << "}\n";

	else if (type_ == "alignat" || type_ == "xalignat") 
		os << "\\begin{" << type_ << star(n) << "}"
		  << "{" << static_cast<unsigned int>(ncols()/2) << "}\n";

	else if (type_ == "xxalignat" || type_ == "multline" || type_ == "gather") 
		os << "\\begin{" << type_ << "}\n";

	else 
		os << "\\begin{unknown" << star(n) << "}";
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

	else if (type_ == "eqnarray" || type_ == "align" || type_ == "alignat"
	      || type_ == "xalignat")
		os << "\n\\end{" << type_ << star(n) << "}\n";

	else if (type_ == "xxalignat" || type_ == "multline" || type_ == "gather")
		os << "\n\\end{" << type_ << "}\n";

	else
		os << "\\end{unknown" << star(n) << "}";
}


void MathHullInset::addRow(row_type row)
{
	nonum_.insert(nonum_.begin() + row + 1, !numberedType());
	label_.insert(label_.begin() + row + 1, string());
	MathGridInset::addRow(row);
}


void MathHullInset::delRow(row_type row)
{
	MathGridInset::delRow(row);
	nonum_.erase(nonum_.begin() + row);
	label_.erase(label_.begin() + row);
}


void MathHullInset::addFancyCol(col_type col)
{
	if (type_ == "equation")
		mutate("eqnarray");
	
	else if (type_ == "eqnarray") {
		mutate("align");
		addFancyCol(col);
	}

	else if (type_ == "align") {
		mutate("alignat");
		addFancyCol(col);
	}

	else if (type_ == "alignat" || type_ == "xalignat" || type_ == "xxalignat") {
		MathGridInset::addCol(col);
		MathGridInset::addCol(col + 1);
	}
}


void MathHullInset::delFancyCol(col_type col)
{
	if (type_ == "alignat" || type_ == "xalignat" || type_ == "xxalignat") {
		MathGridInset::delCol(col + 1);
		MathGridInset::delCol(col);
	}
}


string MathHullInset::nicelabel(row_type row) const
{
	if (nonum_[row])
		return string();
	if (label_[row].empty())
		return string("(#)");
	return "(" + label_[row] + ")";
}


void MathHullInset::glueall()
{
	MathArray ar;
	for (idx_type i = 0; i < nargs(); ++i)
		ar.push_back(cell(i));
	*this = MathHullInset("simple");
	cell(0) = ar;
	setDefaults();
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
	//lyxerr << "mutating from '" << type_ << "' to '" << newtype << "'\n";

	// we try to move along the chain
	// none <-> simple <-> equation <-> eqnarray 

	if (newtype == "dump") {
		dump();
	}

	else if (newtype == type_) {
		// done
	}

	else if (type_ == "none") {
		setType("simple");
		numbered(0, false);
		mutate(newtype);
	}

	else if (type_ == "simple") {
		if (newtype == "none") {
			setType("none");
		} else {
			setType("equation");
			numbered(0, false);
			mutate(newtype);
		}
	}

	else if (type_ == "equation") {
		if (smaller(newtype, type_)) {
			setType("simple");
			mutate(newtype);
		} else if (newtype == "eqnarray") {
			MathGridInset::addCol(1);
			MathGridInset::addCol(1);

			// split it "nicely" on the firest relop
			pos_type pos = firstRelOp(cell(0));
			cell(1) = MathArray(cell(0), pos, cell(0).size());
			cell(0).erase(pos, cell(0).size());

			if (cell(1).size()) {
				cell(2) = MathArray(cell(1), 1, cell(1).size());
				cell(1).erase(1, cell(1).size());
			}
			setType("eqnarray");
			mutate(newtype);
		} else {
			MathGridInset::addCol(1);
			// split it "nicely"
			pos_type pos = firstRelOp(cell(0));
			cell(1) = cell(0);
			cell(0).erase(pos, cell(0).size());
			cell(1).erase(0, pos);
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
			for (row_type row = 0; row < nrows(); ++row) {
				idx_type c = 3 * row + 1;
				cell(c).push_back(cell(c + 1));
			}
			MathGridInset::delCol(2);
			setType("align");
			mutate(newtype);
		}
	}

	else if (type_ == "align") {
		if (smaller(newtype, type_)) {
			MathGridInset::addCol(1);
			setType("eqnarray");
			mutate(newtype);
		} else {
			setType(newtype);
		}
	}

	else if (type_ == "multline") {
		if (newtype == "gather") {
			setType("gather");
		} else {
			lyxerr << "mutation from '" << type_
				<< "' to '" << newtype << "' not implemented"
						 << endl;
		}
	}

	else if (type_ == "gather") {
		if (newtype == "multline") {
			setType("multline");
		} else {
			lyxerr << "mutation from '" << type_
				<< "' to '" << newtype << "' not implemented" << endl;
		}
	}

	else {
		lyxerr << "mutation from '" << type_
					 << "' to '" << newtype << "' not implemented" << endl;
	}
}


void MathHullInset::write(WriteStream & os) const
{
	header_write(os);

	bool n = numberedType();

	for (row_type row = 0; row < nrows(); ++row) {
		for (col_type col = 0; col < ncols(); ++col)
			os << cell(index(row, col)) << eocString(col);
		if (n) {
			if (!label_[row].empty())
				os << "\\label{" << label_[row] << "}";
			if (nonum_[row])
				os << "\\nonumber ";
		}
		os << eolString(row);
	}

	footer_write(os);
}


void MathHullInset::normalize(NormalStream & os) const
{
	os << "[formula " << type_ << " ";
	MathGridInset::normalize(os);
	os << "] ";
}


void MathHullInset::mathmlize(MathMLStream & os) const
{
	MathGridInset::mathmlize(os);
}


void MathHullInset::infoize(std::ostream & os) const
{
	os << "Type: " << type_;
}


void MathHullInset::check() const
{
	lyx::Assert(nonum_.size() == nrows());
	lyx::Assert(label_.size() == nrows());
}
