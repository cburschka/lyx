#include <config.h>

#include "math_macrotemplate.h"
#include "math_macro.h"
#include "macro_support.h"


void  MathMacroTemplate::setTCode(MathedTextCodes t)
{
	tcode = t;
}


MathedTextCodes MathMacroTemplate::getTCode() const
{
	return tcode;
}


int MathMacroTemplate::getNoArgs() const
{
	return nargs;
}


MathMacroTemplate::MathMacroTemplate(string const & nm, int na, int flg):
	MathParInset(LM_ST_TEXT, nm, LM_OT_MACRO), 
	flags(flg), nargs(na)
{
	if (nargs > 0) {
		tcode = LM_TC_ACTIVE_INSET;
		args_.resize(nargs);
		for (int i = 0; i < nargs; ++i) {
			args_[i].setNumber(i + 1);
		}
	} else {
		tcode = LM_TC_INSET;
		// Here is  nargs != args_.size()
		//args = 0;
	}
}


MathMacroTemplate::~MathMacroTemplate()
{
	// prevent to delete already deleted objects
	for (int i = 0; i < nargs; ++i) {
		args_[i].SetData(0);
	}
}


void MathMacroTemplate::setEditMode(bool ed)
{
	if (ed) {
		flags |= MMF_Edit;
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(false);
		}
	}
	else {
		flags &= ~MMF_Edit;
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(true);
		}
	}
}


void MathMacroTemplate::draw(Painter & pain, int x, int y)
{
	int x2, y2;
	bool expnd = (nargs > 0) ? args_[0].getExpand(): false;
	if (flags & MMF_Edit) {
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(false);
		}
		x2 = x; y2 = y;
	} else {
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(true);
		}
		x2 = xo; y2 = yo;
	}
	MathParInset::draw(pain, x, y);
	xo = x2; yo = y2;

	for (int i = 0; i < nargs; ++i) {
		args_[i].setExpand(expnd);
	}
}


void MathMacroTemplate::Metrics()
{
	bool expnd = (nargs > 0) ? args_[0].getExpand(): false;
    
	if (flags & MMF_Edit) {
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(false);
		}
	} else {
		for (int i = 0; i < nargs; ++i) {
			args_[i].setExpand(true);
		}
	}
	MathParInset::Metrics();
    
	for (int i = 0; i < nargs; ++i) {
		args_[i].setExpand(expnd);
	}
}


void MathMacroTemplate::update(MathMacro * macro)
{
	int idx = (macro) ? macro->getArgumentIdx() : 0;
	for (int i = 0; i < nargs; ++i) {
		if (macro) {
			macro->setArgumentIdx(i);
			args_[i].SetData(macro->GetData());
			MathedRowSt * row = macro->getRowSt();
			args_[i].setRowSt(row);
		}
	}	
	if (macro)
		macro->setArgumentIdx(idx);
}


void MathMacroTemplate::WriteDef(ostream & os, bool fragile)
{
	os << "\n\\newcommand{\\" << name << "}";

	if (nargs > 0 ) 
		os << "[" << nargs << "]";

	os << "{";

	for (int i = 0; i < nargs; ++i) {
		args_[i].setExpand(false);
	}	 
	Write(os, fragile); 
	os << "}\n";
}


void MathMacroTemplate::setArgument(MathedArray * a, int i)
{
	args_[i].SetData(a);
}


void MathMacroTemplate::GetMacroXY(int i, int & x, int & y) const
{
	args_[i].GetXY(x, y);
}


MathParInset * MathMacroTemplate::getMacroPar(int i) const
{
	if (i >= 0 && i < nargs) 
		return const_cast<MathParInset *>
		        (static_cast<MathParInset const *>(&args_[i]));
	else 
		return 0;
}


void MathMacroTemplate::SetMacroFocus(int &idx, int x, int y)
{
	for (int i = 0; i < nargs; ++i) {
		if (args_[i].Inside(x, y)) {
			idx = i;
			break;
		}
	}
}
