// -*- C++ -*-
/**
 * \file GWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GWRAPH
#define GWRAP_H

#include "GViewBase.h"

#include "GtkLengthEntry.h"

namespace lyx {
namespace frontend {

class ControlWrap;

/** This class provides a GTK+ implementation of the Wrap Dialog.
 */
class GWrap : public GViewCB<ControlWrap, GViewGladeB> {
public:
	GWrap(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	// apply() won't act when this is true
	bool applylock_;

	Gtk::ComboBox *placementcombo_;
	GtkLengthEntry *widthlengthentry_;
};

} // namespace frontend
} // namespace lyx

#endif // GWRAP_H
