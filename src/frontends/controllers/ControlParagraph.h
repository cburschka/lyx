// -*- C++ -*-
/**
 * \file ControlParagraph.h
 * Copyright 2002 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef CONTROLPARAGRAPH_H
#define CONTROLPARAGRAPH_H

#include <boost/scoped_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "layout.h" // for LyXAlignment

class ParagraphParameters;

/** A controller for Paragraph dialogs.
 */
class ControlParagraph : public ControlDialogBD
{
public:
	///
	ControlParagraph(LyXView &, Dialogs &);
	/** Declaring this out of line allows us to forward declare
	    ParagraphParameters without upsetting boost::scoped_ptr. */
	~ControlParagraph();

	///
	ParagraphParameters & params();
	///
	bool inInset() const;
	///
	LyXAlignment alignPossible() const;

private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();

	///
	boost::scoped_ptr<ParagraphParameters> pp_;
	///
	bool ininset_;
	///
	LyXAlignment alignpos_;
};

#endif // CONTROLPARAGRAPH_H

