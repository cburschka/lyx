// -*- C++ -*-
/**
 * \file ControlNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLNOTE_H
#define CONTROLNOTE_H


#include "Dialog.h"
#include <vector>

class InsetNoteParams;

class ControlNote : public Dialog::Controller {
public:
	///
	ControlNote(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetNoteParams & params() { return *params_.get(); }
	///
	InsetNoteParams const & params() const { return *params_.get(); }
	///
private:
	///
	boost::scoped_ptr<InsetNoteParams> params_;
};

///
void note_gui_tokens(std::vector<std::string> &, std::vector<std::string> &);

#endif // CONTROLNOTE_H
