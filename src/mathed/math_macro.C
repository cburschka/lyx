// -*- C++ -*-
/*
 *  File:        math_macro.C
 *  Purpose:     Implementation of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Mathed
 *
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.2, Mathed & Lyx project.
 *
 *  This code is under the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation "math_macro.h"
#pragma implementation "math_defs.h"
#endif

#include "LString.h"
#include "math_macro.h"
#include "math_iter.h"
#include "math_inset.h"
#include "support/lstrings.h"
#include "debug.h"


ostream & operator<<(ostream & o, MathedTextCodes mtc)
{
	return o << int(mtc);
}

enum MathedMacroFlag {
    MMF_Env= 1,
    MMF_Exp= 2,
    MMF_Edit= 4
};

ostream & operator<<(ostream & o, MathedMacroFlag mmf)
{
	return o << int(mmf);
}

extern int mathed_string_width(short type, int style, byte const* s, int ls);
extern int mathed_string_height(short, int, byte const*, int, int&, int&);


MathMacro::MathMacro(MathMacroTemplate* t): 
    MathParInset(LM_ST_TEXT, "", LM_OT_MACRO), tmplate(t)
{
    nargs = tmplate->getNoArgs();
    tcode = tmplate->getTCode();
    args = new MacroArgumentBase[nargs];
    for (int i = 0; i < nargs; ++i) {
//	if (tmplate->getMacroPar(i)->Permit(LMPF_ALLOW_CR))
//	  args[i].row = new MathedRowSt(tmplate->getMacroPar(i)->GetColumns());
//	else 
	  args[i].row = 0;
/*	int k = tmplate->getMacroPar(i)->GetColumns();
	if (k>0) {
	    args[i].array = new LyxArrayBase;
	    for (int j= 0; j<k-1; ++j) args[i].array->Insert(j, LM_TC_TAB);
	}*/
    }
    idx = 0;
    SetName(tmplate->GetName());
}


MathMacro::MathMacro(MathMacro * m): 
    MathParInset(LM_ST_TEXT, m->GetName(), LM_OT_MACRO)
{
    tmplate = m->tmplate;
    nargs = tmplate->getNoArgs();
    tcode = tmplate->getTCode();
    args = new MacroArgumentBase[nargs];
    idx = 0;
    SetName(tmplate->GetName());
    for (int i = 0; i < tmplate->nargs; ++i) {
	m->setArgumentIdx(i);
	MathedIter it(m->GetData());
	args[i].row = m->args[i].row;
	args[i].array = it.Copy();
    }
}

MathMacro::~MathMacro()
{
    for (idx = 0; idx < nargs; ++idx) {
	MathedIter it(args[idx].array);
	it. Clear();
	delete args[idx].row;
    }
    delete[] args;
}


MathedInset * MathMacro::Clone()
{
    return new MathMacro(this);
}


void MathMacro::Metrics()
{
    if (nargs > 0)
      tmplate->update(this);
    tmplate->Metrics();
    width = tmplate->Width();
    ascent = tmplate->Ascent();
    descent = tmplate->Descent();
}


void MathMacro::draw(Painter & pain, int x, int y)
{
    xo = x;  yo = y;
    Metrics();
    tmplate->update(this);
    tmplate->SetStyle(size);
    tmplate->draw(pain, x, y);
    for (int i = 0; i < nargs; ++i)
      tmplate->GetMacroXY(i, args[i].x, args[i].y);
}


int MathMacro::GetColumns()
{
    return tmplate->getMacroPar(idx)->GetColumns();
}


void MathMacro::GetXY(int & x, int & y) const
{
    x = args[idx].x;  y = args[idx].y;
}


bool MathMacro::Permit(short f)
{
    return (nargs > 0) ?
	    tmplate->getMacroPar(idx)->Permit(f) : MathParInset::Permit(f);
}


void MathMacro::SetFocus(int x, int y)
{
    tmplate->update(this);
    tmplate->SetMacroFocus(idx, x, y);
}


