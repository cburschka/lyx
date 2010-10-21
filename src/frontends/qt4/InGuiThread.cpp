/**
 * \file InGuiThread.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InGuiThread.h"

#include <QThread>
#include <QEventLoop>
#include <QApplication>

namespace lyx {
namespace frontend {


IntoGuiThreadMover::IntoGuiThreadMover()
{
}


void IntoGuiThreadMover::callInGuiThread()
{
	QThread* gui_thread = QApplication::instance()->thread();
	if (QThread::currentThread() == gui_thread) {
		synchronousFunctionCall();
	} else {
		moveToThread(gui_thread);
		connect(this, SIGNAL(triggerCall()), this, SLOT(doFunctionCall()),
		        Qt::QueuedConnection);
		// TODO try with condition, it's maybe cheaper
		QEventLoop loop;
		connect(this, SIGNAL(called()), &loop, SLOT(quit()));
		Q_EMIT triggerCall();
		loop.exec();
	}
}


void IntoGuiThreadMover::doFunctionCall()
{
	synchronousFunctionCall();
	Q_EMIT called();
}


} // namespace frontend
} // namespace lyx

#include "moc_InGuiThread.cpp"
