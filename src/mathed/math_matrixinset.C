#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <vector>
#include <functional>

#include "math_matrixinset.h"
#include "debug.h"
#include "support/LOstream.h"
#include "Painter.h"
#include "LaTeXFeatures.h"


LyXFont WhichFont(short type, int size);

namespace {

string const getAlign(short int type, int cols)
{
	string align;
	switch (type) {
		case LM_OT_ALIGN:
			for (int i = 0; i < cols; ++i)
				align += "Rl";
			break;

		case LM_OT_ALIGNAT:
			for (int i = 0; i < cols; ++i)
				align += "rl";
			break;

		case LM_OT_MULTLINE:
			align = "C";
			break;

		default:
			align = "rcl";
			break;
	}
	return align;
}


string const star(bool n)
{
	return n ? "" : "*";
}


int getCols(short int type)
{
	int col;
	switch (type) {
		case LM_OT_EQNARRAY:
			col = 3;
			break;

		case LM_OT_ALIGN:
		case LM_OT_ALIGNAT:
			col = 2;
			break;

		default:
			col = 1;
	}
	return col;
}

}

MathMatrixInset::MathMatrixInset(MathInsetTypes t)
	: MathGridInset(getCols(t), 1, "formula", t), nonum_(1), label_(1)
{}


MathMatrixInset::MathMatrixInset()
	: MathGridInset(1, 1, "formula", LM_OT_SIMPLE), nonum_(1), label_(1)
{}

MathInset * MathMatrixInset::clone() const
{
	return new MathMatrixInset(*this);
}


void MathMatrixInset::Metrics(MathStyles /* st */)
{
	size_ = (GetType() == LM_OT_SIMPLE) ? LM_ST_TEXT : LM_ST_DISPLAY;

	//LyXFont wfont = WhichFont(LM_TC_BF, size());
	//wfont.setLatex(LyXFont::OFF);

	// let the cells adjust themselves
	MathGridInset::Metrics(size_);

	if (display()) {
		ascent_  += 12;
		descent_ += 12;
	}	

	if (numberedType()) {
		int l = 0;
		for (int row = 0; row < nrows(); ++row)
			l = std::max(l, mathed_string_width(LM_TC_TEXTRM, size(), nicelabel(row)));

		if (l)
			width_ += 30 + l;
	}
}


void MathMatrixInset::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);

	MathGridInset::draw(pain, x, y);

	if (numberedType()) {
		LyXFont wfont = WhichFont(LM_TC_BF, size());
#ifndef NO_LATEX
		wfont.setLatex(LyXFont::OFF);
#endif
		int xx = x + colinfo_.back().offset_ + colinfo_.back().width_ + 20;
		for (int row = 0; row < nrows(); ++row) 
			pain.text(xx, y + rowinfo_[row].offset_, nicelabel(row), wfont);
	}
}


void MathMatrixInset::Write(std::ostream & os, bool fragile) const
{
  header_write(os);

	bool n = numberedType();

	for (int row = 0; row < nrows(); ++row) {
		if (row)
			os << " \\\\\n";
		for (int col = 0; col < ncols(); ++col) {
			if (col)
				os << " & ";
			cell(index(row, col)).Write(os, fragile);
		}
		if (n) {
			if (!label_[row].empty())
				os << "\\label{" << label_[row] << "}";
			if (nonum_[row])
				os << "\\nonumber ";
		}
	}

  footer_write(os);
}


string MathMatrixInset::label(int row) const
{
	return label_[row];
}

void MathMatrixInset::label(int row, string const & label)
{
	label_[row] = label; 
}


void MathMatrixInset::numbered(int row, bool num)
{
	nonum_[row] = !num; 
}


bool MathMatrixInset::numbered(int row) const
{
	return !nonum_[row];
}


bool MathMatrixInset::ams() const
{
	return true;
}


bool MathMatrixInset::display() const
{
	return GetType() != LM_OT_SIMPLE;
}


