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

#include "controllers/Dialog.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "debug.h"

#include <QDockWidget>

namespace lyx {
namespace frontend {

/// Dock Widget container for LyX dialogs.
/// This template class that encapsulates a given Widget inside a
/// QDockWidget and presents a Dialog interface
template<class MyController, class MyWidget>
class DockView : public QDockWidget, public Dialog
{
public:
	DockView(
		GuiViewBase & parent, ///< the main window where to dock.
		std::string const & name, ///< dialog identifier.
		Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0
	)
		: QDockWidget(&parent, flags), name_(name)
	{
		if (flags & Qt::Drawer)
			setFeatures(QDockWidget::NoDockWidgetFeatures);
		MyController * c = new MyController(*this);
		controller_ = c;
		controller_->setLyXView(parent);
		widget_ = new MyWidget(*c);
		setWidget(widget_);
		setWindowTitle(widget_->windowTitle());
		parent.addDockWidget(area, this);
	}
	~DockView() { delete widget_; delete controller_; }

	/// Dialog inherited methods
	//@{
	void applyView() {}
	void hideView()	{ QDockWidget::hide(); }
	void showView()
	{
		widget_->updateView();  // make sure its up-to-date
		QDockWidget::show();
	}
	bool isVisibleView() const { return QDockWidget::isVisible(); }
	void checkStatus() {}
	void redraw() { redrawView(); }
	void redrawView() {}
	void updateData(std::string const &) { updateView(); }
	void updateView()
	{
		widget_->updateView();
		QDockWidget::update();
	}
	bool isClosing() const { return false; }
	void partialUpdateView(int /*id*/) {}
	Controller & controller() { return *controller_; }
	std::string name() const { return name_; }
	//@}
private:
	/// The encapsulated widget.
	MyWidget * widget_;
	Controller * controller_;
	std::string name_;
};

} // frontend
} // lyx

#endif // DOCK_VIEW_H
