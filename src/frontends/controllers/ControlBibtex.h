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


#include "Dialog.h"
#include "insets/insetbibtex.h"
#include <vector>


/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public Dialog::Controller {
public:
	///
	ControlBibtex(Dialog &);

	///
	InsetBibtexParams & params() { return params_; }
	///
	InsetBibtexParams const & params() const { return params_; }
	///
	virtual bool initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	/// Browse for a file
	string const Browse(string const &, string const &, string const &);
	/// get the list of bst files
	void getBibStyles(std::vector<string> & data) const;
	/// get the list of bib files
	void getBibFiles(std::vector<string> & data) const;
	/// build filelists of all availabe bib/bst/cls/sty-files. done through
	/// kpsewhich and an external script, saved in *Files.lst
	void rescanBibStyles() const;

private:
	///
	InsetBibtexParams params_;
};


#endif // CONTROLBIBTEX_H
