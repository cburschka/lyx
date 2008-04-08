/**
 * \file ControlMath.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlMath.h"
#include "debug.h"
#include "FuncRequest.h"

#include "support/lyxalgo.h" // sorted
#include "support/lstrings.h"
#include "support/filetools.h"

#include <functional>

using std::string;
using std::map;

namespace lyx {

using support::compare;
using support::libFileSearch;
using support::subst;

namespace frontend {

ControlMath::ControlMath(Dialog & dialog)
	: Dialog::Controller(dialog)
{
	// FIXME: Ideally, those unicode codepoints would be defined
	// in "lib/symbols". Unfortunately, some of those are already
	// defined with non-unicode ids for use within mathed.
	// FIXME 2: We should fill-in this map with the parsed "symbols"
	// file done in MathFactory.cpp.
	math_symbols_["("] = MathSymbol('(');
	math_symbols_[")"] = MathSymbol(')');
	math_symbols_["{"] = MathSymbol('{');
	math_symbols_["}"] = MathSymbol('}');
	math_symbols_["["] = MathSymbol('[');
	math_symbols_["]"] = MathSymbol(']');
	math_symbols_["|"] = MathSymbol('|');
	math_symbols_["/"] = MathSymbol('/', 54, Font::CMSY_FAMILY);
	math_symbols_["backslash"] = MathSymbol('\\', 110, Font::CMSY_FAMILY);
	math_symbols_["lceil"] = MathSymbol(0x2308, 100, Font::CMSY_FAMILY);
	math_symbols_["rceil"] = MathSymbol(0x2309, 101, Font::CMSY_FAMILY);
	math_symbols_["lfloor"] = MathSymbol(0x230A, 98, Font::CMSY_FAMILY);
	math_symbols_["rfloor"] = MathSymbol(0x230B, 99, Font::CMSY_FAMILY);
	math_symbols_["langle"] = MathSymbol(0x2329, 104, Font::CMSY_FAMILY);
	math_symbols_["rangle"] = MathSymbol(0x232A, 105, Font::CMSY_FAMILY);
	math_symbols_["uparrow"] = MathSymbol(0x2191, 34, Font::CMSY_FAMILY);
	math_symbols_["Uparrow"] = MathSymbol(0x21D1, 42, Font::CMSY_FAMILY);
	math_symbols_["updownarrow"] = MathSymbol(0x2195, 108, Font::CMSY_FAMILY);
	math_symbols_["Updownarrow"] = MathSymbol(0x21D5, 109, Font::CMSY_FAMILY);
	math_symbols_["downarrow"] = MathSymbol(0x2193, 35, Font::CMSY_FAMILY);
	math_symbols_["Downarrow"] = MathSymbol(0x21D3, 43, Font::CMSY_FAMILY);
	math_symbols_["downdownarrows"] = MathSymbol(0x21CA, 184, Font::MSA_FAMILY);
	math_symbols_["downharpoonleft"] = MathSymbol(0x21C3, 188, Font::MSA_FAMILY);
	math_symbols_["downharpoonright"] = MathSymbol(0x21C2, 186, Font::MSA_FAMILY);
	math_symbols_["vert"] = MathSymbol(0x007C, 106, Font::CMSY_FAMILY);
	math_symbols_["Vert"] = MathSymbol(0x2016, 107, Font::CMSY_FAMILY);

	std::map<string, MathSymbol>::const_iterator it = math_symbols_.begin();
	std::map<string, MathSymbol>::const_iterator end = math_symbols_.end();
	for (; it != end; ++it)
		tex_names_[it->second.unicode] = it->first;
}


void ControlMath::dispatchFunc(kb_action action, string const & arg) const
{
	kernel().dispatch(FuncRequest(action, arg));
}


void ControlMath::dispatchInsert(string const & name) const
{
	dispatchFunc(LFUN_MATH_INSERT, '\\' + name);
}


void ControlMath::dispatchSubscript() const
{
	dispatchFunc(LFUN_MATH_INSERT, "_");
}


void ControlMath::dispatchSuperscript() const
{
	dispatchFunc(LFUN_MATH_INSERT, "^");
}


void ControlMath::dispatchCubeRoot() const
{
	dispatchFunc(LFUN_MATH_INSERT, "\\root");
	dispatchFunc(LFUN_SELF_INSERT, "3");
	dispatchFunc(LFUN_CHAR_FORWARD);
}


void ControlMath::dispatchMatrix(string const & str) const
{
	dispatchFunc(LFUN_MATH_MATRIX, str);
}


void ControlMath::dispatchDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_DELIM, str);
}


void ControlMath::dispatchBigDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_BIGDELIM, str);
}


void ControlMath::dispatchToggleDisplay() const
{
	dispatchFunc(LFUN_MATH_DISPLAY);
}


void ControlMath::showDialog(string const & name) const
{
	dispatchFunc(LFUN_DIALOG_SHOW, name);
}


MathSymbol const & ControlMath::mathSymbol(string tex_name) const
{
	map<string, MathSymbol>::const_iterator it =
		math_symbols_.find(tex_name);

	static MathSymbol unknown_symbol;
	if (it == math_symbols_.end())
		return unknown_symbol;

	return it->second;
}


std::string const & ControlMath::texName(char_type math_symbol) const
{
	map<char_type, string>::const_iterator it =
		tex_names_.find(math_symbol);

	static string empty_string;
	if (it == tex_names_.end())
		return empty_string;

	return it->second;
}


char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh", "Pr", ""
};

int const nr_function_names = sizeof(function_names) / sizeof(char const *) - 1;

char const * latex_dots[] = {
	"ldots", "cdots", "vdots", "ddots", ""
};

int const nr_latex_dots = sizeof(latex_dots) / sizeof(char const *) - 1;

char const * latex_deco[] = {
	"widehat", "widetilde", "overbrace", "overleftarrow", "overrightarrow",
	"overline", "underbrace", "underline", "underleftarrow", "underrightarrow",
	"underleftrightarrow", "overleftrightarrow",
	"hat", "acute", "bar", "dot",
	"check", "grave", "vec", "ddot",
	"breve", "tilde", "overset", "underset", ""
};

int const nr_latex_deco = sizeof(latex_deco) / sizeof(char const *) - 1;

char const * latex_arrow[] = {
	"leftarrow", "rightarrow",
	"downarrow", "uparrow", "updownarrow", "leftrightarrow",
	"Leftarrow", "Rightarrow",
	"Downarrow", "Uparrow", "Updownarrow", "Leftrightarrow",
	"Longleftrightarrow", "Longleftarrow", "Longrightarrow",
	"longleftrightarrow", "longleftarrow", "longrightarrow",
	"leftharpoondown", "rightharpoondown",
	"mapsto", "longmapsto",
	"nwarrow", "nearrow",
	"leftharpoonup", "rightharpoonup",
	"hookleftarrow", "hookrightarrow",
	"swarrow", "searrow",
	"rightleftharpoons",
	"",
};

int const nr_latex_arrow = sizeof(latex_arrow) / sizeof(char const *);

char const * latex_bop[] = {
	"pm", "cap", "diamond", "oplus",
	"mp", "cup", "bigtriangleup", "ominus",
	"times", "uplus", "bigtriangledown", "otimes",
	"div", "sqcap", "triangleright", "oslash",
	"cdot", "sqcup", "triangleleft", "odot",
	"star", "vee", "amalg", "bigcirc",
	"setminus", "wedge", "dagger", "circ",
	"bullet", "wr", "ddagger", ""
};

int const nr_latex_bop = sizeof(latex_bop) / sizeof(char const *);

char const * latex_brel[] = {
	"leq", "geq", "equiv", "models",
	"prec", "succ", "sim", "perp",
	"preceq", "succeq", "simeq", "mid",
	"ll", "gg", "asymp", "parallel",
	"subset", "supset", "approx", "smile",
	"subseteq", "supseteq", "cong", "frown",
	"sqsubseteq", "sqsupseteq", "doteq", "neq",
	"in", "ni", "propto", "notin",
	"vdash", "dashv", "bowtie", ""
};

int const nr_latex_brel = sizeof(latex_brel) / sizeof(char const *);

char const * latex_greek[] = {
	"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi",
	"Sigma", "Upsilon", "Phi", "Psi", "Omega",
	"alpha", "beta", "gamma", "delta", "epsilon", "varepsilon", "zeta",
	"eta", "theta", "vartheta", "iota", "kappa", "lambda", "mu",
	"nu", "xi", "pi", "varpi", "rho", "sigma", "varsigma",
	"tau", "upsilon", "phi", "varphi", "chi", "psi", "omega", ""
};

int const nr_latex_greek = sizeof(latex_greek) / sizeof(char const *);

char const * latex_misc[] = {
	"nabla", "partial", "infty", "prime", "ell",
	"emptyset", "exists", "forall", "imath",  "jmath",
	"Re", "Im", "aleph", "wp", "hbar",
	"angle", "top", "bot", "Vert", "neg",
	"flat", "natural", "sharp", "surd", "triangle",
	"diamondsuit", "heartsuit", "clubsuit", "spadesuit",
	"textrm \\AA", "textrm \\O", "mathcircumflex", "_",
	"mathrm T",
	"mathbb N", "mathbb Z", "mathbb Q",
	"mathbb R", "mathbb C", "mathbb H",
	"mathcal F", "mathcal L",
	"mathcal H", "mathcal O",
	"phantom", "vphantom", "hphantom", ""
};

int const nr_latex_misc = sizeof(latex_misc) / sizeof(char const *);

char const * latex_varsz[] = {
	"sum", "int", "intop", "iint", "iintop", "iiint", "iiintop",
	"iiiint", "iiiintop", "dotsint", "dotsintop",
	"oint", "ointop", "oiint", "oiintop", "ointctrclockwise",
	"ointctrclockwiseop", "ointclockwise", "ointclockwiseop",
	"sqint", "sqintop", "sqiint", "sqiintop",
	"prod", "coprod", "bigsqcup",
	"bigotimes", "bigodot", "bigoplus",
	"bigcap", "bigcup", "biguplus",
	"bigvee", "bigwedge", ""
};

int const nr_latex_varsz = sizeof(latex_varsz) / sizeof(char const *);

char const * latex_ams_misc[] = {
	"digamma", "varkappa", "beth", "daleth", "gimel",
	"ulcorner", "urcorner", "llcorner", "lrcorner",
	"hbar", "hslash", "vartriangle",
	"triangledown", "square", "lozenge",
	"circledS", "angle", "measuredangle",
	"nexists", "mho", "Finv",
	"Game", "Bbbk", "backprime",
	"varnothing", "blacktriangle", "blacktriangledown",
	"blacksquare", "blacklozenge", "bigstar",
	"sphericalangle", "complement", "eth",
	"diagup", "diagdown", ""
};

int const nr_latex_ams_misc = sizeof(latex_ams_misc) / sizeof(char const *);

char const * latex_ams_arrows[] = {
	"dashleftarrow", "dashrightarrow",
	"leftleftarrows", "leftrightarrows",
	"rightrightarrows", "rightleftarrows",
	"Lleftarrow", "Rrightarrow",
	"twoheadleftarrow", "twoheadrightarrow",
	"leftarrowtail", "rightarrowtail",
	"looparrowleft", "looparrowright",
	"curvearrowleft", "curvearrowright",
	"circlearrowleft", "circlearrowright",
	"Lsh", "Rsh",
	"upuparrows", "downdownarrows",
	"upharpoonleft", "upharpoonright",
	"downharpoonleft", "downharpoonright",
	"leftrightharpoons", "rightleftharpoons",
	"rightsquigarrow", "leftrightsquigarrow",
	"nleftarrow", "nrightarrow", "nleftrightarrow",
	"nLeftarrow", "nRightarrow", "nLeftrightarrow",
	"multimap",
	""
};

int const nr_latex_ams_arrows = sizeof(latex_ams_arrows) / sizeof(char const *);

char const * latex_ams_rel[] = {
	"leqq", "geqq",
	"leqslant", "geqslant",
	"eqslantless", "eqslantgtr",
	"lesssim", "gtrsim",
	"lessapprox", "gtrapprox",
	"approxeq", "triangleq",
	"lessdot", "gtrdot",
	"lll", "ggg",
	"lessgtr", "gtrless",
	"lesseqgtr", "gtreqless",
	"lesseqqgtr", "gtreqqless",
	"eqcirc", "circeq",
	"thicksim", "thickapprox",
	"backsim", "backsimeq",
	"subseteqq", "supseteqq",
	"Subset", "Supset",
	"sqsubset", "sqsupset",
	"preccurlyeq", "succcurlyeq",
	"curlyeqprec", "curlyeqsucc",
	"precsim", "succsim",
	"precapprox", "succapprox",
	"vartriangleleft", "vartriangleright",
	"trianglelefteq", "trianglerighteq",
	"bumpeq", "Bumpeq",
	"doteqdot", "risingdotseq", "fallingdotseq",
	"vDash", "Vvdash", "Vdash",
	"shortmid", "shortparallel",
	"smallsmile", "smallfrown",
	"blacktriangleleft", "blacktriangleright",
	"because", "therefore",
	"backepsilon",
	"varpropto",
	"between",
	"pitchfork",
	""
};

int const nr_latex_ams_rel = sizeof(latex_ams_rel) / sizeof(char const *);

char const * latex_ams_nrel[] = {
	"nless", "ngtr",
	"nleq", "ngeq",
	"nleqslant", "ngeqslant",
	"nleqq", "ngeqq",
	"lneq", "gneq",
	"lneqq", "gneqq",
	"lvertneqq", "gvertneqq",
	"lnsim", "gnsim",
	"lnapprox", "gnapprox",
	"nprec", "nsucc",
	"npreceq", "nsucceq",
	"precnsim", "succnsim",
	"precnapprox", "succnapprox",
	"subsetneq", "supsetneq",
	"subsetneqq", "supsetneqq",
	"nsubseteq", "nsupseteq", "nsupseteqq",
	"nvdash", "nvDash", "nVDash",
	"varsubsetneq", "varsupsetneq",
	"varsubsetneqq", "varsupsetneqq",
	"ntriangleleft", "ntriangleright",
	"ntrianglelefteq", "ntrianglerighteq",
	"ncong", "nsim",
	"nmid", "nshortmid",
	"nparallel", "nshortparallel",
	"", "", ""
};

int const nr_latex_ams_nrel = sizeof(latex_ams_nrel) / sizeof(char const *);


char const * latex_ams_ops[] = {
	"dotplus", "smallsetminus", "Cap",
	"Cup", "barwedge", "veebar",
	"doublebarwedge", "boxminus", "boxtimes",
	"boxdot", "boxplus", "divideontimes",
	"ltimes", "rtimes", "leftthreetimes",
	"rightthreetimes", "curlywedge", "curlyvee",
	"circleddash", "circledast", "circledcirc",
	"centerdot", "intercal", ""
};

int const nr_latex_ams_ops = sizeof(latex_ams_ops) / sizeof(char const *);


char const *  latex_delimiters[] = {
	"(", ")", "{", "}", "[", "]",
	"lceil", "rceil", "lfloor", "rfloor", "langle", "rangle",
	"uparrow", "updownarrow", "Uparrow", "Updownarrow", "downarrow", "Downarrow",
	"|", "Vert", "/", "backslash", ""
};


int const nr_latex_delimiters = sizeof(latex_delimiters) / sizeof(char const *);

namespace {

struct XPMmap {
	char const * key;
	char const * value;
};


bool operator<(XPMmap const & lhs, XPMmap const & rhs)
{
		return compare(lhs.key, rhs.key) < 0;
}


class CompareKey : public std::unary_function<XPMmap, bool> {
public:
	CompareKey(string const & name) : name_(name) {}
	bool operator()(XPMmap const & other) const {
		return other.key == name_;
	}
private:
	string const name_;
};


// this must be sorted alphabetically
// Upper case comes before lower case
XPMmap sorted_xpm_map[] = {
	{ "Bumpeq", "bumpeq2" },
	{ "Cap", "cap2" },
	{ "Cup", "cup2" },
	{ "Delta", "delta2" },
	{ "Downarrow", "downarrow2" },
	{ "Gamma", "gamma2" },
	{ "Lambda", "lambda2" },
	{ "Leftarrow", "leftarrow2" },
	{ "Leftrightarrow", "leftrightarrow2" },
	{ "Longleftarrow", "longleftarrow2" },
	{ "Longleftrightarrow", "longleftrightarrow2" },
	{ "Longrightarrow", "longrightarrow2" },
	{ "Omega", "omega2" },
	{ "Phi", "phi2" },
	{ "Pi", "pi2" },
	{ "Psi", "psi2" },
	{ "Rightarrow", "rightarrow2" },
	{ "Sigma", "sigma2" },
	{ "Subset", "subset2" },
	{ "Supset", "supset2" },
	{ "Theta", "theta2" },
	{ "Uparrow", "uparrow2" },
	{ "Updownarrow", "updownarrow2" },
	{ "Upsilon", "upsilon2" },
	{ "Vdash", "vdash3" },
	{ "Vert", "vert2" },
	{ "Xi", "xi2" },
	{ "nLeftarrow", "nleftarrow2" },
	{ "nLeftrightarrow", "nleftrightarrow2" },
	{ "nRightarrow", "nrightarrow2" },
	{ "nVDash", "nvdash3" },
	{ "nvDash", "nvdash2" },
	{ "textrm \\AA", "textrm_AA"},
	{ "textrm \\O", "textrm_Oe"},
	{ "vDash", "vdash2" }
};

size_t const nr_sorted_xpm_map = sizeof(sorted_xpm_map) / sizeof(XPMmap);

} // namespace anon


string const find_xpm(string const & name)
{
	XPMmap const * const begin = sorted_xpm_map;
	XPMmap const * const end = begin + nr_sorted_xpm_map;
	BOOST_ASSERT(sorted(begin, end));

	XPMmap const * const it =
		std::find_if(begin, end, CompareKey(name));

	string xpm_name;
	if (it != end)
		xpm_name = it->value;
	else {
		xpm_name = subst(name, "_", "underscore");
		xpm_name = subst(xpm_name, ' ', '_');

		// This way we can have "math-delim { }" on the toolbar.
		xpm_name = subst(xpm_name, "(", "lparen");
		xpm_name = subst(xpm_name, ")", "rparen");
		xpm_name = subst(xpm_name, "[", "lbracket");
		xpm_name = subst(xpm_name, "]", "rbracket");
		xpm_name = subst(xpm_name, "{", "lbrace");
		xpm_name = subst(xpm_name, "}", "rbrace");
		xpm_name = subst(xpm_name, "|", "bars");
		xpm_name = subst(xpm_name, ",", "thinspace");
		xpm_name = subst(xpm_name, ":", "mediumspace");
		xpm_name = subst(xpm_name, ";", "thickspace");
		xpm_name = subst(xpm_name, "!", "negthinspace");
	}

	LYXERR(Debug::GUI) << "find_xpm(" << name << ")\n"
			   << "Looking for math XPM called \""
			   << xpm_name << '"' << std::endl;

	return libFileSearch("images/math/", xpm_name, "xpm").absFilename();
}

} // namespace frontend
} // namespace lyx
