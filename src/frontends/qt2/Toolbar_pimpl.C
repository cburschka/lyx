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
#include "lyxtextclasslist.h"
#include "LyXAction.h"
#include "gettext.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lstrings.h" 

#include "QtView.h"
 
#include "Toolbar_pimpl.h"
 
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qsizepolicy.h>
 
using std::endl;

extern LyXAction lyxaction;

namespace {
 
QPixmap getIconPixmap(int action)
{
	string arg;
	string xpm_name;

	const kb_action act = lyxaction.retrieveActionArg(action, arg);
	string const name = lyxaction.getActionName(act);
	if (!arg.empty())
		xpm_name = subst(name + ' ' + arg, ' ','_');
	else 
		xpm_name = name;

	string fullname = LibFileSearch("images", xpm_name, "xpm");

	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `" 
				       << fullname << "'" << endl;
		return QPixmap(fullname.c_str());
	}

	if (act == LFUN_INSERT_MATH && !arg.empty()) {
#if 0 // FIXME: GUII
		char const ** pixmap =
			get_pixmap_from_symbol(arg.c_str(), 30, 30);
		if (pixmap) {
			lyxerr[Debug::GUI] << "Using mathed-provided icon"
					   << endl;
			return QPixmap(pixmap);
		}
#endif
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

 
Toolbar::Pimpl::Pimpl(LyXView * o, Dialogs &, int, int)
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
			owner_->getLyXFunc()->getStatus(action);
 
		button->setToggleButton(true);
		button->setOn(status.onoff(true));
		button->setEnabled(!status.disabled());
	}

	if (combo_)
		combo_->setEnabled(!owner_->getLyXFunc()->getStatus(LFUN_LAYOUT).disabled());
}


void Toolbar::Pimpl::button_selected(QToolButton * button)
{
	ButtonMap::const_iterator cit = map_.find(button);

	if (cit == map_.end()) {
		lyxerr << "non existent tool button selected !" << endl;
		return;
	}

	owner_->getLyXFunc()->verboseDispatch(cit->second, true);
}
 

void Toolbar::Pimpl::changed_layout(string const & sel)
{
	LyXTextClass const & tc =
		textclasslist[owner_->buffer()->params.textclass];
	
	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if (_((*cit)->name()) == sel) {
			owner_->getLyXFunc()->dispatch(LFUN_LAYOUT, (*cit)->name());
			return;
		}
	}
	lyxerr << "ERROR (Toolbar::Pimpl::layoutSelected): layout not found!"
	       << endl;
}
 

void Toolbar::Pimpl::setLayout(string const & layout)
{
	LyXTextClass const & tc =
		textclasslist[owner_->buffer()->params.textclass];
 
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
		textclasslist[owner_->buffer()->params.textclass];
 
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
#if 0 // popup() is protected
	combo_->popup();
#endif 
}


void Toolbar::Pimpl::set(bool)
{
// FIXME: ???
}


void Toolbar::Pimpl::add(int action, bool)
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
		combo_ = new QComboBox(toolbars_.back());
		QSizePolicy p(QSizePolicy::Minimum, QSizePolicy::Fixed);
		combo_->setSizePolicy(p);
		combo_->setFocusPolicy(QWidget::TabFocus);
		combo_->setMinimumWidth(combo_->sizeHint().width());
 
		QObject::connect(combo_, SIGNAL(activated(const QString &)),
			proxy_.get(), SLOT(layout_selected(const QString &)));
		break;
	}
	default: {
		char const * tooltip = _(lyxaction.helpText(action)).c_str();
 
		QToolButton * tb = 
			new QToolButton(getIconPixmap(action),
			tooltip, tooltip,
			proxy_.get(), SLOT(button_selected()), toolbars_.back());

		map_[tb] = action;
 
		QToolTip::add(tb, tooltip);
		break;
	}
	}
}


void Toolbar::Pimpl::push(int)
{
#if 0
	lyxerr[Debug::GUI] << "Toolbar::push: trying to trigger no `"
			       << nth << '\'' << endl;
	
	if (nth <= 0 || nth >= int(toollist.size())) {
		// item nth not found...
		return;
	}

	fl_trigger_object(toollist[nth - 1].icon);
#endif
}
