// -*- C++ -*-
/**
 * \file ControlLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLLOG_H
#define CONTROLLOG_H

#include "Dialog.h"

#include "support/FileName.h"

namespace lyx {
namespace frontend {

/**
 * A controller for a read-only text browser.
 */
class ControlLog : public Controller {
public:
	///
	ControlLog(Dialog &);
	/** \param data should contain "<logtype> <logfile>"
	 *  where <logtype> is one of "latex", "literate", "lyx2lyx", "vc".
	 */
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }

	/// The title displayed by the dialog reflects the \c LOGTYPE
	docstring const title() const;
	/// put the log file into the ostream
	void getContents(std::ostream & ss) const;

private:
	/// Recognized log file-types
	enum LOGTYPE {
		LatexLog,
		LiterateLog,
		Lyx2lyxLog,
		VCLog
	};

	LOGTYPE type_;
	support::FileName logfile_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLLOG_H