void MathMacro::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
    if (tmplate->flags & MMF_Exp) {
	    lyxerr[Debug::MATHED] << "Expand " << tmplate->flags
				  << ' ' << MMF_Exp << endl; 
	tmplate->update(this);
	tmplate->Write(os);
    } else {
	if (tmplate->flags & MMF_Env) {
		os << "\\begin{"
		   << name
		   << "} ";
	} else {
		os << '\\' << name;
	}
//	if (options) { 
//	  file += '[';
//	  file += options;
//	  file += ']';
//      }
	
	if (!(tmplate->flags & MMF_Env) && nargs > 0) 
		os << '{';
	
	for (int i = 0; i < nargs; ++i) {
	    array = args[i].array;
	    MathParInset::Write(os);
	    if (i < nargs - 1)  
		    os << "}{";
	}   
	if (tmplate->flags & MMF_Env) {
		os << "\\end{"
		   << name
		   << '}';
	} else {
	    if (nargs > 0) 
		    os << '}';
	    else
		    os << ' ';
	}
    }
#else
   string output;
   MathMacro::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathMacro::Write(string &file)
{
    if (tmplate->flags & MMF_Exp) {
	    lyxerr[Debug::MATHED] << "Expand " << tmplate->flags
				  << ' ' << MMF_Exp << endl; 
	tmplate->update(this);
	tmplate->Write(file);
    } else {
	if (tmplate->flags & MMF_Env) {
	  file += "\\begin{";
	  file += name;
	  file += "} ";
	} else {
	  file += '\\';
	  file += name;
	}
//	if (options) { 
//	  file += '[';
//	  file += options;
//	  file += ']';
//      }
	
	if (!(tmplate->flags & MMF_Env) && nargs > 0) 
	  file += '{';
	
	for (int i = 0; i < nargs; ++i) {
	    array = args[i].array;
	    MathParInset::Write(file);
	    if (i < nargs - 1)  
	      file += "}{";
	}   
	if (tmplate->flags & MMF_Env) {
	    file += "\\end{";
	    file += name;
	    file += '}';
	} else {
	    if (nargs > 0) 
	        file += '}';
	    else
	        file += ' ';
	}
    }
}
#endif


/*---------------  Macro argument -----------------------------------*/

MathMacroArgument::MathMacroArgument(int n)
{
    number = n;
    expnd_mode = false;
    SetType(LM_OT_MACRO_ARG);
}


void MathMacroArgument::draw(Painter & pain, int x, int baseline)
{
    if (expnd_mode) {
	MathParInset::draw(pain, x, baseline);
    } else {
#ifdef HAVE_SSTREAM
	    ostringstream ost;
	    ost << '#' << number;
	    drawStr(pain, LM_TC_TEX, size, x, baseline, ost.str().c_str(), 2);
#else
	    char s[3];
	    ostrstream ost(s, 3);
	    ost << '#' << number << '\0';
	    drawStr(pain, LM_TC_TEX, size, x, baseline,
		    reinterpret_cast<unsigned char*>(ost.str()), 2);
#endif
    }
}


void MathMacroArgument::Metrics()
{
    if (expnd_mode) {
	MathParInset::Metrics();
    } else {
#ifdef HAVE_SSTREAM
	    ostringstream ost;
	    ost << '#' << number;
	    width = mathed_string_width(LM_TC_TEX, size, ost.str().c_str(), 2);
	    mathed_string_height(LM_TC_TEX, size,
				 ost.str().c_str(), 2, ascent, descent);
#else
	char s[3];
	ostrstream ost(s, 3);
	ost << '#' << number << '\0';
	width = mathed_string_width(LM_TC_TEX, size,
				    reinterpret_cast<unsigned char*>
				    (ost.str()), 2);
	mathed_string_height(LM_TC_TEX, size,
			     reinterpret_cast<unsigned char*>(ost.str()),
			     2, ascent, descent);
#endif
    }
}


void MathMacroArgument::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
    if (expnd_mode) {
	MathParInset::Write(os);
    } else {
	    os << '#' << number << ' ';
    }
#else
   string output;
   MathMacroArgument::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathMacroArgument::Write(string & file)
{
    if (expnd_mode) {
	MathParInset::Write(file);
    } else {
	file += '#';
	file += tostr(number);
	file += ' ';
    }
}
#endif


