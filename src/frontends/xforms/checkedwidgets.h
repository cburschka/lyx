// -*- C++ -*-
/**
 * \file checkedwidgets.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CHECKEDWIDGETS_H
#define CHECKEDWIDGETS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonControllerBase.h"
#include "forms_fwd.h"

void addCheckedLyXLength(ButtonControllerBase & bc,
			 FL_OBJECT * input, FL_OBJECT * label = 0);

void addCheckedGlueLength(ButtonControllerBase & bc,
			  FL_OBJECT * input, FL_OBJECT * label = 0);

class CheckedLyXLength : public CheckedWidget {
public:
	/** The label widget's label will be turned red if input
	 *  does not make a valid LyXLength.
	 *  If label == 0, then the label of input will be used.
	 */
	CheckedLyXLength(FL_OBJECT * input, FL_OBJECT * label = 0);

private:
	///
	virtual bool check() const;

	///
	FL_OBJECT * input_;
	FL_OBJECT * label_;
};


class CheckedGlueLength : public CheckedWidget {
public:
	/** The label widget's label will be turned red if input
	 *  does not make a valid LyXGlueLength.
	 *  If label == 0, then the label of input will be used.
	 */
	CheckedGlueLength(FL_OBJECT * input, FL_OBJECT * label = 0);

private:
	///
	virtual bool check() const;

	///
	FL_OBJECT * input_;
	FL_OBJECT * label_;
};

#endif // CHECKEDWIDGETS_H
