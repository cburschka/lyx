// -*- C++ -*-
/**
 * \file QCommandBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCOMMANDBUFFER_H
#define QCOMMANDBUFFER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <qtoolbar.h>

class QtView;
class QCommandEdit;
class QListBoxItem;
class ControlCommandBuffer;

class QCommandBuffer : public QToolBar {
	Q_OBJECT
public:
	QCommandBuffer(QtView * view, ControlCommandBuffer & control);

	/// focus the edit widget
	void focus_command();
public slots:
	/// cancel command compose
	void cancel();
	/// dispatch a command
	void dispatch();
	/// tab-complete
	void complete();
	/// select-complete
	void complete_selected(QString const & str);
	/// up
	void up();
	/// down
	void down();
private:
	/// owning view
	QtView * view_;

	/// controller
	ControlCommandBuffer & controller_;

	/// command widget
	QCommandEdit * edit_;
};

#endif // QCOMMANDBUFFER_H
