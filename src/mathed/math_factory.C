#include <config.h>

#include "math_parser.h"
#include "math_arrayinset.h"
#include "math_amsarrayinset.h"
#include "math_binominset.h"
#include "math_boxinset.h"
#include "math_casesinset.h"
#include "math_decorationinset.h"
#include "math_dotsinset.h"
#include "math_fboxinset.h"
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

#include "ref_inset.h"

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
	string extra;
};


key_type wordlist_array[] =
{
	{"!",  "space", ""},
	{",",  "space", ""},
	{":",  "space", ""},
	{";",  "space", ""},
	{"Vmatrix",  "matrix", ""},
	{"acute",  "decoration", ""},
	{"bar",  "decoration", ""},
	{"begin",  "begin", ""},
	{"bf",  "oldfont", ""},
	{"bmatrix",  "matrix", ""},
	{"acute",  "decoration", ""},
	{"breve",  "decoration", ""},
	{"cal",  "oldfont", ""},
	{"cdots",  "dots", ""},
	{"check",  "decoration", ""},
	{"ddot",  "decoration", ""},
	{"dddot",  "decoration", ""},
	{"ddots",  "dots", ""},
	{"displaystyle",  "style", ""},
	{"dot",  "decoration", ""},
	{"dotsb",  "dots", ""},
	{"dotsc",  "dots", ""},
	{"dotsi",  "dots", ""},
	{"dotsm",  "dots", ""},
	{"dotso",  "dots", ""},
	{"end",  "end", ""},
	{"fbox",  "fbox", ""},
	{"frak",  "font", ""},
	{"grave",  "decoration", ""},
	{"hat",  "decoration", ""},
	{"it",  "oldfont", ""},
	{"label",  "label", ""},
	{"ldots",  "dots", ""},
	{"left",  "left", ""},
	{"limits",  "limit", ""},
	{"lyxbox",  "box", ""},
	{"lyxnegspace",  "space", ""},
	{"lyxposspace",  "space", ""},
	{"mathbb",  "font", ""},
	{"mathbf",  "font", ""},
	{"mathcal",  "font", ""},
	{"mathfrak",  "font", ""},
	{"mathit",  "font", ""},
	{"mathnormal",  "font", ""},
	{"mathring",  "decoration", ""},
	{"mathrm",  "font", ""},
	{"mathsf",  "font", ""},
	{"mathtt",  "font", ""},
	{"matrix",  "matrix", ""},
	{"mbox",  "box", ""},
	{"newcommand",  "newcommand", ""},
	{"nolimits",  "limit", ""},
	{"nonumber",  "nonum", ""},
	{"overbrace",  "decoration", ""},
	{"overleftarrow",  "decoration", ""},
	{"overline",  "decoration", ""},
	{"overrightarrow",  "decoration", ""},
	{"overleftrightarrow", "decoration", ""},
	{"pmatrix",  "matrix", ""},
	{"protect",  "protect", ""},
	{"qquad",  "space", ""},
	{"quad",  "space", ""},
	{"right",  "right", ""},
	{"rm",  "oldfont", ""},
	{"scriptscriptstyle",  "style", ""},
	{"scriptstyle",  "style", ""},
	{"text",    "font", "mathtext"},
	{"textbf",  "font", "mathtext"},
	{"textit",  "font", "mathtext"},
	{"textmd",  "font", "mathtext"},
	{"textrm",  "font", "mathtext"},
	{"textsl",  "font", "mathtext"},
	{"textup",  "font", "mathtext"},
	{"textstyle",  "style", ""},
	{"tilde",  "decoration", ""},
	{"tt",  "oldfont", ""},
	{"underbar",  "decoration", ""},
	{"underbrace",  "decoration", ""},
	{"underleftarrow", "decoration", ""},
	{"underline",  "decoration", ""},
	{"underrightarrow", "decoration", ""},
	{"underleftrightarrow", "decoration", ""},
	{"underset",  "underset", ""},
	{"vdots",  "dots", ""},
	{"vec",  "decoration", ""},
	{"vmatrix",  "matrix", ""},
	{"widehat",  "decoration", ""},
	{"widetilde",  "decoration", ""}
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

	lyxerr[Debug::MATHED] << "font " << name << " not available and I can't fake it\n";
	return false;
}


void readSymbols(string const & filename)
{
	lyxerr[Debug::MATHED] << "read symbols from " << filename << "\n";
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
			tmp.inset = "lyxtex";
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
	if (s == "sqrt")
		return MathAtom(new MathSqrtInset);
	if (s == "root")
		return MathAtom(new MathRootInset);
	if (s == "stack")
		return MathAtom(new MathStackrelInset);
	if (s == "binom" || s == "choose")
		return MathAtom(new MathBinomInset(s == "choose"));
	if (s == "over" || s == "frac")
		return MathAtom(new MathFracInset);
	if (s == "atop")
		return MathAtom(new MathFracInset(true));
	if (s == "not")
		return MathAtom(new MathNotInset);
	if (s == "lefteqn")
		return MathAtom(new MathLefteqnInset);
	if (s == "ref")
		return MathAtom(new RefInset);


	latexkeys const * l = in_word_set(s);
	if (l) {
		string const & inset = l->inset;
		lyxerr[Debug::MATHED] << " found inset: '" << inset << "'\n";
		if (inset == "underset")
			return MathAtom(new MathUndersetInset);
		if (inset == "decoration")
			return MathAtom(new MathDecorationInset(l->name));
		if (inset == "space")
			return MathAtom(new MathSpaceInset(l->name));
		if (inset == "dots")
			return MathAtom(new MathDotsInset(l->name));
		if (inset == "box")
			return MathAtom(new MathBoxInset(l->name));
		if (inset == "fbox")
			return MathAtom(new MathFboxInset);
		if (inset == "style")
			return MathAtom(new MathSizeInset(l));
		if (inset == "font")
			return MathAtom(new MathFontInset(l->name));
		if (inset == "oldfont")
			return MathAtom(new MathFontInset(l->name));
		if (inset == "matrix")
			return MathAtom(new MathAMSArrayInset(s));
		return MathAtom(new MathSymbolInset(l));
	}

	if (MathMacroTable::has(s))
		return MathAtom(new MathMacro(s));

	//lyxerr[Debug::MATHED] << "creating inset 2 with name: '" << s << "'\n";
	return MathAtom(new MathUnknownInset(s));
}
