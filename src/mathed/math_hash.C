#include <config.h>

#include "math_parser.h"
#include "lyxlex.h"
#include "debug.h"
#include "support/filetools.h" // LibFileSearch
#include "support/lyxfunctional.h"

#include <vector>
#include <algorithm>


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
	///
	MathSymbolTypes type;
};

latexkeys_a wordlist_array[] = 
{
	{"!",  LM_TK_SPACE, 0, LMB_NONE},
	{"#",  LM_TK_SPECIAL, '#', LMB_NONE},
	{"$",  LM_TK_SPECIAL, '$', LMB_NONE},
	{"%",  LM_TK_SPECIAL, '%', LMB_NONE},
	{"&",  LM_TK_SPECIAL, '&', LMB_NONE},
	{"(",  LM_TK_BEGIN, LM_OT_SIMPLE, LMB_NONE},
	{")",  LM_TK_END, LM_OT_SIMPLE, LMB_NONE},
	{",",  LM_TK_SPACE, 1, LMB_NONE},
	{".",  LM_TK_SPECIAL, '.', LMB_NONE},
	{":",  LM_TK_SPACE, 2, LMB_NONE},
	{";",  LM_TK_SPACE, 3, LMB_NONE},
	{"Pr",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"[",  LM_TK_BEGIN, LM_OT_EQUATION, LMB_NONE},
	{"\\",  LM_TK_NEWLINE, static_cast<unsigned>(-1), LMB_NONE}, // -1 needed in mathed_parse_lines!
	{"]",  LM_TK_END, LM_OT_EQUATION, LMB_NONE},
	{"_",  LM_TK_SPECIAL, '_', LMB_NONE},
	{"acute",  LM_TK_DECORATION, LM_acute, LMB_NONE},
	{"arccos",  LM_TK_FUNC, 0, LMB_NONE},
	{"arcsin",  LM_TK_FUNC, 0, LMB_NONE},
	{"arctan",  LM_TK_FUNC, 0, LMB_NONE},
	{"arg",  LM_TK_FUNC, 0, LMB_NONE},
	{"atop",  LM_TK_ATOP, 0, LMB_NONE},
	{"backslash",  LM_TK_SPECIAL, '\\', LMB_NONE},
	{"bar",  LM_TK_DECORATION, LM_bar, LMB_NONE},
	{"begin",  LM_TK_BEGIN, 0, LMB_NONE},
	{"binom",  LM_TK_BINOM, 0, LMB_NONE},
	{"bmod",  LM_TK_FUNC, 0, LMB_NONE},
	{"breve",  LM_TK_DECORATION, LM_breve, LMB_NONE},
	{"cal",  LM_TK_OLDFONT, LM_TC_CAL, LMB_OPERATOR},
	{"cdots",  LM_TK_DOTS, LM_cdots, LMB_NONE},
	{"check",  LM_TK_DECORATION, LM_check, LMB_NONE},
	{"choose",  LM_TK_CHOOSE, 0, LMB_NONE},
	{"cos",  LM_TK_FUNC, 0, LMB_NONE},
	{"cosh",  LM_TK_FUNC, 0, LMB_NONE},
	{"cot",  LM_TK_FUNC, 0, LMB_NONE},
	{"coth",  LM_TK_FUNC, 0, LMB_NONE},
	{"csc",  LM_TK_FUNC, 0, LMB_NONE},
	{"ddot",  LM_TK_DECORATION, LM_ddot, LMB_NONE},
	{"ddots",  LM_TK_DOTS, LM_ddots, LMB_NONE},
	{"deg",  LM_TK_FUNC, 0, LMB_NONE},
	{"det",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"dim",  LM_TK_FUNC, 0, LMB_NONE},
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY, LMB_NONE},
	{"dot",  LM_TK_DECORATION, LM_dot, LMB_NONE},
	{"end",  LM_TK_END, 0, LMB_NONE},
	{"exp",  LM_TK_FUNC, 0, LMB_NONE},
	{"frac",  LM_TK_FRAC, 0, LMB_NONE},
	{"gcd",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"grave",  LM_TK_DECORATION, LM_grave, LMB_NONE},
	{"hat",  LM_TK_DECORATION, LM_hat, LMB_NONE},
	{"hom",  LM_TK_FUNC, 0, LMB_NONE},
	{"inf",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"ker",  LM_TK_FUNC, 0, LMB_NONE},
	{"kern",  LM_TK_KERN, 0, LMB_NONE},
	{"label",  LM_TK_LABEL, 0, LMB_NONE},
	{"ldots",  LM_TK_DOTS, LM_ldots, LMB_NONE},
	{"left",  LM_TK_LEFT, 0, LMB_NONE},
	{"lg",  LM_TK_FUNC, 0, LMB_NONE},
	{"lim",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"liminf",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"limits",  LM_TK_LIMIT, 1 , LMB_NONE},
	{"limsup",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"ln",  LM_TK_FUNC, 0, LMB_NONE},
	{"log",  LM_TK_FUNC, 0, LMB_NONE},
	{"mathbf",  LM_TK_FONT, LM_TC_BF, LMB_NONE},
	{"mathcal",  LM_TK_FONT, LM_TC_CAL, LMB_NONE},
	{"mathit",  LM_TK_FONT, LM_TC_IT, LMB_NONE},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR, LMB_NONE},
	{"mathrm",  LM_TK_FONT, LM_TC_RM, LMB_NONE},
	{"mathsf",  LM_TK_FONT, LM_TC_SF, LMB_NONE},
	{"mathtt",  LM_TK_FONT, LM_TC_TT, LMB_NONE},
	{"max",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"min",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 , LMB_NONE},
	{"nolimits",  LM_TK_LIMIT, static_cast<unsigned>(-1), LMB_NONE},
	{"nonumber",  LM_TK_NONUM, 0, LMB_NONE},
	{"not",  LM_TK_NOT, LM_not, LMB_NONE},
	{"over",  LM_TK_OVER, 0, LMB_NONE},
	{"overbrace",  LM_TK_DECORATION, LM_overbrace, LMB_NONE},
	{"overleftarrow",  LM_TK_DECORATION, LM_overleftarrow, LMB_NONE},
	{"overline",  LM_TK_DECORATION, LM_overline, LMB_NONE},
	{"overrightarrow",  LM_TK_DECORATION, LM_overightarrow, LMB_NONE},
	{"protect",  LM_TK_PROTECT, 0, LMB_RELATION},
	{"qquad",  LM_TK_SPACE, 5, LMB_NONE},
	{"quad",  LM_TK_SPACE, 4, LMB_NONE},
	{"right",  LM_TK_RIGHT, 0, LMB_NONE},
	{"root",  LM_TK_ROOT, 0, LMB_NONE},
	//{"scriptscriptstyle",  LM_TK_STY, LM_ST_SCRIPTSCRIPT, LMB_NONE},
	//{"scriptstyle",  LM_TK_STY, LM_ST_SCRIPT, LMB_NONE},
	{"sec",  LM_TK_FUNC, 0, LMB_NONE},
	{"sin",  LM_TK_FUNC, 0, LMB_NONE},
	{"sinh",  LM_TK_FUNC, 0, LMB_NONE},
	{"sqrt",  LM_TK_SQRT, 0, LMB_NONE},
	{"stackrel",  LM_TK_STACK, 0, LMB_NONE},
	{"sup",  LM_TK_FUNCLIM, 0, LMB_NONE},
	{"tan",  LM_TK_FUNC, 0, LMB_NONE},
	{"tanh",  LM_TK_FUNC, 0, LMB_NONE},
	{"textrm",  LM_TK_FONT, LM_TC_TEXTRM, LMB_NONE},
	//{"textstyle",  LM_TK_STY, LM_ST_TEXT, LMB_NONE},
	{"tilde",  LM_TK_DECORATION, LM_tilde, LMB_NONE},
	{"underbrace",  LM_TK_DECORATION, LM_underbrace, LMB_NONE},
	{"underline",  LM_TK_DECORATION, LM_underline, LMB_NONE},
	{"vdots",  LM_TK_DOTS, LM_vdots, LMB_NONE},
	{"vec",  LM_TK_DECORATION, LM_vec, LMB_NONE},
	{"widehat",  LM_TK_DECORATION, LM_widehat, LMB_NONE},
	{"widetilde",  LM_TK_DECORATION, LM_widetilde, LMB_NONE},
	{"{",  LM_TK_SPECIAL, '{', LMB_NONE},
	{"|",  LM_TK_UNDEF, '|', LMB_NONE},
	{"}",  LM_TK_SPECIAL, '}', LMB_NONE},
	{"", LM_TK_SPECIAL, 0, LMB_NONE}

};


std::vector<latexkeys> wordlist;

bool initialized = false;

} // namespace anon

void ReadSymbols(string const & filename)
{
	for(latexkeys_a * p = wordlist_array; !string(p->name).empty(); ++p) {
		latexkeys tmp;
		tmp.name = p->name;
		tmp.token = p->token;
		tmp.id = p->id;
		tmp.latex_font_id = 0;
		tmp.type = p->type;
		wordlist.push_back(tmp);
	}

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

		if (font == "cmsy")
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

	std::sort(wordlist.begin(), wordlist.end());
}


latexkeys const * in_word_set(string const & str)
{
	if (!initialized) {
		lyxerr[Debug::MATHED] << "Reading symbols file\n";
		string const file = LibFileSearch(string(), "symbols");
		if (file.empty())
			lyxerr << "Could not find symbols file" << endl;
                else
			ReadSymbols(file);
		initialized = true;
        }

	std::vector<latexkeys>::iterator it =
		std::find_if(wordlist.begin(), wordlist.end(),
			     lyx::compare_memfun(&latexkeys::Name, str));
	return  (it != wordlist.end()) ? &(*it) : 0;
}
