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
#include <QMutexLocker>


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
		connect(this, SIGNAL(triggerFunctionCall()), 
		        this, SLOT(doFunctionCall()), Qt::QueuedConnection);
		QMutexLocker lock(&sync_mutex_);
		Q_EMIT triggerFunctionCall();
		condition_.wait(&sync_mutex_);
	}
}


void IntoGuiThreadMover::doFunctionCall()
{
	synchronousFunctionCall();
	condition_.wakeOne();
}


} // namespace frontend
} // namespace lyx

#include "moc_InGuiThread.cpp"
