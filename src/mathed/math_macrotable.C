#include <config.h>

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


MathMacroTable MathMacroTable::mathMTable;

bool MathMacroTable::built = false;


MathMacro * MathMacroTable::getMacro(string const & name) const
{
	MathMacroTemplate * mt = getTemplate(name);
	return (mt) ? new MathMacro(mt): 0;
}


// The search is currently linear but will be binary or hash, later.
MathMacroTemplate * MathMacroTable::getTemplate(string const & name) const
{
	for (size_type i = 0; i < macro_table.size(); ++i) {
		if (name == macro_table[i]->GetName()) 
			return macro_table[i];
	}

	return 0;
}

void MathMacroTable::addTemplate(MathMacroTemplate * m)
{
	macro_table.push_back(m);
}


// All this stuff aparently leaks because it's created here and is not 
// deleted never, but it have to live all the LyX sesion. OK, would not
// so hard to do it in the MacroTable destructor, but this doesn't harm
// seriously, so don't bother me with purify results here.   ;-)

void MathMacroTable::builtinMacros()
{
	MathedIter iter;
	MathParInset * inset;// *arg;
	MathedArray * array2;
    
	built = true;
    
	lyxerr[Debug::MATHED] << "Building macros" << endl;
    
	// This macro doesn't have arguments
	MathMacroTemplate * m = new MathMacroTemplate("notin");  // this leaks
	addTemplate(m);
	MathedArray * array = new MathedArray; // this leaks
	iter.SetData(array);
	iter.Insert(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not)); // this leaks
	m->SetData(array);
    
	// These two are only while we are still with LyX 2.x
	m = new MathMacroTemplate("emptyset"); // this leaks
	addTemplate(m);
	array = new MathedArray; // this leaks
	iter.SetData(array);
	iter.Insert(new MathAccentInset('O', LM_TC_RM, LM_not)); // this leaks
	m->SetData(array);
    
	m = new MathMacroTemplate("perp"); // this leaks
	addTemplate(m);
	array = new MathedArray; // this leaks
	iter.SetData(array);
	iter.Insert(LM_bot, LM_TC_BOP);
	m->SetData(array);

	// binom has two arguments
	m = new MathMacroTemplate("binom", 2);
	addTemplate(m);
	array = new MathedArray; 
	m->SetData(array);
	iter.SetData(array);
	inset = new MathDelimInset('(', ')');
	iter.Insert(inset, LM_TC_ACTIVE_INSET);
	array = new MathedArray; 
	iter.SetData(array);
	MathFracInset * frac = new MathFracInset(LM_OT_ATOP);
	iter.Insert(frac, LM_TC_ACTIVE_INSET);
	inset->SetData(array);
	array = new MathedArray;
	array2 = new MathedArray;  
	iter.SetData(array);
	iter.Insert(m->getMacroPar(0));
	iter.SetData(array2);
	iter.Insert(m->getMacroPar(1));
	frac->SetData(array, array2);

/*
  // Cases has 1 argument
  m = new MathMacroTemplate("cases", 1, MMF_Env); // this leaks
  addTemplate(m);
  array = new MathedArray; // this leaks
  iter.SetData(array);
  arg = new MathMatrixInset(2, 1); // this leaks

  m->setArgument(arg);
  arg->SetAlign('c', "ll");
  iter.Insert(arg, LM_TC_ACTIVE_INSET);
  inset = new MathDelimInset('{', '.'); // this leaks
  inset->SetData(array);
  array = new MathedArray; // this leaks
  iter.SetData(array);
  iter.Insert(inset, LM_TC_ACTIVE_INSET);
  m->SetData(array);
  

  // the environment substack has 1 argument
  m = new MathMacroTemplate("substack", 1, MMF_Env); // this leaks
  addTemplate(m);     
  arg = new MathMatrixInset(1, 1); // this leaks
  m->setArgument(arg);
  arg->SetType(LM_OT_MACRO);
  array = new MathedArray; // this leaks
  iter.SetData(array);
  iter.Insert(arg, LM_TC_ACTIVE_INSET);
  m->SetData(array);*/
}
