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


MathMacroTemplate::MathMacroTemplate(string const & nm, int na):
	MathParInset(LM_ST_TEXT, nm, LM_OT_MACRO), 
	edit_(false),
	nargs_(na)
{
	if (nargs_ > 0) {
		tcode_ = LM_TC_ACTIVE_INSET;
		for (int i = 0; i < nargs_; ++i) {
			args_.push_back(MathMacroArgument(i + 1));
		}
		//for (int i = 0; i < nargs_; ++i) {
		//	MathMacroArgument * ma = new MathMacroArgument(i + 1);
		//	args_.push_back(boost::shared_ptr<MathMacroArgument>(ma));
		//}
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
		edit_ = true;
		for (int i = 0; i < nargs_; ++i) {
			args_[i].setExpand(false);
		}
	} else {
		edit_ = false;
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
	if (edit_) {
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
	bool const expnd = (nargs_ > 0) ? args_[0].getExpand() : false;
    
	if (edit_) {
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


void MathMacroTemplate::update(MathMacro const & macro)
{
	for (int i = 0; i < nargs_; ++i) {
		args_[i] = macro.getArg(i);
	}
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


void MathMacroTemplate::GetMacroXY(int i, int & x, int & y) const
{
	args_[i].GetXY(x, y);
}


MathParInset * MathMacroTemplate::getMacroPar(int i) const
{
	if (i >= 0 && i < nargs_) {
		MathParInset * p = const_cast<MathParInset *>
		        (static_cast<MathParInset const *>(&args_[i]));
		Assert(p);
		return p;
	} else 
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
