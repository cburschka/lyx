/**
 * \file GuiToc.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiToc.h"
#include "GuiView.h"
#include "DockView.h"
#include "TocWidget.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "FuncRequest.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

using namespace std;

namespace lyx {
namespace frontend {

GuiToc::GuiToc(GuiView & parent, Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "toc", qt_("Outline"), area, flags)
{
	widget_ = new TocWidget(parent, this);
	setWidget(widget_);
}


GuiToc::~GuiToc()
{
	delete widget_;
}


void GuiToc::updateView()
{
#ifndef Q_WS_MACX
	widget_->updateView();
	return;
#endif

	widget_->updateView();

	// For Mac: switch to a docked TOC in fullscreen mode.
	// We use the features() here instead of WindowFlags because
	// the latter are not reliable (always returns Qt::Drawer).
	if (!(features() & DockWidgetClosable) && lyxview().isFullScreen()) {
		setWindowFlags(Qt::Widget);
		setFeatures(DockWidgetClosable);
		// Setting features hides the dialog, see Qt's doc.
		show();
	} else if ((features() & DockWidgetClosable) && !lyxview().isFullScreen()) {
		setWindowFlags(Qt::Drawer);
		setFeatures(NoDockWidgetFeatures);
		// Setting features hides the dialog, see Qt's doc.
		show();
	}
}


bool GuiToc::initialiseParams(string const & data)
{
	widget_->init(toqstr(data));
	return true;
}


void GuiToc::dispatchParams()
{
}


void GuiToc::enableView(bool enable)
{
	if (!enable)
		// In the opposite case, updateView() will be called anyway.
		widget_->updateView();
}

void GuiToc::doDispatch(Cursor & cur, FuncRequest const & cmd)
{
	widget_->doDispatch(cur, cmd);
}


Dialog * createGuiToc(GuiView & lv)
{
	GuiView & guiview = static_cast<GuiView &>(lv);
#ifdef Q_WS_MACX
	// On Mac show as a drawer at the right
	return new GuiToc(guiview, Qt::RightDockWidgetArea, Qt::Drawer);
#else
	return new GuiToc(guiview);
#endif
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiToc.cpp"
