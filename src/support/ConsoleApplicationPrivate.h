/**
 * \file ConsoleApplicationPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONSOPLEAPPLICATIONPRIVATE_H
#define CONSOPLEAPPLICATIONPRIVATE_H

#include "support/qstring_helpers.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>

#include <string>


namespace lyx {

namespace support {

class ConsoleApplication;

class ConsoleApplicationPrivate : public QCoreApplication
{
	Q_OBJECT
public:
	ConsoleApplicationPrivate(ConsoleApplication * owner,
		std::string const & app, int & argc, char ** argv)
		: QCoreApplication(argc, argv), owner_(owner)
	{
		setOrganizationName("LyX");
		setOrganizationDomain("lyx.org");
		setApplicationName(toqstr(app));

		qsrand(QDateTime::currentDateTime().toTime_t());
	}
	int execute()
	{
		// set timer to do the work asynchronously after the event
		// loop was started
		QTimer::singleShot(0, this, SLOT(doExec()));
		// start event loop
		return exec();
	}
private Q_SLOTS:
	void doExec()
	{
		owner_->doExec();
	}
private:
	ConsoleApplication * owner_;
};


} // namespace support
} // namespace lyx

#endif // CONSOPLEAPPLICATIONPRIVATE_H
