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

namespace lyx {
namespace frontend {

/** A controller for the TabularCreate Dialog.
 */
class ControlTabularCreate : public Controller {
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
	virtual kb_action getLfun() const { return LFUN_TABULAR_INSERT; }

	///
	typedef std::pair<size_t, size_t> rowsCols;
	///
	rowsCols & params() { return params_; }

private:
	/// rows, cols params
	rowsCols params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLTABULARCREATE_H
