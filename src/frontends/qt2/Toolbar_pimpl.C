/**
 * \file Toolbar_pimpl.C
 * See the file COPYING.
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 * \uathor John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ToolbarDefaults.h"
#include "debug.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "BufferView.h"
#include "buffer.h"
#include "LyXAction.h"
#include "gettext.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lstrings.h" 

#include "QtView.h"
 
#include "Toolbar_pimpl.h"
 
#include <boost/tuple/tuple.hpp>
 
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qsizepolicy.h>
 
using std::endl;

namespace {
 
QPixmap getIconPixmap(int action)
{
	FuncRequest f = lyxaction.retrieveActionArg(action);
 
	string xpm_name;

	if (f.action == LFUN_INSERT_MATH && !f.argument.empty()) {
		xpm_name = "math/" + subst(f.argument, ' ', '_');
	} else {
		string const name = lyxaction.getActionName(f.action);
		if (!f.argument.empty())
			xpm_name = subst(name + ' ' + f.argument, ' ','_');
		else 
			xpm_name = name;
	}

	string fullname = LibFileSearch("images", xpm_name, "xpm");

	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `" 
				       << fullname << "'" << endl;
		return QPixmap(fullname.c_str());
	}

	lyxerr << "Unable to find icon `" << xpm_name << "'" << endl;
	fullname = LibFileSearch("images", "unknown", "xpm");
	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Using default `unknown' icon" 
				       << endl;
	}
	return QPixmap(fullname.c_str());
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

	if (combo_)
		combo_->setEnabled(!owner_->getLyXFunc().getStatus(LFUN_LAYOUT).disabled());
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
	LyXTextClass const & tc =
		owner_->buffer()->params.getLyXTextClass();
	
	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if (_((*cit)->name()) == sel) {
			owner_->getLyXFunc().dispatch(FuncRequest(LFUN_LAYOUT, (*cit)->name()));
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
 
	string const & name = _(tc[layout]->name());
 
	int i;
 
	for (i = 0; i < combo_->count(); ++i) {
		if (name == combo_->text(i).latin1())
			break;
	}

	if (i == combo_->count()) {
		lyxerr << "Trying to select non existent layout type "
			<< name << endl;
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
			combo_->insertItem(_((*cit)->name()).c_str());
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
			_(lyxaction.helpText(action)).c_str(), "",
			proxy_.get(), SLOT(button_selected()), toolbars_.back());

		map_[tb] = action;
		break;
	}
	}
}
