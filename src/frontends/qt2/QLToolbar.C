/**
 * \file qt2/QLToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "lyxfunc.h"

#include "QtView.h"
#include "QLToolbar.h"
#include "qt_helpers.h"

#include <qcombobox.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

namespace {

LyXTextClass const & getTextClass(LyXView const & lv)
{
	return lv.buffer()->params().getLyXTextClass();
}

QMainWindow::ToolBarDock getPosition(ToolbarBackend::Flags const & flags)
{
	if (flags & ToolbarBackend::TOP)
		return QMainWindow::Top;
	if (flags & ToolbarBackend::BOTTOM)
		return QMainWindow::Bottom;
	if (flags & ToolbarBackend::LEFT)
		return QMainWindow::Left;
	if (flags & ToolbarBackend::RIGHT)
		return QMainWindow::Right;
	return QMainWindow::Top;
}

} // namespace anon


QLayoutBox::QLayoutBox(QWidget * parent, QtView & owner)
	: combo_(new QComboBox(parent)),
	  owner_(owner)
{
	QSizePolicy p(QSizePolicy::Minimum, QSizePolicy::Fixed);
	combo_->setSizePolicy(p);
	combo_->setFocusPolicy(QWidget::ClickFocus);
	combo_->setMinimumWidth(combo_->sizeHint().width());

	QObject::connect(combo_, SIGNAL(activated(const QString &)),
			 this, SLOT(selected(const QString &)));
}


void QLayoutBox::set(string const & layout)
{
	LyXTextClass const & tc = getTextClass(owner_);

	QString const & name = qt_(tc[layout]->name());

	int i = 0;
	for (; i < combo_->count(); ++i) {
		if (name == combo_->text(i))
			break;
	}

	if (i == combo_->count()) {
		lyxerr << "Trying to select non existent layout type "
			<< fromqstr(name) << endl;
		return;
	}

	combo_->setCurrentItem(i);
}


void QLayoutBox::update()
{
	LyXTextClass const & tc = getTextClass(owner_);

	combo_->setUpdatesEnabled(false);

	combo_->clear();

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		// ignore obsolete entries
		if ((*it)->obsoleted_by().empty())
			combo_->insertItem(qt_((*it)->name()));
	}

	// needed to recalculate size hint
	combo_->hide();
	combo_->setMinimumWidth(combo_->sizeHint().width());
	combo_->show();

	combo_->setUpdatesEnabled(true);
	combo_->update();
}


void QLayoutBox::clear()
{
	combo_->clear();
}


void QLayoutBox::open()
{
	combo_->popup();
}


void QLayoutBox::setEnabled(bool enable)
{
	// Workaround for Qt bug where setEnabled(true) closes
	// the popup
	if (enable != combo_->isEnabled())
		combo_->setEnabled(enable);
}


void QLayoutBox::selected(const QString & str)
{
	string const sel = fromqstr(str);

	owner_.centralWidget()->setFocus();

	LyXTextClass const & tc = getTextClass(owner_);

	LyXTextClass::const_iterator it  = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		string const & name = (*it)->name();
		// Yes, the _() is correct
		if (_(name) == sel) {
			FuncRequest const func(LFUN_LAYOUT, name, 
					       FuncRequest::UI);
			owner_.getLyXFunc().dispatch(func);
			return;
		}
	}
	lyxerr << "ERROR (QLayoutBox::selected): layout not found!" << endl;
}

} // namespace frontend
} // namespace lyx

Toolbars::ToolbarPtr make_toolbar(ToolbarBackend::Toolbar const & tbb,
				  LyXView & owner)
{
	using lyx::frontend::QLToolbar;
	return Toolbars::ToolbarPtr(new QLToolbar(tbb, owner));
}

namespace lyx {
namespace frontend {

QLToolbar::QLToolbar(ToolbarBackend::Toolbar const & tbb, LyXView & owner)
	: owner_(dynamic_cast<QtView &>(owner)),
	  toolbar_(new QToolBar(qt_(tbb.gui_name), &owner_,
				getPosition(tbb.flags)))
{
	// give visual separation between adjacent toolbars
	toolbar_->addSeparator();

	ToolbarBackend::item_iterator it = tbb.items.begin();
	ToolbarBackend::item_iterator end = tbb.items.end();
	for (; it != end; ++it)
		add(it->first, it->second);
}


void QLToolbar::add(FuncRequest const & func, string const & tooltip)
{
	switch (func.action) {
	case ToolbarBackend::SEPARATOR:
		toolbar_->addSeparator();
		break;
	case ToolbarBackend::LAYOUTS:
		layout_.reset(new QLayoutBox(toolbar_, owner_));
		break;
	case ToolbarBackend::MINIBUFFER:
		owner_.addCommandBuffer(toolbar_);
		toolbar_->setHorizontalStretchable(true);
		break;
	default: {
		if (owner_.getLyXFunc().getStatus(func).unknown())
			break;
		QPixmap p = QPixmap(toolbarbackend.getIcon(func).c_str());
		QToolButton * button =
			new QToolButton(p, toqstr(tooltip), "",
			this, SLOT(clicked()), toolbar_);

		map_[button] = func;
		break;
	}
	}
}


void QLToolbar::hide(bool)
{
	toolbar_->hide();
}


void QLToolbar::show(bool)
{
	toolbar_->show();
}


void QLToolbar::update()
{
	ButtonMap::const_iterator p = map_.begin();
	ButtonMap::const_iterator end = map_.end();

	for (; p != end; ++p) {
		QToolButton * button = p->first;
		FuncRequest const & func = p->second;

		FuncStatus const status =
			owner_.getLyXFunc().getStatus(func);

		button->setToggleButton(true);
		button->setOn(status.onoff(true));
		button->setEnabled(status.enabled());
	}
}


void QLToolbar::clicked()
{
	QToolButton const * const_button =
		static_cast<QToolButton const *>(sender());
	QToolButton  * button =
		const_cast<QToolButton *>(const_button);

	ButtonMap::const_iterator it = map_.find(button);

	if (it != map_.end())
		owner_.getLyXFunc().dispatch(it->second);
	else
		lyxerr << "non existent tool button selected !" << endl;
}

} // namespace frontend
} // namespace lyx
