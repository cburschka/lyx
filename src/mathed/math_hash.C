#include <config.h>

#include "math_parser.h"
#include "lyxlex.h"
#include "debug.h"
#include "support/filetools.h" // LibFileSearch
#include "support/lyxfunctional.h"

#include <vector>
#include <algorithm>

using std::endl;

bool operator<(const latexkeys & a, const latexkeys & b)
{
	return string(a.name) < string(b.name);
}

namespace {

// This lists needs to remain sorted all the time!

struct latexkeys_a {
	///
	char const * name;
	///
	short token;
	///
	unsigned int id;
};

latexkeys_a wordlist_array[] = 
{
	{"!",  LM_TK_SPACE, 0},
	{"#",  LM_TK_SPECIAL, '#'},
	{"$",  LM_TK_SPECIAL, '$'},
	{"%",  LM_TK_SPECIAL, '%'},
	{"&",  LM_TK_SPECIAL, '&'},
	{"(",  LM_TK_BEGIN, LM_OT_SIMPLE},
	{")",  LM_TK_END, LM_OT_SIMPLE},
	{",",  LM_TK_SPACE, 1},
	{".",  LM_TK_SPECIAL, '.'},
	{":",  LM_TK_SPACE, 2},
	{";",  LM_TK_SPACE, 3},
	{"Pr",  LM_TK_FUNCLIM, 0},
	{"[",  LM_TK_BEGIN, LM_OT_EQUATION},
	{"\\",  LM_TK_NEWLINE, static_cast<unsigned>(-1)}, // -1 needed in mathed_parse_lines!
	{"]",  LM_TK_END, LM_OT_EQUATION},
	{"_",  LM_TK_SPECIAL, '_'},
	{"acute",  LM_TK_DECORATION, LM_acute},
	{"arccos",  LM_TK_FUNC, 0},
	{"arcsin",  LM_TK_FUNC, 0},
	{"arctan",  LM_TK_FUNC, 0},
	{"arg",  LM_TK_FUNC, 0},
	{"atop",  LM_TK_ATOP, 0},
	{"backslash",  LM_TK_SPECIAL, '\\'},
	{"bar",  LM_TK_DECORATION, LM_bar},
	{"begin",  LM_TK_BEGIN, 0},
	{"binom",  LM_TK_BINOM, 0},
	{"bmod",  LM_TK_FUNC, 0},
	{"breve",  LM_TK_DECORATION, LM_breve},
	{"cal",  LM_TK_OLDFONT, LM_TC_CAL},
	{"cdots",  LM_TK_DOTS, LM_cdots},
	{"check",  LM_TK_DECORATION, LM_check},
	{"choose",  LM_TK_CHOOSE, 0},
	{"cos",  LM_TK_FUNC, 0},
	{"cosh",  LM_TK_FUNC, 0},
	{"cot",  LM_TK_FUNC, 0},
	{"coth",  LM_TK_FUNC, 0},
	{"csc",  LM_TK_FUNC, 0},
	{"ddot",  LM_TK_DECORATION, LM_ddot},
	{"ddots",  LM_TK_DOTS, LM_ddots},
	{"deg",  LM_TK_FUNC, 0},
	{"det",  LM_TK_FUNCLIM, 0},
	{"dim",  LM_TK_FUNC, 0},
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY},
	{"dot",  LM_TK_DECORATION, LM_dot},
	{"end",  LM_TK_END, 0},
	{"exp",  LM_TK_FUNC, 0},
	{"frac",  LM_TK_FRAC, 0},
	{"gcd",  LM_TK_FUNCLIM, 0},
	{"grave",  LM_TK_DECORATION, LM_grave},
	{"hat",  LM_TK_DECORATION, LM_hat},
	{"hom",  LM_TK_FUNC, 0},
	{"inf",  LM_TK_FUNCLIM, 0},
	{"ker",  LM_TK_FUNC, 0},
	{"kern",  LM_TK_KERN, 0},
	{"label",  LM_TK_LABEL, 0},
	{"ldots",  LM_TK_DOTS, LM_ldots},
	{"left",  LM_TK_LEFT, 0},
	{"lg",  LM_TK_FUNC, 0},
	{"lim",  LM_TK_FUNCLIM, 0},
	{"liminf",  LM_TK_FUNCLIM, 0},
	{"limits",  LM_TK_LIMIT, 1 },
	{"limsup",  LM_TK_FUNCLIM, 0},
	{"ln",  LM_TK_FUNC, 0},
	{"log",  LM_TK_FUNC, 0},
	{"mathbb",  LM_TK_FONT, LM_TC_BB},
	{"mathbf",  LM_TK_FONT, LM_TC_BF},
	{"mathcal",  LM_TK_FONT, LM_TC_CAL},
	{"mathit",  LM_TK_FONT, LM_TC_IT},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR},
	{"mathrm",  LM_TK_FONT, LM_TC_RM},
	{"mathsf",  LM_TK_FONT, LM_TC_SF},
	{"mathtt",  LM_TK_FONT, LM_TC_TT},
	{"max",  LM_TK_FUNCLIM, 0},
	{"min",  LM_TK_FUNCLIM, 0},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 },
	{"nolimits",  LM_TK_LIMIT, static_cast<unsigned>(-1)},
	{"nonumber",  LM_TK_NONUM, 0},
	{"not",  LM_TK_NOT, LM_not},
	{"over",  LM_TK_OVER, 0},
	{"overbrace",  LM_TK_DECORATION, LM_overbrace},
	{"overleftarrow",  LM_TK_DECORATION, LM_overleftarrow},
	{"overline",  LM_TK_DECORATION, LM_overline},
	{"overrightarrow",  LM_TK_DECORATION, LM_overightarrow},
	{"protect",  LM_TK_PROTECT, 0},
	{"qquad",  LM_TK_SPACE, 5},
	{"quad",  LM_TK_SPACE, 4},
	{"right",  LM_TK_RIGHT, 0},
	{"root",  LM_TK_ROOT, 0},
	//{"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT},
	//{"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT},
	{"sec",  LM_TK_FUNC, 0},
	{"sin",  LM_TK_FUNC, 0},
	{"sinh",  LM_TK_FUNC, 0},
	{"sqrt",  LM_TK_SQRT, 0},
	{"stackrel",  LM_TK_STACK, 0},
	{"sup",  LM_TK_FUNCLIM, 0},
	{"tan",  LM_TK_FUNC, 0},
	{"tanh",  LM_TK_FUNC, 0},
	{"textrm",  LM_TK_FONT, LM_TC_TEXTRM},
	//{"textstyle",  LM_TK_STY, LM_ST_TEXT},
	{"tilde",  LM_TK_DECORATION, LM_tilde},
	{"underbrace",  LM_TK_DECORATION, LM_underbrace},
	{"underline",  LM_TK_DECORATION, LM_underline},
	{"vdots",  LM_TK_DOTS, LM_vdots},
	{"vec",  LM_TK_DECORATION, LM_vec},
	{"widehat",  LM_TK_DECORATION, LM_widehat},
	{"widetilde",  LM_TK_DECORATION, LM_widetilde},
	{"{",  LM_TK_SPECIAL, '{'},
	{"|",  LM_TK_UNDEF, '|'},
	{"}",  LM_TK_SPECIAL, '}'},
	{"", LM_TK_SPECIAL, 0}
};


