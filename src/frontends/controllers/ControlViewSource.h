// -*- C++ -*-
/**
 * \file ControlViewSource.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLVIEWSOURCE_H
#define CONTROLVIEWSOURCE_H

#include "Dialog.h"

namespace lyx {
namespace frontend {

/**
 * A controller for a read-only text browser.
 */
class ControlViewSource : public Dialog::Controller {
public:
	///
	ControlViewSource(Dialog &);
	/** \param source source code to be displayed
	 */
	virtual bool initialiseParams(std::string const & source);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	///
	bool canApply() const { return true; }
	///
	virtual bool canApplyToReadOnly() const { return true; }

	/// The title displayed by the dialog reflects source type.
	docstring const title() const;

	/** get the source code of selected paragraphs, or the whole document
		\param fullSource get full source code
	 */
	docstring const updateContent(bool fullSource);
	/** get the cursor position in the source code
	 */
	std::pair<int, int> getRows() const;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLVIEWSOURCE_H
