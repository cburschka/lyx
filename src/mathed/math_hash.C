#include <config.h>
#include <map>

#include "math_defs.h"
#include "math_parser.h"
#include "support/lstrings.h"
#include <iostream>

namespace {

latexkeys const wordlist[] = 
{
	{"setminus",  LM_TK_SYM, LM_setminus},
	{"ldots",  LM_TK_DOTS, LM_ldots},
	{"limits",  LM_TK_LIMIT, 1 },
	{"stackrel",  LM_TK_STACK, 0},
	{"ll",  LM_TK_SYM, LM_ll},
	{"label",  LM_TK_LABEL, 0},
	{"lceil",  LM_TK_SYM, LM_lceil},
	{"sqrt",  LM_TK_SQRT, 0},
	{"times",  LM_TK_SYM, LM_times},
	{"subset",  LM_TK_SYM, LM_subset},
	{"supset",  LM_TK_SYM, LM_supset},
	{"spadesuit",  LM_TK_SYM, LM_spadesuit},
	{"Re",  LM_TK_SYM, LM_Re},
	{"left",  LM_TK_LEFT, 0},
	{"smile",  LM_TK_SYM, LM_smile},
	{"exists",  LM_TK_SYM, LM_exists},
	{"Vert",  LM_TK_SYM, LM_Vert},
	//{"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT},
	{"uplus",  LM_TK_SYM, LM_uplus},
	{"ell",  LM_TK_SYM, LM_ell},
	{"langle",  LM_TK_SYM, LM_langle},
	//{"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT},
	{"zeta",  LM_TK_SYM, LM_zeta},
	{"sigma",  LM_TK_SYM, LM_sigma},
	{"arccos",  LM_TK_FUNC, 0},
	{"sup",  LM_TK_FUNCLIM, 0},
	{"sharp",  LM_TK_SYM, LM_sharp},
	{"sqcup",  LM_TK_SYM, LM_sqcup},
	{"sqcap",  LM_TK_SYM, LM_sqcap},
	{"approx",  LM_TK_SYM, LM_approx},
	{"triangleleft",  LM_TK_SYM, LM_triangleleft},
	{"triangleright",  LM_TK_SYM, LM_triangleright},
	{"tilde",  LM_TK_DECORATION, LM_tilde},
	{"lambda",  LM_TK_SYM, LM_lambda},
	{"emptyset",  LM_TK_MACRO, LM_emptyset},
	{"triangle",  LM_TK_SYM, LM_triangle},
	//{"textstyle",  LM_TK_STY, LM_ST_TEXT},
	{"tau",  LM_TK_SYM, LM_tau},
	{"limsup",  LM_TK_FUNCLIM, 0},
	{"partial",  LM_TK_SYM, LM_partial},
	{"parallel",  LM_TK_SYM, LM_parallel},
	{"infty",  LM_TK_SYM, LM_infty},
	{"kappa",  LM_TK_SYM, LM_kappa},
	{"xi",  LM_TK_SYM, LM_xi},
	{"leftharpoonup",  LM_TK_SYM, LM_leftharpoonup},
	{"theta",  LM_TK_SYM, LM_theta},
	{"Xi",  LM_TK_SYM, LM_Xi},
	{"searrow",  LM_TK_SYM, LM_searrow},
	{"swarrow",  LM_TK_SYM, LM_swarrow},
	{"top",  LM_TK_SYM, LM_top},
	{"Rightarrow",  LM_TK_SYM, LM_Rightarrow},
	{"underline",  LM_TK_DECORATION, LM_underline},
	{"underbrace",  LM_TK_DECORATION, LM_underbrace},
	{"eta",  LM_TK_SYM, LM_eta},
	{"acute",  LM_TK_DECORATION, LM_acute},
	{"angle",  LM_TK_SYM, LM_angle},
	{"exp",  LM_TK_FUNC, 0},
	{"leftarrow",  LM_TK_SYM, LM_leftarrow},
	{"prime",  LM_TK_SYM, LM_prime},
	{"int",  LM_TK_BIGSYM, LM_int},
	{"longleftarrow",  LM_TK_SYM, LM_longleftarrow},
	{"leftrightarrow",  LM_TK_SYM, LM_leftrightarrow},
	{"longrightarrow",  LM_TK_SYM, LM_longrightarrow},
	{"Gamma",  LM_TK_SYM, LM_Gamma},
	{"Pi",  LM_TK_SYM, LM_Pi},
	{"Phi",  LM_TK_SYM, LM_Phi},
	{"Psi",  LM_TK_SYM, LM_Psi},
	{"longleftrightarrow",  LM_TK_SYM, LM_longleftrightarrow},
	{"alpha",  LM_TK_SYM, LM_alpha},
	{"widehat",  LM_TK_DECORATION, LM_widehat},
	{"sin",  LM_TK_FUNC, 0},
	{"asymp",  LM_TK_SYM, LM_asymp},
	{"nolimits",  LM_TK_LIMIT, -1},
	{"perp",  LM_TK_MACRO, LM_perp},
	{"wedge",  LM_TK_SYM, LM_wedge},
	{"ln",  LM_TK_FUNC, 0},
	{"widetilde",  LM_TK_DECORATION, LM_widetilde},
	{"Omega",  LM_TK_SYM, LM_Omega},
	{"natural",  LM_TK_SYM, LM_natural},
	{"iota",  LM_TK_SYM, LM_iota},
	{"uparrow",  LM_TK_SYM, LM_uparrow},
	{"Sigma",  LM_TK_SYM, LM_Sigma},
	{"pi",  LM_TK_SYM, LM_pi},
	{"phi",  LM_TK_SYM, LM_phi},
	{"psi",  LM_TK_SYM, LM_psi},
	{"updownarrow",  LM_TK_SYM, LM_updownarrow},
	{"star",  LM_TK_SYM, LM_star},
	{"leftharpoondown",  LM_TK_SYM, LM_leftharpoondown},
	{"wp",  LM_TK_SYM, LM_wp},
	{"not",  LM_TK_DECORATION, LM_not},
	{"tan",  LM_TK_FUNC, 0},
	{"Theta",  LM_TK_SYM, LM_Theta},
	{"rceil",  LM_TK_SYM, LM_rceil},
	{"lfloor",  LM_TK_SYM, LM_lfloor},
	{"rightleftharpoons",  LM_TK_SYM, LM_rightleftharpoons},
	{"cos",  LM_TK_FUNC, 0},
	{"sec",  LM_TK_FUNC, 0},
	{"succ",  LM_TK_SYM, LM_succ},
	{"cdots",  LM_TK_DOTS, LM_cdots},
	{"epsilon",  LM_TK_SYM, LM_epsilon},
	{"ker",  LM_TK_FUNC, 0},
	{"nu",  LM_TK_SYM, LM_nu},
	{"Delta",  LM_TK_SYM, LM_Delta},
	{"forall",  LM_TK_SYM, LM_forall},
	{"liminf",  LM_TK_FUNCLIM, 0},
	{"Uparrow",  LM_TK_SYM, LM_Uparrow},
	{"upsilon",  LM_TK_SYM, LM_upsilon},
	{"right",  LM_TK_RIGHT, 0},
	{"Updownarrow",  LM_TK_SYM, LM_Updownarrow},
	{"Pr",  LM_TK_FUNCLIM, 0},
	{"nabla",  LM_TK_SYM, LM_nabla},
	{"arcsin",  LM_TK_FUNC, 0},
	{"arctan",  LM_TK_FUNC, 0},
	{"flat",  LM_TK_SYM, LM_flat},
	{"check",  LM_TK_DECORATION, LM_check},
	{"rangle",  LM_TK_SYM, LM_rangle},
	{"cot",  LM_TK_FUNC, 0},
	{"cdot",  LM_TK_SYM, LM_cdot},
	{"clubsuit",  LM_TK_SYM, LM_clubsuit},
	{"in",  LM_TK_SYM, LM_in},
	{"ni",  LM_TK_SYM, LM_ni},
	{"Downarrow",  LM_TK_SYM, LM_Downarrow},
	{"Upsilon",  LM_TK_SYM, LM_Upsilon},
	{"sim",  LM_TK_SYM, LM_sim},
	{"sum",  LM_TK_BIGSYM, LM_sum},
	{"models",  LM_TK_SYM, LM_models},
	{"nearrow",  LM_TK_SYM, LM_nearrow},
	{"nwarrow",  LM_TK_SYM, LM_nwarrow},
	{"max",  LM_TK_FUNCLIM, 0},
	{"Im",  LM_TK_SYM, LM_Im},
	{"lim",  LM_TK_FUNCLIM, 0},
	{"rightharpoonup",  LM_TK_SYM, LM_rightharpoonup},
	{"mathcal",  LM_TK_FONT, LM_TC_CAL},
	{"cap",  LM_TK_SYM, LM_cap},
	{"cup",  LM_TK_SYM, LM_cup},
	{"prec",  LM_TK_SYM, LM_prec},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR},
	{"wr",  LM_TK_SYM, LM_wr},
	{"inf",  LM_TK_FUNCLIM, 0},
	{"bigoplus",  LM_TK_BIGSYM, LM_oplus},
	{"biguplus",  LM_TK_BIGSYM, LM_biguplus},
	{"bigotimes",  LM_TK_BIGSYM, LM_otimes},
	{"rightarrow",  LM_TK_SYM, LM_rightarrow},
	{"mathit",  LM_TK_FONT, LM_TC_IT},
	{"textrm",  LM_TK_FONT, LM_TC_TEXTRM},
	{"mathtt",  LM_TK_FONT, LM_TC_TT},
	{"chi",  LM_TK_SYM, LM_chi},
	{"simeq",  LM_TK_SYM, LM_simeq},
	{"succeq",  LM_TK_SYM, LM_succeq},
	{"notin",  LM_TK_MACRO, LM_notin},
	{"subseteq",  LM_TK_SYM, LM_subseteq},
	{"supseteq",  LM_TK_SYM, LM_supseteq},
	{"sqsubseteq",  LM_TK_SYM, LM_sqsubseteq},
	{"sqsupseteq",  LM_TK_SYM, LM_sqsupseteq},
	{"leq",  LM_TK_SYM, LM_leq},
	{"surd",  LM_TK_SYM, LM_surd},
	{"ddots",  LM_TK_DOTS, LM_ddots},
	{"mu",  LM_TK_SYM, LM_mu},
	{"bot",  LM_TK_SYM, LM_bot},
	{"bullet",  LM_TK_SYM, LM_bullet},
	{"bigodot",  LM_TK_BIGSYM, LM_bigodot},
	{"sinh",  LM_TK_FUNC, 0},
	{"jmath",  LM_TK_SYM, LM_jmath},
	{"mp",  LM_TK_SYM, LM_mp},
	{"pm",  LM_TK_SYM, LM_pm},
	{"nonumber",  LM_TK_NONUM, 0},
	{"breve",  LM_TK_DECORATION, LM_breve},
	{"bigvee",  LM_TK_BIGSYM, LM_vee},
	{"bowtie",  LM_TK_SYM, LM_bowtie},
	{"bigwedge",  LM_TK_BIGSYM, LM_wedge},
	{"frown",  LM_TK_SYM, LM_frown},
	{"rightharpoondown",  LM_TK_SYM, LM_rightharpoondown},
	{"det",  LM_TK_FUNCLIM, 0},
	{"dot",  LM_TK_DECORATION, LM_dot},
	{"ddot",  LM_TK_DECORATION, LM_ddot},
	{"lg",  LM_TK_FUNC, 0},
	{"log",  LM_TK_FUNC, 0},
	{"oplus",  LM_TK_SYM, LM_oplus},
	{"ominus",  LM_TK_SYM, LM_ominus},
	{"otimes",  LM_TK_SYM, LM_otimes},
	{"beta",  LM_TK_SYM, LM_beta},
	{"diamondsuit",  LM_TK_SYM, LM_diamondsuit},
	{"rfloor",  LM_TK_SYM, LM_rfloor},
	{"end",  LM_TK_END, 0},
	{"hat",  LM_TK_DECORATION, LM_hat},
	{"tanh",  LM_TK_FUNC, 0},
	{"vdots",  LM_TK_DOTS, LM_vdots},
	{"bigcap",  LM_TK_BIGSYM, LM_cap},
	{"bigcup",  LM_TK_BIGSYM, LM_cup},
	{"bigsqcup",  LM_TK_BIGSYM, LM_bigsqcup},
	{"heartsuit",  LM_TK_SYM, LM_heartsuit},
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY},
	{"longmapsto",  LM_TK_SYM, LM_longmapsto},
	{"bigtriangleup",  LM_TK_SYM, LM_bigtriangleup},
	{"preceq",  LM_TK_SYM, LM_preceq},
	{"delta",  LM_TK_SYM, LM_delta},
	{"odot",  LM_TK_SYM, LM_odot},
	{"oint",  LM_TK_BIGSYM, LM_oint},
	{"grave",  LM_TK_DECORATION, LM_grave},
	{"pmod",  LM_TK_PMOD, 0},
	{"prod",  LM_TK_BIGSYM, LM_prod},
	{"frac",  LM_TK_FRAC, 0},
	{"csc",  LM_TK_FUNC, 0},
	{"circ",  LM_TK_SYM, LM_circ},
	{"aleph",  LM_TK_SYM, LM_aleph},
	{"min",  LM_TK_FUNCLIM, 0},
	{"overline",  LM_TK_DECORATION, LM_overline},
	{"arg",  LM_TK_FUNC, 0},
	{"overbrace",  LM_TK_DECORATION, LM_overbrace},
	{"amalg",  LM_TK_SYM, LM_amalg},
	{"gamma",  LM_TK_SYM, LM_gamma},
	{"vee",  LM_TK_SYM, LM_vee},
	{"equiv",  LM_TK_SYM, LM_equiv},
	{"omega",  LM_TK_SYM, LM_omega},
	{"downarrow",  LM_TK_SYM, LM_downarrow},
	{"imath",  LM_TK_SYM, LM_imath},
	{"propto",  LM_TK_SYM, LM_propto},
	{"begin",  LM_TK_BEGIN, 0},
	{"Lambda",  LM_TK_SYM, LM_Lambda},
	{"varsigma",  LM_TK_SYM, LM_varsigma},
	{"vartheta",  LM_TK_SYM, LM_vartheta},
	{"neq",  LM_TK_SYM, LM_neq},
	{"hookleftarrow",  LM_TK_SYM, LM_hookleftarrow},
	{"hookrightarrow",  LM_TK_SYM, LM_hookrightarrow},
	{"bigtriangledown",  LM_TK_SYM, LM_bigtriangledown},
	{"mathbf",  LM_TK_FONT, LM_TC_BF},
	{"mathsf",  LM_TK_FONT, LM_TC_SF},
	{"bar",  LM_TK_DECORATION, LM_bar},
	{"varpi",  LM_TK_SYM, LM_varpi},
	{"varphi",  LM_TK_SYM, LM_varphi},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 },
	{"overleftarrow",  LM_TK_DECORATION, LM_overleftarrow},
	{"overrightarrow",  LM_TK_DECORATION, LM_overightarrow},
	{"Leftarrow",  LM_TK_SYM, LM_Leftarrow},
	{"Longleftarrow",  LM_TK_SYM, LM_Longleftarrow},
	{"Leftrightarrow",  LM_TK_SYM, LM_Leftrightarrow},
	{"Longrightarrow",  LM_TK_SYM, LM_Longrightarrow},
	{"neg",  LM_TK_SYM, LM_neg},
	{"bigcirc",  LM_TK_SYM, LM_bigcirc},
	{"Longleftrightarrow",  LM_TK_SYM, LM_Longleftrightarrow},
	{"dagger",  LM_TK_SYM, LM_dagger},
	{"ddagger",  LM_TK_SYM, LM_ddagger},
	{"hbar",  LM_TK_SYM, LM_hbar},
	{"coprod",  LM_TK_BIGSYM, LM_coprod},
	{"mathrm",  LM_TK_FONT, LM_TC_RM},
	{"varepsilon",  LM_TK_SYM, LM_varepsilon},
	{"cosh",  LM_TK_FUNC, 0},
	{"coth",  LM_TK_FUNC, 0},
	{"rho",  LM_TK_SYM, LM_rho},
	{"cong",  LM_TK_SYM, LM_cong},
	{"vec",  LM_TK_DECORATION, LM_vec},
	{"dim",  LM_TK_FUNC, 0},
	{"mid",  LM_TK_SYM, LM_mid},
	{"hom",  LM_TK_FUNC, 0},
	{"bmod",  LM_TK_FUNC, 0},
	{"quad",  LM_TK_SPACE, LM_quad},
	{"doteq",  LM_TK_SYM, LM_doteq},
	{"qquad",  LM_TK_SPACE, LM_qquad},
	{"mapsto",  LM_TK_SYM, LM_mapsto},
	{"backslash",  LM_TK_SYM, LM_backslash},
	{"diamond",  LM_TK_SYM, LM_diamond},
	{"geq",  LM_TK_SYM, LM_geq},
	{"deg",  LM_TK_FUNC, 0},
	{"gcd",  LM_TK_FUNCLIM, 0},
	{"gg",  LM_TK_SYM, LM_gg},
	{"div",  LM_TK_SYM, LM_div},
	{"dashv",  LM_TK_SYM, LM_dashv},
	{"oslash",  LM_TK_SYM, LM_oslash},
	{"vdash",  LM_TK_SYM, LM_vdash},
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
