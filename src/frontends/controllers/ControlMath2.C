/**
 * \file ControlMath2.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlMath2.h"
#include "Kernel.h"
#include "funcrequest.h"


ControlMath2::ControlMath2(Dialog & dialog)
	: Dialog::Controller(dialog)
{}


void ControlMath2::dispatchFunc(kb_action action, string const & arg) const
{
	kernel().dispatch(FuncRequest(action, arg));
}


void ControlMath2::dispatchInsert(string const & name) const
{
	dispatchFunc(LFUN_INSERT_MATH, '\\' + name);
}


void ControlMath2::dispatchSubscript() const
{
	dispatchFunc(LFUN_SUBSCRIPT);
}


void ControlMath2::dispatchSuperscript() const
{
	dispatchFunc(LFUN_SUPERSCRIPT);
}


void ControlMath2::dispatchCubeRoot() const
{
	dispatchFunc(LFUN_INSERT_MATH, "\\root");
	dispatchFunc(LFUN_SELFINSERT, "3");
	dispatchFunc(LFUN_RIGHT);
}


void ControlMath2::dispatchMatrix(string const & str) const
{
	dispatchFunc(LFUN_INSERT_MATRIX, str);
}


void ControlMath2::dispatchDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_DELIM, str);
}


void ControlMath2::dispatchToggleDisplay() const
{
	dispatchFunc(LFUN_MATH_DISPLAY);
}


void ControlMath2::showDialog(string const & name) const
{
	dispatchFunc(LFUN_DIALOG_SHOW, name);
}