std::vector<string> const MathMatrixInset::getLabelList() const
{
	std::vector<string> res;
	for (int row = 0; row < nrows(); ++row)
		if (!label_[row].empty() && nonum_[row] != 1)
			res.push_back(label_[row]);
	return res;
}


bool MathMatrixInset::numberedType() const
{
	if (GetType() == LM_OT_SIMPLE)
		return false;
	for (int row = 0; row < nrows(); ++row)
		if (!nonum_[row])
			return true;
	return false;
}


void MathMatrixInset::Validate(LaTeXFeatures & features)
{
	features.amsstyle = ams();

	// Validation is necessary only if not using AMS math.
	// To be safe, we will always run mathedValidate.
	//if (features.amsstyle)
	//  return;

	//Validate1(features);

	features.boldsymbol = true;
	features.binom      = true;
}

/*
void MathMatrixInset::Validate1(LaTeXFeatures & features)
{
	MathIter it(cell());

	while (it.OK() && !(features.binom && features.boldsymbol)) {
		MathInset * p = it.GetInset();
		if (p) {
			p = it.GetActiveInset();
			if (p) {
				if (!features.binom && p->GetType() == LM_OT_MACRO &&
				    p->name() == "binom") {
					features.binom = true;
				} else {
					for (int i = 0; i <= p->getMaxArgumentIdx(); ++i) {
						p->setArgumentIdx(i);
						Validate1(features, p);
					}
				}
			} else {
				if (!features.boldsymbol && p->name() == "boldsymbol") 
					features.boldsymbol = true;
			}
		}
		it.Next();
	}
}
*/


void MathMatrixInset::header_write(std::ostream & os) const
{
	bool n = numberedType();

	switch (GetType()) {
		case LM_OT_SIMPLE:
			os << "\\("; 
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
			os << "\\begin{align" << star(n) << "}";
			break;

		case LM_OT_ALIGNAT:
			os << "\\begin{alignat" << star(n) << "}"
			   << "{" << ncols()/2 << "}\n";
			break;
		default:
			os << "\\begin{unknown" << star(n) << "}";
	}
}


void MathMatrixInset::footer_write(std::ostream & os) const
{
	bool n = numberedType();

	switch (GetType()) {
		case LM_OT_SIMPLE:
			os << "\\)";
			break;

		case LM_OT_EQUATION:
			if (n)
				os << "\\end{equation" << star(n) << "}\n"; 
			else
				os << "\\]\n"; 
			break;

		case LM_OT_EQNARRAY:
			os << "\\end{eqnarray" << star(n) << "}\n";
			break;

		case LM_OT_ALIGN:
			os << "\\end{align" << star(n) << "}\n";
			break;

		case LM_OT_ALIGNAT:
			os << "\\end{alignat" << star(n) << "}\n";
			break;

		default:
			os << "\\end{unknown" << star(n) << "}";
	}
}


void MathMatrixInset::addRow(int row) 
{
	nonum_.insert(nonum_.begin() + row + 1, !numberedType());
	label_.insert(label_.begin() + row + 1, string());
	MathGridInset::addRow(row);
}

void MathMatrixInset::appendRow()
{
	nonum_.push_back(!numberedType());
	label_.push_back(string());
	MathGridInset::appendRow();
}


void MathMatrixInset::delRow(int row) 
{
	MathGridInset::delRow(row);
	nonum_.erase(nonum_.begin() + row);
	label_.erase(label_.begin() + row);
}

void MathMatrixInset::addCol(int col)
{
	switch (GetType()) {
		case LM_OT_EQUATION:
			mutate(LM_OT_EQNARRAY);
			break;

		case LM_OT_EQNARRAY:
			mutate(LM_OT_ALIGN);
			addCol(col);
			break;

		case LM_OT_ALIGN:
		case LM_OT_ALIGNAT:
			MathGridInset::addCol(col);
			halign(col, 'l');
			MathGridInset::addCol(col);
			halign(col, 'r');
			break;

		default:
			break;
	}
}

