// -*- C++ -*-
/**
 * \file ControlErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLERRORLIST_H
#define CONTROLERRORLIST_H

#include "ErrorList.h"
#include "Dialog.h"

namespace lyx {
namespace frontend {

/** A controller for the ErrorList dialog.
 */
class ControlErrorList : public Controller {
public:
	///
	ControlErrorList(Dialog & parent);
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}

	/// goto this error in the parent bv
	void goTo(int item);
	/// return the parent document name
	std::string const & name();
	///
	ErrorList const & errorList() const;
private:
	///
	std::string error_type_;
	///
	std::string name_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLERRORLIST_H
