/**
 * \file ControlMath.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlMath.h"
#include "ViewBase.h"

#include "BCView.h"

#include "debug.h"
#include "funcrequest.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"
#include "support/filetools.h"


ControlMath::ControlMath(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d), active_(0)
{}


void ControlMath::apply()
{
	view().apply();
}


void ControlMath::dispatchFunc(kb_action action, string const & arg) const
{
	lv_.dispatch(FuncRequest(action, arg));
}


void ControlMath::insertSymbol(string const & sym, bool bs) const
{
	if (bs)
		lv_.dispatch(FuncRequest(LFUN_INSERT_MATH, '\\' + sym));
	else
		lv_.dispatch(FuncRequest(LFUN_INSERT_MATH, sym));
}


void ControlMath::addDaughter(void * key, ViewBase * v,
			      BCView * bcview, ButtonPolicy * bcpolicy)
{
	if (daughters_.find(key) != daughters_.end())
		return;

	daughters_[key] = DaughterPtr(new GUIMathSub(lv_, d_, *this,
						     v, bcview, bcpolicy));
}


void ControlMath::showDaughter(void * key)
{
	Store::iterator it = daughters_.find(key);
	GUIMathSub * const new_active =
		(it == daughters_.end()) ? 0 : it->second.get();

	if (active_ != new_active) {
		if (active_ )
			active_->controller().hide();
		active_ = new_active;
	}

	if (active_)
		active_->controller().show();
}



ControlMathSub::ControlMathSub(LyXView & lv, Dialogs & d, ControlMath const & p)
	: ControlDialogBD(lv, d),
	  parent_(p)
{}


void ControlMathSub::apply()
{
	view().apply();
}


void ControlMathSub::dispatchFunc(kb_action action, string const & arg) const
{
	parent_.dispatchFunc(action, arg);
}


void ControlMathSub::insertSymbol(string const & sym, bool bs) const
{
	parent_.insertSymbol(sym, bs);
}


GUIMathSub::GUIMathSub(LyXView & lv, Dialogs & d,
		       ControlMath const & p,
		       ViewBase * v,
		       BCView * bcview,
		       ButtonPolicy * bcpolicy)
	: controller_(lv, d, p), view_(v)
{
	controller_.setView(*view_);
	view_->setController(controller_);
	controller_.bc().view(bcview);
	controller_.bc().bp(bcpolicy);
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


string const find_xpm(string const & name)
{
#warning Use a static table for this (Lgb)
	// And get O(log n) lookup (Lgb)

	string xpm_name = subst(name, ' ', '_');
	if (xpm_name == "(") xpm_name = "lparen";
	else if (xpm_name == ")") xpm_name = "rparen";
	else if (xpm_name == "[") xpm_name = "lbracket";
	else if (xpm_name == "]") xpm_name = "rbracket";
	else if (xpm_name == "{") xpm_name = "lbrace";
	else if (xpm_name == "}") xpm_name = "rbrace";
	else if (xpm_name == "|") xpm_name = "bars";
	else if (xpm_name == "_") xpm_name = "underscore";
	else if (xpm_name == "textrm_Å") xpm_name = "textrm_A";
	else if (xpm_name == "textrm_Ø") xpm_name = "textrm_0";
	else if (xpm_name == "Bumpeq") xpm_name = "bumpeq2";
	else if (xpm_name == "Cap") xpm_name = "cap2";
	else if (xpm_name == "Cup") xpm_name = "cup2";
	else if (xpm_name == "Delta") xpm_name = "delta2";
	else if (xpm_name == "Downarrow") xpm_name = "downarrow2";
	else if (xpm_name == "Gamma") xpm_name = "gamma2";
	else if (xpm_name == "Lambda") xpm_name = "lambda2";
	else if (xpm_name == "Leftarrow") xpm_name = "leftarrow2";
	else if (xpm_name == "Leftrightarrow") xpm_name = "leftrightarrow2";
	else if (xpm_name == "Longleftarrow") xpm_name = "longleftarrow2";
	else if (xpm_name == "Longleftrightarrow") xpm_name = "longleftrightarrow2";
	else if (xpm_name == "Longrightarrow") xpm_name = "longrightarrow2";
	else if (xpm_name == "nLeftarrow") xpm_name = "nleftarrow2";
	else if (xpm_name == "nLeftrightarrow") xpm_name = "nleftrightarrow2";
	else if (xpm_name == "nRightarrow") xpm_name = "nrightarrow2";
	else if (xpm_name == "nvDash") xpm_name = "nvdash2";
	else if (xpm_name == "nVDash") xpm_name = "nvdash3";
	else if (xpm_name == "Omega") xpm_name = "omega2";
	else if (xpm_name == "Phi") xpm_name = "phi2";
	else if (xpm_name == "Pi") xpm_name = "pi2";
	else if (xpm_name == "Psi") xpm_name = "psi2";
	else if (xpm_name == "Rightarrow") xpm_name = "rightarrow2";
	else if (xpm_name == "Sigma") xpm_name = "sigma2";
	else if (xpm_name == "Subset") xpm_name = "subset2";
	else if (xpm_name == "Supset") xpm_name = "supset2";
	else if (xpm_name == "Theta") xpm_name = "theta2";
	else if (xpm_name == "Uparrow") xpm_name = "uparrow2";
	else if (xpm_name == "Updownarrow") xpm_name = "updownarrow2";
	else if (xpm_name == "Upsilon") xpm_name = "upsilon2";
	else if (xpm_name == "vDash") xpm_name = "vdash2";
	else if (xpm_name == "Vdash") xpm_name = "vdash3";
	else if (xpm_name == "Xi") xpm_name = "xi2";

	lyxerr[Debug::GUI] << "Looking for math XPM called \""
		<< xpm_name << '"' << std::endl;

	return LibFileSearch("images/math/", xpm_name, "xpm");
}
