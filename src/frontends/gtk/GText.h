// -*- C++ -*-
/**
 * \file GText.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GTEXT_H
#define GTEXT_H

#include "GViewBase.h"
#include "LString.h"

class ControlCommand;

class GText : public GViewCB<ControlCommand, GViewGladeB>
{
public:
	GText(Dialog & parent, string const & title, string const & label);
private:
	virtual void apply();
	virtual void update();
	virtual void doBuild();
	void onEntryChanged();
	string const label_;
	Gtk::Entry * entry_;
};

#endif
