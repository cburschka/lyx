// -*- C++ -*-
/**
 * \file GERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GERT_H
#define GERT_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlERT;

/** This class provides a GTK+ implementation of the ERT Dialog.
 */
class GERT : public GViewCB<ControlERT, GViewGladeB> {
public:
	GERT(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	// apply() won't act when this is true
	bool applylock_;

	Gtk::RadioButton * inlineradio_;
	Gtk::RadioButton * openradio_;
	Gtk::RadioButton * collapsedradio_;
};

} // namespace frontend
} // namespace lyx

#endif // GERT_H
