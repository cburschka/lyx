#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_parser.h"
#include "debug.h"
#include "support.h" // math_font_available


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
	lyxerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first
			<< " [" << it->second->asMacroTemplate()->nargs() << "] : "
			<< it->second->cell(0) << "\n";
	lyxerr << "------------------------------------------\n";
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


void MathMacroTable::create(string const & name, int na, string const & text)
{
	MathAtom t(new MathMacroTemplate(name, na));
	mathed_parse_cell(t->cell(0), text);
	macro_table[name] = t;
}


void MathMacroTable::create(string const & name, int na, MathArray const & ar)
{
	MathAtom t(new MathMacroTemplate(name, na));
	t->cell(0) = ar;
	macro_table[name] = t;
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
D
	//create("emptyset",     0, "\\not0");
	create("notin",        0, "\\not\\in");
	create("slash",        0, "/");

	// fontmath.ltx

	create("lnot",         0, "\\neg");
	create("land",         0, "\\wedge");
	create("lor",          0, "\\vee");
	create("ne",           0, "\\neq");
	create("le",           0, "\\leq");
	create("ge",           0, "\\geq");
	create("owns",         0, "\\ni");
	create("gets",         0, "\\leftarrow");
	create("to",           0, "\\rightarrow");
	create("|",            0, "\\parallel");

	create("longleftrightarrow", 0, "\\leftarrow\\kern-6mu\\rightarrow");
	create("Longleftrightarrow", 0, "\\Leftarrow\\kern-6mu\\Rightarrow");
	create("doteq", 0, "\\stackrel{\\cdot}{=}");

	//create("models",         0, "|\\kern-9mu=");

	if (math_font_available(LM_TC_CMSY)) {
		create("longrightarrow", 0, "\\lyxbar\\kern-5mu\\rightarrow");
		create("longleftarrow",  0, "\\leftarrow\\kern-5mu\\lyxbar");
		create("mapsto",  0, "\\mapstochar\\rightarrow");
		create("longmapsto",  0, "\\mapstochar\\lyxbar\\kern-5mu\\rightarrow");
	}

	if (math_font_available(LM_TC_CMR)) {
		create("Longrightarrow", 0, "\\lyxeq\\kern-3mu\\Rightarrow");
		create("Longleftarrow",  0, "\\Leftarrow\\kern-9mu\\lyxeq");
	}

	if (math_font_available(LM_TC_CMM)) {
		create("hookrightarrow", 0, "\\lhook\\kern-5mu\\rightarrow");
		create("hookleftarrow",  0, "\\leftarrow\\kern-5mu\\rhook");
		create("bowtie",         0, "\\triangleright\\kern-3mu\\triangleleft");
	}

	if (math_font_available(LM_TC_MSA)) {
		//amsfonts.sty

		create("dashrightarrow", 0, "\\lyxdabar\\lyxdabar\\lyxright");
		create("dashleftarrow", 0, "\\lyxleft\\lyxdabar\\lyxdabar");
		create("dasharrow",    0, "\\dashrightarrow");
		create("Box",          0, "\\square");
		create("Diamond",      0, "\\lozenge");
		create("leadsto",      0, "\\rightsquigarrow");

		// amssymb.sty

		create("restriction",  0, "\\upharpoonright");
		create("Doteq",        0, "\\doteqdot");
		create("doublecup",    0, "\\Cup");
		create("doublecap",    0, "\\Cap");
		create("llless",       0, "\\lll");
		create("gggtr",        0, "\\ggg");
	}

	//create("lint",       4, "\\int_#1^#2#3 d#4");
	//create("silentmult", 0, "\\cdot");
	//create("binom",        2, "\\left(\\frac#1#2\\right)");
}
