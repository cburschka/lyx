
#ifdef __GNUG__
#pragma implementation 
#endif

#include <config.h>

#include "math_parser.h"
#include "math_arrayinset.h"
#include "math_amsarrayinset.h"
#include "math_binominset.h"
#include "math_boxinset.h"
#include "math_casesinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_ertinset.h"
#include "math_fboxinset.h"
#include "math_frameboxinset.h"
#include "math_fontinset.h"
#include "math_fontoldinset.h"
#include "math_fracinset.h"
#include "math_kerninset.h"
#include "math_inferinset.h"
#include "math_lefteqninset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_macroarg.h"
#include "math_parboxinset.h"
#include "math_rootinset.h"
#include "math_sizeinset.h"
#include "math_spaceinset.h"
#include "math_splitinset.h"
#include "math_sqrtinset.h"
#include "math_stackrelinset.h"
#include "math_substackinset.h"
#include "math_symbolinset.h"
#include "math_undersetinset.h"
#include "math_unknowninset.h"
#include "math_xarrowinset.h"
#include "math_xymatrixinset.h"
#include "math_xyarrowinset.h"

//#include "insets/insetref.h"
#include "ref_inset.h"

#include "math_metricsinfo.h"
#include "debug.h"
#include "math_support.h"
#include "Lsstream.h"
#include "support/filetools.h" // LibFileSearch
#include "frontends/lyx_gui.h"

#include <map>
#include <fstream>


namespace {

// file scope
typedef std::map<string, latexkeys> WordList;
WordList theWordList;


bool math_font_available(string & name)
{
	LyXFont f;
	augmentFont(f, name);

	// Do we have the font proper?
	if (lyx_gui::font_available(f))
		return true;

	// can we fake it?
	if (name == "eufrak") {
		name = "lyxfakefrak";
		return true;
	}

	lyxerr[Debug::MATHED]
		<< "font " << name << " not available and I can't fake it\n";
	return false;
}


void initSymbols()
{
	string const filename = LibFileSearch(string(), "symbols");
	lyxerr[Debug::MATHED] << "read symbols from " << filename << "\n";
	if (filename.empty()) {
		lyxerr << "Could not find symbols file\n";
		return;
	}

	std::ifstream fs(filename.c_str());
	string line;
	bool skip = false;
	while (std::getline(fs, line)) {
		int charid     = 0;
		int fallbackid = 0;
		if (line.size() > 0 && line[0] == '#')
			continue;

		// special case of \iffont / \fi
		if (line.size() >= 7 && line.substr(0, 6) == "iffont") {
			istringstream is(line);
			string tmp;
			is >> tmp;
			is >> tmp;
			skip = !math_font_available(tmp);
			continue;
		} else if (line.size() >= 3 && line.substr(0, 3) == "end") {
			skip = false;
			continue;
		} else if (skip)
			continue;

		// special case of pre-defined macros
		if (line.size() > 8 && line.substr(0, 5) == "\\def\\") {
			//lyxerr << "defining: '" << line << "'\n";
			istringstream is(line);
			MathMacroTable::create(MathAtom(new MathMacroTemplate(is)));
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
			lyxerr[Debug::MATHED] << "skipping line '" << line << "'\n";
			lyxerr[Debug::MATHED]
				<< tmp.name << ' ' << tmp.inset << ' ' << tmp.extra << "\n";
			continue;
		}

		if (isFontName(tmp.inset)) {
			// tmp.inset _is_ the fontname here.
			// create fallbacks if necessary
			if (tmp.extra=="func" || tmp.extra=="funclim" || tmp.extra=="special") {
				lyxerr[Debug::MATHED] << "symbol abuse for " << tmp.name << "\n";
				tmp.draw = tmp.name;
			} else if (math_font_available(tmp.inset)) {
				lyxerr[Debug::MATHED] << "symbol available for " << tmp.name << "\n";
				tmp.draw += char(charid);
			} else if (fallbackid) {
				if (tmp.inset == "cmex")
					tmp.inset  = "lyxsymbol";
				else
					tmp.inset  = "lyxboldsymbol";
				lyxerr[Debug::MATHED] << "symbol fallback for " << tmp.name << "\n";
				tmp.draw += char(fallbackid); 
			} else {
				lyxerr[Debug::MATHED] << "faking " << tmp.name << "\n";
				tmp.draw = tmp.name;
				tmp.inset = "lyxtex";
			}
		} else {
			// it's a proper inset
			lyxerr[Debug::MATHED] << "inset " << tmp.inset << " used for "
				<< tmp.name << "\n";
		}

		if (theWordList.find(tmp.name) != theWordList.end())
			lyxerr[Debug::MATHED] << "readSymbols: inset " << tmp.name
				<< " already exists.\n";
		else
			theWordList[tmp.name] = tmp;
		lyxerr[Debug::MATHED] << "read symbol '" << tmp.name
					<<  "  inset: " << tmp.inset
					<<  "  draw: " << int(tmp.draw[0])
					<<  "  extra: " << tmp.extra
					<< "'\n";
	}
}


} // namespace anon


