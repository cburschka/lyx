/**
 * \file QMath.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
 
#include "commandtags.h"
#include "funcrequest.h"
#include "LyXView.h"
#include "BufferView.h"
 
#include "QMathDialog.h"
#include "QMath.h"

#include "iconpalette.h"

// FIXME temporary HACK ! 
void createMathPanel()
{
	static QMath * dialog = 0; 
	if (!dialog) {
		dialog = new QMath();
		dialog->build_dialog();
	}
	dialog->do_show();
}
 
 
QMath::QMath()
{
}


void QMath::do_show()
{
	dialog_->show();
}
 
 
void QMath::build_dialog()
{
	dialog_ = new QMathDialog(this);
}

 
// needless to say, this can't last for long
extern BufferView * current_view;

void QMath::subscript()
{
	current_view->owner()->dispatch(FuncRequest(LFUN_SUBSCRIPT));
}


void QMath::superscript()
{
	current_view->owner()->dispatch(FuncRequest(LFUN_SUPERSCRIPT));
}


void QMath::insert(string const & name)
{
	current_view->owner()->dispatch(FuncRequest(LFUN_INSERT_MATH, '\\' + name));
}
 

void QMath::insertCubeRoot()
{
	current_view->owner()->dispatch(FuncRequest(LFUN_INSERT_MATH, "\\root"));
	current_view->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "3"));
	current_view->owner()->dispatch(FuncRequest(LFUN_RIGHT));
}


void QMath::insertMatrix()
{
	current_view->owner()->dispatch(FuncRequest(LFUN_INSERT_MATRIX, "2 2"));
}

 
void QMath::toggleDisplay()
{
	current_view->owner()->dispatch(FuncRequest(LFUN_MATH_DISPLAY));
}
