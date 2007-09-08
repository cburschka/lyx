// -*- C++ -*-
/**
 * \file ControlPrint.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLPRINT_H
#define CONTROLPRINT_H

#include "Dialog.h"

#include "support/docstring.h"

#include <boost/scoped_ptr.hpp>

namespace lyx {

class PrinterParams;

namespace frontend {

/** A controller for Print dialogs.
 */
class ControlPrint : public Controller {
public:
	///
	ControlPrint(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual bool canApplyToReadOnly() const { return true; }
	///
	virtual kb_action getLfun() const { return LFUN_BUFFER_PRINT; }

	/// Browse for a file
	docstring const browse(docstring const &) const;
	///
	PrinterParams & params() const;
private:
	///
	boost::scoped_ptr<PrinterParams> params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPRINT_H
