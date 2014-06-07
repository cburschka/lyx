// -*- C++ -*-
/**
 * \file ProgressInterface.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SUPPORT_PROGRESSINTERFACE_H
#define LYX_SUPPORT_PROGRESSINTERFACE_H

#include "support/strfwd.h"

class QString;

namespace lyx {
namespace support {


class ProgressInterface
{
public:
	virtual ~ProgressInterface() {}

	/// will be Signals in Qt classes
	virtual void processStarted(QString const &) = 0;
	virtual void processFinished(QString const &) = 0;
	virtual void appendMessage(QString const &) = 0;
	virtual void appendError(QString const &) = 0;
	virtual void clearMessages() = 0;
	virtual void lyxerrFlush() = 0;

	/// Alert interface
	virtual void warning(QString const & title, QString const & message) = 0;
	virtual void toggleWarning(QString const & title, QString const & msg, QString const & formatted) = 0;
	virtual void error(QString const & title, QString const & message, QString const & details) = 0;
	virtual void information(QString const & title, QString const & message) = 0;
	virtual int prompt(docstring const & title, docstring const & question,
			   int default_button, int cancel_button,
			   docstring const & b1, docstring const & b2) = 0;

	virtual void lyxerrConnect() = 0;
	virtual void lyxerrDisconnect() = 0;

	static void setInstance(ProgressInterface*);
	static ProgressInterface* instance();

protected:
	ProgressInterface() {}
};



} // namespace support
} // namespace lyx

#endif // LYX_SUPPORT_PROGRESSINTERFACE_H

