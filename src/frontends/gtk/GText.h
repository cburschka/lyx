// -*- C++ -*-
/**
 * \file GText.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTEXT_H
#define GTEXT_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlCommand;

class GText : public GViewCB<ControlCommand, GViewGladeB>
{
public:
	GText(Dialog & parent, std::string const & title, std::string const & label);
private:
	virtual void apply();
	virtual void update();
	virtual void doBuild();
	void onEntryChanged();
	std::string const label_;
	Gtk::Entry * entry_;
};

} // namespace frontend
} // namespace lyx

#endif
