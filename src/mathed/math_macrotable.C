#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_parser.h"
#include "array.h"
#include "math_accentinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_inset.h"
#include "debug.h"
#include "support/LAssert.h"


using namespace std;


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
	cerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		cerr << it->first << " [" << it->second->nargs() << "] : "
			<< it->second << endl;
	cerr << "------------------------------------------\n";
}


void MathMacroTable::updateTemplate(MathMacroTemplate * par)
{
	table_type::iterator pos = macro_table.find(par->name());

	if (pos == macro_table.end())
		lyxerr << "MathMacroTable::updateTemplate: no template with name '"
				<< par->name() << "' available.\n";
	else
		pos->second = par;
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
    
	lyxerr[Debug::MATHED] << "Building macros\n";
    
	// This macro doesn't have arguments
	{
		MathMacroTemplate * t = new MathMacroTemplate("notin", 0);
		t->push_back(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not));
		insertTemplate(t);
	}

/*
	// This macro doesn't have arguments
	{
		MathMacroTemplate & m = createTemplate("silentmult", 0);
		istringstream is("\\cdot\0");
		mathed_parser_file(is, 0);
		MathMatrixInset * p = &m;
  	mathed_parse(m.array, p, 0);
	}
*/

	{
		MathMacroTemplate * t = new MathMacroTemplate("emptyset", 0);
		t->push_back(new MathAccentInset('0', LM_TC_RM, LM_not));
		insertTemplate(t);
	}

	{
		MathMacroTemplate * t = new MathMacroTemplate("to", 0);
		t->push_back(LM_rightarrow, LM_TC_SYMB);
		insertTemplate(t);
	}

	{
		MathMacroTemplate * t = new MathMacroTemplate("perp", 0);
		t->push_back(LM_bot, LM_TC_BOP);
		insertTemplate(t);
	}
/*
	{
		MathMacroTemplate & m = createTemplate("lint", 4);
		istringstream is("\\int_{#1}^{#2}#3 d#4\0");
		mathed_parser_file(is, 0);
		MathMatrixInset * p = &m;
  	mathed_parse(m.array, p, 0);
	}
*/
/*
	{
		MathMacroTemplate * t = new MathMacroTemplate("binomii", 2);
		istringstream is("\\left(\\frac{#1}{#2}\\right)\0");
		mathed_parser_file(is, 0);
  	mathed_parse(t->array, t, 0);
		insertTemplate(t);
	}
*/

	// binom has two arguments
	{
		MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
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
