#include <config.h>

#include "math_parser.h"
#include "math_arrayinset.h"
#include "math_amsarrayinset.h"
#include "math_binominset.h"
#include "math_boxinset.h"
#include "math_casesinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_fontinset.h"
#include "math_fracinset.h"
#include "math_kerninset.h"
#include "math_lefteqninset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macroarg.h"
#include "math_notinset.h"
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

#include "math_metricsinfo.h"
#include "debug.h"
#include "math_support.h"
#include "Lsstream.h"
#include "support/filetools.h" // LibFileSearch
#include "frontends/font_loader.h"

#include <map>
#include <fstream>


namespace {

// file scope
typedef std::map<string, latexkeys> WordList;
WordList theWordList;


struct key_type {
	///
	string name;
	///
	string inset;
	///
	int id;
};


key_type wordlist_array[] =
{
	{"!",  "space", 0},
	{"(",  "begin", LM_OT_SIMPLE},
	{")",  "end", LM_OT_SIMPLE},
	{",",  "space", 1},
	{":",  "space", 2},
	{";",  "space", 3},
	{"[",  "begin", LM_OT_EQUATION},
	{"]",  "end", LM_OT_EQUATION},
	{"acute",  "decoration", 0},
	{"bar",  "decoration", 0},
	{"begin",  "begin", 0},
	{"bf",  "oldfont", 0},
	{"breve",  "decoration", 0},
	{"cal",  "oldfont", 0},
	{"cdots",  "dots", 0},
	{"check",  "decoration", 0},
	{"ddot",  "decoration", 0},
	{"dddot",  "decoration", 0},
	{"ddots",  "dots", 0},
	{"displaystyle",  "style", LM_ST_DISPLAY},
	{"dot",  "decoration", 0},
	{"dotsb",  "dots", 0},
	{"dotsc",  "dots", 0},
	{"dotsi",  "dots", 0},
	{"dotsm",  "dots", 0},
	{"dotso",  "dots", 0},
	{"end",  "end", 0},
	{"frak",  "font", 0},
	{"grave",  "decoration", 0},
	{"hat",  "decoration", 0},
	{"it",  "oldfont", 0},
	{"label",  "label", 0},
	{"ldots",  "dots", 0},
	{"left",  "left", 0},
	{"limits",  "limit", 1 },
	{"lyxbox",  "box", 0},
	{"lyxnegspace",  "space", 6},
	{"mathbb",  "font", 0},
	{"mathbf",  "font", 0},
	{"mathcal",  "font", 0},
	{"mathfrak",  "font", 0},
	{"mathit",  "font", 0},
	{"mathnormal",  "font", 0},
	{"mathring",  "decoration", 0},
	{"mathrm",  "font", 0},
	{"mathsf",  "font", 0},
	{"mathtt",  "font", 0},
	{"mbox",  "box", 0},
	{"newcommand",  "newcommand", 0 },
	{"nolimits",  "limit", -1},
	{"nonumber",  "nonum", 0},
	{"overbrace",  "decoration", 0},
	{"overleftarrow",  "decoration", 0},
	{"overline",  "decoration", 0},
	{"overrightarrow",  "decoration", 0},
	{"overleftrightarrow", "decoration", 0},
	{"protect",  "protect", 0},
	{"qquad",  "space", 5},
	{"quad",  "space", 4},
	{"right",  "right", 0},
	{"rm",  "oldfont", 0},
	{"scriptscriptstyle",  "style", LM_ST_SCRIPTSCRIPT},
	{"scriptstyle",  "style", LM_ST_SCRIPT},
	{"textbf",  "font", 1},
	{"textit",  "font", 1},
	{"textmd",  "font", 1},
	{"textrm",  "font", 1},
	{"textsl",  "font", 1},
	{"textup",  "font", 1},
	{"textstyle",  "style", LM_ST_TEXT},
	{"tilde",  "decoration", 0},
	{"tt",  "oldfont", 0},
	{"underbar",  "decoration", 0},
	{"underbrace",  "decoration", 0},
	{"underleftarrow", "decoration", 0},
	{"underline",  "decoration", 0},
	{"underrightarrow", "decoration", 0},
	{"underleftrightarrow", "decoration", 0},
	{"underset",  "underset", 0},
	{"vdots",  "dots", 0},
	{"vec",  "decoration", 0},
	{"widehat",  "decoration", 0},
	{"widetilde",  "decoration", 0}
};


bool math_font_available(string & name)
{
	LyXFont f;
	augmentFont(f, name);

	// Do we have the font proper?
	if (fontloader.available(f))
		return true;

	// can we fake it?
	if (name == "eufrak") {
		name = "lyxfakefrak";
		return true;
	}

	lyxerr << "font " << name << " not available and I can't fake it\n";
	return false;
}


void readSymbols(string const & filename)
{
	std::ifstream fs(filename.c_str());
	while (fs) {
		int charid     = 0;
		int fallbackid = 0;
		latexkeys tmp;
		string line;
		getline(fs, line);
		istringstream is(line);
		is	>> tmp.name
				>> tmp.inset
				>> charid
				>> fallbackid
				>> tmp.extra
				>> tmp.xmlname;
		if (!is)
			continue;

		// tmp.inset _is_ the fontname here.
		// create fallbacks if necessary
		if (tmp.extra == "func" || tmp.extra == "funclim" || tmp.extra=="special") {
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
			tmp.inset = "lyxredtext";
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


void initSymbols()
{
	unsigned const n = sizeof(wordlist_array) / sizeof(wordlist_array[0]);
	for (key_type * p = wordlist_array; p != wordlist_array + n; ++p) {
		latexkeys tmp;
		tmp.name  = p->name;
		tmp.inset = p->inset;
		tmp.draw  = p->name;
		theWordList[p->name] = tmp;
	}

	lyxerr[Debug::MATHED] << "reading symbols file\n";
	string const file = LibFileSearch(string(), "symbols");
	if (file.empty())
		lyxerr << "Could not find symbols file\n";
	else
		readSymbols(file);
}


} // namespace anon


latexkeys const * in_word_set(string const & str)
{
	static bool initialized = false;

	if (!initialized) {
		initSymbols();
		initialized = true;
	}

	WordList::iterator it = theWordList.find(str);
	//lyxerr << "looking up '" << str << "' found: "
	// << (it != theWordList.end()) << "\n";
	return (it != theWordList.end()) ? &(it->second) : 0;
}


MathAtom createMathInset(string const & s)
{
	lyxerr[Debug::MATHED] << "creating inset with name: '" << s << "'\n";
	if (s.size() == 2 && s[0] == '#' && s[1] >= '1' && s[1] <= '9')
		return MathAtom(new MathMacroArgument(s[1] - '0'));

	if (s.size() == 3 && s[0] == '\\' && s[1] == '#'
			&& s[2] >= '1' && s[2] <= '9')
		return MathAtom(new MathMacroArgument(s[2] - '0'));
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
	if (s == "pmatrix" || s == "bmatrix" || s == "vmatrix" || s == "Vmatrix" ||
		  s == "matrix")
		return MathAtom(new MathAMSArrayInset(s));
	if (s == "sqrt")
		return MathAtom(new MathSqrtInset);
	if (s == "root")
		return MathAtom(new MathRootInset);
	if (s == "stack")
		return MathAtom(new MathStackrelInset);
	if (s == "binom" || s == "choose")
		return MathAtom(new MathBinomInset);
	if (s == "over" || s == "frac")
		return MathAtom(new MathFracInset);
	if (s == "atop")
		return MathAtom(new MathFracInset(true));
	if (s == "not")
		return MathAtom(new MathNotInset);
	if (s == "lefteqn")
		return MathAtom(new MathLefteqnInset);

	latexkeys const * l = in_word_set(s);
	if (l) {
		string const & inset = l->inset;
		lyxerr[Debug::MATHED] << " found inset: '" << inset << "'\n";
		if (inset == "underset")
			return MathAtom(new MathUndersetInset);
		if (inset == "decoration")
			return MathAtom(new MathDecorationInset(l->name));
		//if (inset == "space")
		//	return MathAtom(new MathSpaceInset(l->id));
		if (inset == "dots")
			return MathAtom(new MathDotsInset(l->name));
		if (inset == "box")
			return MathAtom(new MathBoxInset(l->name));
		if (inset == "style")
			return MathAtom(new MathSizeInset(l));
		if (inset == "font")
			return MathAtom(new MathFontInset(l->name));
		if (inset == "oldfont")
			return MathAtom(new MathFontInset(l->name));
		return MathAtom(new MathSymbolInset(l));
	}

	if (MathMacroTable::has(s))
		return MathAtom(new MathMacro(s));

	//lyxerr[Debug::MATHED] << "creating inset 2 with name: '" << s << "'\n";
	return MathAtom(new MathUnknownInset(s));
}