/* --------------------- MathMacroTemplate ---------------------------*/

MathMacroTemplate::MathMacroTemplate(char const * nm, int na, int flg):
    MathParInset(LM_ST_TEXT, nm, LM_OT_MACRO), 
    flags(flg), nargs(na)
{
    if (nargs > 0) {
	tcode = LM_TC_ACTIVE_INSET;
	args = new MathMacroArgument[nargs];
	for (int i = 0; i < nargs; ++i) {
	    args[i].setNumber(i + 1);
	}
    } else 
      tcode = LM_TC_INSET;
}


MathMacroTemplate::~MathMacroTemplate()
{
    // prevent to delete already deleted objects
    for (int i = 0; i < nargs; ++i) {
	args[i].SetData(0);
    }
    delete[] args;
}


void MathMacroTemplate::setEditMode(bool ed)
{
    if (ed) {
	flags |= MMF_Edit;
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(false);
	}
    } else {
	flags &= ~MMF_Edit;
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(true);
	}
    }
}


void MathMacroTemplate::draw(Painter & pain, int x, int y)
{
    int x2, y2;
    bool expnd = (nargs > 0) ? args[0].getExpand(): false;
    if (flags & MMF_Edit) {
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(false);
	}
      x2 = x; y2 = y;
    } else {
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(true);
	}
      x2 = xo; y2 = yo;
    }
    MathParInset::draw(pain, x, y);
    xo = x2; yo = y2;
    
    for (int i = 0; i < nargs; ++i) {
	args[i].setExpand(expnd);
    }
}


void MathMacroTemplate::Metrics()
{
    bool expnd = (nargs>0) ? args[0].getExpand(): false;
    
    if (flags & MMF_Edit) {
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(false);
	}
    } else {
	for (int i = 0; i < nargs; ++i) {
	    args[i].setExpand(true);
	}
    }
    MathParInset::Metrics();
    
    for (int i = 0; i < nargs; ++i) {
	args[i].setExpand(expnd);
    }
}


void MathMacroTemplate::update(MathMacro * macro)
{
    int idx = (macro) ? macro->getArgumentIdx(): 0;
    for (int i = 0; i < nargs; ++i) {
	if (macro) {
	    macro->setArgumentIdx(i);
	    args[i].SetData(macro->GetData());
	    MathedRowSt *row = macro->getRowSt();
	    args[i].setRowSt(row);
	}
    }	
    if (macro)
      macro->setArgumentIdx(idx);
}
    

void MathMacroTemplate::WriteDef(ostream & os)
{
	os << "\n\\newcommand{\\" << name << "}";
      
    if (nargs > 0 ) 
	    os << "[" << nargs << "]";
    
    os << "{";
    
    for (int i = 0; i < nargs; ++i) {
	args[i].setExpand(false);
    }	 
    Write(os); 
    os << "}\n";
}


#ifndef USE_OSTREAM_ONLY
void MathMacroTemplate::WriteDef(string & file)
{
    file += "\n\\newcommand{\\";
    file += name;
    file += '}';
      
    if (nargs > 0 ) {
      file += '[';
      file += tostr(nargs);
      file += ']';
    }
    
    file += '{';
    
    for (int i = 0; i < nargs; ++i) {
	args[i].setExpand(false);
    }	 
    Write(file); 
    file += "}\n";
}
#endif


void MathMacroTemplate::setArgument(LyxArrayBase * a, int i)
{
    args[i].SetData(a);
}


void MathMacroTemplate::GetMacroXY(int i, int & x, int & y) const
{
    args[i].GetXY(x, y);
}


MathParInset * MathMacroTemplate::getMacroPar(int i) const
{
    return (i >= 0 && i < nargs) ? static_cast<MathParInset*>(&args[i]) : 0;
}


void MathMacroTemplate::SetMacroFocus(int &idx, int x, int y)
{
    for (int i = 0; i < nargs; ++i) {
	if (args[i].Inside(x, y)) {
	    idx = i;
	    break;
	}
    }
}


/* -------------------------- MathMacroTable -----------------------*/

