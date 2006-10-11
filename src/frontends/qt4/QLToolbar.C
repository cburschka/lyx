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

#include "GuiView.h"
#include "QLToolbar.h"
#include "Action.h"
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


} // namespace anon


QLayoutBox::QLayoutBox(QToolBar * toolbar, GuiView & owner)
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
		if (name == combo_->itemText(i))
			break;
	}

	if (i == combo_->count()) {
		lyxerr << "Trying to select non existent layout type "
			<< fromqstr(name) << endl;
		return;
	}

	combo_->setCurrentIndex(i);
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
			combo_->addItem(qt_((*it)->name()));
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
	combo_->showPopup();
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


QLToolbar::QLToolbar(ToolbarBackend::Toolbar const & tbb, GuiView & owner)
	: QToolBar(qt_(tbb.gui_name), &owner), owner_(owner)
{
	// give visual separation between adjacent toolbars
	addSeparator();

	// allowing the toolbars to tear off is too easily done,
	// and we don't save their orientation anyway. Disable the handle.
	setMovable(false);

	ToolbarBackend::item_iterator it = tbb.items.begin();
	ToolbarBackend::item_iterator end = tbb.items.end();
	// FIXME UNICODE: ToolbarBackend!!!
	for (; it != end; ++it)
		add(it->first, lyx::from_utf8(it->second));
}


void QLToolbar::add(FuncRequest const & func, docstring const & tooltip)
{
	switch (func.action) {
	case ToolbarBackend::SEPARATOR:
		addSeparator();
		break;
	case ToolbarBackend::LAYOUTS:
		layout_.reset(new QLayoutBox(this, owner_));
		break;
	case ToolbarBackend::MINIBUFFER:
		owner_.addCommandBuffer(this);
		/// \todo find a Qt4 equivalent to setHorizontalStretchable(true);
		//setHorizontalStretchable(true);
		break;
	case LFUN_TABULAR_INSERT: {
		QToolButton * tb = new QToolButton;
		tb->setCheckable(true);
		tb->setIcon(QPixmap(toqstr(toolbarbackend.getIcon(func))));
		tb->setToolTip(toqstr(tooltip));
		tb->setFocusPolicy(Qt::NoFocus);
		InsertTableWidget * iv = new InsertTableWidget(owner_, tb);
		connect(tb, SIGNAL(toggled(bool)), iv, SLOT(show(bool)));
		connect(iv, SIGNAL(visible(bool)), tb, SLOT(setChecked(bool)));
		connect(this, SIGNAL(updated()), iv, SLOT(updateParent()));
		addWidget(tb);
		break;
		}
	default: {
		if (lyx::getStatus(func).unknown())
			break;

		Action * action = new Action(owner_, toolbarbackend.getIcon(func), lyx::docstring(), func, tooltip);
		addAction(action);
		ActionVector.push_back(action);
		break;
		}
	}
}


void QLToolbar::hide(bool)
{
	QToolBar::hide();
}


void QLToolbar::show(bool)
{
	QToolBar::show();
}


void QLToolbar::update()
{
	for (size_t i=0; i<ActionVector.size(); ++i)
		ActionVector[i]->update();

	// emit signal
	updated();
}


} // namespace frontend
} // namespace lyx

#include "QLToolbar_moc.cpp"
