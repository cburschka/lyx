#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_parser.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "debug.h"


MathArray mathed_parse_cell(string const &);


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
	lyxerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first << " [" << it->second->nargs() << "] : "
			<< it->second << "\n";
	lyxerr << "------------------------------------------\n";
}


void MathMacroTable::insertTemplate(MathMacroTemplate * p)
{
	macro_table[p->name()] = p;
}


MathMacroTemplate & MathMacroTable::provideTemplate(string const & name)
{
	builtinMacros();
	
	table_type::iterator pos = macro_table.find(name);

	if (pos == macro_table.end()) {
		lyxerr << "MathMacroTable::provideTemplate: no template with name '"
		       << name << "' available.\n";
	}
		
	return *pos->second;
}


void MathMacroTable::createTemplate
	(string const & name, int na, string const & text)
{
	MathMacroTemplate * t = new MathMacroTemplate(name, na);
	t->cell(0) = mathed_parse_cell(text);
	insertTemplate(t);
#ifdef WITH_WARNINGS
#warning who frees this?
#endif
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
   
	createTemplate("emptyset",     0, "\\not0");
	createTemplate("ge",           0, "\\geq");
	createTemplate("gets",         0, "\\leftarrow");
	createTemplate("land",         0, "\\wedge");
	createTemplate("le",           0, "\\leq");
	createTemplate("lor",          0, "\\vee");
	createTemplate("notin",        0, "\\not\\in");
	createTemplate("perp",         0, "\\bot");
	createTemplate("to",           0, "\\rightarrow");
	//createTemplate("lint",       4, "\\int_{#1}^{#2}#3 d#4");
	//createTemplate("silentmult", 0, "\\cdot");
	//createTemplate("binomi",     2, "\\left(\\frac{#1}{#2}\\right)");

	// binom has two arguments
	{
		MathFracInset * frac = new MathFracInset("atop");
		frac->cell(0).push_back(new MathMacroArgument(1));
		frac->cell(1).push_back(new MathMacroArgument(2));

		MathInset * inset = new MathDelimInset('(', ')');
		inset->push_back(frac);

		MathMacroTemplate * t = new MathMacroTemplate("binom", 2);
		t->push_back(inset);

		insertTemplate(t);
	}

/*
	{
		MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
		frac->cell(0)->push_back(new MathMacroArgument(1));
		frac->cell(1)->push_back(new MathMacroArgument(2));

		MathMacroTemplate * t = new MathMacroTemplate("choose", 2);
		t->push_back(frac);

		insertTemplate(t);
	}
*/
}
