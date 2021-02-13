// -*- C++ -*-
/**
 * \file DockView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DOCK_VIEW_H
#define DOCK_VIEW_H

#include "Dialog.h"

#include <QDockWidget>

namespace lyx {
namespace frontend {

/// Dock Widget container for LyX dialogs.
/**
 * This template class that encapsulates a given Widget inside a
 * QDockWidget and presents a Dialog interface
 * FIXME: create a DockView.cpp file
 **/
class DockView : public QDockWidget, public Dialog
{
	Q_OBJECT

public:
	DockView(GuiView & parent, ///< the main window where to dock.
	         QString const & name, ///< dialog identifier.
	         QString const & title, ///< dialog title.
	         Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock
								   /// (and also drawer)
	         Qt::WindowFlags flags = {});

	virtual ~DockView() {}

	QWidget * asQWidget() override { return this; }
	QWidget const * asQWidget() const override { return this; }

	/// We don't want to restore geometry session for dock widgets.
	void restoreSession() override {}

	void keyPressEvent(QKeyEvent * ev) override;

	/// Dialog inherited methods
	//@{
	void applyView() override {}
	bool isClosing() const override { return false; }
	bool needBufferOpen() const override { return false; }
	//@}

protected Q_SLOTS:
	void onBufferViewChanged() override {}
};

} // namespace frontend
} // namespace lyx

#endif // DOCK_VIEW_H
