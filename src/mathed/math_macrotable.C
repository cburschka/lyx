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

MathMacroTable MathMacroTable::mathMTable;

bool MathMacroTable::built = false;


MathMacro * MathMacroTable::createMacro(string const & name) const
{
	boost::shared_ptr<MathMacroTemplate> mt = getTemplate(name);
	return (mt.get()) ? new MathMacro(mt) : 0;
}


boost::shared_ptr<MathMacroTemplate> const
MathMacroTable::getTemplate(string const & name) const
{
	table_type::const_iterator cit = macro_table.find(name);
	if (cit != macro_table.end()) return (*cit).second;
	return boost::shared_ptr<MathMacroTemplate>();
}


void
MathMacroTable::addTemplate(boost::shared_ptr<MathMacroTemplate> const & m)
{
	Assert(m.get());
	macro_table[m->GetName()] = m;
}


void MathMacroTable::builtinMacros()
{
	built = true;
    
	lyxerr[Debug::MATHED] << "Building macros" << endl;
    
	// This macro doesn't have arguments
	{
		boost::shared_ptr<MathMacroTemplate> m(new MathMacroTemplate("notin", 0));
		addTemplate(m);
		MathedArray array;
		MathedIter iter(&array);
		iter.insertInset(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not),
				 LM_TC_INSET);
		m->setData(array);
	}
    
	// These two are only while we are still with LyX 2.x
	{
		boost::shared_ptr<MathMacroTemplate> m(new MathMacroTemplate("emptyset", 0));
		addTemplate(m); 
		MathedArray array;
		MathedIter iter(&array);
		iter.insertInset(new MathAccentInset('O', LM_TC_RM, LM_not),
				 LM_TC_INSET);
		m->setData(array);
	}
    
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
}
