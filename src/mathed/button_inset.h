// -*- C++ -*-

/** 
 *  \file button_inset.h
 *
 *  This file is part of LyX, the document processor.
 *  Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS.
 */

#ifndef BUTTON_INSET_H
#define BUTTON_INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_nestinset.h"

/// try to implement the button-like insets "natively" for mathed
class ButtonInset: public MathNestInset {
public:
	///
	ButtonInset();
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;

protected:
	/// This should provide the text for the button
	virtual string screenLabel() const = 0;
};

#endif
