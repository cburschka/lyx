/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -a -p -o -t -G -D keywords  */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math_defs.h"
#include "math_parser.h"
//struct latexkeys { char* name; short token; int id; };

#define TOTAL_KEYWORDS 269
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 18
#define MIN_HASH_VALUE 8
#define MAX_HASH_VALUE 490
/* maximum key range = 483, duplicates = 40 */

static unsigned int
hash (register const char *str, register int len)
{
  static unsigned short asso_values[] =
    {
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
     491, 491, 491, 491, 491, 491, 491, 491,  95, 491,
     491,  45, 491,  10, 491, 491, 255, 491, 491,  65,
      30, 491,   0,  70,  80,  70,  15, 491,  10, 491,
     491, 491, 491, 491, 491, 491, 491,  50, 200, 140,
     219,  35, 135, 237, 230,  69,   0,  20,  10, 180,
     105, 247,  55, 208, 125,   0,  25,  42, 255,  75,
       5,   0,   0, 491, 491, 491, 491, 491,
    };
  return len + asso_values[str[len - 1]] + asso_values[str[0]];
}

static struct latexkeys wordlist[] =
{
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"setminus",  LM_TK_SYM, LM_setminus},
      {"ldots",  LM_TK_DOTS, LM_ldots},
      {"limits",  LM_TK_LIMIT, 1 },
      {"stackrel",  LM_TK_STACK},
      {"ll",  LM_TK_SYM, LM_ll},
      {"label",  LM_TK_LABEL},
      {"lceil",  LM_TK_SYM, LM_lceil},
      {"sqrt",  LM_TK_SQRT},
      {"times",  LM_TK_SYM, LM_times},
      {"subset",  LM_TK_SYM, LM_subset},
      {"supset",  LM_TK_SYM, LM_supset},
      {"spadesuit",  LM_TK_SYM, LM_spadesuit},
      {"Re",  LM_TK_SYM, LM_Re},
      {"left",  LM_TK_LEFT},
      {"smile",  LM_TK_SYM, LM_smile},
      {"exists",  LM_TK_SYM, LM_exists},
      {"Vert",  LM_TK_SYM, LM_Vert},
      {"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT},
      {"uplus",  LM_TK_SYM, LM_uplus},
      {"ell",  LM_TK_SYM, LM_ell},
      {"langle",  LM_TK_SYM, LM_langle},
      {"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT},
      {"zeta",  LM_TK_SYM, LM_zeta},
      {"sigma",  LM_TK_SYM, LM_sigma},
      {"arccos",  LM_TK_FUNC},
      {"sup",  LM_TK_FUNCLIM},
      {"sharp",  LM_TK_SYM, LM_sharp},
      {"sqcup",  LM_TK_SYM, LM_sqcup},
      {"sqcap",  LM_TK_SYM, LM_sqcap},
      {"approx",  LM_TK_SYM, LM_approx},
      {"triangleleft",  LM_TK_SYM, LM_triangleleft},
      {"triangleright",  LM_TK_SYM, LM_triangleright},
      {"tilde",  LM_TK_ACCENT, LM_tilde},
      {"lambda",  LM_TK_SYM, LM_lambda},
      {"emptyset",  LM_TK_MACRO, LM_emptyset},
      {"triangle",  LM_TK_SYM, LM_triangle},
      {"textstyle",  LM_TK_STY, LM_ST_TEXT},
      {"tau",  LM_TK_SYM, LM_tau},
      {"limsup",  LM_TK_FUNCLIM},
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
      {"underline",  LM_TK_WIDE, LM_underline},
      {"underbrace",  LM_TK_WIDE, LM_underbrace},
      {"eta",  LM_TK_SYM, LM_eta},
      {"acute",  LM_TK_ACCENT, LM_acute},
      {"angle",  LM_TK_SYM, LM_angle},
      {"exp",  LM_TK_FUNC},
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
      {"widehat",  LM_TK_WIDE, LM_widehat},
      {"sin",  LM_TK_FUNC},
      {"asymp",  LM_TK_SYM, LM_asymp},
      {"nolimits",  LM_TK_LIMIT, 0 },
      {"perp",  LM_TK_MACRO, LM_perp},
      {"wedge",  LM_TK_SYM, LM_wedge},
      {"ln",  LM_TK_FUNC},
      {"widetilde",  LM_TK_WIDE, LM_widetilde},
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
      {"not",  LM_TK_ACCENT, LM_not},
      {"tan",  LM_TK_FUNC},
      {"Theta",  LM_TK_SYM, LM_Theta},
      {"rceil",  LM_TK_SYM, LM_rceil},
      {"lfloor",  LM_TK_SYM, LM_lfloor},
      {"rightleftharpoons",  LM_TK_SYM, LM_rightleftharpoons},
      {"cos",  LM_TK_FUNC},
      {"sec",  LM_TK_FUNC},
      {"succ",  LM_TK_SYM, LM_succ},
      {"cdots",  LM_TK_DOTS, LM_cdots},
      {"epsilon",  LM_TK_SYM, LM_epsilon},
      {"ker",  LM_TK_FUNC},
      {"nu",  LM_TK_SYM, LM_nu},
      {"Delta",  LM_TK_SYM, LM_Delta},
      {"forall",  LM_TK_SYM, LM_forall},
      {"liminf",  LM_TK_FUNCLIM},
      {"Uparrow",  LM_TK_SYM, LM_Uparrow},
      {"upsilon",  LM_TK_SYM, LM_upsilon},
      {"right",  LM_TK_RIGHT},
      {"Updownarrow",  LM_TK_SYM, LM_Updownarrow},
      {"Pr",  LM_TK_FUNCLIM},
      {"nabla",  LM_TK_SYM, LM_nabla},
      {"arcsin",  LM_TK_FUNC},
      {"arctan",  LM_TK_FUNC},
      {"flat",  LM_TK_SYM, LM_flat},
      {"check",  LM_TK_ACCENT, LM_check},
      {"rangle",  LM_TK_SYM, LM_rangle},
      {"cot",  LM_TK_FUNC},
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
      {"max",  LM_TK_FUNCLIM},
      {"Im",  LM_TK_SYM, LM_Im},
      {"lim",  LM_TK_FUNCLIM},
      {"rightharpoonup",  LM_TK_SYM, LM_rightharpoonup},
      {"mathcal",  LM_TK_FONT, LM_TC_CAL},
      {"cap",  LM_TK_SYM, LM_cap},
      {"cup",  LM_TK_SYM, LM_cup},
      {"prec",  LM_TK_SYM, LM_prec},
      {"mathnormal",  LM_TK_FONT, LM_TC_NORMAL},
      {"wr",  LM_TK_SYM, LM_wr},
      {"inf",  LM_TK_FUNCLIM},
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
      {"sinh",  LM_TK_FUNC},
      {"jmath",  LM_TK_SYM, LM_jmath},
      {"mp",  LM_TK_SYM, LM_mp},
      {"pm",  LM_TK_SYM, LM_pm},
      {"nonumber",  LM_TK_NONUM},
      {"breve",  LM_TK_ACCENT, LM_breve},
      {"bigvee",  LM_TK_BIGSYM, LM_vee},
      {"bowtie",  LM_TK_SYM, LM_bowtie},
      {"bigwedge",  LM_TK_BIGSYM, LM_wedge},
      {"frown",  LM_TK_SYM, LM_frown},
      {"rightharpoondown",  LM_TK_SYM, LM_rightharpoondown},
      {"det",  LM_TK_FUNCLIM},
      {"dot",  LM_TK_ACCENT, LM_dot},
      {"ddot",  LM_TK_ACCENT, LM_ddot},
      {"lg",  LM_TK_FUNC},
      {"log",  LM_TK_FUNC},
      {"oplus",  LM_TK_SYM, LM_oplus},
      {"ominus",  LM_TK_SYM, LM_ominus},
      {"otimes",  LM_TK_SYM, LM_otimes},
      {"beta",  LM_TK_SYM, LM_beta},
      {"diamondsuit",  LM_TK_SYM, LM_diamondsuit},
      {"rfloor",  LM_TK_SYM, LM_rfloor},
      {"end",  LM_TK_END},
      {"hat",  LM_TK_ACCENT, LM_hat},
      {"tanh",  LM_TK_FUNC},
      {"vdots",  LM_TK_DOTS, LM_vdots},
      {"bigcap",  LM_TK_BIGSYM, LM_cap},
      {"bigcup",  LM_TK_BIGSYM, LM_cup},
      {"bigsqcup",  LM_TK_BIGSYM, LM_bigsqcup},
      {"heartsuit",  LM_TK_SYM, LM_heartsuit},
      {"displaystyle",  LM_TK_STY, LM_ST_DISPLAY},
      {"longmapsto",  LM_TK_SYM, LM_longmapsto},
      {"bigtriangleup",  LM_TK_SYM, LM_bigtriangleup},
      {"preceq",  LM_TK_SYM, LM_preceq},
      {"delta",  LM_TK_SYM, LM_delta},
      {"odot",  LM_TK_SYM, LM_odot},
      {"oint",  LM_TK_BIGSYM, LM_oint},
      {"grave",  LM_TK_ACCENT, LM_grave},
      {"pmod",  LM_TK_PMOD},
      {"prod",  LM_TK_BIGSYM, LM_prod},
      {"frac",  LM_TK_FRAC},
      {"csc",  LM_TK_FUNC},
      {"circ",  LM_TK_SYM, LM_circ},
      {"aleph",  LM_TK_SYM, LM_aleph},
      {"min",  LM_TK_FUNCLIM},
      {"overline",  LM_TK_WIDE, LM_overline},
      {"arg",  LM_TK_FUNC},
      {"overbrace",  LM_TK_WIDE, LM_overbrace},
      {"amalg",  LM_TK_SYM, LM_amalg},
      {"gamma",  LM_TK_SYM, LM_gamma},
      {"vee",  LM_TK_SYM, LM_vee},
      {"equiv",  LM_TK_SYM, LM_equiv},
      {"omega",  LM_TK_SYM, LM_omega},
      {"downarrow",  LM_TK_SYM, LM_downarrow},
      {"imath",  LM_TK_SYM, LM_imath},
      {"propto",  LM_TK_SYM, LM_propto},
      {"begin",  LM_TK_BEGIN},
      {"Lambda",  LM_TK_SYM, LM_Lambda},
      {"varsigma",  LM_TK_SYM, LM_varsigma},
      {"vartheta",  LM_TK_SYM, LM_vartheta},
      {"neq",  LM_TK_SYM, LM_neq},
      {"hookleftarrow",  LM_TK_SYM, LM_hookleftarrow},
      {"hookrightarrow",  LM_TK_SYM, LM_hookrightarrow},
      {"bigtriangledown",  LM_TK_SYM, LM_bigtriangledown},
      {"mathbf",  LM_TK_FONT, LM_TC_BF},
      {"mathsf",  LM_TK_FONT, LM_TC_SF},
      {"bar",  LM_TK_ACCENT, LM_bar},
      {"varpi",  LM_TK_SYM, LM_varpi},
      {"varphi",  LM_TK_SYM, LM_varphi},
      {"newcommand",  LM_TK_NEWCOMMAND },
      {"overleftarrow",  LM_TK_WIDE, LM_overleftarrow},
      {"overrightarrow",  LM_TK_WIDE, LM_overightarrow},
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
      {"cosh",  LM_TK_FUNC},
      {"coth",  LM_TK_FUNC},
      {"rho",  LM_TK_SYM, LM_rho},
      {"cong",  LM_TK_SYM, LM_cong},
      {"vec",  LM_TK_ACCENT, LM_vec},
      {"dim",  LM_TK_FUNC},
      {"mid",  LM_TK_SYM, LM_mid},
      {"hom",  LM_TK_FUNC},
      {"bmod",  LM_TK_FUNC},
      {"quad",  LM_TK_SPACE, LM_quad},
      {"doteq",  LM_TK_SYM, LM_doteq},
      {"qquad",  LM_TK_SPACE, LM_qquad},
      {"mapsto",  LM_TK_SYM, LM_mapsto},
      {"backslash",  LM_TK_SYM, LM_backslash},
      {"diamond",  LM_TK_SYM, LM_diamond},
      {"geq",  LM_TK_SYM, LM_geq},
      {"deg",  LM_TK_FUNC},
      {"gcd",  LM_TK_FUNCLIM},
      {"gg",  LM_TK_SYM, LM_gg},
      {"div",  LM_TK_SYM, LM_div},
      {"dashv",  LM_TK_SYM, LM_dashv},
      {"oslash",  LM_TK_SYM, LM_oslash},
      {"vdash",  LM_TK_SYM, LM_vdash},
};

static short lookup[] =
{
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   8,  -1,  -1,  -1,  -1,  -1,
        -1,   9,  10,  -1,  11,  -1,  -1,  -1,  12,  -1,  -1, 491, -13,  -2,
        -1,  15,  16, 491, -17,  -2,  19,  -1,  -1,  20,  -1,  21,  22,  23,
        -1,  -1,  24,  -1,  25,  26,  27,  -1,  -1,  28,  29,  -1,  30,  31,
        32,  -1,  33,  -1, 635,  37,  38,  39,  -1,  40,  41,  -1, 617,  44,
        45,  46,  47,  48,  49,  50,  51,  -1,  52,  -1,  53,  54, 588,  57,
        -1,  58,  59,  60,  61,  -1, 491, -62,  -2,  64,  65,  66,  -1,  67,
        68, 565,  71,  72, 499,  75,  -1,  76,  -1,  77,  78,  -1,  79, -73,
        -2,  80,  81,  82,  -1,  83,  -1,  84,  85,  -1,  86,  87,  88,  89,
        90, 501,  93,  94,  95,  -1,  96, 493,  -1,  99, -97,  -2, -91,  -2,
       100, 101, 102, 517, 105, 106,  -1, 107, 108, 109, 110, 497, 113,  -1,
       114, 115, 116, 117,-111,  -2, 118, 491,-119,  -2, 121, 122, 123,  -1,
       124, 125,-103,  -2,  -1, 126, -69,  -2, 491,-127,  -2, 129, -55,  -2,
       130, 491,-131,  -2, 133, 492, 136,-134,  -2,  -1, 137, 138, 139, -42,
        -2, 140, 495, 143, 144,  -1, 145,-141,  -2, -34,  -3, 146, 613, 149,
       150, 605, 154, 155, 156, 157, 503,  -1, 491,-160,  -2, 162,  -1, 163,
       491,-164,  -2,  -1, 166,-158,  -2, 167, 168,  -1, 169, 170,  -1, 577,
       173,  -1, 174, 554,  -1, 177,  -1, 178, 179, 543, 182, 183, 184,  -1,
       185, 509, 188, 189, 190, 191, 192, 193, 194, 499,  -1, 197, 198,  -1,
       199, 200, 201, 202,-195,  -2,-186,  -2, 203,  -1, 500, 206, 492, 209,
      -207,  -2,  -1, 210, 211, 212,-204,  -2, 213,  -1, 498, 216, 494, 219,
        -1, 220,-217,  -2,-214,  -2,-180,  -2, 221, 222, 223,-175,  -2,  -1,
       224,  -1, 225, 226,  -1, 491,-227,  -2, 229,  -1, 230, 231, 232, 491,
      -233,  -2,-171,  -2,-151,  -3, 235, 236, 237,-147,  -2,  -1, 238, 239,
       240,  -1,  -1, 241,  -1,  -1,  -1, 242, 498, 245,  -1, 246, 247,  -1,
       248, 249,-243,  -2,  -1,  -1,  -1,  -1,  -1, 250,  -1,  -1,  -1,  -1,
        -1, 251, 252,  -1,  -1,  -1, 253,  -1,  -1,  -1, 492, 256,-254,  -2,
        -1,  -1,  -1, 257,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1, 258,  -1,  -1,  -1, 491,-259,  -2,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1, 261,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1, 262,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 263, 492, 266,
      -264,  -2,  -1,  -1,  -1, 267,  -1,  -1,  -1,  -1,  -1, 268,  -1,  -1,
       269,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 491,-270,  -2,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       272, 273,  -1, 274,  -1,  -1,  -1, 275,  -1,  -1,  -1,  -1,  -1,  -1,
       276,
};

struct latexkeys *
in_word_set (register const char *str, register int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int idx = lookup[key];

          if (idx >= 0 && idx < MAX_HASH_VALUE)
            {
              register char const * s = wordlist[idx].name;

              if (*s == *str && !strcmp (str + 1, s + 1))
                return &wordlist[idx];
            }
          else if (idx < 0 && idx >= -MAX_HASH_VALUE)
            return 0;
          else
            {
              register int offset = key + idx + (idx > 0 ? -MAX_HASH_VALUE : MAX_HASH_VALUE);
              register struct latexkeys *base = &wordlist[-lookup[offset]];
              register struct latexkeys *ptr = base + -lookup[offset + 1];

              while (--ptr >= base)
                if (*str == *ptr->name && !strcmp (str + 1, ptr->name + 1))
                  return ptr;
            }
        }
    }
  return 0;
}

latexkeys *lm_get_key_by_id(int t, short tk)
{
   latexkeys* l = &wordlist[MIN_HASH_VALUE+TOTAL_KEYWORDS];
   latexkeys* base = &wordlist[MIN_HASH_VALUE];
   while (--l >= base) {
     if (t==l->id && tk==l->token)
       return l;
   }
   return 0;
}

latexkeys *lm_get_key_by_index(int i)
{
   if (i>0 && i<TOTAL_KEYWORDS+2)
     return &wordlist[i];
   else
     return 0;
}
