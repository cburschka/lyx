/**
 * \file ControlMath.C
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include "ControlMath.h"

char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh", ""
};

int const nr_function_names = sizeof(function_names) / sizeof(char const *) - 1;

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

char const * latex_dots[] = {
	"ldots", "cdots", "vdots", "ddots"
};

int const nr_latex_dots = sizeof(latex_dots) / sizeof(char const *);

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
	"backsimeq", "subseteqq", "Subset","sqsupset", "succcurlyeq", "curlyeqsucc",
	"sqsubset", "preccurlyeq", "curlyeqprec", "succsim", "succapprox", "vartriangleright",
	"precsim", "precapprox", "vartriangleleft", "trianglerighteq", "Vdash", "shortmid",
	"trianglelefteq", "vDash", "Vvdash", "shortparallel", "between", "pitchfork",
	"smallsmile", "smallfrown", "bumpeq", "varpropto", "blacktriangleleft", "therefore",
	"Bumpeq", "geqq", "geqslant", "backepsilon", "blacktriangleright", "because", ""
};

int const nr_latex_ams_rel = sizeof(latex_ams_rel) / sizeof(char const *);

char const * latex_ams_nrel[] = {
	"nless", "nleq", "nleqslant", "ngeqslant", "ngeqq", "gneq",
	"nleqq", "lneq", "lneqq","gneqq", "gvertneqq", "gnsim",
	"lvertneqq", "lnsim", "lnapprox", "gnapprox", "nsucc", "nsucceq",
	"nprec", "npreceq", "precnsim","succnsim", "succnapprox", "ncong",
	"precnapprox", "nsim", "nshortmid", "nshortparallel", "nparallel", "nvDash",

	"nmid", "nvdash", "nvDash","nVDash", "ntriangleright", "ntrianglerighteq",
	"ntriangleleft", "ntrianglelefteq", "nsubseteq", "nsupseteq", "nsupseteqq", "supsetneq",
	"subsetneq", "varsubsetneq", "subsetneqq", "varsupsetneq", "supsetneqq", "varsupsetneqq",
	"varsubsetneqq", "ngtr", "ngeq","", "", ""
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
