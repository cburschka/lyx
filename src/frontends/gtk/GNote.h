// -*- C++ -*-
/**
 * \file GNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GNOTE_H
#define GNOTE_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlNote;

/** This class provides a GTK+ implementation of the Note Dialog.
 */
class GNote : public GViewCB<ControlNote, GViewGladeB> {
public:
	GNote(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	// apply() won't act when this is true
	bool applylock_;

	Gtk::RadioButton * lyxnoteradio_;
	Gtk::RadioButton * commentradio_;
	Gtk::RadioButton * greyedoutradio_;
};

} // namespace frontend
} // namespace lyx

#endif // GNOTE_H
