// -*- C++ -*-
/**
 * \file ControlBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLBIBTEX_H
#define CONTROLBIBTEX_H

#include "ControlCommand.h"

/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public ControlCommand {
public:
	///
	ControlBibtex(LyXView &, Dialogs &);
	/// Browse for a .bib file
	std::string const browseBib(std::string const & in_name) const;
	/// Browse for a .bst file
	std::string const browseBst(std::string const & in_name) const;
	/// get the list of bst files
	string const getBibStyles() const;
	/// build filelists of all availabe bst/cls/sty-files. done through
	/// kpsewhich and an external script, saved in *Files.lst
	void rescanBibStyles() const;
private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
};

#endif // CONTROLBIBTEX_H
