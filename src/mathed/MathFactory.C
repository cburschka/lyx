/**
 * \file MathFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathFactory.h"

#include "InsetMathAMSArray.h"
#include "InsetMathArray.h"
#include "InsetMathBinom.h"
#include "InsetMathBoldSymbol.h"
#include "InsetMathBoxed.h"
#include "InsetMathBox.h"
#include "InsetMathCases.h"
#include "InsetMathColor.h"
#include "InsetMathDecoration.h"
#include "InsetMathDFrac.h"
#include "InsetMathDots.h"
#include "InsetMathFBox.h"
#include "InsetMathFont.h"
#include "InsetMathFontOld.h"
#include "InsetMathFrac.h"
#include "InsetMathFrameBox.h"
#include "InsetMathKern.h"
#include "InsetMathLefteqn.h"
#include "InsetMathMacro.h"
#include "InsetMathMakebox.h"
#include "InsetMathOverset.h"
#include "InsetMathPhantom.h"
#include "InsetMathRef.h"
#include "InsetMathRoot.h"
#include "InsetMathSize.h"
#include "InsetMathSpace.h"
#include "InsetMathSplit.h"
#include "InsetMathSqrt.h"
#include "InsetMathStackrel.h"
#include "InsetMathSubstack.h"
#include "InsetMathSymbol.h"
#include "InsetMathTabular.h"
#include "InsetMathTFrac.h"
#include "InsetMathUnderset.h"
#include "InsetMathUnknown.h"
#include "InsetMathXArrow.h"
#include "InsetMathXYMatrix.h"
#include "MathMacroArgument.h"
#include "MathMacroTable.h"
#include "MathMacroTemplate.h"
#include "MathParser.h"
#include "MathSupport.h"

#include "debug.h"

#include "support/filetools.h" // LibFileSearch
#include "support/lstrings.h"

#include "frontends/FontLoader.h"

#include <fstream>
#include <sstream>


namespace lyx {

using support::libFileSearch;
using support::split;

using std::string;
using std::endl;
using std::istringstream;

bool has_math_fonts;


namespace {

// file scope
typedef std::map<string, latexkeys> WordList;

WordList theWordList;


bool math_font_available(string & name)
{
	LyXFont f;
	augmentFont(f, name);

	// Do we have the font proper?
	if (theFontLoader().available(f))
		return true;

	// can we fake it?
	if (name == "eufrak") {
		name = "lyxfakefrak";
		return true;
	}

	lyxerr[Debug::MATHED]
		<< "font " << name << " not available and I can't fake it"
		<< endl;
	return false;
}


void initSymbols()
{
	string const filename = libFileSearch(string(), "symbols");
	lyxerr[Debug::MATHED] << "read symbols from " << filename << endl;
	if (filename.empty()) {
		lyxerr << "Could not find symbols file" << endl;
		return;
	}

	std::ifstream fs(filename.c_str());
	string line;
	bool skip = false;
	while (getline(fs, line)) {
		int charid     = 0;
		int fallbackid = 0;
		if (!line.empty() && line[0] == '#')
			continue;

		// special case of iffont/else/endif
		if (line.size() >= 7 && line.substr(0, 6) == "iffont") {
			istringstream is(line);
			string tmp;
			is >> tmp;
			is >> tmp;
			skip = !math_font_available(tmp);
			continue;
		} else if (line.size() >= 4 && line.substr(0, 4) == "else") {
			skip = !skip;
		} else if (line.size() >= 5 && line.substr(0, 5) == "endif") {
			skip = false;
			continue;
		} else if (skip)
			continue;

		// special case of pre-defined macros
		if (line.size() > 8 && line.substr(0, 5) == "\\def\\") {
			//lyxerr << "macro definition: '" << line << '\'' << endl;
			MacroTable::globalMacros().insert(line);
			continue;
		}

		istringstream is(line);
		latexkeys tmp;
		is >> tmp.name >> tmp.inset;
		if (isFontName(tmp.inset))
			is >> charid >> fallbackid >> tmp.extra >> tmp.xmlname;
		else
			is >> tmp.extra;
		if (!is) {
			lyxerr[Debug::MATHED] << "skipping line '" << line << '\'' << endl;
			lyxerr[Debug::MATHED]
				<< tmp.name << ' ' << tmp.inset << ' ' << tmp.extra << endl;
			continue;
		}

		if (isFontName(tmp.inset)) {
			// tmp.inset _is_ the fontname here.
			// create fallbacks if necessary

			// store requirements as long as we can
			if (tmp.inset == "msa" || tmp.inset == "msb")
				tmp.requires = "amssymb";
			// See http://bugzilla.lyx.org/show_bug.cgi?id=1942
			// else if (tmp.inset == "wasy")
			//	tmp.requires = "wasysym";

			// symbol font is not available sometimes
			string symbol_font = "lyxsymbol";

			if (tmp.extra == "func" || tmp.extra == "funclim" || tmp.extra == "special") {
				lyxerr[Debug::MATHED] << "symbol abuse for " << tmp.name << endl;
				tmp.draw = tmp.name;
			} else if (math_font_available(tmp.inset)) {
				lyxerr[Debug::MATHED] << "symbol available for " << tmp.name << endl;
				tmp.draw += char(charid);
			} else if (fallbackid && math_font_available(symbol_font)) {
				if (tmp.inset == "cmex")
					tmp.inset  = "lyxsymbol";
				else
					tmp.inset  = "lyxboldsymbol";
				lyxerr[Debug::MATHED] << "symbol fallback for " << tmp.name << endl;
				tmp.draw += char(fallbackid);
			} else {
				lyxerr[Debug::MATHED] << "faking " << tmp.name << endl;
				tmp.draw = tmp.name;
				tmp.inset = "lyxtex";
			}
		} else {
			// it's a proper inset
			lyxerr[Debug::MATHED] << "inset " << tmp.inset
					      << " used for " << tmp.name
					      << endl;
		}

		if (theWordList.find(tmp.name) != theWordList.end())
			lyxerr[Debug::MATHED]
				<< "readSymbols: inset " << tmp.name
				<< " already exists." << endl;
		else
			theWordList[tmp.name] = tmp;

		lyxerr[Debug::MATHED]
			<< "read symbol '" << tmp.name
			<< "  inset: " << tmp.inset
			<< "  draw: " << int(tmp.draw.empty() ? 0 : tmp.draw[0])
			<< "  extra: " << tmp.extra
			<< '\'' << endl;
	}
	string tmp = "cmm";
	string tmp2 = "cmsy";
	has_math_fonts = math_font_available(tmp) && math_font_available(tmp2);
}


} // namespace anon


void initMath()
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		initParser();
		initSymbols();
	}
}


latexkeys const * in_word_set(string const & str)
{
	WordList::iterator it = theWordList.find(str);
	return it != theWordList.end() ? &(it->second) : 0;
}


MathAtom createInsetMath(string const & s)
{
	//lyxerr << "creating inset with name: '" << s << '\'' << endl;
	latexkeys const * l = in_word_set(s);
	if (l) {
		string const & inset = l->inset;
		//lyxerr << " found inset: '" << inset << '\'' << endl;
		if (inset == "ref")
			return MathAtom(new RefInset(l->name));
		if (inset == "overset")
			return MathAtom(new InsetMathOverset);
		if (inset == "underset")
			return MathAtom(new InsetMathUnderset);
		if (inset == "decoration")
			return MathAtom(new InsetMathDecoration(l));
		if (inset == "space")
			return MathAtom(new InsetMathSpace(l->name));
		if (inset == "dots")
			return MathAtom(new InsetMathDots(l));
		if (inset == "mbox")
			// return MathAtom(new InsetMathMBox);
			// InsetMathMBox is proposed to replace InsetMathBox,
			// but is not ready yet (it needs a BufferView for
			// construction)
			return MathAtom(new InsetMathBox(l->name));
//		if (inset == "fbox")
//			return MathAtom(new InsetMathFBox(l));
		if (inset == "style")
			return MathAtom(new InsetMathSize(l));
		if (inset == "font")
			return MathAtom(new InsetMathFont(l));
		if (inset == "oldfont")
			return MathAtom(new InsetMathFontOld(l));
		if (inset == "matrix")
			return MathAtom(new InsetMathAMSArray(s));
		if (inset == "split")
			return MathAtom(new InsetMathSplit(s));
		if (inset == "big")
			// we can't create a InsetMathBig, since the argument
			// is missing.
			return MathAtom(new InsetMathUnknown(s));
		return MathAtom(new InsetMathSymbol(l));
	}

	if (s.size() == 2 && s[0] == '#' && s[1] >= '1' && s[1] <= '9')
		return MathAtom(new MathMacroArgument(s[1] - '0'));
	if (s.size() == 3 && s[0] == '\\' && s[1] == '#'
			&& s[2] >= '1' && s[2] <= '9')
		return MathAtom(new MathMacroArgument(s[2] - '0'));
	if (s == "boxed")
		return MathAtom(new InsetMathBoxed());
	if (s == "fbox")
		return MathAtom(new InsetMathFBox());
	if (s == "framebox")
		return MathAtom(new InsetMathFrameBox);
	if (s == "makebox")
		return MathAtom(new InsetMathMakebox);
	if (s == "kern")
		return MathAtom(new InsetMathKern);
	if (s == "xymatrix")
		return MathAtom(new InsetMathXYMatrix);
	if (s == "xrightarrow" || s == "xleftarrow")
		return MathAtom(new InsetMathXArrow(s));
	if (s == "split" || s == "gathered" || s == "aligned" || s == "alignedat")
		return MathAtom(new InsetMathSplit(s));
	if (s == "cases")
		return MathAtom(new InsetMathCases);
	if (s == "substack")
		return MathAtom(new InsetMathSubstack);
	if (s == "subarray" || s == "array")
		return MathAtom(new InsetMathArray(s, 1, 1));
	if (s == "sqrt")
		return MathAtom(new InsetMathSqrt);
	if (s == "root")
		return MathAtom(new InsetMathRoot);
	if (s == "tabular")
		return MathAtom(new InsetMathTabular(s, 1, 1));
	if (s == "stackrel")
		return MathAtom(new InsetMathStackrel);
	if (s == "binom" || s == "choose")
		return MathAtom(new InsetMathBinom(s == "choose"));
	if (s == "frac")
		return MathAtom(new InsetMathFrac);
	if (s == "over")
		return MathAtom(new InsetMathFrac(InsetMathFrac::OVER));
	if (s == "nicefrac")
		return MathAtom(new InsetMathFrac(InsetMathFrac::NICEFRAC));
	//if (s == "infer")
	//	return MathAtom(new MathInferInset);
	if (s == "atop")
		return MathAtom(new InsetMathFrac(InsetMathFrac::ATOP));
	if (s == "lefteqn")
		return MathAtom(new InsetMathLefteqn);
	if (s == "boldsymbol")
		return MathAtom(new InsetMathBoldSymbol);
	if (s == "color" || s == "normalcolor")
		return MathAtom(new InsetMathColor(true));
	if (s == "textcolor")
		return MathAtom(new InsetMathColor(false));
	if (s == "dfrac")
		return MathAtom(new InsetMathDFrac);
	if (s == "tfrac")
		return MathAtom(new InsetMathTFrac);
	if (s == "hphantom")
		return MathAtom(new InsetMathPhantom(InsetMathPhantom::hphantom));
	if (s == "phantom")
		return MathAtom(new InsetMathPhantom(InsetMathPhantom::phantom));
	if (s == "vphantom")
		return MathAtom(new InsetMathPhantom(InsetMathPhantom::vphantom));

	if (MacroTable::globalMacros().has(s))
		return MathAtom(new MathMacro(s,
			MacroTable::globalMacros().get(s).numargs()));
	//if (MacroTable::localMacros().has(s))
	//	return MathAtom(new MathMacro(s,
	//		MacroTable::localMacros().get(s).numargs()));

	//lyxerr << "creating unknown inset '" << s << "'" << endl;
	return MathAtom(new InsetMathUnknown(s));
}


bool createInsetMath_fromDialogStr(string const & str, MathArray & ar)
{
	// An example str:
	// "ref LatexCommand \\ref{sec:Title}\n\\end_inset\n\n";
	string name;
	string body = split(str, name, ' ');

	if (name != "ref" )
		return false;

	// body comes with a head "LatexCommand " and a
	// tail "\nend_inset\n\n". Strip them off.
	string trimmed;
	body = split(body, trimmed, ' ');
	split(body, trimmed, '\n');

	mathed_parse_cell(ar, trimmed);
	if (ar.size() != 1)
		return false;

	return ar[0].nucleus();
}


} // namespace lyx
