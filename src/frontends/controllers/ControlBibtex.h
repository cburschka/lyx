// -*- C++ -*-
/**
 * \file ControlBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLBIBTEX_H
#define CONTROLBIBTEX_H


#include "ControlCommand.h"
#include <vector>


/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public ControlCommand {
public:
	///
	ControlBibtex(Dialog &);

	/// Browse for a file
	string const Browse(string const &, string const &, string const &);
	/// get the list of bst files
	void getBibStyles(std::vector<string> & data) const;
	/// get the list of bib files
	void getBibFiles(std::vector<string> & data) const;
	/// build filelists of all availabe bib/bst/cls/sty-files. done through
	/// kpsewhich and an external script, saved in *Files.lst
	void rescanBibStyles() const;
};


#endif // CONTROLBIBTEX_H
