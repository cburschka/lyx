#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "debug.h"
#include "Painter.h"
#include "LaTeXFeatures.h"
#include "support/LAssert.h"

#include <vector>

using std::endl;

namespace {

	int getCols(MathInsetTypes type)
	{
		switch (type) {
			case LM_OT_EQNARRAY:
				return 3;
			case LM_OT_ALIGN:
			case LM_OT_ALIGNAT:
			case LM_OT_XALIGNAT:
			case LM_OT_XXALIGNAT:
				return 2;
			default:;
		}
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

	MathInsetTypes typecode(string const & s)
	{
		if (s == "equation")  return LM_OT_EQUATION;
		if (s == "display")   return LM_OT_EQUATION;
		if (s == "eqnarray")  return LM_OT_EQNARRAY;
		if (s == "align")     return LM_OT_ALIGN;
		if (s == "alignat")   return LM_OT_ALIGNAT;
		if (s == "xalignat")  return LM_OT_XALIGNAT;
		if (s == "xxalignat") return LM_OT_XXALIGNAT;
		if (s == "multline")  return LM_OT_MULTLINE;
		if (s == "gather")    return LM_OT_GATHER;
		return LM_OT_SIMPLE;
	}	


	string normalName(MathInsetTypes t)
	{
		switch (t) {
			case LM_OT_EQUATION:  return "equation";
			case LM_OT_EQNARRAY:  return "eqnarray";
			case LM_OT_ALIGN:     return "align";
			case LM_OT_ALIGNAT:   return "alignat";
			case LM_OT_XALIGNAT:  return "xalignat";
			case LM_OT_XXALIGNAT: return "xxalignat";
			case LM_OT_MULTLINE:  return "multline";
			case LM_OT_GATHER:    return "gather";
			case LM_OT_SIMPLE:    return "simple";
			default: break;
		}
		return "unknown";
	}	

} // end anon namespace


MathHullInset::MathHullInset()
	: MathGridInset(1, 1), objtype_(LM_OT_SIMPLE), nonum_(1), label_(1)
{
	setDefaults();
}


MathHullInset::MathHullInset(MathInsetTypes t)
	: MathGridInset(getCols(t), 1), objtype_(t), nonum_(1), label_(1)
{
	setDefaults();
}


MathHullInset::MathHullInset(MathInsetTypes t, col_type cols)
	: MathGridInset(cols, 1), objtype_(t), nonum_(1), label_(1)
{
	setDefaults();
}


MathInset * MathHullInset::clone() const
{
	return new MathHullInset(*this);
}


char MathHullInset::defaultColAlign(col_type col)
{
	switch (getType()) {
		case LM_OT_ALIGN:
		case LM_OT_ALIGNAT:
		case LM_OT_XALIGNAT:
		case LM_OT_XXALIGNAT:
			return "rl"[col & 1];
		case LM_OT_EQNARRAY:
			return "rcl"[col];
		default:;
	}
	return 'c';
}


int MathHullInset::defaultColSpace(col_type col)
{
	switch (getType()) {
		case LM_OT_ALIGN:
		case LM_OT_ALIGNAT:
			return 0;
		case LM_OT_XALIGNAT:
			return (col & 1) ? 20 : 0;
		case LM_OT_XXALIGNAT:
			return (col & 1) ? 40 : 0;
		default:;
	}
	return 0;
}


void MathHullInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	mi_.style = (getType() == LM_OT_SIMPLE) ? LM_ST_TEXT : LM_ST_DISPLAY;

	// let the cells adjust themselves
	MathGridInset::metrics(mi_);

	if (display()) {
		ascent_  += 12;
		descent_ += 12;
	}	

	if (numberedType()) {
		int l = 0;
		for (row_type row = 0; row < nrows(); ++row)
			l = std::max(l, mathed_string_width(LM_TC_BF, mi_, nicelabel(row)));

		if (l)
			width_ += 30 + l;
	}

	// make it at least as high as the current font
	int asc = 0;
	int des = 0;
	math_font_max_dim(LM_TC_TEXTRM, mi_, asc, des);
	ascent_  = std::max(ascent_,  asc);
	descent_ = std::max(descent_, des);
}