void initMath()
{
	static bool initialized = false;
	if (!initialized) {
		initSymbols();
		initialized = true;
	}
}


latexkeys const * in_word_set(string const & str)
{
	WordList::iterator it = theWordList.find(str);
	//lyxerr << "looking up '" << str << "' found: "
	// << (it != theWordList.end()) << "\n";
	return (it != theWordList.end()) ? &(it->second) : 0;
}


MathAtom createMathInset(string const & s)
{
	lyxerr[Debug::MATHED] << "creating inset with name: '" << s << "'\n";
	latexkeys const * l = in_word_set(s);
	if (l) {
		string const & inset = l->inset;
		lyxerr[Debug::MATHED] << " found inset: '" << inset << "'\n";
		if (inset == "ref")
			return MathAtom(new RefInset(l->name));
		if (inset == "underset")
			return MathAtom(new MathUndersetInset);
		if (inset == "decoration")
			return MathAtom(new MathDecorationInset(l));
		if (inset == "space")
			return MathAtom(new MathSpaceInset(l->name));
		if (inset == "dots")
			return MathAtom(new MathDotsInset(l));
		if (inset == "mbox")
			return MathAtom(new MathBoxInset(l->name));
		if (inset == "parbox")
			return MathAtom(new MathParboxInset);
		if (inset == "fbox")
			return MathAtom(new MathFboxInset(l));
		if (inset == "style")
			return MathAtom(new MathSizeInset(l));
		if (inset == "font")
			return MathAtom(new MathFontInset(l));
		if (inset == "oldfont")
			return MathAtom(new MathFontOldInset(l));
		if (inset == "matrix")
			return MathAtom(new MathAMSArrayInset(s));
		return MathAtom(new MathSymbolInset(l));
	}

	if (s.size() == 2 && s[0] == '#' && s[1] >= '1' && s[1] <= '9')
		return MathAtom(new MathMacroArgument(s[1] - '0'));
	if (s.size() == 3 && s[0] == '\\' && s[1] == '#'
			&& s[2] >= '1' && s[2] <= '9')
		return MathAtom(new MathMacroArgument(s[2] - '0'));
	if (s == "framebox")
		return MathAtom(new MathFrameboxInset);
	if (s == "kern")
		return MathAtom(new MathKernInset);
	if (s == "xymatrix")
		return MathAtom(new MathXYMatrixInset);
	if (s == "xrightarrow" || s == "xleftarrow")
		return MathAtom(new MathXArrowInset(s));
	if (s == "split" || s == "gathered" || s == "aligned")
		return MathAtom(new MathSplitInset(s));
	if (s == "cases")
		return MathAtom(new MathCasesInset);
	if (s == "substack")
		return MathAtom(new MathSubstackInset);
	if (s == "subarray" || s == "array")
		return MathAtom(new MathArrayInset(s, 1, 1));
	if (s == "sqrt")
		return MathAtom(new MathSqrtInset);
	if (s == "root")
		return MathAtom(new MathRootInset);
	if (s == "stackrel")
		return MathAtom(new MathStackrelInset);
	if (s == "binom" || s == "choose")
		return MathAtom(new MathBinomInset(s == "choose"));
	if (s == "over" || s == "frac")
		return MathAtom(new MathFracInset);
	//if (s == "infer")
	//	return MathAtom(new MathInferInset);
	if (s == "atop")
		return MathAtom(new MathFracInset(true));
	if (s == "lefteqn")
		return MathAtom(new MathLefteqnInset);
	if (s == "lyxert")
		return MathAtom(new MathErtInset);

	if (MathMacroTable::has(s))
		return MathAtom(new MathMacro(s));

	//lyxerr[Debug::MATHED] << "creating inset 2 with name: '" << s << "'\n";
	return MathAtom(new MathUnknownInset(s));
}
