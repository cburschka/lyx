/*
 *  File:        formula.h
 *  Purpose:     Implementation of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation "formulamacro.h"
#endif

#include "formulamacro.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_macro.h"
#include "lyx_main.h"
#include "bufferlist.h"
#include "lyx_cb.h"
#include "BufferView.h"
#include "lyxscreen.h"
#include "lyxdraw.h"
#include "gettext.h"


InsetFormulaMacro::InsetFormulaMacro()
       : InsetFormula(true)
{
    tmacro = 0;
    opened = false;
}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na, bool /*e*/)
        : InsetFormula(true), name(nm)
{
    tmacro = MathMacroTable::mathMTable.getTemplate(name.c_str());
    if (!tmacro) {
	tmacro = new MathMacroTemplate(name.c_str(), na);
	MathMacroTable::mathMTable.addTemplate(tmacro);
    }
    opened = false;
}


InsetFormulaMacro::~InsetFormulaMacro()
{
    par = 0;
}


Inset * InsetFormulaMacro::Clone() const
{
   return new InsetFormulaMacro(name);
}


void InsetFormulaMacro::Write(ostream & os)
{
	os << "FormulaMacro ";
	Latex(os, 0);
}


int InsetFormulaMacro::Latex(ostream & os, signed char /*fragile*/)
{
    int ret = 1;
    tmacro->WriteDef(os);
    return ret;
}


int InsetFormulaMacro::Latex(string &file, signed char /*fragile*/)
{
    int ret = 1;
    tmacro->WriteDef(file);
    return ret;
}


int InsetFormulaMacro::Linuxdoc(string &/*file*/)
{
    return 0;
}


int InsetFormulaMacro::DocBook(string &/*file*/)
{
    return 0;
}


void InsetFormulaMacro::Read(LyXLex & lex)
{
    FILE * file = lex.getFile();
    mathed_parser_file(file, lex.GetLineNo());   
    mathed_parse(0, 0, (MathParInset **)&tmacro);
    
    // Update line number
    lex.setLineNo(mathed_parser_lineno());
    
    MathMacroTable::mathMTable.addTemplate(tmacro);
    name = tmacro->GetName();
    par = tmacro;
}


int InsetFormulaMacro::Ascent(LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::Ascent(f);
    }
    return f.maxAscent()+3;
}


int InsetFormulaMacro::Descent(LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::Descent(f);
    }
    return f.maxDescent()+1;
}


int InsetFormulaMacro::Width(LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::Width(f);
    }
    string ilabel(_("Macro: "));
    ilabel += name;
    return 6 + f.stringWidth(ilabel);
}


void InsetFormulaMacro::Draw(LyXFont font, LyXScreen & scr,
			     int baseline, float & x)
{
    tmacro->update();
    if (opened) {
	tmacro->setEditMode(true);
	InsetFormula::Draw(font, scr, baseline, x);
	tmacro->setEditMode(false);	
    } else {
	font.setColor(LyXFont::MATH);
	
	int y = baseline - Ascent(font) + 1;
	int w = Width(font) - 2, h = (Ascent(font) + Descent(font) - 2);

	
	scr.fillRectangle(gc_lighted, int(x), y,  w,  h);
	scr.drawFrame(FL_UP_FRAME, int(x), y, w, h, FL_BLACK, -1); 
	
        string s(_("Macro: "));
        s += name;
        scr.drawString(font, s, baseline, int(x +2));
	x +=  Width(font) - 1;
    }
}


void InsetFormulaMacro::Edit(int x, int y)
{
    opened = true;
    par = static_cast<MathParInset*>(tmacro->Clone());
    InsetFormula::Edit(x, y);
}

	       
void InsetFormulaMacro::InsetUnlock()
{
    opened = false;
    LyxArrayBase * tarray = tmacro->GetData();
    MathedIter it(tarray);
    it.Clear();
    tmacro->SetData(par->GetData());
    tmacro->setEditMode(false);
    InsetFormula::InsetUnlock();
}


bool InsetFormulaMacro::LocalDispatch(int action, char const * arg)
{
    if (action == LFUN_MATH_MACROARG) {
	int i = atoi(arg) - 1;
	if (i >= 0 && i < tmacro->getNoArgs()) {
	    mathcursor->Insert(tmacro->getMacroPar(i), LM_TC_INSET);
	    InsetFormula::UpdateLocal();
	}
	
	return true;
    }
    tmacro->setEditMode(true);
    tmacro->Metrics();
    bool result = InsetFormula::LocalDispatch(action, arg);
    tmacro->setEditMode(false);
    
    return result;
}
