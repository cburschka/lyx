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
#include "insets/InsetNote.h"
#include <vector>

namespace lyx {
namespace frontend {

class ControlNote : public Controller
{
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
	InsetNoteParams & params() { return params_; }
	///
	InsetNoteParams const & params() const { return params_; }
	///
private:
	///
	InsetNoteParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLNOTE_H
