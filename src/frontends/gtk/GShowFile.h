// -*- C++ -*-
/**
 * \file GShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GSHOWFILE_H
#define GSHOWFILE_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlShowFile;

/**
 * This class provides a GTK+ implementation of a dialog to browse through a
 * text file.
 */
class GShowFile : public GViewCB<ControlShowFile, GViewGladeB> {
public:
	///
	GShowFile(Dialog &);

	// Functions accessible to the Controller.

	virtual void doBuild();
	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();

	Glib::RefPtr<Gtk::TextBuffer> contentbuffer_;
};

} // namespace frontend
} // namespace lyx

#endif // GFILE_H
