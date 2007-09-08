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

#include <boost/scoped_ptr.hpp>

#include <string>

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
		std::string const & title, ///< Window title (shown in the top title bar).
		Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0
		)
		: QDockWidget(toqstr(title), &parent, flags),
		Dialog(parent, title)
	{
		if (flags & Qt::Drawer)
			setFeatures(QDockWidget::NoDockWidgetFeatures);
		MyController * controller = new MyController(*this);
		setController(controller);
		widget_.reset(new MyWidget(*controller));
		setWidget(widget_.get());
		parent.addDockWidget(area, this);
	}

	/// Dialog inherited methods
	//@{
	void applyView() {}
	void hideView()	{ QDockWidget::hide(); }
	void showView()	{ QDockWidget::show(); }
	bool isVisibleView() const { return QDockWidget::isVisible(); }
	void redrawView() {}
	void updateView()
	{
		widget_->updateView();
		QDockWidget::update();
	}
	//@}
private:
	/// The encapsulated widget.
	boost::scoped_ptr<MyWidget> widget_;
};

} // frontend
} // lyx

#endif // DOCK_VIEW_H
