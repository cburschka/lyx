// -*- C++ -*-
/**
 * \file ControlPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLPREFS_H
#define CONTROLPREFS_H

#include "ControlDialog_impl.h"
#include "LString.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "LColor.h"

#include <utility>

class Converters;
class Formats;

class ControlPrefs : public ControlDialogBI {
public:
	ControlPrefs(LyXView &, Dialogs &);

	// FIXME: we should probably devolve the individual
	// settings to methods here. But for now, this will
	// do

	LyXRC & rc() { return rc_; }

	LyXRC const & rc() const { return rc_; }

	/// various file pickers
	string const browsebind(string const & file);
	string const browseUI(string const & file);
	string const browsekbmap(string const & file);
	string const browsedict(string const & file);

	/// general browse
	string const browse(string const & file, string const & title);
	
	/// browse directory
	string const browsedir(string const & path, string const & title);

	/// redraw widgets (for xforms color change)
	void redrawGUI();

	/// set a color
	void setColor(LColor::color col, string const & hex);

	/// update the screen fonts after change
	void updateScreenFonts();

	/// set global converters
	void setConverters(Converters const & conv);

	/// set global formats
	void setFormats(Formats const & form);

private:
	/// get current lyxrc
	virtual void setParams();

	/// apply current lyxrc
	virtual void apply();

	/// temporary lyxrc
	LyXRC rc_;
};

#endif // CONTROLPREFS_H
