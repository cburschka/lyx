#include <config.h>
#include <map>

#include "math_defs.h"
#include "math_parser.h"
#include "support/lstrings.h"
#include <iostream>

namespace {

latexkeys const wordlist[] = 
{
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY, LMB_NONE},
	//{"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT, LMB_NONE},
	//{"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT, LMB_NONE},
	//{"textstyle",  LM_TK_STY, LM_ST_TEXT, LMB_NONE},
	{"Delta",  LM_TK_SYM, LM_Delta, LMB_NONE},
	{"Downarrow",  LM_TK_SYM, LM_Downarrow, LMB_NONE},
	{"Gamma",  LM_TK_SYM, LM_Gamma, LMB_NONE},
	{"Im",  LM_TK_SYM, LM_Im, LMB_NONE},
	{"Lambda",  LM_TK_SYM, LM_Lambda, LMB_NONE},
	{"Leftarrow",  LM_TK_SYM, LM_Leftarrow, LMB_NONE},
	{"Leftrightarrow",  LM_TK_SYM, LM_Leftrightarrow, LMB_NONE},
	{"Longleftarrow",  LM_TK_SYM, LM_Longleftarrow, LMB_NONE},
	{"Longleftrightarrow",  LM_TK_SYM, LM_Longleftrightarrow, LMB_NONE},
	{"Longrightarrow",  LM_TK_SYM, LM_Longrightarrow, LMB_NONE},
	{"Omega",  LM_TK_SYM, LM_Omega, LMB_NONE},
	{"Phi",  LM_TK_SYM, LM_Phi, LMB_NONE},
	{"Pi",  LM_TK_SYM, LM_Pi, LMB_NONE},
	{"Pr",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"Psi",  LM_TK_SYM, LM_Psi, LMB_NONE},
	{"Re",  LM_TK_SYM, LM_Re, LMB_NONE},
	{"Rightarrow",  LM_TK_SYM, LM_Rightarrow, LMB_NONE},
	{"Sigma",  LM_TK_SYM, LM_Sigma, LMB_NONE},
	{"Theta",  LM_TK_SYM, LM_Theta, LMB_NONE},
	{"Uparrow",  LM_TK_SYM, LM_Uparrow, LMB_NONE},
	{"Updownarrow",  LM_TK_SYM, LM_Updownarrow, LMB_NONE},
	{"Upsilon",  LM_TK_SYM, LM_Upsilon, LMB_NONE},
	{"Vert",  LM_TK_SYM, LM_Vert, LMB_NONE},
	{"Xi",  LM_TK_SYM, LM_Xi, LMB_NONE},
	{"acute",  LM_TK_DECORATION, LM_acute, LMB_NONE},
	{"aleph",  LM_TK_SYM, LM_aleph, LMB_NONE},
	{"alpha",  LM_TK_SYM, LM_alpha, LMB_NONE},
	{"amalg",  LM_TK_SYM, LM_amalg, LMB_OPERATOR},
	{"angle",  LM_TK_SYM, LM_angle, LMB_NONE},
	{"approx",  LM_TK_SYM, LM_approx, LMB_RELATION},
	{"arccos",  LM_TK_FUNC, 0, LMB_NONE},
	{"arcsin",  LM_TK_FUNC, 0, LMB_NONE},
	{"arctan",  LM_TK_FUNC, 0, LMB_NONE},
	{"arg",  LM_TK_FUNC, 0, LMB_NONE},
	{"asymp",  LM_TK_SYM, LM_asymp, LMB_RELATION},
	{"backslash",  LM_TK_SYM, LM_backslash, LMB_NONE},
	{"bar",  LM_TK_DECORATION, LM_bar, LMB_NONE},
	{"begin",  LM_TK_BEGIN, 0, LMB_NONE},
	{"beta",  LM_TK_SYM, LM_beta, LMB_NONE},
	{"bigcap",  LM_TK_BIGSYM, LM_cap, LMB_NONE},
	{"bigcirc",  LM_TK_SYM, LM_bigcirc, LMB_OPERATOR},
	{"bigcup",  LM_TK_BIGSYM, LM_cup, LMB_NONE},
	{"bigodot",  LM_TK_BIGSYM, LM_bigodot, LMB_NONE},
	{"bigoplus",  LM_TK_BIGSYM, LM_oplus, LMB_NONE},
	{"bigotimes",  LM_TK_BIGSYM, LM_otimes, LMB_NONE},
	{"bigsqcup",  LM_TK_BIGSYM, LM_bigsqcup, LMB_NONE},
	{"bigtriangledown",  LM_TK_SYM, LM_bigtriangledown, LMB_OPERATOR},
	{"bigtriangleup",  LM_TK_SYM, LM_bigtriangleup, LMB_OPERATOR},
	{"biguplus",  LM_TK_BIGSYM, LM_biguplus, LMB_NONE},
	{"bigvee",  LM_TK_BIGSYM, LM_vee, LMB_NONE},
	{"bigwedge",  LM_TK_BIGSYM, LM_wedge, LMB_NONE},
	{"bmod",  LM_TK_FUNC, 0, LMB_NONE},
	{"bot",  LM_TK_SYM, LM_bot, LMB_NONE},
	{"bowtie",  LM_TK_SYM, LM_bowtie, LMB_RELATION},
	{"breve",  LM_TK_DECORATION, LM_breve, LMB_NONE},
	{"bullet",  LM_TK_SYM, LM_bullet, LMB_OPERATOR},
	{"cap",  LM_TK_SYM, LM_cap, LMB_OPERATOR},
	{"cdot",  LM_TK_SYM, LM_cdot, LMB_OPERATOR},
	{"cdots",  LM_TK_DOTS, LM_cdots, LMB_NONE},
	{"check",  LM_TK_DECORATION, LM_check, LMB_NONE},
	{"chi",  LM_TK_SYM, LM_chi, LMB_NONE},
	{"circ",  LM_TK_SYM, LM_circ, LMB_OPERATOR},
	{"clubsuit",  LM_TK_SYM, LM_clubsuit, LMB_NONE},
	{"cong",  LM_TK_SYM, LM_cong, LMB_RELATION},
	{"coprod",  LM_TK_BIGSYM, LM_coprod, LMB_NONE},
	{"cos",  LM_TK_FUNC, 0, LMB_NONE},
	{"cosh",  LM_TK_FUNC, 0, LMB_NONE},
	{"cot",  LM_TK_FUNC, 0, LMB_NONE},
	{"coth",  LM_TK_FUNC, 0, LMB_NONE},
	{"csc",  LM_TK_FUNC, 0, LMB_NONE},
	{"cup",  LM_TK_SYM, LM_cup, LMB_OPERATOR},
	{"dagger",  LM_TK_SYM, LM_dagger, LMB_OPERATOR},
	{"dashv",  LM_TK_SYM, LM_dashv, LMB_RELATION},
	{"ddagger",  LM_TK_SYM, LM_ddagger, LMB_OPERATOR},
	{"ddot",  LM_TK_DECORATION, LM_ddot, LMB_NONE},
	{"ddots",  LM_TK_DOTS, LM_ddots, LMB_NONE},
	{"deg",  LM_TK_FUNC, 0, LMB_NONE},
	{"delta",  LM_TK_SYM, LM_delta, LMB_NONE},
	{"det",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"diamond",  LM_TK_SYM, LM_diamond, LMB_OPERATOR},
	{"diamondsuit",  LM_TK_SYM, LM_diamondsuit, LMB_NONE},
	{"dim",  LM_TK_FUNC, 0, LMB_NONE},
	{"div",  LM_TK_SYM, LM_div, LMB_OPERATOR},
	{"dot",  LM_TK_DECORATION, LM_dot, LMB_NONE},
	{"doteq",  LM_TK_SYM, LM_doteq, LMB_RELATION},
	{"downarrow",  LM_TK_SYM, LM_downarrow, LMB_NONE},
	{"ell",  LM_TK_SYM, LM_ell, LMB_NONE},
	{"emptyset",  LM_TK_MACRO, LM_emptyset, LMB_NONE},
	{"end",  LM_TK_END, 0, LMB_NONE},
	{"epsilon",  LM_TK_SYM, LM_epsilon, LMB_NONE},
	{"equiv",  LM_TK_SYM, LM_equiv, LMB_RELATION},
	{"eta",  LM_TK_SYM, LM_eta, LMB_NONE},
	{"exists",  LM_TK_SYM, LM_exists, LMB_NONE},
	{"exp",  LM_TK_FUNC, 0, LMB_NONE},
	{"flat",  LM_TK_SYM, LM_flat, LMB_NONE},
	{"forall",  LM_TK_SYM, LM_forall, LMB_NONE},
	{"frac",  LM_TK_FRAC, 0, LMB_NONE},
	{"frown",  LM_TK_SYM, LM_frown, LMB_RELATION},
	{"gamma",  LM_TK_SYM, LM_gamma, LMB_NONE},
	{"gcd",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"geq",  LM_TK_SYM, LM_geq, LMB_RELATION},
	{"gg",  LM_TK_SYM, LM_gg, LMB_RELATION},
	{"grave",  LM_TK_DECORATION, LM_grave, LMB_NONE},
	{"hat",  LM_TK_DECORATION, LM_hat, LMB_NONE},
	{"hbar",  LM_TK_SYM, LM_hbar, LMB_NONE},
	{"heartsuit",  LM_TK_SYM, LM_heartsuit, LMB_NONE},
	{"hom",  LM_TK_FUNC, 0, LMB_NONE},
	{"hookleftarrow",  LM_TK_SYM, LM_hookleftarrow, LMB_NONE},
	{"hookrightarrow",  LM_TK_SYM, LM_hookrightarrow, LMB_NONE},
	{"imath",  LM_TK_SYM, LM_imath, LMB_NONE},
	{"in",  LM_TK_SYM, LM_in, LMB_RELATION},
	{"inf",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"infty",  LM_TK_SYM, LM_infty, LMB_NONE},
	{"int",  LM_TK_BIGSYM, LM_int, LMB_NONE},
	{"iota",  LM_TK_SYM, LM_iota, LMB_NONE},
	{"jmath",  LM_TK_SYM, LM_jmath, LMB_NONE},
	{"kappa",  LM_TK_SYM, LM_kappa, LMB_NONE},
	{"ker",  LM_TK_FUNC, 0, LMB_NONE},
	{"label",  LM_TK_LABEL, 0, LMB_NONE},
	{"lambda",  LM_TK_SYM, LM_lambda, LMB_NONE},
	{"langle",  LM_TK_SYM, LM_langle, LMB_NONE},
	{"lceil",  LM_TK_SYM, LM_lceil, LMB_NONE},
	{"ldots",  LM_TK_DOTS, LM_ldots, LMB_NONE},
	{"left",  LM_TK_LEFT, 0, LMB_NONE},
	{"leftarrow",  LM_TK_SYM, LM_leftarrow, LMB_NONE},
	{"leftharpoondown",  LM_TK_SYM, LM_leftharpoondown, LMB_NONE},
	{"leftharpoonup",  LM_TK_SYM, LM_leftharpoonup, LMB_NONE},
	{"leftrightarrow",  LM_TK_SYM, LM_leftrightarrow, LMB_NONE},
	{"leq",  LM_TK_SYM, LM_leq, LMB_RELATION},
	{"lfloor",  LM_TK_SYM, LM_lfloor, LMB_NONE},
	{"lg",  LM_TK_FUNC, 0, LMB_NONE},
	{"lim",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"liminf",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"limits",  LM_TK_LIMIT, 1 , LMB_NONE},
	{"limsup",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"ll",  LM_TK_SYM, LM_ll, LMB_RELATION},
	{"ln",  LM_TK_FUNC, 0, LMB_NONE},
	{"log",  LM_TK_FUNC, 0, LMB_NONE},
	{"longleftarrow",  LM_TK_SYM, LM_longleftarrow, LMB_NONE},
	{"longleftrightarrow",  LM_TK_SYM, LM_longleftrightarrow, LMB_NONE},
	{"longmapsto",  LM_TK_SYM, LM_longmapsto, LMB_NONE},
	{"longrightarrow",  LM_TK_SYM, LM_longrightarrow, LMB_NONE},
	{"mapsto",  LM_TK_SYM, LM_mapsto, LMB_NONE},
	{"mathbf",  LM_TK_FONT, LM_TC_BF, LMB_NONE},
	{"mathcal",  LM_TK_FONT, LM_TC_CAL, LMB_NONE},
	{"mathit",  LM_TK_FONT, LM_TC_IT, LMB_NONE},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR, LMB_NONE},
	{"mathrm",  LM_TK_FONT, LM_TC_RM, LMB_NONE},
	{"mathsf",  LM_TK_FONT, LM_TC_SF, LMB_NONE},
	{"mathtt",  LM_TK_FONT, LM_TC_TT, LMB_NONE},
	{"max",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"mid",  LM_TK_SYM, LM_mid, LMB_RELATION},
	{"min",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"models",  LM_TK_SYM, LM_models, LMB_RELATION},
	{"mp",  LM_TK_SYM, LM_mp, LMB_OPERATOR},
	{"mu",  LM_TK_SYM, LM_mu, LMB_NONE},
	{"nabla",  LM_TK_SYM, LM_nabla, LMB_NONE},
	{"natural",  LM_TK_SYM, LM_natural, LMB_NONE},
	{"nearrow",  LM_TK_SYM, LM_nearrow, LMB_NONE},
	{"neg",  LM_TK_SYM, LM_neg, LMB_NONE},
	{"neq",  LM_TK_SYM, LM_neq, LMB_RELATION},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 , LMB_NONE},
	{"ni",  LM_TK_SYM, LM_ni, LMB_RELATION},
	{"nolimits",  LM_TK_LIMIT, -1, LMB_NONE},
	{"nonumber",  LM_TK_NONUM, 0, LMB_NONE},
	{"not",  LM_TK_DECORATION, LM_not, LMB_NONE},
	{"notin",  LM_TK_MACRO, LM_notin, LMB_RELATION},
	{"nu",  LM_TK_SYM, LM_nu, LMB_NONE},
	{"nwarrow",  LM_TK_SYM, LM_nwarrow, LMB_NONE},
	{"odot",  LM_TK_SYM, LM_odot, LMB_OPERATOR},
	{"oint",  LM_TK_BIGSYM, LM_oint, LMB_NONE},
	{"omega",  LM_TK_SYM, LM_omega, LMB_NONE},
	{"ominus",  LM_TK_SYM, LM_ominus, LMB_OPERATOR},
	{"oplus",  LM_TK_SYM, LM_oplus, LMB_OPERATOR},
	{"oslash",  LM_TK_SYM, LM_oslash, LMB_OPERATOR},
	{"otimes",  LM_TK_SYM, LM_otimes, LMB_OPERATOR},
	{"overbrace",  LM_TK_DECORATION, LM_overbrace, LMB_NONE},
	{"overleftarrow",  LM_TK_DECORATION, LM_overleftarrow, LMB_NONE},
	{"overline",  LM_TK_DECORATION, LM_overline, LMB_NONE},
	{"overrightarrow",  LM_TK_DECORATION, LM_overightarrow, LMB_NONE},
	{"parallel",  LM_TK_SYM, LM_parallel, LMB_RELATION},
	{"partial",  LM_TK_SYM, LM_partial, LMB_NONE},
	{"perp",  LM_TK_MACRO, LM_perp, LMB_RELATION},
	{"phi",  LM_TK_SYM, LM_phi, LMB_NONE},
	{"pi",  LM_TK_SYM, LM_pi, LMB_NONE},
	{"pm",  LM_TK_SYM, LM_pm, LMB_OPERATOR},
	{"pmod",  LM_TK_PMOD, 0, LMB_NONE},
	{"prec",  LM_TK_SYM, LM_prec, LMB_RELATION},
	{"preceq",  LM_TK_SYM, LM_preceq, LMB_RELATION},
	{"prime",  LM_TK_SYM, LM_prime, LMB_NONE},
	{"prod",  LM_TK_BIGSYM, LM_prod, LMB_NONE},
	{"propto",  LM_TK_SYM, LM_propto, LMB_RELATION},
	{"psi",  LM_TK_SYM, LM_psi, LMB_NONE},
	{"qquad",  LM_TK_SPACE, LM_qquad, LMB_NONE},
	{"quad",  LM_TK_SPACE, LM_quad, LMB_NONE},
	{"rangle",  LM_TK_SYM, LM_rangle, LMB_NONE},
	{"rceil",  LM_TK_SYM, LM_rceil, LMB_NONE},
	{"rfloor",  LM_TK_SYM, LM_rfloor, LMB_NONE},
	{"rho",  LM_TK_SYM, LM_rho, LMB_NONE},
	{"right",  LM_TK_RIGHT, 0, LMB_NONE},
	{"rightarrow",  LM_TK_SYM, LM_rightarrow, LMB_NONE},
	{"rightharpoondown",  LM_TK_SYM, LM_rightharpoondown, LMB_NONE},
	{"rightharpoonup",  LM_TK_SYM, LM_rightharpoonup, LMB_NONE},
	{"rightleftharpoons",  LM_TK_SYM, LM_rightleftharpoons, LMB_NONE},
	{"searrow",  LM_TK_SYM, LM_searrow, LMB_NONE},
	{"sec",  LM_TK_FUNC, 0, LMB_NONE},
	{"setminus",  LM_TK_SYM, LM_setminus, LMB_OPERATOR},
	{"sharp",  LM_TK_SYM, LM_sharp, LMB_NONE},
	{"sigma",  LM_TK_SYM, LM_sigma, LMB_NONE},
	{"sim",  LM_TK_SYM, LM_sim, LMB_RELATION},
	{"simeq",  LM_TK_SYM, LM_simeq, LMB_RELATION},
	{"sin",  LM_TK_FUNC, 0, LMB_NONE},
	{"sinh",  LM_TK_FUNC, 0, LMB_NONE},
	{"smile",  LM_TK_SYM, LM_smile, LMB_RELATION},
	{"spadesuit",  LM_TK_SYM, LM_spadesuit, LMB_NONE},
	{"sqcap",  LM_TK_SYM, LM_sqcap, LMB_OPERATOR},
	{"sqcup",  LM_TK_SYM, LM_sqcup, LMB_OPERATOR},
	{"sqrt",  LM_TK_SQRT, 0, LMB_NONE},
	{"sqsubseteq",  LM_TK_SYM, LM_sqsubseteq, LMB_RELATION},
	{"sqsupseteq",  LM_TK_SYM, LM_sqsupseteq, LMB_RELATION},
	{"stackrel",  LM_TK_STACK, 0, LMB_NONE},
	{"star",  LM_TK_SYM, LM_star, LMB_OPERATOR},
	{"subset",  LM_TK_SYM, LM_subset, LMB_RELATION},
	{"subseteq",  LM_TK_SYM, LM_subseteq, LMB_RELATION},
	{"succ",  LM_TK_SYM, LM_succ, LMB_RELATION},
	{"succeq",  LM_TK_SYM, LM_succeq, LMB_RELATION},
	{"sum",  LM_TK_BIGSYM, LM_sum, LMB_NONE},
	{"sup",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"supset",  LM_TK_SYM, LM_supset, LMB_RELATION},
	{"supseteq",  LM_TK_SYM, LM_supseteq, LMB_RELATION},
	{"surd",  LM_TK_SYM, LM_surd, LMB_NONE},
	{"swarrow",  LM_TK_SYM, LM_swarrow, LMB_NONE},
	{"tan",  LM_TK_FUNC, 0, LMB_NONE},
	{"tanh",  LM_TK_FUNC, 0, LMB_NONE},
	{"tau",  LM_TK_SYM, LM_tau, LMB_NONE},
	{"textrm",  LM_TK_FONT, LM_TC_TEXTRM, LMB_NONE},
	{"theta",  LM_TK_SYM, LM_theta, LMB_NONE},
	{"tilde",  LM_TK_DECORATION, LM_tilde, LMB_NONE},
	{"times",  LM_TK_SYM, LM_times, LMB_OPERATOR},
	{"top",  LM_TK_SYM, LM_top, LMB_NONE},
	{"triangle",  LM_TK_SYM, LM_triangle, LMB_NONE},
	{"triangleleft",  LM_TK_SYM, LM_triangleleft, LMB_OPERATOR},
	{"triangleright",  LM_TK_SYM, LM_triangleright, LMB_OPERATOR},
	{"underbrace",  LM_TK_DECORATION, LM_underbrace, LMB_NONE},
	{"underline",  LM_TK_DECORATION, LM_underline, LMB_NONE},
	{"uparrow",  LM_TK_SYM, LM_uparrow, LMB_NONE},
	{"updownarrow",  LM_TK_SYM, LM_updownarrow, LMB_NONE},
	{"uplus",  LM_TK_SYM, LM_uplus, LMB_OPERATOR},
	{"upsilon",  LM_TK_SYM, LM_upsilon, LMB_NONE},
	{"varepsilon",  LM_TK_SYM, LM_varepsilon, LMB_NONE},
	{"varphi",  LM_TK_SYM, LM_varphi, LMB_NONE},
	{"varpi",  LM_TK_SYM, LM_varpi, LMB_NONE},
	{"varsigma",  LM_TK_SYM, LM_varsigma, LMB_NONE},
	{"vartheta",  LM_TK_SYM, LM_vartheta, LMB_NONE},
	{"vdash",  LM_TK_SYM, LM_vdash, LMB_RELATION},
	{"vdots",  LM_TK_DOTS, LM_vdots, LMB_NONE},
	{"vec",  LM_TK_DECORATION, LM_vec, LMB_NONE},
	{"vee",  LM_TK_SYM, LM_vee, LMB_OPERATOR},
	{"wedge",  LM_TK_SYM, LM_wedge, LMB_OPERATOR},
	{"widehat",  LM_TK_DECORATION, LM_widehat, LMB_NONE},
	{"widetilde",  LM_TK_DECORATION, LM_widetilde, LMB_NONE},
	{"wp",  LM_TK_SYM, LM_wp, LMB_NONE},
	{"wr",  LM_TK_SYM, LM_wr, LMB_OPERATOR},
	{"xi",  LM_TK_SYM, LM_xi, LMB_NONE},
	{"zeta",  LM_TK_SYM, LM_zeta, LMB_NONE},
};


