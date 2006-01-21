// -*- C++ -*-
/**
 * \file GBibItem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GBIBITEM_H
#define GBIBITEM_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlCommand;

/** This class provides a GTK+ implementation of the BibItem Dialog.
 */
class GBibItem : public GViewCB<ControlCommand, GViewGladeB> {
public:
	GBibItem(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();
	void changed();

	Gtk::Entry * keyentry_;
	Gtk::Entry * labelentry_;
};

} // namespace frontend
} // namespace lyx

#endif // GBIBITEM_H
