// -*- C++ -*-
/**
 * \file QCommandBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCOMMANDBUFFER_H
#define QCOMMANDBUFFER_H

#include "controllers/ControlCommandBuffer.h"

#include <QWidget>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class QCommandEdit;
class GuiView;

class QCommandBuffer : public QWidget {
	Q_OBJECT
public:
	QCommandBuffer(GuiView * view);

	/// focus the edit widget
	void focus_command();
public Q_SLOTS:
	/// cancel command compose
	void cancel();
	/// dispatch a command
	void dispatch();
	/// tab-complete
	void complete();
	/// select-complete
	void complete_selected(QListWidgetItem *);
	/// up
	void up();
	/// down
	void down();
	/// leave and hide the command buffer
	void hideParent();
private:
	/// owning view
	GuiView * view_;

	/// controller
	ControlCommandBuffer controller_;

	/// command widget
	QCommandEdit * edit_;
};

} // namespace frontend
} // namespace lyx

#endif // QCOMMANDBUFFER_H
