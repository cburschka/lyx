// -*- C++ -*-
/**
 * \file ControlChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCHANGES_H
#define CONTROLCHANGES_H

#include "Dialog.h"

#include "support/docstring.h"

namespace lyx {
namespace frontend {

/**
 * A controller for the merge changes dialog.
 */
class ControlChanges : public Controller {
public:
	///
	ControlChanges(Dialog &);
	/// Nothing to initialise in this case.
	virtual bool initialiseParams(std::string const &) { return true; }
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	/// always true since dispatchParams() is empty
	virtual bool canApply() const { return true; }

	/// find the next change and highlight it
	void next();

	/// return date of change
	docstring const getChangeDate();

	/// return author of change
	docstring const getChangeAuthor();

	/// accept the current change
	void accept();

	/// reject the current change
	void reject();
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCHANGES_H
