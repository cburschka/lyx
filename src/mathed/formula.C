/*
*  File:        formula.C
*  Purpose:     Implementation of formula inset
*  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
*  Created:     January 1996
*  Description: Allows the edition of math paragraphs inside Lyx.
*
*  Copyright: 1996-1998 Alejandro Aguilar Sierra
*
*  Version: 0.4, Lyx project.
*
*   You are free to use and modify this code under the terms of
*   the GNU General Public Licence version 2 or later.
*/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "formula.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_charinset.h"
#include "math_arrayinset.h"
#include "math_deliminset.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/filetools.h" // LibFileSearch
#include "LyXView.h"
#include "Painter.h"
#include "lyxrc.h"
#include "math_matrixinset.h"
#include "mathed/support.h"

using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;


namespace {

	string captureOutput(string const & cmd, string const & data)
	{
		string outfile = lyx::tempName(string(), "mathextern");
		string full =  "echo '" + data + "' | (" + cmd + ") > " + outfile;
		lyxerr << "calling: " << full << "\n";
		Systemcalls dummy(Systemcalls::System, full, 0);
		string out = GetFileContents(outfile);
		lyxerr << "result: '" << out << "'\n";
		return out;
	}


	MathArray pipeThroughMaple(string const & extra, MathArray const & ar)
	{
		string header = "readlib(latex):\n";

		// remove the \\it for variable names
		//"#`latex/csname_font` := `\\it `:"
		header +=
			"`latex/csname_font` := ``:\n";

		// export matrices in (...) instead of [...]
		header +=
			"`latex/latex/matrix` := "
				"subs(`[`=`(`, `]`=`)`,"
					"eval(`latex/latex/matrix`)):\n";

		// replace \\cdots with proper '*'
		header +=
			"`latex/latex/*` := "
				"subs(`\\,`=`\\cdot `,"
					"eval(`latex/latex/*`)):\n";

		//"#`latex/latex/symbol` "
		//	" := subs((\\'_\\' = \\'`\\_`\\',eval(`latex/latex/symbol`)): ";

		string trailer = "quit;";
		ostringstream os;
		MapleStream ms(os);
		ms << ar;
		string expr = os.str();

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			// try to fix missing '*' the hard way by using mint
			//
			// ... > echo "1A;" | mint -i 1 -S -s -q
			// on line     1: 1A;
			//                 ^ syntax error - 
			//                   Probably missing an operator such as * p
			//
			lyxerr << "checking expr: '" << expr << "'\n";
			string out = captureOutput("mint -i 1 -S -s -q -q", expr + ";");
			if (out.empty())
				break; // expression syntax is ok
			istringstream is(out);
			string line;
			getline(is, line);
			if (line.find("on line") != 0)
				break; // error message not identified
			getline(is, line);
			string::size_type pos = line.find('^');
			if (pos == string::npos || pos < 15)
				break; // caret position not found
			pos -= 15; // skip the "on line ..." part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		string full = "latex(" +  extra + '(' + expr + "));";
		string out = captureOutput("maple -q", header + full + trailer);

		// change \_ into _

		//
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}
		
	
	MathArray pipeThroughOctave(string const &, MathArray const & ar)
	{
		ostringstream os;
		OctaveStream vs(os);
		vs << ar;
		string expr = os.str();
		string out;

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			//
			// try to fix missing '*' the hard way 
			// parse error:
			// >>> ([[1 2 3 ];[2 3 1 ];[3 1 2 ]])([[1 2 3 ];[2 3 1 ];[3 1 2 ]])
			//                                   ^
			//
			lyxerr << "checking expr: '" << expr << "'\n";
 			out = captureOutput("octave -q 2>&1", expr);
			lyxerr << "checking out: '" << out << "'\n";

			// leave loop if expression syntax is probably ok
			if (out.find("parse error:") == string::npos)
				break;

			// search line with single caret
			istringstream is(out);
			string line;
			while (is) {
				getline(is, line);
				lyxerr << "skipping line: '" << line << "'\n";
				if (line.find(">>> ") != string::npos)
					break;
			}

			// found line with error, next line is the one with caret
			getline(is, line);
			string::size_type pos = line.find('^');
			lyxerr << "caret line: '" << line << "'\n";
			lyxerr << "found caret at pos: '" << pos << "'\n";
			if (pos == string::npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the ">>> " part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		if (out.size() < 6)
			return MathArray();

		// remove 'ans = '
		out = out.substr(6);

		// parse output as matrix or single number
		MathAtom at(new MathArrayInset(out));
		MathArrayInset const * mat = at.nucleus()->asArrayInset();
		MathArray res;
		if (mat->ncols() == 1 && mat->nrows() == 1)
			res.push_back(mat->cell(0));
		else {
			res.push_back(MathAtom(new MathDelimInset("(", ")")));
			res.back()->cell(0).push_back(at);
		}
		return res;
	}


	MathArray pipeThroughExtern(string const & arg, MathArray const & ar)
	{
		string lang;
		string extra;
		istringstream iss(arg.c_str());
		iss >> lang >> extra;
		if (extra.empty())
			extra = "noextra";	

		if (lang == "octave")
			return pipeThroughOctave(extra, ar);

		if (lang == "maple")
			return pipeThroughMaple(extra, ar);

		// create normalized expression
		ostringstream os;
		os << "[" << extra << ' ';
		ar.writeNormal(os); 
		os << "]";
		string data = os.str().c_str();

		// search external script
		string file = LibFileSearch("mathed", "extern_" + lang);
		if (file.empty()) {
			lyxerr << "converter to '" << lang << "' not found\n";
			return MathArray();
		}
		
		// run external sript
		string out = captureOutput(file, data);
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}

}


InsetFormula::InsetFormula()
	: par_(MathAtom(new MathMatrixInset))
{}


InsetFormula::InsetFormula(MathInsetTypes t)
	: par_(MathAtom(new MathMatrixInset(t)))
{}


InsetFormula::InsetFormula(string const & s) 
{
	if (s.size()) {
		bool res = mathed_parse_normal(par_, s);

		if (!res)
			res = mathed_parse_normal(par_, "$" + s + "$");

		if (!res) {
			lyxerr << "cannot interpret '" << s << "' as math\n";
			par_ = MathAtom(new MathMatrixInset(LM_OT_SIMPLE));
		}
	}
	metrics();
}


Inset * InsetFormula::clone(Buffer const &, bool) const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(Buffer const * buf, ostream & os) const
{
	os << "Formula ";
	latex(buf, os, false, false);
}


int InsetFormula::latex(Buffer const * buf, ostream & os, bool fragil, bool)
	const
{
	MathWriteInfo wi(buf, os, fragil);
	par_->write(wi);
	return 1;
}


int InsetFormula::ascii(Buffer const * buf, ostream & os, int) const
{
	MathWriteInfo wi(buf, os, false);
	par_->write(wi);
	return 1;
}


int InsetFormula::linuxdoc(Buffer const * buf, ostream & os) const
{
	return docbook(buf, os);
}


int InsetFormula::docbook(Buffer const * buf, ostream & os) const
{
	MathMLStream ms(os);
	ms << "<equation><alt>";
	int res = ascii(buf, ms.os_, 0);
	ms << "</alt>\n<mml>" << par_.nucleus() << "<mml></equation>";
	return res + 1;
}


void InsetFormula::read(Buffer const *, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	metrics();
}


void InsetFormula::draw(BufferView * bv, LyXFont const & font,
			int y, float & xx, bool) const
{
	int x = int(xx) - 1;
	y -= 2;

	Painter & pain = bv->painter();

	metrics(bv, font);
	int w = par_->width();
	int h = par_->height();
	int a = par_->ascent();
	pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

	if (mathcursor && mathcursor->formula() == this) {
		mathcursor->drawSelection(pain);
		pain.rectangle(x, y - a, w, h, LColor::mathframe);
	}

	par_->draw(pain, x, y);
	xx += par_->width();
	xo_ = x;
	yo_ = y;

	setCursorVisible(false);
}


vector<string> const InsetFormula::getLabelList() const
{
	return mat()->getLabelList();
}


UpdatableInset::RESULT
InsetFormula::localDispatch(BufferView * bv, kb_action action,
	 string const & arg)
{
	RESULT result = DISPATCHED;

	switch (action) {

		case LFUN_BREAKLINE: 
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->breakLine();
			mathcursor->normalize();
			updateLocal(bv, true);
			break;

		case LFUN_MATH_NUMBER:
		{
			//lyxerr << "toggling all numbers\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				bool old = mat()->numberedType();
				for (MathInset::row_type row = 0; row < par_->nrows(); ++row)
					mat()->numbered(row, !old);
				bv->owner()->message(old ? _("No number") : _("Number"));
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_MATH_NONUMBER:
		{
			//lyxerr << "toggling line number\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				MathCursor::row_type row = mathcursor->row();
				bool old = mat()->numbered(row);
				bv->owner()->message(old ? _("No number") : _("Number"));
				mat()->numbered(row, !old);
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_INSERT_LABEL:
		{
			bv->lockedInsetStoreUndo(Undo::INSERT);

			MathCursor::row_type row = mathcursor->row();
			string old_label = mat()->label(row);
			string new_label = arg;

			if (new_label.empty()) {
				string const default_label =
					(lyxrc.label_init_length >= 0) ? "eq:" : "";
				pair<bool, string> const res = old_label.empty()
					? askForText(_("Enter new label to insert:"), default_label)
					: askForText(_("Enter label:"), old_label);
				
				lyxerr << "res: " << res.first << " - '" << res.second << "'\n";
				if (!res.first)
					break;
				new_label = frontStrip(strip(res.second));
			}

			//if (new_label == old_label)
			//	break;  // Nothing to do

			if (!new_label.empty()) {
				lyxerr << "setting label to '" << new_label << "'\n";
				mat()->numbered(row, true);
			}

			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->redraw();

			mat()->label(row, new_label);

			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_MUTATE:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			int x;
			int y;
			mathcursor->getPos(x, y);
			mat()->mutate(arg);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_EXTERN:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			handleExtern(arg);
			// re-compute inset dimension
			metrics(bv);
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_DISPLAY:
		{
			int x;
			int y;
			mathcursor->getPos(x, y);
			if (mat()->getType() == LM_OT_SIMPLE)
				mat()->mutate(LM_OT_EQUATION);
			else
				mat()->mutate(LM_OT_SIMPLE);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}
		
		case LFUN_PASTESELECTION:
		{
			string const clip = bv->getClipboard();
  		if (!clip.empty())
				mathed_parse_normal(par_, clip);
			break;
		}

		case LFUN_MATH_COLUMN_INSERT:
		{
			if (mat()->getType() == LM_OT_ALIGN)
				mat()->mutate(LM_OT_ALIGNAT);
			mat()->addCol(mat()->ncols());
			mathcursor->normalize();
			updateLocal(bv, true);
		}

		default:
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}

	return result;
}


void InsetFormula::handleExtern(const string & arg)
{
	// where are we?
	if (!mathcursor)
		return; 

	bool selected = mathcursor->selection();

	MathArray ar;
	if (selected) {
		mathcursor->selGet(ar);
		lyxerr << "use selection: " << ar << "\n";
	} else {
		mathcursor->end();
		mathcursor->stripFromLastEqualSign();
		ar = mathcursor->cursor().cell();
		mathcursor->insert(MathAtom(new MathCharInset('=', LM_TC_VAR)));
		//lyxerr << "use whole cell: " << ar << "\n";
	}

	mathcursor->insert(pipeThroughExtern(arg, ar));
}


bool InsetFormula::display() const
{
	return mat()->getType() != LM_OT_SIMPLE;
}


MathMatrixInset const * InsetFormula::mat() const
{
	lyx::Assert(par_->asMatrixInset());
	return par_->asMatrixInset();
}


MathMatrixInset * InsetFormula::mat()
{
	lyx::Assert(par_->asMatrixInset());
	return par_->asMatrixInset();
}


Inset::Code InsetFormula::lyxCode() const
{
	return Inset::MATH_CODE;
}


void InsetFormula::validate(LaTeXFeatures & features) const
{
	par_->validate(features);
}


bool InsetFormula::insetAllowed(Inset::Code code) const
{
	return code == Inset::LABEL_CODE && display(); 
}


int InsetFormula::ascent(BufferView *, LyXFont const &) const
{
	return par_->ascent() + 2;
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	return par_->descent() - 2;
}


int InsetFormula::width(BufferView * bv, LyXFont const & font) const
{
	metrics(bv, font);
	return par_->width();
}


MathInsetTypes InsetFormula::getType() const
{
	return mat()->getType();
}
