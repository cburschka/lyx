/**
 * \file QCommandBuffer.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QCOMMANDBUFFER_H
#define QCOMMANDBUFFER_H

#include <config.h>
#include "LString.h"
 
#include <qtoolbar.h>
#include <qtimer.h>
 
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
	void complete_selected(const QString & str);
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