std::vector<latexkeys> wordlist;

bool initialized = false;

} // namespace anon


void ReadSymbols(string const & filename)
{
	LyXLex lex(0, 0);
	lex.setFile(filename);
	while (lex.isOK()) {
		latexkeys tmp;
		string font;
		string type;
		
		if (lex.next())
			tmp.name = lex.getString();
		else
			break;

		if (lex.next())
			font = lex.getString();
		if (lex.next())
			tmp.latex_font_id = lex.getInteger();
		if (lex.next())
			tmp.id = lex.getInteger();
		if (lex.next())
			type = lex.getString();

		if (font == "cmr")
			tmp.token = LM_TK_CMR;
		else if (font == "cmsy")
			tmp.token = LM_TK_CMSY;
		else if (font == "cmm")
			tmp.token = LM_TK_CMM;
		else if (font == "cmex")
			tmp.token = LM_TK_CMEX;
		else if (font == "msa")
			tmp.token = LM_TK_MSA;
		else if (font == "msb")
			tmp.token = LM_TK_MSB;
		else
			tmp.token = LM_TK_SYM;

		if (type == "mathrel")
			tmp.type = LMB_RELATION;
		else if (type == "mathbin")
			tmp.type = LMB_OPERATOR;
		else
			tmp.type = LMB_NONE;

		wordlist.push_back(tmp);
	}
}


void InitSymbols()
{
	for (latexkeys_a * p = wordlist_array; !string(p->name).empty(); ++p) {
		latexkeys tmp;
		tmp.name   = p->name;
		tmp.token  = p->token;
		tmp.id     = p->id;
		tmp.type   = LMB_NONE;
		tmp.latex_font_id = 0;
		wordlist.push_back(tmp);
	}

	lyxerr[Debug::MATHED] << "Reading symbols file\n";
	string const file = LibFileSearch(string(), "symbols");
	if (file.empty())
		lyxerr << "Could not find symbols file" << endl;
	else
		ReadSymbols(file);

	std::sort(wordlist.begin(), wordlist.end());
}


latexkeys const * in_word_set(string const & str)
{
	if (!initialized) {
		InitSymbols();
		initialized = true;
	}

	std::vector<latexkeys>::iterator it =
		std::find_if(wordlist.begin(), wordlist.end(),
			     lyx::compare_memfun(&latexkeys::Name, str));
	return  (it != wordlist.end()) ? &(*it) : 0;
}
