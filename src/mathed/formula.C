/**
 * \file formula.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "formula.h"
#include "formulamacro.h"
#include "math_data.h"
#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "math_parser.h"

#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "textpainter.h"

#include "frontends/Alert.h"

#include "support/std_sstream.h"

using lyx::support::trim;

using std::endl;
using std::max;

using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostringstream;
using std::pair;


namespace {

bool openNewInset(LCursor & cur, InsetBase * inset)
{
	if (!cur.bv().insertInset(inset)) {
		delete inset;
		return false;
	}
	inset->edit(cur, true);
	return true;
}


} // namespace anon




std::auto_ptr<InsetBase> InsetFormula::clone() const
{
	return auto_ptr<InsetBase>(new InsetFormula(*this));
}


void InsetFormula::write(Buffer const &, std::ostream & os) const
{
	WriteStream wi(os, false, false);
	os << fileInsetLabel() << ' ';
	MathHullInset::write(wi);
}


void InsetFormula::read(Buffer const &, LyXLex & lex)
{
	MathAtom at;
	mathed_parse_normal(at, lex);
	MathHullInset::operator=(*at->asHullInset());
}



/////////////////////////////////////////////

void mathDispatchCreation(LCursor & cur, FuncRequest const & cmd,
	bool display)
{
	// use selection if available..
	//string sel;
	//if (action == LFUN_MATH_IMPORT_SELECTION)
	//	sel = "";
	//else

	string sel =
		cur.bv().getLyXText()->selectionAsString(*cur.bv().buffer(), false);

	if (sel.empty()) {
		InsetBase * f = new MathHullInset;
		if (openNewInset(cur, f)) {
			cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
			// don't do that also for LFUN_MATH_MODE unless you want end up with
			// always changing to mathrm when opening an inlined inset
			// -- I really hate "LyXfunc overloading"...
			if (display)
				f->dispatch(cur, FuncRequest(LFUN_MATH_DISPLAY));
			f->dispatch(cur, FuncRequest(LFUN_INSERT_MATH, cmd.argument));
		}
	} else {
		// create a macro if we see "\\newcommand" somewhere, and an ordinary
		// formula otherwise
		InsetBase * f;
		if (sel.find("\\newcommand") == string::npos &&
				sel.find("\\def") == string::npos)
			f = new MathHullInset(sel);
		else
			f = new InsetFormulaMacro(sel);
		cur.bv().getLyXText()->cutSelection(true, false);
		openNewInset(cur, f);
	}
	cmd.message(N_("Math editor mode"));
}


void mathDispatch(LCursor & cur, FuncRequest const & cmd)
{
	if (!cur.bv().available())
		return;

	switch (cmd.action) {

		case LFUN_MATH_DISPLAY:
			mathDispatchCreation(cur, cmd, true);
			break;

		case LFUN_MATH_MODE:
			mathDispatchCreation(cur, cmd, false);
			break;

		case LFUN_MATH_IMPORT_SELECTION:
			mathDispatchCreation(cur, cmd, false);
			break;

/*
		case LFUN_MATH_MACRO:
			if (cmd.argument.empty())
				cmd.errorMessage(N_("Missing argument"));
			else {
				string s = cmd.argument;
				string const s1 = token(s, ' ', 1);
				int const nargs = s1.empty() ? 0 : atoi(s1);
				string const s2 = token(s, ' ', 2);
				string const type = s2.empty() ? "newcommand" : s2;
				openNewInset(cur, new InsetFormulaMacro(token(s, ' ', 0), nargs, s2));
			}
			break;

		case LFUN_INSERT_MATH:
		case LFUN_INSERT_MATRIX:
		case LFUN_MATH_DELIM: {
			MathHullInset * f = new MathHullInset;
			if (openNewInset(cur, f)) {
				cur.inset()->dispatch(cur, FuncRequest(LFUN_MATH_MUTATE, "simple"));
				cur.inset()->dispatch(cur, cmd);
			}
			break;
		}
*/

		default:
			break;
	}
}
