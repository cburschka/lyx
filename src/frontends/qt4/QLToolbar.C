/**
 * \file qt4/QLToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
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
#include "QLAction.h"
#include "qt_helpers.h"
#include "InsertTableWidget.h"

#include <QComboBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QPixmap>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

namespace {

LyXTextClass const & getTextClass(LyXView const & lv)
{
	return lv.buffer()->params().getLyXTextClass();
}

/// \todo Remove Qt::Dock getPosition(ToolbarBackend::Flags const & flags) if not needed anymore
Qt::Dock getPosition(ToolbarBackend::Flags const & flags)
{
	if (flags & ToolbarBackend::TOP)
		return Qt::DockTop;
	if (flags & ToolbarBackend::BOTTOM)
		return Qt::DockBottom;
	if (flags & ToolbarBackend::LEFT)
		return Qt::DockLeft;
	if (flags & ToolbarBackend::RIGHT)
		return Qt::DockRight;
	return Qt::DockTop;
}

Qt::ToolBarArea getToolBarPosition(ToolbarBackend::Flags const & flags)
{
	if (flags & ToolbarBackend::TOP)
		return Qt::TopToolBarArea;
	if (flags & ToolbarBackend::BOTTOM)
		return Qt::BottomToolBarArea;
	if (flags & ToolbarBackend::LEFT)
		return Qt::LeftToolBarArea;
	if (flags & ToolbarBackend::RIGHT)
		return Qt::RightToolBarArea;
	return Qt::TopToolBarArea;
}

} // namespace anon


QLayoutBox::QLayoutBox(QToolBar * toolbar, QtView & owner)
	: owner_(owner)
{
	combo_ = new QComboBox;
	combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo_->setFocusPolicy(Qt::ClickFocus);
	combo_->setMinimumWidth(combo_->sizeHint().width());
	combo_->setMaxVisibleItems(100);

	QObject::connect(combo_, SIGNAL(activated(const QString &)),
			 this, SLOT(selected(const QString &)));

	toolbar->addWidget(combo_);
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

	layoutSelected(owner_, sel);
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
	  toolbar_(new QToolBar(qt_(tbb.gui_name), (QWidget*) &owner_)) //, getPosition(tbb.flags)))
{
	/// \toto Move \a addToolBar call into QView because, in Qt4,
	/// the ToolBars placement is the duty of QMainWindow (aka QView)
	Qt::ToolBarArea tba = getToolBarPosition(tbb.flags);
	switch(tba) {
	case Qt::TopToolBarArea:
		owner_.addToolBar(tba, toolbar_);
		owner_.addToolBarBreak(tba);
		break;
//	case Qt::BottomToolBarArea:
		//bottomToolbarVector.push_back(toolbar_);
//		owner_.addToolBar(tba, toolbar_);
//		//if owner_.insertToolBarBreak(toolbar_);
		break;
	default:
		owner_.addToolBar(Qt::TopToolBarArea, toolbar_);
		owner_.addToolBarBreak(Qt::TopToolBarArea);
		break;
	}

	// give visual separation between adjacent toolbars
	toolbar_->addSeparator();

	// allowing the toolbars to tear off is too easily done,
	// and we don't save their orientation anyway. Disable the handle.
	toolbar_->setMovable(false);

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
		/// \todo find a Qt4 equivalent to setHorizontalStretchable(true);
		//toolbar_->setHorizontalStretchable(true);
		break;
	case LFUN_TABULAR_INSERT: {
		QToolButton * tb = new QToolButton;
		tb->setCheckable(true);
		tb->setIcon(QPixmap(toqstr(toolbarbackend.getIcon(func))));
		tb->setToolTip(toqstr(tooltip));
		tb->setFocusPolicy(Qt::NoFocus);
		InsertTableWidget * iv = new InsertTableWidget(owner_, func, tb);
		connect(tb, SIGNAL(toggled(bool)), iv, SLOT(show(bool)));
		connect(iv, SIGNAL(visible(bool)), tb, SLOT(setChecked(bool)));
		connect(this, SIGNAL(updated()), iv, SLOT(updateParent()));
		toolbar_->addWidget(tb);
		break;
		}
	default: {
		if (owner_.getLyXFunc().getStatus(func).unknown())
			break;

		QLAction * action = new QLAction(owner_, toolbarbackend.getIcon(func), "", func, tooltip);
		toolbar_->addAction(action);
		ActionVector.push_back(action);
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
	for (size_t i=0; i<ActionVector.size(); ++i)
		ActionVector[i]->update();

	emit updated();
}


} // namespace frontend
} // namespace lyx

#include "QLToolbar_moc.cpp"
