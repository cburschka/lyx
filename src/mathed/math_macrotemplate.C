#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotemplate.h"
#include "math_macro.h"
#include "macro_support.h"
#include "support/LOstream.h"
#include "support/LAssert.h"

using std::ostream;


MathMacroTemplate::MathMacroTemplate(string const & nm, int na, int flg):
	MathParInset(LM_ST_TEXT, nm, LM_OT_MACRO), 
	flags_(flg), nargs_(na)
{
	if (nargs_ > 0) {
		tcode_ = LM_TC_ACTIVE_INSET;
		args_.resize(nargs_);
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setNumber(i + 1);
		}
	} else {
		tcode_ = LM_TC_INSET;
		// Here is  nargs != args_.size()
		//args = 0;
	}
}


void  MathMacroTemplate::setTCode(MathedTextCodes t)
{
	tcode_ = t;
}


MathedTextCodes MathMacroTemplate::getTCode() const
{
	return tcode_;
}


int MathMacroTemplate::getNoArgs() const
{
	return nargs_;
}


void MathMacroTemplate::setEditMode(bool ed)
{
	if (ed) {
		flags_ |= MMF_Edit;
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(false);
		}
	} else {
		flags_ &= ~MMF_Edit;
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(true);
		}
	}
}


void MathMacroTemplate::draw(Painter & pain, int x, int y)
{
	int x2;
	int y2;
	bool expnd = (nargs_ > 0) ? args_[0].getExpand() : false;
	if (flags_ & MMF_Edit) {
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(false);
		}
		x2 = x;
		y2 = y;
	} else {
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(true);
		}
		x2 = xo();
		y2 = yo();
	}
	MathParInset::draw(pain, x, y);
	xo(x2);
	yo(y2);

	for (int i = 0; i < nargs_; ++i) {
		args_[i].setExpand(expnd);
	}
}


void MathMacroTemplate::Metrics()
{
	bool expnd = (nargs_ > 0) ? args_[0].getExpand() : false;
    
	if (flags_ & MMF_Edit) {
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(false);
		}
	} else {
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(true);
		}
	}
	MathParInset::Metrics();
    
	for (int i = 0; i < nargs_; ++i) {
		args_[i].setExpand(expnd);
	}
}


void MathMacroTemplate::update(MathMacro * macro)
{
	Assert(macro);
	int idx = macro->getArgumentIdx();
	for (int i = 0; i < nargs_; ++i) {
			macro->setArgumentIdx(i);
			args_[i].setData(macro->GetData());
			MathedRowSt * row = macro->getRowSt();
			args_[i].setRowSt(row);
	}	
	macro->setArgumentIdx(idx);
}


void MathMacroTemplate::WriteDef(ostream & os, bool fragile)
{
	os << "\n\\newcommand{\\" << name << "}";

	if (nargs_ > 0 ) 
		os << "[" << nargs_ << "]";

	os << "{";

	for (int i = 0; i < nargs_; ++i) {
		args_[i].setExpand(false);
	}	 
	Write(os, fragile);
	os << "}\n";
}


void MathMacroTemplate::setArgument(MathedArray * a, int i)
{
	args_[i].setData(*a);
}


void MathMacroTemplate::GetMacroXY(int i, int & x, int & y) const
{
	args_[i].GetXY(x, y);
}


MathParInset * MathMacroTemplate::getMacroPar(int i) const
{
	if (i >= 0 && i < nargs_) 
		return const_cast<MathParInset *>
		        (static_cast<MathParInset const *>(&args_[i]));
	else 
		return 0;
}


void MathMacroTemplate::SetMacroFocus(int &idx, int x, int y)
{
	for (int i = 0; i < nargs_; ++i) {
		if (args_[i].Inside(x, y)) {
			idx = i;
			break;
		}
	}
}
