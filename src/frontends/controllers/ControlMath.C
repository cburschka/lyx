/**
 * \file ControlMath.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlMath.h"
#include "Kernel.h"
#include "debug.h"
#include "funcrequest.h"

#include "support/LAssert.h"
#include "support/lyxalgo.h" // sorted
#include "support/lstrings.h"
#include "support/filetools.h"


using namespace lyx::support;


ControlMath::ControlMath(Dialog & dialog)
	: Dialog::Controller(dialog)
{}


void ControlMath::dispatchFunc(kb_action action, string const & arg) const
{
	kernel().dispatch(FuncRequest(action, arg));
}


void ControlMath::dispatchInsert(string const & name) const
{
	dispatchFunc(LFUN_INSERT_MATH, '\\' + name);
}


void ControlMath::dispatchSubscript() const
{
	dispatchFunc(LFUN_SUBSCRIPT);
}


void ControlMath::dispatchSuperscript() const
{
	dispatchFunc(LFUN_SUPERSCRIPT);
}


void ControlMath::dispatchCubeRoot() const
{
	dispatchFunc(LFUN_INSERT_MATH, "\\root");
	dispatchFunc(LFUN_SELFINSERT, "3");
	dispatchFunc(LFUN_RIGHT);
}


void ControlMath::dispatchMatrix(string const & str) const
{
	dispatchFunc(LFUN_INSERT_MATRIX, str);
}


void ControlMath::dispatchDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_DELIM, str);
}


void ControlMath::dispatchToggleDisplay() const
{
	dispatchFunc(LFUN_MATH_DISPLAY);
}


void ControlMath::showDialog(string const & name) const
{
	dispatchFunc(LFUN_DIALOG_SHOW, name);
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
	"breve", "tilde", ""
};

int const nr_latex_deco = sizeof(latex_deco) / sizeof(char const *) - 1;

char const * latex_arrow[] = {
	"downarrow", "leftarrow", "Downarrow", "Leftarrow",
	"hookleftarrow", "rightarrow", "uparrow", "Rightarrow", "Uparrow",
	"hookrightarrow", "updownarrow", "Leftrightarrow", "leftharpoonup",
	"rightharpoonup", "rightleftharpoons", "leftrightarrow", "Updownarrow",
	"leftharpoondown", "rightharpoondown", "mapsto",
	"Longleftarrow", "Longrightarrow", "Longleftrightarrow",
	"longleftrightarrow", "longleftarrow", "longrightarrow", "longmapsto",
	"nwarrow", "nearrow", "swarrow", "searrow",  "",
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
	"textrm Å", "textrm Ø", "mathcircumflex", "_",
	"mathrm T",
	"mathbb N", "mathbb Z", "mathbb Q",
	"mathbb R", "mathbb C", "mathbb H",
	"mathcal F", "mathcal L",
	"mathcal H", "mathcal O", ""
};

int const nr_latex_misc = sizeof(latex_misc) / sizeof(char const *);

char const * latex_varsz[] = {
	"sum", "int", "oint",
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
	"dashrightarrow", "dashleftarrow", "leftleftarrows",
	"leftrightarrows", "Lleftarrow", "twoheadleftarrow",
	"leftarrowtail", "looparrowleft", "leftrightharpoons",
	"curvearrowleft", "circlearrowleft", "Lsh",
	"upuparrows", "upharpoonleft", "downharpoonleft",
	"multimap", "leftrightsquigarrow", "rightrightarrows",
	"rightleftarrows", "rightrightarrows", "rightleftarrows",
	"twoheadrightarrow", "rightarrowtail", "looparrowright",
	"rightleftharpoons", "curvearrowright", "circlearrowright",
	"Rsh", "downdownarrows", "upharpoonright",
	"downharpoonright", "rightsquigarrow",
	"nleftarrow", "nrightarrow", "nLeftarrow",
	"nRightarrow", "nleftrightarrow", "nLeftrightarrow", ""
};

int const nr_latex_ams_arrows = sizeof(latex_ams_arrows) / sizeof(char const *);

char const * latex_ams_rel[] = {
	"leqq", "leqslant", "eqslantless", "eqslantgtr", "gtrsim", "gtrapprox",
	"lesssim", "lessapprox", "approxeq", "gtrdot", "ggg", "gtrless",
	"lessdot", "lll", "lessgtr", "gtreqless", "gtreqqless", "eqcirc",
	"lesseqgtr", "lesseqqgtr", "doteqdot", "circeq", "triangleq", "thicksim",
	"risingdotseq", "fallingdotseq", "backsim", "thickapprox", "supseteqq", "Supset",
	"backsimeq", "subseteqq", "Subset", "sqsupset", "succcurlyeq", "curlyeqsucc",
	"sqsubset", "preccurlyeq", "curlyeqprec", "succsim", "succapprox", "vartriangleright",
	"precsim", "precapprox", "vartriangleleft", "trianglerighteq", "Vdash", "shortmid",
	"trianglelefteq", "vDash", "Vvdash", "shortparallel", "between", "pitchfork",
	"smallsmile", "smallfrown", "bumpeq", "varpropto", "blacktriangleleft", "therefore",
	"Bumpeq", "geqq", "geqslant", "backepsilon", "blacktriangleright", "because", ""
};

int const nr_latex_ams_rel = sizeof(latex_ams_rel) / sizeof(char const *);

char const * latex_ams_nrel[] = {
	"nless", "nleq", "nleqslant", "ngeqslant", "ngeqq", "gneq",
	"nleqq", "lneq", "lneqq", "gneqq", "gvertneqq", "gnsim",
	"lvertneqq", "lnsim", "lnapprox", "gnapprox", "nsucc", "nsucceq",
	"nprec", "npreceq", "precnsim","succnsim", "succnapprox", "ncong",
	"precnapprox", "nsim", "nshortmid", "nshortparallel", "nparallel", "nvDash",

	"nmid", "nvdash", "nvDash", "nVDash", "ntriangleright", "ntrianglerighteq",
	"ntriangleleft", "ntrianglelefteq", "nsubseteq", "nsupseteq", "nsupseteqq", "supsetneq",
	"subsetneq", "varsubsetneq", "subsetneqq", "varsupsetneq", "supsetneqq", "varsupsetneqq",
	"varsubsetneqq", "ngtr", "ngeq", "", "", ""
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


namespace {

struct XPMmap {
	char const * key;
	char const * value;
};


bool operator<(XPMmap const & lhs, XPMmap const & rhs)
{
		return compare(lhs.key, rhs.key) < 0;
}


struct CompareKey {
	CompareKey(string const & name) : name_(name) {}
	bool operator()(XPMmap const & other) {
		return compare(other.key, name_.c_str()) == 0;
	}
private:
	string const name_;
};


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
	{ "Xi", "xi2" },
	{ "nLeftarrow", "nleftarrow2" },
	{ "nLeftrightarrow", "nleftrightarrow2" },
	{ "nRightarrow", "nrightarrow2" },
	{ "nVDash", "nvdash3" },
	{ "nvDash", "nvdash2" },
	{ "textrm_Å", "textrm_A" },
	{ "textrm_Ø", "textrm_0" },
	{ "vDash", "vdash2" }
};

size_t const nr_sorted_xpm_map = sizeof(sorted_xpm_map) / sizeof(XPMmap);

} // namespace anon


string const find_xpm(string const & name)
{
	XPMmap const * const begin = sorted_xpm_map;
	XPMmap const * const end = begin + nr_sorted_xpm_map;
	Assert(lyx::sorted(begin, end));

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
	}

	lyxerr[Debug::GUI] << "find_xpm(" << name << ")\n"
			   << "Looking for math XPM called \""
			   << xpm_name << '"' << std::endl;

	return LibFileSearch("images/math/", xpm_name, "xpm");
}
