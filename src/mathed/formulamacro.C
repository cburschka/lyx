/*
 *  File:        formula.h
 *  Purpose:     Implementation of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "formulamacro.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "Painter.h"
#include "font.h"
#include "support/lyxlib.h"
#include "mathed/support.h"
#include "support/LOstream.h"
#include "debug.h"

using std::ostream;
using std::istream;

InsetFormulaMacro::InsetFormulaMacro()
	: InsetFormula(true)
{
	tmacro_ = 0;
	opened_ = false;
}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
        : InsetFormula(true), name_(nm)
{
	tmacro_ = MathMacroTable::mathMTable.getTemplate(name_);
	if (!tmacro_) {
		tmacro_ = new MathMacroTemplate(name_, na);
		MathMacroTable::mathMTable.addTemplate(tmacro_);
	}
	opened_ = false;
}


InsetFormulaMacro::~InsetFormulaMacro()
{
	par = 0;
}


Inset * InsetFormulaMacro::Clone(Buffer const &) const
{
	return new InsetFormulaMacro(name_);
}


void InsetFormulaMacro::Write(Buffer const *, ostream & os) const
{
	os << "FormulaMacro ";
	tmacro_->WriteDef(os, false);
}


int InsetFormulaMacro::Latex(Buffer const *, ostream & os, bool /*fragile*/, 
			     bool /*free_spacing*/) const
{
	tmacro_->WriteDef(os, true); // or false?
	return 2;
}


int InsetFormulaMacro::Linuxdoc(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


int InsetFormulaMacro::DocBook(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


void InsetFormulaMacro::Read(Buffer const *, LyXLex & lex)
{
	istream & is = lex.getStream();
	mathed_parser_file(is, lex.GetLineNo());
	MathedArray ar;
	
	mathed_parse(ar, 0, reinterpret_cast<MathParInset **>(&tmacro_));
	// since tmacro_ == 0 when mathed_parse is called we need to sett
	// its contents explicitly afterwards (Lgb)
	tmacro_->setData(ar);
	
	// Update line number
	lex.setLineNo(mathed_parser_lineno());
	
	MathMacroTable::mathMTable.addTemplate(tmacro_);
	name_ = tmacro_->GetName();
	par = tmacro_;

	// reading of end_inset in the inset!!!
	while (lex.IsOK()) {
		lex.nextToken();
		if (lex.GetString() == "\\end_inset")
			break;
	}
}


int InsetFormulaMacro::ascent(BufferView * pain, LyXFont const & f) const
{
	if (opened_) {
		return InsetFormula::ascent(pain, f);
	}
	return lyxfont::maxAscent(f) + 3;
}


int InsetFormulaMacro::descent(BufferView * pain, LyXFont const & f) const
{
	if (opened_) {
		return InsetFormula::descent(pain, f);
	}
	return lyxfont::maxDescent(f) + 1;
}


int InsetFormulaMacro::width(BufferView * bv, LyXFont const & f) const
{
	if (opened_) {
		return InsetFormula::width(bv, f);
	}
	string ilabel(_("Macro: "));
	ilabel += name_;
	return 6 + lyxfont::width(ilabel, f);
}


void InsetFormulaMacro::draw(BufferView * bv, LyXFont const & f,
			     int baseline, float & x, bool cleared) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);
	if (opened_) {
		tmacro_->setEditMode(true);
		InsetFormula::draw(bv, font, baseline, x, cleared);
		tmacro_->setEditMode(false);	
	} else {
		font.setColor(LColor::math);
		
		int const y = baseline - ascent(bv, font) + 1;
		int const w = width(bv, font) - 2;
		int const h = (ascent(bv, font) + descent(bv, font) - 2);
	
		pain.fillRectangle(int(x), y, w, h, LColor::mathbg);
		pain.rectangle(int(x), y, w, h, LColor::mathframe);
		
		string s(_("Macro: "));
		s += name_;
		pain.text(int(x + 2), baseline, s, font);
		x +=  width(bv, font) - 1;
	}
}


string const InsetFormulaMacro::EditMessage() const 
{
	return _("Math macro editor mode");
}


void InsetFormulaMacro::Edit(BufferView * bv, int x, int y,unsigned int button)
{
	opened_ = true;
	par = static_cast<MathParInset*>(tmacro_->Clone());
	InsetFormula::Edit(bv, x, y, button);
}

	       
void InsetFormulaMacro::InsetUnlock(BufferView * bv)
{
	opened_ = false;
	tmacro_->setData(par->GetData());
	tmacro_->setEditMode(false);
	InsetFormula::InsetUnlock(bv);
}


UpdatableInset::RESULT
InsetFormulaMacro::LocalDispatch(BufferView * bv,
				 kb_action action, string const & arg)
{
	if (action == LFUN_MATH_MACROARG) {
		int const i = lyx::atoi(arg) - 1;
		if (i >= 0 && i < tmacro_->getNoArgs()) {
			mathcursor->insertInset(tmacro_->getMacroPar(i),
						 LM_TC_INSET);
			InsetFormula::UpdateLocal(bv);
		}
	
		return DISPATCHED;
	}
	tmacro_->setEditMode(true);
	tmacro_->Metrics();
	RESULT result = InsetFormula::LocalDispatch(bv, action, arg);
	tmacro_->setEditMode(false);
    
	return result;
}
