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

#include "Dialog.h"
#include "Converter.h"
#include "Format.h"
#include "LyXRC.h"
#include "Mover.h"

#include <string>
#include <vector>

namespace lyx {

class Color_color;

namespace frontend {

class ControlPrefs : public Controller {
public:
	ControlPrefs(Dialog &);
	///
	virtual bool initialiseParams(std::string const &);
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return false; }

	LyXRC & rc() { return rc_; }
	LyXRC const & rc() const { return rc_; }

	Converters & converters() { return converters_; }
	Converters const & converters() const { return converters_; }

	Formats & formats() { return formats_; }
	Formats const & formats() const { return formats_; }

	Movers & movers() { return movers_; }
	Movers const & movers() const { return movers_; }

	/// various file pickers
	docstring const browsebind(docstring const & file) const;
	docstring const browseUI(docstring const & file) const;
	docstring const browsekbmap(docstring const & file) const;
	docstring const browsedict(docstring const & file) const;

	/// general browse
	docstring const browse(docstring const & file,
				 docstring const & title) const;

	/// browse directory
	docstring const browsedir(docstring const & path,
				    docstring const & title) const;

	/// set a color
	void setColor(Color_color col, std::string const & hex);

	/// update the screen fonts after change
	void updateScreenFonts();

	/// adjust the prefs paper sizes
	PAPER_SIZE toPaperSize(int i) const;
	/// adjust the prefs paper sizes
	int fromPaperSize(PAPER_SIZE papersize) const;

private:
	/// temporary lyxrc
	LyXRC rc_;

	/// temporary converters
	Converters converters_;

	/// temporary formats
	Formats formats_;

	/// temporary movers
	Movers movers_;

	/// A list of colors to be dispatched
	std::vector<std::string> colors_;

	bool redraw_gui_;
	bool update_screen_font_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPREFS_H
