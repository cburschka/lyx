// -*- C++ -*-
/**
 * \file GTableCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTABLE_CREATE_H
#define GTABLE_CREATE_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlTableCreate;

class GTableCreate : public GViewCB<ControlTabularCreate, GViewGladeB>
{
public:
	GTableCreate(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update() {};
	Gtk::SpinButton *rows_;
	Gtk::SpinButton *columns_;
};

} // namespace frontend
} // namespace lyx

#endif
