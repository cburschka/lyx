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
#include <boost/scoped_ptr.hpp>

class PrinterParams;

namespace lyx {
namespace frontend {

/** A controller for Print dialogs.
 */
class ControlPrint : public Dialog::Controller {
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

	/// Browse for a file
	std::string const browse(std::string const &) const;
	///
	PrinterParams & params() const;
private:
	///
	boost::scoped_ptr<PrinterParams> params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPRINT_H
