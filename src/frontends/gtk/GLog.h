// -*- C++ -*-
/**
 * \file GLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GLOG_H
#define GLOG_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlLog;

/**
 * This class provides a GTK+ implementation of a dialog to browse through a
 * log file.
 */
class GLog : public GViewCB<ControlLog, GViewGladeB> {
public:
	GLog(Dialog &);

	// Create the dialog
	virtual void doBuild();
	// Set the Params variable for the Controller.
	virtual void apply() {}
	// Update dialog (load log into textbuffer)
	virtual void update();

	Glib::RefPtr<Gtk::TextBuffer> contentbuffer_;
};

} // namespace frontend
} // namespace lyx

#endif // GLOG_H
