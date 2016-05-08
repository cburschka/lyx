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
	: DockView(parent, "toc", qt_("Outline"), area, flags), is_closing_(false)
{
	widget_ = new TocWidget(parent, this);
	setWidget(widget_);
	setFocusProxy(widget_);
}


GuiToc::~GuiToc()
{
	delete widget_;
}


void GuiToc::updateView()
{
	widget_->updateView();
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
		widget_->updateViewNow();
}


void GuiToc::closeEvent(QCloseEvent * /*event*/)
{
	is_closing_ = true;
	static_cast<GuiView *>(parent())->updateToolbars();
	is_closing_ = false;
}


void GuiToc::doDispatch(Cursor & cur, FuncRequest const & cmd)
{
	widget_->doDispatch(cur, cmd);
}


bool GuiToc::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	return widget_->getStatus(cur, cmd, status);
}


Dialog * createGuiToc(GuiView & lv)
{
	GuiToc * toc;
#ifdef Q_OS_MAC
	// On Mac show at the right and floating
	toc = new GuiToc(lv, Qt::RightDockWidgetArea);
	toc->setFloating(true);
#else
	toc = new GuiToc(lv);
#endif
	return toc;
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiToc.cpp"
