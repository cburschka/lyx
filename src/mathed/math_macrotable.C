#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "debug.h"
#include "math_support.h" // math_font_available


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
/*
	lyxerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first
			<< " [" << it->second->asMacroTemplate()->nargs() << "] : "
			<< it->second->cell(0) << "\n";
	lyxerr << "------------------------------------------\n";
*/
}


MathAtom & MathMacroTable::provide(string const & name)
{
	builtinMacros();

	table_type::iterator pos = macro_table.find(name);

	if (pos == macro_table.end()) {
		lyxerr << "MathMacroTable::provideTemplate: no template with name '"
		       << name << "' available.\n";
	}

	return pos->second;
}


void MathMacroTable::create(string const & name, int na)
{
	macro_table[name] = MathAtom(new MathMacroTemplate(name, na));
}


void MathMacroTable::create
	(string const & name, int na, MathArray const & ar1, MathArray const & ar2)
{
	MathAtom t(new MathMacroTemplate(name, na));
	t->cell(0) = ar1;
	t->cell(1) = ar2;
	macro_table[name] = t;
}


void MathMacroTable::define(string const & def)
{
	string name;
	mathed_parse_macro(name, def);
}


bool MathMacroTable::has(string const & name)
{
	builtinMacros();
	return macro_table.find(name) != macro_table.end();
}


void MathMacroTable::builtinMacros()
{
	static bool built = false;

	if (built)
		return;

	built = true;
	//lyxerr[Debug::MATHED] << "Building macros\n";
	//define("\\def\\emptyset{\\not0}");
	define("\\def\\notin{\\not\\in}");
	define("\\def\\slash{/}");
	//define("\\def\\mathcircumflex{\\^}");

	// fontmath.ltx

	define("\\def\\lnot{\\neg}");
	define("\\def\\land{\\wedge}");
	define("\\def\\lor{\\vee}");
	define("\\def\\ne{\\neq}");
	define("\\def\\le{\\leq}");
	define("\\def\\ge{\\geq}");
	define("\\def\\owns{\\ni}");
	define("\\def\\gets{\\leftarrow}");
	define("\\def\\to{\\rightarrow}");
	define("\\def\\|{\\Vert}");

	define("\\def\\longleftrightarrow{\\leftarrow\\kern-8mu\\rightarrow}");
	define("\\def\\Longleftrightarrow{\\Leftarrow\\kern-8mu\\Rightarrow}");
	define("\\def\\doteq{\\stackrel{\\cdot}{\\=}}");

	if (math_font_available(LM_TC_CMSY)) {
		define("\\def\\longrightarrow{\\lyxbar\\kern-6mu\\rightarrow}");
		define("\\def\\longleftarrow{\\leftarrow\\kern-6mu\\lyxbar}");
		define("\\def\\mapsto{\\mapstochar\\kern-4mu\\rightarrow}");
		define("\\def\\longmapsto{\\mapstochar\\kern-3mu\\lyxbar\\kern-6mu\\rightarrow}");
	}

	if (math_font_available(LM_TC_CMR) && math_font_available(LM_TC_CMSY)) {
		define("\\def\\Longrightarrow{\\lyxeq\\kern-5mu\\Rightarrow}");
		define("\\def\\Longleftarrow{\\Leftarrow\\kern-5mu\\lyxeq}");
		define("\\def\\models{\\vert\\kern-3mu\\lyxeq}");
	}

	if (math_font_available(LM_TC_CMM)) {
		define("\\def\\hookrightarrow{\\lhook\\kern-8mu\\rightarrow}");
		define("\\def\\hookleftarrow{\\leftarrow\\kern-8mu\\rhook}");
		define("\\def\\bowtie{\\triangleright\\kern-2mu\\triangleleft}");
	}

	if (math_font_available(LM_TC_MSA)) {
		//amsfonts.sty

		define("\\def\\dashrightarrow{\\lyxdabar\\lyxdabar\\lyxright}");
		define("\\def\\dashleftarrow{\\lyxleft\\lyxdabar\\lyxdabar}");
		define("\\def\\dasharrow{\\dashrightarrow}");
		define("\\def\\Box{\\square}");
		define("\\def\\Diamond{\\lozenge}");
		define("\\def\\leadsto{\\rightsquigarrow}");

		// amssymb.sty

		define("\\def\\restriction{\\upharpoonright}");
		define("\\def\\Doteq{\\doteqdot}");
		define("\\def\\doublecup{\\Cup}");
		define("\\def\\doublecap{\\Cap}");
		define("\\def\\llless{\\lll}");
		define("\\def\\gggtr{\\ggg}");
	}

	if (math_font_available(LM_TC_MSB)) {
		define("\\def\\Join{\\ltimes\\kern-12mu\\rtimes}");
	}

	//define("\def\lint",       4, "\\int_#1^#2#3 d#4}");
	//define("\\def\\silentmult{\\cdot}");
	//define("\def\binom",        2, "\\left(\\frac#1#2\\right)}");
}
