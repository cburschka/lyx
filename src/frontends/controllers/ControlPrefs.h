// -*- C++ -*-
/**
 * \file ControlPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLPREFS_H
#define CONTROLPREFS_H


#include "ControlDialog_impl.h"
#include "lyxrc.h"


class Converters;
class LColor_color;
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
	std::string const browsebind(std::string const & file) const;
	std::string const browseUI(std::string const & file) const;
	std::string const browsekbmap(std::string const & file) const;
	std::string const browsedict(std::string const & file) const;

	/// general browse
	std::string const browse(std::string const & file,
				 std::string const & title) const;

	/// browse directory
	std::string const browsedir(std::string const & path,
				    std::string const & title) const;

	/// redraw widgets (for xforms color change)
	void redrawGUI();

	/// set a color
	void setColor(LColor_color col, std::string const & hex);

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
