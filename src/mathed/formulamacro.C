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
#include "BufferView.h"
#include "gettext.h"
#include "Painter.h"


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


void InsetFormulaMacro::Write(ostream & os) const
{
	os << "FormulaMacro ";
	Latex(os, 0);
}


int InsetFormulaMacro::Latex(ostream & os, signed char /*fragile*/) const
{
    int ret = 1;
    tmacro->WriteDef(os);
    return ret;
}


int InsetFormulaMacro::Latex(string &file, signed char /*fragile*/) const
{
    int ret = 1;
    tmacro->WriteDef(file);
    return ret;
}


int InsetFormulaMacro::Linuxdoc(string &/*file*/) const
{
    return 0;
}


int InsetFormulaMacro::DocBook(string &/*file*/) const
{
    return 0;
}


void InsetFormulaMacro::Read(LyXLex & lex)
{
	istream & is = lex.getStream();
	mathed_parser_file(is, lex.GetLineNo());   
	mathed_parse(0, 0, reinterpret_cast<MathParInset **>(&tmacro));
    
	// Update line number
	lex.setLineNo(mathed_parser_lineno());
	
	MathMacroTable::mathMTable.addTemplate(tmacro);
	name = tmacro->GetName();
	par = tmacro;
}


int InsetFormulaMacro::ascent(Painter & pain, LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::ascent(pain, f);
    }
    return f.maxAscent()+3;
}


int InsetFormulaMacro::descent(Painter & pain, LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::descent(pain, f);
    }
    return f.maxDescent()+1;
}


int InsetFormulaMacro::width(Painter & pain, LyXFont const & f) const
{
    if (opened) {
	tmacro->update();
	return InsetFormula::width(pain, f);
    }
    string ilabel(_("Macro: "));
    ilabel += name;
    return 6 + f.stringWidth(ilabel);
}


void InsetFormulaMacro::draw(Painter & pain, LyXFont const & f,
			     int baseline, float & x) const
{
	LyXFont font(f);
	tmacro->update();
	if (opened) {
		tmacro->setEditMode(true);
		InsetFormula::draw(pain, font, baseline, x);
		tmacro->setEditMode(false);	
	} else {
		font.setColor(LColor::math);
		
		int y = baseline - ascent(pain, font) + 1;
		int w = width(pain, font) - 2;
		int h = (ascent(pain, font) + descent(pain, font) - 2);

	
		pain.fillRectangle(int(x), y, w, h, LColor::mathbg);
		pain.rectangle(int(x), y, w, h, LColor::mathframe);
		
		string s(_("Macro: "));
		s += name;
		pain.text(int(x + 2), baseline, s, font);
		x +=  width(pain, font) - 1;
	}
}


void InsetFormulaMacro::Edit(BufferView * bv, int x, int y)
{
    opened = true;
    par = static_cast<MathParInset*>(tmacro->Clone());
    InsetFormula::Edit(bv, x, y);
}

	       
void InsetFormulaMacro::InsetUnlock(BufferView * bv)
{
    opened = false;
    LyxArrayBase * tarray = tmacro->GetData();
    MathedIter it(tarray);
    it.Clear();
    tmacro->SetData(par->GetData());
    tmacro->setEditMode(false);
    InsetFormula::InsetUnlock(bv);
}


bool InsetFormulaMacro::LocalDispatch(BufferView * bv,
				      int action, char const * arg)
{
    if (action == LFUN_MATH_MACROARG) {
	int i = atoi(arg) - 1;
	if (i >= 0 && i < tmacro->getNoArgs()) {
	    mathcursor->Insert(tmacro->getMacroPar(i), LM_TC_INSET);
	    InsetFormula::UpdateLocal(bv);
	}
	
	return true;
    }
    tmacro->setEditMode(true);
    tmacro->Metrics();
    bool result = InsetFormula::LocalDispatch(bv, action, arg);
    tmacro->setEditMode(false);
    
    return result;
}