void MathMatrixInset::delCol(int col)
{
	switch (GetType()) {
		case LM_OT_ALIGN:
			MathGridInset::delCol(col);
			break;

		default:
			break;
	}
}


string MathMatrixInset::nicelabel(int row) const
{
	if (nonum_[row])
		return string();
	if (label_[row].empty())
		return string("(#)");
	return "(" + label_[row] + ")";
}


namespace {
	short typecode(string const & s)
	{
		if (s == "equation")
			return LM_OT_EQUATION;
		if (s == "display")
			return LM_OT_EQUATION;
		if (s == "eqnarray")
			return LM_OT_EQNARRAY;
		if (s == "align")
			return LM_OT_ALIGN;
		if (s == "xalign")
			return LM_OT_XALIGN;
		if (s == "xxalign")
			return LM_OT_XXALIGN;
		if (s == "multline")
			return LM_OT_MULTLINE;
		return LM_OT_SIMPLE;
	}	
}

void MathMatrixInset::mutate(string const & newtype)
{
	if (newtype == "dump") {
		dump();
		return;
	}
	//lyxerr << "mutating from '" << GetType() << "' to '" << newtype << "'\n";
	mutate(typecode(newtype));
}

void MathMatrixInset::glueall()
{
	MathArray ar;
	for (int i = 0; i < nargs(); ++i)
		ar.push_back(cell(i));
	*this = MathMatrixInset(LM_OT_SIMPLE);
	cell(0) = ar;
}

void MathMatrixInset::mutate(short newtype)
{
	//lyxerr << "mutating from '" << GetType() << "' to '" << newtype << "'\n";

	if (newtype == GetType())
		return;

	switch (GetType()) {
		case LM_OT_SIMPLE:
			SetType(LM_OT_EQUATION);
			numbered(false);
			mutate(newtype);
			break;

		case LM_OT_EQUATION:
			switch (newtype) {
				case LM_OT_SIMPLE:
					SetType(LM_OT_SIMPLE);
					break;

				case LM_OT_ALIGN:
					MathGridInset::addCol(1);
					halign("rl");
					SetType(LM_OT_ALIGN);
					break;

				default:
					MathGridInset::addCol(1);
					MathGridInset::addCol(1);
					halign("rcl");
					SetType(LM_OT_EQNARRAY);
					mutate(newtype);
					break;
				}
			break;

		case LM_OT_EQNARRAY:
			switch (newtype) {
				case LM_OT_SIMPLE:
				case LM_OT_EQUATION: {
					string label;
					bool allnonum = true;
					for (int r = 0; r < nrows(); ++r) {
						label += label_[r];
						if (!nonum_[r])
							allnonum = false;
					}
					glueall();
					mutate(newtype);
					label_[0] = label;
					nonum_[0] = allnonum;
					break;
				}

				case LM_OT_ALIGN:
				default:
					for (int row = 0; row < nrows(); ++row) {
						int c = 3 * row + 1;
						cell(c).push_back(cell(c + 1));
					}
					MathGridInset::delCol(2);
					SetType(LM_OT_ALIGN);
					halign("rl");
					mutate(newtype);
					break;
			}
			break;

		case LM_OT_ALIGN:
			switch (newtype) {
				case LM_OT_SIMPLE:
				case LM_OT_EQUATION:
				case LM_OT_EQNARRAY:
					MathGridInset::addCol(1);
					SetType(LM_OT_EQNARRAY);
					halign("rcl");
					mutate(newtype);
					break;
				
				default:
					lyxerr << "mutation from '" << GetType()
						<< "' to '" << newtype << "' not implemented\n";
					break;
			}
			break;

		default:
			lyxerr << "mutation from '" << GetType()
				<< "' to '" << newtype << "' not implemented\n";
	}
}
