// -*- C++ -*-
/**
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \file ControlShowFile.h
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */
#ifndef CONTROLSHOWFILE_H
#define CONTROLSHOWFILE_H

#include "Dialog.h"

/** A controller for the ShowFile dialog. */

class ControlShowFile : public Dialog::Controller {
public:
	///
	ControlShowFile(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return false; }
	///
	std::string getFileContents();
	///
	std::string getFileName();

private:
	///
	std::string filename_;
};

#endif // CONTROLSHOWFILE_H
