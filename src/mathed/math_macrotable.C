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


MathArray mathed_parse_cell(string const &);


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
	lyxerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first << " [" << it->second.nargs() << "] : "
			<< it->second << "\n";
	lyxerr << "------------------------------------------\n";
}


void MathMacroTable::insertTemplate(MathMacroTemplate const & p)
{
	if (macro_table.find(p.name()) != macro_table.end()) 
		lyxerr << "macro '" << p.name() << "' not new\n";
	macro_table[p.name()] = p;
}


MathMacroTemplate & MathMacroTable::provideTemplate(string const & name)
{
	builtinMacros();
	
	table_type::iterator pos = macro_table.find(name);

	if (pos == macro_table.end()) {
		lyxerr << "MathMacroTable::provideTemplate: no template with name '"
		       << name << "' available.\n";
	}
		
	return pos->second;
}


void MathMacroTable::createTemplate
	(string const & name, int na, string const & text)
{
	MathMacroTemplate t(name, na);
	t.cell(0) = mathed_parse_cell(text);
	insertTemplate(t);
}


bool MathMacroTable::hasTemplate(string const & name)
{
	builtinMacros();
	return macro_table.find(name) != macro_table.end();
}


MathMacro * MathMacroTable::cloneTemplate(string const & name)
{
	return new MathMacro(provideTemplate(name));
}


void MathMacroTable::builtinMacros()
{
	static bool built = false;
	
	if (built)
		return;	

	built = true;
	//lyxerr[Debug::MATHED] << "Building macros\n";
   
	//createTemplate("emptyset",     0, "\\not0");
	createTemplate("notin",        0, "\\not\\in");

	// fontmath.ltx

	createTemplate("lnot",         0, "\\neg");
	createTemplate("land",         0, "\\wedge");
	createTemplate("lor",          0, "\\vee");
	createTemplate("ne",           0, "\\neq");
	createTemplate("le",           0, "\\leq");
	createTemplate("ge",           0, "\\geq");
	createTemplate("owns",         0, "\\ni");
	createTemplate("gets",         0, "\\leftarrow");
	createTemplate("to",           0, "\\rightarrow");

	//amsfonts.sty

	createTemplate("dasharrow",    0, "\\dashrightarrow");
	createTemplate("Box",          0, "\\square");
	createTemplate("Diamond",      0, "\\lozenge");
	createTemplate("leadsto",      0, "\\rightsquigarrow");

	// amssymb.sty

	createTemplate("restriction",  0, "\\upharpoonright");
	createTemplate("Doteq",        0, "\\doteqdot");
	createTemplate("doublecup",    0, "\\Cup");
	createTemplate("doublecap",    0, "\\Cap");
	createTemplate("llless",       0, "\\lll");
	createTemplate("gggtr",        0, "\\ggg");

// #ifdef WITH_WARNINGS
// #warning 9em looks like too much but it is somehow working on screen..
// #endif WITH_WARNINGS
// 	createTemplate("ll",           0, "<\\kern-9em<");
// 	createTemplate("gg",           0, ">\\kern-9em>");

  	//createTemplate("lint",       4, "\\int_#1^#2#3 d#4");
  	//createTemplate("silentmult", 0, "\\cdot");
  	//createTemplate("binom",        2, "\\left(\\frac#1#2\\right)");
}
