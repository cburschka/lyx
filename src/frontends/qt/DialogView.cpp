// -*- C++ -*-
/**
 * \file DialogView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "DialogView.h"


namespace lyx {
namespace frontend {

DialogView::DialogView(GuiView & lv, QString const & name, QString const & title)
	: QDialog(&lv), Dialog(lv, name, title)
{
	connect(&lv, SIGNAL(bufferViewChanged()),
	        this, SLOT(onBufferViewChanged()));

	// remove question marks from Windows dialogs
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}


void DialogView::closeEvent(QCloseEvent * ev)
{
	clearParams();
	Dialog::disconnect();
	ev->accept();
}


void DialogView::hideEvent(QHideEvent * ev)
{
	if (!ev->spontaneous()) {
		clearParams();
		Dialog::disconnect();
		ev->accept();
	}
}

} // namespace frontend
} // namespace lyx

#include "moc_DialogView.cpp"
