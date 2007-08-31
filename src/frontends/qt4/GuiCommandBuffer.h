// -*- C++ -*-
/**
 * \file GuiCommandBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMMANDBUFFER_H
#define GUICOMMANDBUFFER_H

#include "ControlCommandBuffer.h"

#include <QWidget>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiCommandEdit;
class GuiView;

class GuiCommandBuffer : public QWidget {
	Q_OBJECT
public:
	GuiCommandBuffer(GuiViewBase * view);

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
	GuiViewBase * view_;
	/// controller
	ControlCommandBuffer controller_;
	/// command widget
	GuiCommandEdit * edit_;
};

} // namespace frontend
} // namespace lyx

#endif // GUICOMMANDBUFFER_H
