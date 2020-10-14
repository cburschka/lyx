// -*- C++ -*-
/**
 * \file DockView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "DockView.h"
#include "GuiView.h"


namespace lyx {
namespace frontend {


DockView::DockView(GuiView & parent, QString const & name,
                   QString const & title, Qt::DockWidgetArea area,
                   Qt::WindowFlags flags)
	: QDockWidget(&parent, flags), Dialog(parent, name, title)
{
	setObjectName(name);
	parent.addDockWidget(area, this);
	hide();
	connect(&parent, SIGNAL(bufferViewChanged()),
	        this, SLOT(onBufferViewChanged()));
}


void DockView::keyPressEvent(QKeyEvent * ev)
{
	if (ev->key() == Qt::Key_Escape) {
		QMainWindow * mw = static_cast<QMainWindow *>(parent());
		if (!mw) {
			ev->ignore();
			return;
		}
		mw->activateWindow();
		mw->setFocus();
		if (isFloating())
			hide();
		ev->accept();
	}
}


} // namespace frontend
} // namespace lyx

#include "moc_DockView.cpp"