void MathHullInset::draw(Painter & pain, int x, int y) const
{
	MathGridInset::draw(pain, x, y);

	if (numberedType()) {
		int const xx = x + colinfo_.back().offset_ + colinfo_.back().width_ + 20;
		for (row_type row = 0; row < nrows(); ++row) {
			int const yy = y + rowinfo_[row].offset_;
			drawStr(pain, LM_TC_BF, mi_, xx, yy, nicelabel(row));
		}
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
	return true;
/*
	return 
		objtype_ == LM_OT_ALIGN ||
		objtype_ == LM_OT_MULTLINE ||
		objtype_ == LM_OT_GATHER ||
		objtype_ == LM_OT_ALIGNAT ||
		objtype_ == LM_OT_XALIGNAT ||
		objtype_ == LM_OT_XXALIGNAT;
*/
}


bool MathHullInset::display() const
{
	return getType() != LM_OT_SIMPLE;
}


std::vector<string> const MathHullInset::getLabelList() const
{
	std::vector<string> res;
	for (row_type row = 0; row < nrows(); ++row)
		if (!label_[row].empty() && nonum_[row] != 1)
			res.push_back(label_[row]);
	return res;
}


bool MathHullInset::numberedType() const
{
	if (getType() == LM_OT_SIMPLE || getType() == LM_OT_XXALIGNAT)
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

	switch (getType()) {
		case LM_OT_SIMPLE:
			os << '$';
			if (cell(0).empty())
				os << ' ';
			break;

		case LM_OT_EQUATION:
			if (n)
				os << "\\begin{equation" << star(n) << "}\n"; 
			else
				os << "\\[\n"; 
			break;

		case LM_OT_EQNARRAY:
			os << "\\begin{eqnarray" << star(n) << "}\n";
			break;

		case LM_OT_ALIGN:
			os << "\\begin{align" << star(n) << "}\n";
			break;

		case LM_OT_ALIGNAT:
			os << "\\begin{alignat" << star(n) << "}"
			  << "{" << static_cast<unsigned int>(ncols()/2) << "}\n";
			break;

		case LM_OT_XALIGNAT:
			os << "\\begin{xalignat" << star(n) << "}"
			   << "{" << static_cast<unsigned int>(ncols()/2) << "}\n";
			break;

		case LM_OT_XXALIGNAT:
			os << "\\begin{xxalignat}" 
			   << "{" << static_cast<unsigned int>(ncols()/2) << "}\n";
			break;

		case LM_OT_MULTLINE:
			os << "\\begin{multline}\n";
			break;

		case LM_OT_GATHER:
			os << "\\begin{gather}\n";
			break;

		default:
			os << "\\begin{unknown" << star(n) << "}";
	}
}


void MathHullInset::footer_write(WriteStream & os) const
{
	bool n = numberedType();

	switch (getType()) {
		case LM_OT_SIMPLE:
			os << '$';
			break;

		case LM_OT_EQUATION:
			if (n)
				os << "\\end{equation" << star(n) << "}\n"; 
			else
				os << "\\]\n"; 
			break;

		case LM_OT_EQNARRAY:
			os << "\n\\end{eqnarray" << star(n) << "}\n";
			break;

		case LM_OT_ALIGN:
			os << "\n\\end{align" << star(n) << "}\n";
			break;

		case LM_OT_ALIGNAT:
			os << "\n\\end{alignat" << star(n) << "}\n";
			break;

		case LM_OT_XALIGNAT:
			os << "\n\\end{xalignat" << star(n) << "}\n";
			break;

		case LM_OT_XXALIGNAT:
			os << "\n\\end{xxalignat}\n";
			break;

		case LM_OT_MULTLINE:
			os << "\n\\end{multline}\n";
			break;

		case LM_OT_GATHER:
			os << "\n\\end{gather}\n";
			break;

		default:
			os << "\\end{unknown" << star(n) << "}";
	}
}


void MathHullInset::addRow(row_type row) 
{
	nonum_.insert(nonum_.begin() + row + 1, !numberedType());
	label_.insert(label_.begin() + row + 1, string());
	MathGridInset::addRow(row);
}


void MathHullInset::appendRow()
{
	nonum_.push_back(!numberedType());
	label_.push_back(string());
	MathGridInset::appendRow();
}


void MathHullInset::delRow(row_type row) 
{
	MathGridInset::delRow(row);
	nonum_.erase(nonum_.begin() + row);
	label_.erase(label_.begin() + row);
}


void MathHullInset::addCol(col_type col)
{
	switch (getType()) {
		case LM_OT_EQUATION:
			mutate(LM_OT_EQNARRAY);
			break;

		case LM_OT_EQNARRAY:
			mutate(LM_OT_ALIGN);
			addCol(col);
			break;

		case LM_OT_ALIGN:
			mutate(LM_OT_ALIGNAT);
			addCol(col);
			break;

		case LM_OT_ALIGNAT:
		case LM_OT_XALIGNAT:
		case LM_OT_XXALIGNAT:
			MathGridInset::addCol(col);
			MathGridInset::addCol(col + 1);
			break;

		default:
			break;
	}
}


void MathHullInset::delCol(col_type col)
{
	switch (getType()) {
		case LM_OT_ALIGNAT:
		case LM_OT_XALIGNAT:
		case LM_OT_XXALIGNAT:
			MathGridInset::delCol(col + 1);
			MathGridInset::delCol(col);
			break;
		default:
			break;
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


void MathHullInset::mutate(string const & newtype)
{
	if (newtype == "dump") {
		dump();
		return;
	}
	//lyxerr << "mutating from '" << getType() << "' to '" << newtype << "'\n";
	mutate(typecode(newtype));
}


void MathHullInset::glueall()
{
	MathArray ar;
	for (idx_type i = 0; i < nargs(); ++i)
		ar.push_back(cell(i));
	*this = MathHullInset(LM_OT_SIMPLE);
	cell(0) = ar;
}


MathInsetTypes MathHullInset::getType() const
{
	return objtype_;
}


void MathHullInset::setType(MathInsetTypes t)
{
	objtype_ = t;
	setDefaults();
}



void MathHullInset::mutate(MathInsetTypes newtype)
{
	//lyxerr << "mutating from '" << getType() << "' to '" << newtype << "'\n";

	if (newtype == getType())
		return;

	switch (getType()) {
		case LM_OT_SIMPLE:
			setType(LM_OT_EQUATION);
			numbered(0, false);
			mutate(newtype);
			break;

		case LM_OT_EQUATION:
			switch (newtype) {
				case LM_OT_SIMPLE:
					setType(LM_OT_SIMPLE);
					break;

				case LM_OT_ALIGN: 
				case LM_OT_ALIGNAT:
				case LM_OT_XALIGNAT:
				case LM_OT_XXALIGNAT: {

					MathGridInset::addCol(1);

					// split it "nicely"
					pos_type pos = firstRelOp(cell(0));	
					cell(1) = cell(0);
					cell(0).erase(pos, cell(0).size());
					cell(1).erase(0, pos);
					setType(LM_OT_ALIGN);
					mutate(newtype);
					break;
				}

				case LM_OT_EQNARRAY:
				default:
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

					setType(LM_OT_EQNARRAY);
					mutate(newtype);
					break;
				}
			break;

		case LM_OT_EQNARRAY:
			switch (newtype) {
				case LM_OT_SIMPLE:
				case LM_OT_EQUATION: {
					// set correct (no)numbering
					bool allnonum = true;
					for (row_type row = 0; row < nrows(); ++row) {
						if (!nonum_[row])
							allnonum = false;
					}

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
					break;
				}

				case LM_OT_ALIGN:
				case LM_OT_ALIGNAT:
				case LM_OT_XALIGNAT:
				case LM_OT_XXALIGNAT:
				default: {
					for (row_type row = 0; row < nrows(); ++row) {
						idx_type c = 3 * row + 1;
						cell(c).push_back(cell(c + 1));
					}
					MathGridInset::delCol(2);
					setType(LM_OT_ALIGN);
					mutate(newtype);
					break;
				}
			}
			break;

		case LM_OT_ALIGN:
			switch (newtype) {
				case LM_OT_SIMPLE:
				case LM_OT_EQUATION:
				case LM_OT_EQNARRAY:
					MathGridInset::addCol(1);
					setType(LM_OT_EQNARRAY);
					mutate(newtype);
					break;
				
				case LM_OT_ALIGNAT:
				case LM_OT_XALIGNAT:
				case LM_OT_XXALIGNAT:
					setType(newtype);
					break;

				default:
					lyxerr << "mutation from '" << getType()
						<< "' to '" << newtype << "' not implemented"
					       << endl;
					break;
			}
			break;

		case LM_OT_MULTLINE:
			switch (newtype) {
				case LM_OT_GATHER:
					setType(LM_OT_GATHER);
					break;
				default:
					lyxerr << "mutation from '" << getType()
						<< "' to '" << newtype << "' not implemented"
					       << endl;
					break;
			}

		case LM_OT_GATHER:
			switch (newtype) {
				case LM_OT_MULTLINE:
					setType(LM_OT_MULTLINE);
					break;
				default:
					lyxerr << "mutation from '" << getType()
						<< "' to '" << newtype << "' not implemented"
					       << endl;
					break;
			}

		default:
			lyxerr << "mutation from '" << getType()
			       << "' to '" << newtype << "' not implemented"
			       << endl;
			break;
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
	os << "[formula " << normalName(getType()) << " ";
	MathGridInset::normalize(os);
	os << "] ";
}


void MathHullInset::mathmlize(MathMLStream & os) const
{
	MathGridInset::mathmlize(os);
}


void MathHullInset::check() const
{
	lyx::Assert(nonum_.size() == nrows());
	lyx::Assert(label_.size() == nrows());
}


