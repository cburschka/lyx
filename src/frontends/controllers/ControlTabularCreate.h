// -*- C++ -*-
/**
 * \file ControlTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLTABULARCREATE_H
#define CONTROLTABULARCREATE_H

#include "Dialog.h"
#include <utility>

/** A controller for the TabularCreate Dialog.
 */
class ControlTabularCreate : public Dialog::Controller {
public:
	///
	ControlTabularCreate(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	typedef std::pair<unsigned int, unsigned int> rowsCols;
	///
	rowsCols & params() { return params_; }

private:
	/// rows, cols params
	rowsCols params_;
};

#endif // CONTROLTABULARCREATE_H
