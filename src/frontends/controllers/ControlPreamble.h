// -*- C++ -*-
/**
 * \file ControlPreamble.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLPREAMBLE_H
#define CONTROLPREAMBLE_H

#include "Dialog.h"

namespace lyx {
namespace frontend {

/** A controller for Preamble dialogs.
 */
class ControlPreamble : public Dialog::Controller {
public:
	///
	ControlPreamble(Dialog &);
	///
	virtual bool initialiseParams(std::string const &);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	std::string const & params() const;
	///
	void params(std::string const & newparams);
private:
	///
	std::string params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPREAMBLE_H
