#include <config.h>
#include <map>

#include "math_defs.h"
#include "math_parser.h"
#include "support/lstrings.h"
#include <iostream>

namespace {

latexkeys const wordlist[] = 
{
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY},
	//{"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT},
	//{"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT},
	//{"textstyle",  LM_TK_STY, LM_ST_TEXT},
	{"Delta",  LM_TK_SYM, LM_Delta},
	{"Downarrow",  LM_TK_SYM, LM_Downarrow},
	{"Gamma",  LM_TK_SYM, LM_Gamma},
	{"Im",  LM_TK_SYM, LM_Im},
	{"Lambda",  LM_TK_SYM, LM_Lambda},
	{"Leftarrow",  LM_TK_SYM, LM_Leftarrow},
	{"Leftrightarrow",  LM_TK_SYM, LM_Leftrightarrow},
	{"Longleftarrow",  LM_TK_SYM, LM_Longleftarrow},
	{"Longleftrightarrow",  LM_TK_SYM, LM_Longleftrightarrow},
	{"Longrightarrow",  LM_TK_SYM, LM_Longrightarrow},
	{"Omega",  LM_TK_SYM, LM_Omega},
	{"Phi",  LM_TK_SYM, LM_Phi},
	{"Pi",  LM_TK_SYM, LM_Pi},
	{"Pr",  LM_TK_FUNCLIM, 0},
	{"Psi",  LM_TK_SYM, LM_Psi},
	{"Re",  LM_TK_SYM, LM_Re},
	{"Rightarrow",  LM_TK_SYM, LM_Rightarrow},
	{"Sigma",  LM_TK_SYM, LM_Sigma},
	{"Theta",  LM_TK_SYM, LM_Theta},
	{"Uparrow",  LM_TK_SYM, LM_Uparrow},
	{"Updownarrow",  LM_TK_SYM, LM_Updownarrow},
	{"Upsilon",  LM_TK_SYM, LM_Upsilon},
	{"Vert",  LM_TK_SYM, LM_Vert},
	{"Xi",  LM_TK_SYM, LM_Xi},
	{"acute",  LM_TK_DECORATION, LM_acute},
	{"aleph",  LM_TK_SYM, LM_aleph},
	{"alpha",  LM_TK_SYM, LM_alpha},
	{"amalg",  LM_TK_SYM, LM_amalg},
	{"angle",  LM_TK_SYM, LM_angle},
	{"approx",  LM_TK_SYM, LM_approx},
	{"arccos",  LM_TK_FUNC, 0},
	{"arcsin",  LM_TK_FUNC, 0},
	{"arctan",  LM_TK_FUNC, 0},
	{"arg",  LM_TK_FUNC, 0},
	{"asymp",  LM_TK_SYM, LM_asymp},
	{"backslash",  LM_TK_SYM, LM_backslash},
	{"bar",  LM_TK_DECORATION, LM_bar},
	{"begin",  LM_TK_BEGIN, 0},
	{"beta",  LM_TK_SYM, LM_beta},
	{"bigcap",  LM_TK_BIGSYM, LM_cap},
	{"bigcirc",  LM_TK_SYM, LM_bigcirc},
	{"bigcup",  LM_TK_BIGSYM, LM_cup},
	{"bigodot",  LM_TK_BIGSYM, LM_bigodot},
	{"bigoplus",  LM_TK_BIGSYM, LM_oplus},
	{"bigotimes",  LM_TK_BIGSYM, LM_otimes},
	{"bigsqcup",  LM_TK_BIGSYM, LM_bigsqcup},
	{"bigtriangledown",  LM_TK_SYM, LM_bigtriangledown},
	{"bigtriangleup",  LM_TK_SYM, LM_bigtriangleup},
	{"biguplus",  LM_TK_BIGSYM, LM_biguplus},
	{"bigvee",  LM_TK_BIGSYM, LM_vee},
	{"bigwedge",  LM_TK_BIGSYM, LM_wedge},
	{"bmod",  LM_TK_FUNC, 0},
	{"bot",  LM_TK_SYM, LM_bot},
	{"bowtie",  LM_TK_SYM, LM_bowtie},
	{"breve",  LM_TK_DECORATION, LM_breve},
	{"bullet",  LM_TK_SYM, LM_bullet},
	{"cap",  LM_TK_SYM, LM_cap},
	{"cdot",  LM_TK_SYM, LM_cdot},
	{"cdots",  LM_TK_DOTS, LM_cdots},
	{"check",  LM_TK_DECORATION, LM_check},
	{"chi",  LM_TK_SYM, LM_chi},
	{"circ",  LM_TK_SYM, LM_circ},
	{"clubsuit",  LM_TK_SYM, LM_clubsuit},
	{"cong",  LM_TK_SYM, LM_cong},
	{"coprod",  LM_TK_BIGSYM, LM_coprod},
	{"cos",  LM_TK_FUNC, 0},
	{"cosh",  LM_TK_FUNC, 0},
	{"cot",  LM_TK_FUNC, 0},
	{"coth",  LM_TK_FUNC, 0},
	{"csc",  LM_TK_FUNC, 0},
	{"cup",  LM_TK_SYM, LM_cup},
	{"dagger",  LM_TK_SYM, LM_dagger},
	{"dashv",  LM_TK_SYM, LM_dashv},
	{"ddagger",  LM_TK_SYM, LM_ddagger},
	{"ddot",  LM_TK_DECORATION, LM_ddot},
	{"ddots",  LM_TK_DOTS, LM_ddots},
	{"deg",  LM_TK_FUNC, 0},
	{"delta",  LM_TK_SYM, LM_delta},
	{"det",  LM_TK_FUNCLIM, 0},
	{"diamond",  LM_TK_SYM, LM_diamond},
	{"diamondsuit",  LM_TK_SYM, LM_diamondsuit},
	{"dim",  LM_TK_FUNC, 0},
	{"div",  LM_TK_SYM, LM_div},
	{"dot",  LM_TK_DECORATION, LM_dot},
	{"doteq",  LM_TK_SYM, LM_doteq},
	{"downarrow",  LM_TK_SYM, LM_downarrow},
	{"ell",  LM_TK_SYM, LM_ell},
	{"emptyset",  LM_TK_MACRO, LM_emptyset},
	{"end",  LM_TK_END, 0},
	{"epsilon",  LM_TK_SYM, LM_epsilon},
	{"equiv",  LM_TK_SYM, LM_equiv},
	{"eta",  LM_TK_SYM, LM_eta},
	{"exists",  LM_TK_SYM, LM_exists},
	{"exp",  LM_TK_FUNC, 0},
	{"flat",  LM_TK_SYM, LM_flat},
	{"forall",  LM_TK_SYM, LM_forall},
	{"frac",  LM_TK_FRAC, 0},
	{"frown",  LM_TK_SYM, LM_frown},
	{"gamma",  LM_TK_SYM, LM_gamma},
	{"gcd",  LM_TK_FUNCLIM, 0},
	{"geq",  LM_TK_SYM, LM_geq},
	{"gg",  LM_TK_SYM, LM_gg},
	{"grave",  LM_TK_DECORATION, LM_grave},
	{"hat",  LM_TK_DECORATION, LM_hat},
	{"hbar",  LM_TK_SYM, LM_hbar},
	{"heartsuit",  LM_TK_SYM, LM_heartsuit},
	{"hom",  LM_TK_FUNC, 0},
	{"hookleftarrow",  LM_TK_SYM, LM_hookleftarrow},
	{"hookrightarrow",  LM_TK_SYM, LM_hookrightarrow},
	{"imath",  LM_TK_SYM, LM_imath},
	{"in",  LM_TK_SYM, LM_in},
	{"inf",  LM_TK_FUNCLIM, 0},
	{"infty",  LM_TK_SYM, LM_infty},
	{"int",  LM_TK_BIGSYM, LM_int},
	{"iota",  LM_TK_SYM, LM_iota},
	{"jmath",  LM_TK_SYM, LM_jmath},
	{"kappa",  LM_TK_SYM, LM_kappa},
	{"ker",  LM_TK_FUNC, 0},
	{"label",  LM_TK_LABEL, 0},
	{"lambda",  LM_TK_SYM, LM_lambda},
	{"langle",  LM_TK_SYM, LM_langle},
	{"lceil",  LM_TK_SYM, LM_lceil},
	{"ldots",  LM_TK_DOTS, LM_ldots},
	{"left",  LM_TK_LEFT, 0},
	{"leftarrow",  LM_TK_SYM, LM_leftarrow},
	{"leftharpoondown",  LM_TK_SYM, LM_leftharpoondown},
	{"leftharpoonup",  LM_TK_SYM, LM_leftharpoonup},
	{"leftrightarrow",  LM_TK_SYM, LM_leftrightarrow},
	{"leq",  LM_TK_SYM, LM_leq},
	{"lfloor",  LM_TK_SYM, LM_lfloor},
	{"lg",  LM_TK_FUNC, 0},
	{"lim",  LM_TK_FUNCLIM, 0},
	{"liminf",  LM_TK_FUNCLIM, 0},
	{"limits",  LM_TK_LIMIT, 1 },
	{"limsup",  LM_TK_FUNCLIM, 0},
	{"ll",  LM_TK_SYM, LM_ll},
	{"ln",  LM_TK_FUNC, 0},
	{"log",  LM_TK_FUNC, 0},
	{"longleftarrow",  LM_TK_SYM, LM_longleftarrow},
	{"longleftrightarrow",  LM_TK_SYM, LM_longleftrightarrow},
	{"longmapsto",  LM_TK_SYM, LM_longmapsto},
	{"longrightarrow",  LM_TK_SYM, LM_longrightarrow},
	{"mapsto",  LM_TK_SYM, LM_mapsto},
	{"mathbf",  LM_TK_FONT, LM_TC_BF},
	{"mathcal",  LM_TK_FONT, LM_TC_CAL},
	{"mathit",  LM_TK_FONT, LM_TC_IT},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR},
	{"mathrm",  LM_TK_FONT, LM_TC_RM},
	{"mathsf",  LM_TK_FONT, LM_TC_SF},
	{"mathtt",  LM_TK_FONT, LM_TC_TT},
	{"max",  LM_TK_FUNCLIM, 0},
	{"mid",  LM_TK_SYM, LM_mid},
	{"min",  LM_TK_FUNCLIM, 0},
	{"models",  LM_TK_SYM, LM_models},
	{"mp",  LM_TK_SYM, LM_mp},
	{"mu",  LM_TK_SYM, LM_mu},
	{"nabla",  LM_TK_SYM, LM_nabla},
	{"natural",  LM_TK_SYM, LM_natural},
	{"nearrow",  LM_TK_SYM, LM_nearrow},
	{"neg",  LM_TK_SYM, LM_neg},
	{"neq",  LM_TK_SYM, LM_neq},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 },
	{"ni",  LM_TK_SYM, LM_ni},
	{"nolimits",  LM_TK_LIMIT, -1},
	{"nonumber",  LM_TK_NONUM, 0},
	{"not",  LM_TK_DECORATION, LM_not},
	{"notin",  LM_TK_MACRO, LM_notin},
	{"nu",  LM_TK_SYM, LM_nu},
	{"nwarrow",  LM_TK_SYM, LM_nwarrow},
	{"odot",  LM_TK_SYM, LM_odot},
	{"oint",  LM_TK_BIGSYM, LM_oint},
	{"omega",  LM_TK_SYM, LM_omega},
	{"ominus",  LM_TK_SYM, LM_ominus},
	{"oplus",  LM_TK_SYM, LM_oplus},
	{"oslash",  LM_TK_SYM, LM_oslash},
	{"otimes",  LM_TK_SYM, LM_otimes},
	{"overbrace",  LM_TK_DECORATION, LM_overbrace},
	{"overleftarrow",  LM_TK_DECORATION, LM_overleftarrow},
	{"overline",  LM_TK_DECORATION, LM_overline},
	{"overrightarrow",  LM_TK_DECORATION, LM_overightarrow},
	{"parallel",  LM_TK_SYM, LM_parallel},
	{"partial",  LM_TK_SYM, LM_partial},
	{"perp",  LM_TK_MACRO, LM_perp},
	{"phi",  LM_TK_SYM, LM_phi},
	{"pi",  LM_TK_SYM, LM_pi},
	{"pm",  LM_TK_SYM, LM_pm},
	{"pmod",  LM_TK_PMOD, 0},
	{"prec",  LM_TK_SYM, LM_prec},
	{"preceq",  LM_TK_SYM, LM_preceq},
	{"prime",  LM_TK_SYM, LM_prime},
	{"prod",  LM_TK_BIGSYM, LM_prod},
	{"propto",  LM_TK_SYM, LM_propto},
	{"psi",  LM_TK_SYM, LM_psi},
	{"qquad",  LM_TK_SPACE, LM_qquad},
	{"quad",  LM_TK_SPACE, LM_quad},
	{"rangle",  LM_TK_SYM, LM_rangle},
	{"rceil",  LM_TK_SYM, LM_rceil},
	{"rfloor",  LM_TK_SYM, LM_rfloor},
	{"rho",  LM_TK_SYM, LM_rho},
	{"right",  LM_TK_RIGHT, 0},
	{"rightarrow",  LM_TK_SYM, LM_rightarrow},
	{"rightharpoondown",  LM_TK_SYM, LM_rightharpoondown},
	{"rightharpoonup",  LM_TK_SYM, LM_rightharpoonup},
	{"rightleftharpoons",  LM_TK_SYM, LM_rightleftharpoons},
	{"searrow",  LM_TK_SYM, LM_searrow},
	{"sec",  LM_TK_FUNC, 0},
	{"setminus",  LM_TK_SYM, LM_setminus},
	{"sharp",  LM_TK_SYM, LM_sharp},
	{"sigma",  LM_TK_SYM, LM_sigma},
	{"sim",  LM_TK_SYM, LM_sim},
	{"simeq",  LM_TK_SYM, LM_simeq},
	{"sin",  LM_TK_FUNC, 0},
	{"sinh",  LM_TK_FUNC, 0},
	{"smile",  LM_TK_SYM, LM_smile},
	{"spadesuit",  LM_TK_SYM, LM_spadesuit},
	{"sqcap",  LM_TK_SYM, LM_sqcap},
	{"sqcup",  LM_TK_SYM, LM_sqcup},
	{"sqrt",  LM_TK_SQRT, 0},
	{"sqsubseteq",  LM_TK_SYM, LM_sqsubseteq},
	{"sqsupseteq",  LM_TK_SYM, LM_sqsupseteq},
	{"stackrel",  LM_TK_STACK, 0},
	{"star",  LM_TK_SYM, LM_star},
	{"subset",  LM_TK_SYM, LM_subset},
	{"subseteq",  LM_TK_SYM, LM_subseteq},
	{"succ",  LM_TK_SYM, LM_succ},
	{"succeq",  LM_TK_SYM, LM_succeq},
	{"sum",  LM_TK_BIGSYM, LM_sum},
	{"sup",  LM_TK_FUNCLIM, 0},
	{"supset",  LM_TK_SYM, LM_supset},
	{"supseteq",  LM_TK_SYM, LM_supseteq},
	{"surd",  LM_TK_SYM, LM_surd},
	{"swarrow",  LM_TK_SYM, LM_swarrow},
	{"tan",  LM_TK_FUNC, 0},
	{"tanh",  LM_TK_FUNC, 0},
	{"tau",  LM_TK_SYM, LM_tau},
	{"textrm",  LM_TK_FONT, LM_TC_TEXTRM},
	{"theta",  LM_TK_SYM, LM_theta},
	{"tilde",  LM_TK_DECORATION, LM_tilde},
	{"times",  LM_TK_SYM, LM_times},
	{"top",  LM_TK_SYM, LM_top},
	{"triangle",  LM_TK_SYM, LM_triangle},
	{"triangleleft",  LM_TK_SYM, LM_triangleleft},
	{"triangleright",  LM_TK_SYM, LM_triangleright},
	{"underbrace",  LM_TK_DECORATION, LM_underbrace},
	{"underline",  LM_TK_DECORATION, LM_underline},
	{"uparrow",  LM_TK_SYM, LM_uparrow},
	{"updownarrow",  LM_TK_SYM, LM_updownarrow},
	{"uplus",  LM_TK_SYM, LM_uplus},
	{"upsilon",  LM_TK_SYM, LM_upsilon},
	{"varepsilon",  LM_TK_SYM, LM_varepsilon},
	{"varphi",  LM_TK_SYM, LM_varphi},
	{"varpi",  LM_TK_SYM, LM_varpi},
	{"varsigma",  LM_TK_SYM, LM_varsigma},
	{"vartheta",  LM_TK_SYM, LM_vartheta},
	{"vdash",  LM_TK_SYM, LM_vdash},
	{"vdots",  LM_TK_DOTS, LM_vdots},
	{"vec",  LM_TK_DECORATION, LM_vec},
	{"vee",  LM_TK_SYM, LM_vee},
	{"wedge",  LM_TK_SYM, LM_wedge},
	{"widehat",  LM_TK_DECORATION, LM_widehat},
	{"widetilde",  LM_TK_DECORATION, LM_widetilde},
	{"wp",  LM_TK_SYM, LM_wp},
	{"wr",  LM_TK_SYM, LM_wr},
	{"xi",  LM_TK_SYM, LM_xi},
	{"zeta",  LM_TK_SYM, LM_zeta},
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
