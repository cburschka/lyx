#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "array.h"
#include "math_accentinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_parinset.h"
#include "debug.h"
#include "support/LAssert.h"

using std::endl;
using std::make_pair;

MathMacroTable::table_type MathMacroTable::macro_table;

bool MathMacroTable::built = false;


void MathMacroTable::dump()
{
	using std::cerr;

	cerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		cerr << it->first << ": " << it->second->GetData() << endl;
	cerr << "------------------------------------------\n";
}


MathMacroTemplate &
MathMacroTable::provideTemplate(string const & name, int na)
{
	if (!built)
		builtinMacros();
	
	if (macro_table.find(name) == macro_table.end())
		macro_table.insert(make_pair(name, new MathMacroTemplate(name, na)));
	
	return *(macro_table.find(name)->second);
}


MathMacroTemplate &
MathMacroTable::provideTemplate(string const & name)
{
	if (!built)
		builtinMacros();
	
	return *macro_table[name];
}


bool MathMacroTable::hasTemplate(string const & name)
{
	if (!built)
		builtinMacros();
	
	return macro_table.find(name) != macro_table.end();
}


MathMacro * MathMacroTable::cloneTemplate(string const & name)
{
	return new MathMacro(provideTemplate(name));
}


void MathMacroTable::builtinMacros()
{
	built = true;
    
	lyxerr[Debug::MATHED] << "Building macros" << endl;
    
	// This macro doesn't have arguments
	{
		MathMacroTemplate & m = provideTemplate("notin", 0);
		m.push_back(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not), LM_TC_INSET);
	}

	// This macro doesn't have arguments
	{
		MathMacroTemplate & m = provideTemplate("silentmult", 0);
		istringstream is("\\cdot\0");
		mathed_parser_file(is, 0);
		MathParInset * p = &m;
  	mathed_parse(m.array, p, 0);
	}

	{
		MathMacroTemplate & m = provideTemplate("emptyset", 0);
		m.push_back(new MathAccentInset('0', LM_TC_RM, LM_not), LM_TC_INSET);
	}

	{
		MathMacroTemplate & m = provideTemplate("perp", 0);
		m.GetData().push_back(LM_bot, LM_TC_BOP);
	}

	{
		MathMacroTemplate & m = provideTemplate("lint", 4);
		istringstream is("\\int_{#1}^{#2}#3 d#4\0");
		mathed_parser_file(is, 0);
		MathParInset * p = &m;
  	mathed_parse(m.array, p, 0);
	}

	// binom has two arguments
	{
		MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
		frac->push_back(new MathMacroArgument(1), LM_TC_INSET);
		frac->denom()->push_back(new MathMacroArgument(2), LM_TC_INSET);

		MathParInset * inset = new MathDelimInset('(', ')');
		inset->push_back(frac, LM_TC_ACTIVE_INSET);

		MathMacroTemplate & m = provideTemplate("binom", 2);
		m.push_back(inset, LM_TC_ACTIVE_INSET);
	}
}
