#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_iter.h"
#include "array.h"
#include "math_accentinset.h"
#include "math_deliminset.h"
#include "math_fracinset.h"
#include "math_parinset.h"
#include "debug.h"
#include "support/LAssert.h"

using std::endl;

MathMacroTable::table_type MathMacroTable::macro_table;

bool MathMacroTable::built = false;


void MathMacroTable::dump()
{
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
		MathedIter iter(&m.GetData());
		iter.insertInset(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not),
				 LM_TC_INSET);
	}

	// These two are only while we are still with LyX 2.x
	{
		MathMacroTemplate & m = provideTemplate("emptyset", 0);
		MathedIter iter(&m.GetData());
		iter.insertInset(new MathAccentInset('0', LM_TC_RM, LM_not),
				 LM_TC_INSET);
	}

	{
		MathMacroTemplate & m = provideTemplate("perp", 0);
		MathedIter iter(&m.GetData());
		iter.insert(LM_bot, LM_TC_BOP);
	}

	// binom has two arguments
	{
		MathMacroTemplate & m = provideTemplate("binom", 2);
		MathedIter iter(&m.GetData());

		MathParInset * inset = new MathDelimInset('(', ')');
		iter.insertInset(inset, LM_TC_ACTIVE_INSET);

		MathedArray array2;
		MathedIter iter2(&array2);
		MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
		iter2.insertInset(frac, LM_TC_ACTIVE_INSET);
		frac->setData(array2);

		MathedArray array3;
		MathedIter iter3(&array3);
		iter3.insertInset(new MathMacroArgument(1), LM_TC_INSET);

		MathedArray array4;
		MathedIter iter4(&array4);
		iter4.insertInset(new MathMacroArgument(2), LM_TC_INSET);

		frac->SetData(array3, array4);
	}

/*    
	{
		boost::shared_ptr<MathMacroTemplate> m(new MathMacroTemplate("perp", 0));
		addTemplate(m);
		MathedArray array;
		MathedIter iter(&array);
		iter.insert(LM_bot, LM_TC_BOP);
		m->setData(array);
	}

	// binom has two arguments
	{
		boost::shared_ptr<MathMacroTemplate> m(new MathMacroTemplate("binom", 2));
		addTemplate(m);
		MathedArray array;
		m->setData(array);
		MathedIter iter(&array);
		MathParInset * inset = new MathDelimInset('(', ')');
		iter.insertInset(inset, LM_TC_ACTIVE_INSET);
		array = MathedArray();
		MathedIter iter2(&array);
		MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
		iter2.insertInset(frac, LM_TC_ACTIVE_INSET);
		inset->setData(array);
		array = MathedArray();
		MathedArray array2;
		MathedIter iter3(&array);
		iter3.insertInset(m->getMacroPar(0), LM_TC_INSET);
		MathedIter iter4(&array2);
		iter4.insertInset(m->getMacroPar(1), LM_TC_INSET);
		frac->SetData(array, array2);
	}
*/
}