MathMacroTable::MathMacroTable(int n) : max_macros(n)
{
    macro_table = new MathMacroTemplateP[max_macros];
    num_macros = 0;
}


MathMacroTable::~MathMacroTable()
{
    delete[] macro_table;
}


// The search is currently linear but will be binary or hash, later.
MathMacroTemplate * MathMacroTable::getTemplate(char const * name) const
{
    for (int i = 0; i < num_macros; ++i) {
      if (strcmp(name, macro_table[i]->GetName()) == 0) 
	return macro_table[i];
    }
    
    return 0;
}

void MathMacroTable::addTemplate(MathMacroTemplate * m)
{
    if (num_macros < max_macros)
      macro_table[num_macros++] = m;
    else
	    lyxerr << "Error (MathMacroTable::addTemplate): "
		    "Macro table exhausted!" << endl;
}


// All this stuff aparently leaks because it's created here and is not 
// deleted never, but it have to live all the LyX sesion. OK, would not
// so hard to do it in the MacroTable destructor, but this doesn't harm
// seriously, so don't bother me with purify results here.   ;-)

void MathMacroTable::builtinMacros()
{
    MathedIter iter;
    MathParInset * inset;// *arg;
    LyxArrayBase * array2;
    
    built = true;
    
    lyxerr[Debug::MATHED] << "Building macros" << endl;
    
    // This macro doesn't have arguments
    MathMacroTemplate * m = new MathMacroTemplate("notin");  // this leaks
    addTemplate(m);
    LyxArrayBase * array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    iter.Insert(new MathAccentInset(LM_in, LM_TC_BOPS, LM_not)); // this leaks
    m->SetData(array);
    
    // These two are only while we are still with LyX 2.x
    m = new MathMacroTemplate("emptyset"); // this leaks
    addTemplate(m);
    array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    iter.Insert(new MathAccentInset('O', LM_TC_RM, LM_not)); // this leaks
    m->SetData(array);
    
    m = new MathMacroTemplate("perp"); // this leaks
    addTemplate(m);
    array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    iter.Insert(LM_bot, LM_TC_BOP);
    m->SetData(array);

    // binom has two arguments
    m = new MathMacroTemplate("binom", 2);
    addTemplate(m);
    array = new LyxArrayBase; 
    m->SetData(array);
    iter.SetData(array);
    inset = new MathDelimInset('(', ')');
    iter.Insert(inset, LM_TC_ACTIVE_INSET);
    array = new LyxArrayBase; 
    iter.SetData(array);
    MathFracInset *frac = new MathFracInset(LM_OT_ATOP);
    iter.Insert(frac, LM_TC_ACTIVE_INSET);
    inset->SetData(array);
    array = new LyxArrayBase;
    array2 = new LyxArrayBase;  
    iter.SetData(array);
    iter.Insert(m->getMacroPar(0));
    iter.SetData(array2);
    iter.Insert(m->getMacroPar(1));
    frac->SetData(array, array2);

/*
    // Cases has 1 argument
    m = new MathMacroTemplate("cases", 1, MMF_Env); // this leaks
    addTemplate(m);
    array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    arg = new MathMatrixInset(2, 1); // this leaks

    m->setArgument(arg);
    arg->SetAlign('c', "ll");
    iter.Insert(arg, LM_TC_ACTIVE_INSET);
    inset = new MathDelimInset('{', '.'); // this leaks
    inset->SetData(array);
    array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    iter.Insert(inset, LM_TC_ACTIVE_INSET);
    m->SetData(array);
  

    // the environment substack has 1 argument
    m = new MathMacroTemplate("substack", 1, MMF_Env); // this leaks
    addTemplate(m);     
    arg = new MathMatrixInset(1, 1); // this leaks
    m->setArgument(arg);
    arg->SetType(LM_OT_MACRO);
    array = new LyxArrayBase; // this leaks
    iter.SetData(array);
    iter.Insert(arg, LM_TC_ACTIVE_INSET);
    m->SetData(array);*/
}


MathMacroTable MathMacroTable::mathMTable(255);
bool MathMacroTable::built = false;
