/**
 * \file qt2/Toolbar_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ToolbarDefaults.h"
#include "debug.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "BufferView.h"
#include "buffer.h"
#include "LyXAction.h"
#include "qt_helpers.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include "ControlMath.h"

#include <boost/tuple/tuple.hpp>

#include "QtView.h"
#include "Toolbar_pimpl.h"

#include <qtoolbar.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qsizepolicy.h>

using std::endl;

namespace {

QPixmap getIconPixmap(int action)
{
	FuncRequest f = lyxaction.retrieveActionArg(action);

	string fullname;

	if (f.action == LFUN_INSERT_MATH && !f.argument.empty()) {
		fullname = find_xpm(f.argument.substr(1));
	} else {
		string const name = lyxaction.getActionName(f.action);
		string xpm_name(name);

		if (!f.argument.empty())
			xpm_name = subst(name + ' ' + f.argument, ' ','_');

		fullname = LibFileSearch("images", xpm_name, "xpm");
	}


	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `"
				   << fullname << '\'' << endl;
		return QPixmap(toqstr(fullname));
	}

	lyxerr << "Unable to find icon `" << fullname << '\'' << endl;
	fullname = LibFileSearch("images", "unknown", "xpm");
	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Using default `unknown' icon"
				   << endl;
	}
	return QPixmap(toqstr(fullname));
}

} // namespace anon


class QLComboBox : public QComboBox {
public:
	QLComboBox(QWidget * parent) : QComboBox(parent) {}
	void popup() { QComboBox::popup(); }
};


Toolbar::Pimpl::Pimpl(LyXView * o, int, int)
	: owner_(static_cast<QtView *>(o)),
	combo_(0)
{
	proxy_.reset(new ToolbarProxy(*this));
}


Toolbar::Pimpl::~Pimpl()
{
}


void Toolbar::Pimpl::update()
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


void Toolbar::Pimpl::button_selected(QToolButton * button)
{
	ButtonMap::const_iterator cit = map_.find(button);

	if (cit == map_.end()) {
		lyxerr << "non existent tool button selected !" << endl;
		return;
	}

	owner_->getLyXFunc().dispatch(cit->second, true);
}


void Toolbar::Pimpl::changed_layout(string const & sel)
{
	owner_->centralWidget()->setFocus();

	LyXTextClass const & tc =
		owner_->buffer()->params.getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		// Yes, the _() is correct
		if (_((*cit)->name()) == sel) {
			owner_->getLyXFunc().dispatch(FuncRequest(LFUN_LAYOUT, (*cit)->name()), true);
			return;
		}
	}
	lyxerr << "ERROR (Toolbar::Pimpl::layoutSelected): layout not found!"
	       << endl;
}


void Toolbar::Pimpl::setLayout(string const & layout)
{
	LyXTextClass const & tc =
		owner_->buffer()->params.getLyXTextClass();

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


void Toolbar::Pimpl::updateLayoutList(bool force)
{
	// if we don't need an update, don't ...
	if (combo_->count() && !force)
		return;

	LyXTextClass const & tc =
		owner_->buffer()->params.getLyXTextClass();

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


void Toolbar::Pimpl::clearLayoutList()
{
	combo_->clear();
}


void Toolbar::Pimpl::openLayoutList()
{
	combo_->popup();
}


void Toolbar::Pimpl::add(int action)
{
	if (!toolbars_.size()) {
		toolbars_.push_back(new QToolBar(owner_));
	}

	switch (action) {
	case ToolbarDefaults::SEPARATOR:
		toolbars_.back()->addSeparator();
		break;
	case ToolbarDefaults::NEWLINE:
		toolbars_.push_back(new QToolBar(owner_));
		break;
	case ToolbarDefaults::LAYOUTS: {
		combo_ = new QLComboBox(toolbars_.back());
		QSizePolicy p(QSizePolicy::Minimum, QSizePolicy::Fixed);
		combo_->setSizePolicy(p);
		combo_->setFocusPolicy(QWidget::ClickFocus);
		combo_->setMinimumWidth(combo_->sizeHint().width());

		QObject::connect(combo_, SIGNAL(activated(const QString &)),
			proxy_.get(), SLOT(layout_selected(const QString &)));
		break;
	}
	default: {
		QToolButton * tb =
			new QToolButton(getIconPixmap(action),
			qt_(lyxaction.helpText(action)), "",
			proxy_.get(), SLOT(button_selected()), toolbars_.back());

		map_[tb] = action;
		break;
	}
	}
}
