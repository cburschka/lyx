/**
 * \file qt2/QLToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "buffer.h"

#include "QtView.h"
#include "QLToolbar.h"

#include <qcombobox.h>
#include <qtoolbar.h>

using std::endl;


class QLComboBox : public QComboBox {
public:
	QLComboBox(QWidget * parent) : QComboBox(parent) {}
	void popup() { QComboBox::popup(); }
};


QLToolbar::QLToolbar(LyXView * o)
	: owner_(static_cast<QtView *>(o)),
	  combo_(0)
{
	proxy_.reset(new ToolbarProxy(*this));
}


void QLToolbar::displayToolbar(ToolbarBackend::Toolbar const & tb, bool show)
{
	QToolBar * qtb = toolbars_[tb.name];
	if (show) {
		qtb->show();
	} else {
		qtb->hide();
	}
}


void QLToolbar::update()
{
	ButtonMap::const_iterator p = map_.begin();
	ButtonMap::const_iterator end = map_.end();

	for (; p != end; ++p) {
		QToolButton * button = p->first;
		int action = p->second;

		FuncStatus const status =
			owner_->getLyXFunc().getStatus(action);

		button->setToggleButton(true);
		button->setOn(status.onoff(true));
		button->setEnabled(!status.disabled());
	}

	bool const enable = !owner_->getLyXFunc().getStatus(LFUN_LAYOUT).disabled();

	// Workaround for Qt bug where setEnabled(true) closes
	// the popup
	if (combo_ && enable != combo_->isEnabled())
		combo_->setEnabled(enable);
}


void QLToolbar::button_selected(QToolButton * button)
{
	ButtonMap::const_iterator cit = map_.find(button);

	if (cit == map_.end()) {
		lyxerr << "non existent tool button selected !" << endl;
		return;
	}

	owner_->getLyXFunc().dispatch(cit->second, true);
}


void QLToolbar::changed_layout(string const & sel)
{
	owner_->centralWidget()->setFocus();

	LyXTextClass const & tc =
		owner_->buffer()->params().getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		// Yes, the _() is correct
		if (_((*cit)->name()) == sel) {
			owner_->getLyXFunc().dispatch(FuncRequest(LFUN_LAYOUT, (*cit)->name()), true);
			return;
		}
	}
	lyxerr << "ERROR (QLToolbar::layoutSelected): layout not found!"
	       << endl;
}


void QLToolbar::setLayout(string const & layout)
{
	if (!combo_)
		return;

	LyXTextClass const & tc =
		owner_->buffer()->params().getLyXTextClass();

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


void QLToolbar::updateLayoutList()
{
	if (!combo_)
		return;

	LyXTextClass const & tc =
		owner_->buffer()->params().getLyXTextClass();

	combo_->setUpdatesEnabled(false);

	combo_->clear();

	LyXTextClass::const_iterator cit = tc.begin();
	LyXTextClass::const_iterator end = tc.end();
	for (; cit != end; ++cit) {
		// ignore obsolete entries
		if ((*cit)->obsoleted_by().empty())
			combo_->insertItem(qt_((*cit)->name()));
	}

	// needed to recalculate size hint
	combo_->hide();
	combo_->setMinimumWidth(combo_->sizeHint().width());
	combo_->show();

	combo_->setUpdatesEnabled(true);
	combo_->update();
}


void QLToolbar::clearLayoutList()
{
	if (!combo_)
		return;

	Toolbar::clearLayoutList();
	combo_->clear();
}


void QLToolbar::openLayoutList()
{
	if (!combo_)
		return;

	combo_->popup();
}


namespace {

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

};


void QLToolbar::add(ToolbarBackend::Toolbar const & tb)
{
	QToolBar * qtb = new QToolBar(qt_(tb.name), owner_, getPosition(tb.flags));
	// give visual separation between adjacent toolbars
	qtb->addSeparator();

	ToolbarBackend::item_iterator it = tb.items.begin();
	ToolbarBackend::item_iterator end = tb.items.end();
	for (; it != end; ++it)
		add(qtb, it->first, it->second);

	toolbars_[tb.name] = qtb;
	displayToolbar(tb, tb.flags & ToolbarBackend::ON);

}


void QLToolbar::add(QToolBar * tb, int action, string const & tooltip)
{
	switch (action) {
	case ToolbarBackend::SEPARATOR:
		tb->addSeparator();
		break;
	case ToolbarBackend::LAYOUTS: {
		combo_ = new QLComboBox(tb);
		QSizePolicy p(QSizePolicy::Minimum, QSizePolicy::Fixed);
		combo_->setSizePolicy(p);
		combo_->setFocusPolicy(QWidget::ClickFocus);
		combo_->setMinimumWidth(combo_->sizeHint().width());

		QObject::connect(combo_, SIGNAL(activated(const QString &)),
			proxy_.get(), SLOT(layout_selected(const QString &)));
		break;
	}
	case ToolbarBackend::MINIBUFFER:
		owner_->addCommandBuffer(tb);
		tb->setHorizontalStretchable(true);
		break;
	default: {
		if (owner_->getLyXFunc().getStatus(action).unknown())
			break;
		QPixmap p = QPixmap(toolbarbackend.getIcon(action).c_str());
		QToolButton * button =
			new QToolButton(p, toqstr(tooltip), "",
			proxy_.get(), SLOT(button_selected()), tb);

		map_[button] = action;
		break;
	}
	}
}
