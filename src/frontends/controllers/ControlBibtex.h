// -*- C++ -*-
/**
 * \file ControlBibtex.h
 * Read the file COPYING
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLBIBTEX_H
#define CONTROLBIBTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public ControlCommand
{
public:
	///
	ControlBibtex(LyXView &, Dialogs &);
	/// Browse for a file
	string const Browse(string const &, string const &, string const &);
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
