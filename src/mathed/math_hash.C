#include <config.h>

#include "math_parser.h"
#include "lyxlex.h"
#include "debug.h"
#include "support/filetools.h" // LibFileSearch

#include <map>


namespace {

std::map<string, latexkeys> theWordList;


struct key_type {
	///
	char const * name;
	///
	short token;
	///
	unsigned int id;
};

key_type wordlist_array[] = 
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
	{"]",  LM_TK_END, LM_OT_EQUATION},
	{"_",  LM_TK_SPECIAL, '_'},
	{"acute",  LM_TK_DECORATION, 0},
	{"arccos",  LM_TK_FUNC, 0},
	{"arcsin",  LM_TK_FUNC, 0},
	{"arctan",  LM_TK_FUNC, 0},
	{"arg",  LM_TK_FUNC, 0},
	{"atop",  LM_TK_ATOP, 0},
	{"bar",  LM_TK_DECORATION, 0},
	{"begin",  LM_TK_BEGIN, 0},
	{"bf",  LM_TK_OLDFONT, LM_TC_BF},
	{"binom",  LM_TK_BINOM, 0},
	{"bmod",  LM_TK_FUNC, 0},
	{"breve",  LM_TK_DECORATION, 0},
	{"cal",  LM_TK_OLDFONT, LM_TC_CAL},
	{"cdots",  LM_TK_DOTS, 0},
	{"check",  LM_TK_DECORATION, 0},
	{"choose",  LM_TK_CHOOSE, 0},
	{"cos",  LM_TK_FUNC, 0},
	{"cosh",  LM_TK_FUNC, 0},
	{"cot",  LM_TK_FUNC, 0},
	{"coth",  LM_TK_FUNC, 0},
	{"csc",  LM_TK_FUNC, 0},
	{"ddot",  LM_TK_DECORATION, 0},
	{"ddots",  LM_TK_DOTS, 0},
	{"deg",  LM_TK_FUNC, 0},
	{"det",  LM_TK_FUNCLIM, 0},
	{"dim",  LM_TK_FUNC, 0},
	//{"displaystyle",  LM_TK_STY, LM_ST_DISPLAY},
	{"dot",  LM_TK_DECORATION, 0},
	{"end",  LM_TK_END, 0},
	{"exp",  LM_TK_FUNC, 0},
	{"frac",  LM_TK_FRAC, 0},
	{"gcd",  LM_TK_FUNCLIM, 0},
	{"grave",  LM_TK_DECORATION, 0},
	{"hat",  LM_TK_DECORATION, 0},
	{"hom",  LM_TK_FUNC, 0},
	{"inf",  LM_TK_FUNCLIM, 0},
	{"it",  LM_TK_OLDFONT, LM_TC_IT},
	{"ker",  LM_TK_FUNC, 0},
	{"kern",  LM_TK_KERN, 0},
	{"label",  LM_TK_LABEL, 0},
	{"lefteqn",  LM_TK_LEFTEQN, 1},
	{"ldots",  LM_TK_DOTS, 0},
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
	{"mathfrak",  LM_TK_FONT, LM_TC_EUFRAK},
	{"mathit",  LM_TK_FONT, LM_TC_IT},
	{"mathnormal",  LM_TK_FONT, LM_TC_VAR},
	{"mathrm",  LM_TK_FONT, LM_TC_RM},
	{"mathsf",  LM_TK_FONT, LM_TC_SF},
	{"mathtt",  LM_TK_FONT, LM_TC_TT},
	{"max",  LM_TK_FUNCLIM, 0},
	//{"mbox",  LM_TK_BOX, 0},
	{"min",  LM_TK_FUNCLIM, 0},
	{"newcommand",  LM_TK_NEWCOMMAND, 0 },
	{"nolimits",  LM_TK_LIMIT, static_cast<unsigned>(-1)},
	{"nonumber",  LM_TK_NONUM, 0},
	{"not",  LM_TK_NOT, 0},
	{"over",  LM_TK_OVER, 0},
	{"overbrace",  LM_TK_DECORATION, 0},
	{"overleftarrow",  LM_TK_DECORATION, 0},
	{"overline",  LM_TK_DECORATION, 0},
	{"overrightarrow",  LM_TK_DECORATION, 0},
	{"protect",  LM_TK_PROTECT, 0},
	{"qquad",  LM_TK_SPACE, 5},
	{"quad",  LM_TK_SPACE, 4},
	{"right",  LM_TK_RIGHT, 0},
	{"rm",  LM_TK_OLDFONT, LM_TC_RM},
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
	{"tilde",  LM_TK_DECORATION, 0},
	{"tt",  LM_TK_OLDFONT, LM_TC_TT},
	{"underbrace",  LM_TK_DECORATION, 0},
	{"underline",  LM_TK_DECORATION, 0},
	{"vdots",  LM_TK_DOTS, 0},
	{"vec",  LM_TK_DECORATION, 0},
	{"widehat",  LM_TK_DECORATION, 0},
	{"widetilde",  LM_TK_DECORATION, 0},
	{"{",  LM_TK_SPECIAL, '{'},
	{"}",  LM_TK_SPECIAL, '}'}
};



MathTokenEnum tokenEnum(const string & font)
{
	if (font == "cmr")
		return LM_TK_CMR;
	if (font == "cmsy")
		return LM_TK_CMSY;
	if (font == "cmm")
		return LM_TK_CMM;
	if (font == "cmex")
		return LM_TK_CMEX;
	if (font == "msa")
		return LM_TK_MSA;
	if (font == "msb")
		return LM_TK_MSB;
	return LM_TK_SYM;
}


void readSymbols(string const & filename)
{
	LyXLex lex(0, 0);
	lex.setFile(filename);
	while (lex.isOK() && lex.next()) {
		latexkeys tmp;
		tmp.name = lex.getString();
		if (lex.next())
			tmp.token = tokenEnum(lex.getString());
		if (lex.next())
			tmp.latex_font_id = lex.getInteger();
		if (lex.next())
			tmp.id = lex.getInteger();
		if (lex.next())
			tmp.type = lex.getString();
		if (lex.next())
			tmp.xmlname = lex.getString();
		if (theWordList.find(tmp.name) != theWordList.end())
			lyxerr << "readSymbols: token " << tmp.name
			       << " already exists.\n";
		else
			theWordList[tmp.name] = tmp;
	}
}

void initSymbols()
{
	unsigned const n = sizeof(wordlist_array) / sizeof(wordlist_array[0]);
	for (key_type * p = wordlist_array; p != wordlist_array + n; ++p) {
		latexkeys tmp;
		tmp.name          = p->name;
		tmp.token         = p->token;
		tmp.id            = p->id;
		tmp.latex_font_id = 0;
		theWordList[p->name] = tmp;
	}

	lyxerr[Debug::MATHED] << "reading symbols file\n";
	string const file = LibFileSearch(string(), "symbols");
	if (file.empty())
		lyxerr << "Could not find symbols file\n";
	else
		readSymbols(file);
}


} // namespace anon


latexkeys const * in_word_set(string const & str)
{
	static bool initialized = false;

	if (!initialized) {
		initSymbols();
		initialized = true;
	}

	std::map<string, latexkeys>::iterator it = theWordList.find(str);
	return (it != theWordList.end()) ? &(it->second) : 0;
}