struct symbolindex {
	unsigned int id;
	short token;

	symbolindex(unsigned int i, short t) : id(i), token(t)
	{}

	bool operator<(symbolindex const & s) const
	{
		return (id < s.id) || (id == s.id && token < s.token);
	}
};


// global maps 
std::map<symbolindex, int>  LatexkeyById;
std::map<string, int>       LatexkeyByName;


// helper structure to initialize the maps on startup:
struct init {
	init() {
		int const n = sizeof(wordlist)/sizeof(wordlist[0]);
		for (latexkeys const * it = wordlist; it != wordlist + n; ++it) {
			if (LatexkeyByName.find(it->name) != LatexkeyByName.end()) {
				std::cerr << "math_hash.C: Bug: Duplicate entry: " 
					  << it->name << std::endl;
			}
			LatexkeyByName[it->name] = it - wordlist;
			if (it->id != 0 && 
			    LatexkeyById.find(symbolindex(it->id, it->token)) !=
			    LatexkeyById.end()) {
				std::cerr << "math_hash.C: Bug: Duplicate entry: "
					  << it->name << " Id: "
					  << it->id << " token: " << it->token
					  << std::endl;
			}
			LatexkeyById[symbolindex(it->id, it->token)] = it - wordlist;
		}
	}
};

// the "Initializer": Its default constructor is executed on loading and
// fills the maps
static init dummy;

} // namespace anon


latexkeys const * in_word_set(string const & str)
{
	std::map<string, int>::const_iterator pos = LatexkeyByName.find(str);
	return pos == LatexkeyByName.end() ? 0 : &wordlist[pos->second];
}


latexkeys const * lm_get_key_by_id(unsigned int id, short tc)
{
	std::map<symbolindex, int>::const_iterator pos
		= LatexkeyById.find(symbolindex(id, tc));
	return pos == LatexkeyById.end() ? 0 : &wordlist[pos->second];
}
