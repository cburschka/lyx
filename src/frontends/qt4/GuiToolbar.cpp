/**
 * \file qt4/GuiToolbar.cpp
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

#include "Buffer.h"
#include "BufferParams.h"
#include "debug.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "IconPalette.h"
#include "LyXFunc.h"
#include "ToolbarBackend.h"

#include "GuiView.h"
#include "GuiCommandBuffer.h"
#include "GuiToolbar.h"
#include "LyXAction.h"
#include "Action.h"
#include "qt_helpers.h"
#include "InsertTableWidget.h"

#include "support/filetools.h"
#include "support/lstrings.h"

#include "controllers/ControlMath.h"

#include <QComboBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QPixmap>

namespace lyx {

using std::string;
using std::endl;
using support::FileName;
using support::libFileSearch;
using support::subst;

namespace frontend {

static TextClass const & textClass(LyXView const & lv)
{
	return lv.buffer()->params().getTextClass();
}


/////////////////////////////////////////////////////////////////////
//
// GuiLayoutBox
//
/////////////////////////////////////////////////////////////////////

GuiLayoutBox::GuiLayoutBox(QToolBar * toolbar, GuiViewBase & owner)
	: owner_(owner)
{
	combo_ = new QComboBox;
	combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo_->setFocusPolicy(Qt::ClickFocus);
	combo_->setMinimumWidth(combo_->sizeHint().width());
	combo_->setMaxVisibleItems(100);

	QObject::connect(combo_, SIGNAL(activated(QString)),
			 this, SLOT(selected(QString)));

	toolbar->addWidget(combo_);
}


void GuiLayoutBox::set(docstring const & layout)
{
	TextClass const & tc = textClass(owner_);

	QString const & name = toqstr(translateIfPossible(tc[layout]->name()));

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


void GuiLayoutBox::update()
{
	TextClass const & tc = textClass(owner_);

	combo_->setUpdatesEnabled(false);
	combo_->clear();

	TextClass::const_iterator it = tc.begin();
	TextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		// ignore obsolete entries
		if ((*it)->obsoleted_by().empty())
			combo_->addItem(toqstr(translateIfPossible((*it)->name())));
	}

	// needed to recalculate size hint
	combo_->hide();
	combo_->setMinimumWidth(combo_->sizeHint().width());
	combo_->show();

	combo_->setUpdatesEnabled(true);
	combo_->update();
}


void GuiLayoutBox::clear()
{
	combo_->clear();
}


void GuiLayoutBox::open()
{
	combo_->showPopup();
}


void GuiLayoutBox::setEnabled(bool enable)
{
	// Workaround for Qt bug where setEnabled(true) closes
	// the popup
	if (enable != combo_->isEnabled())
		combo_->setEnabled(enable);
}


void GuiLayoutBox::selected(const QString & str)
{
	owner_.setFocus();

	layoutSelected(owner_, qstring_to_ucs4(str));
}



/////////////////////////////////////////////////////////////////////
//
// GuiToolbar
//
/////////////////////////////////////////////////////////////////////


GuiToolbar::GuiToolbar(ToolbarInfo const & tbinfo, GuiViewBase & owner)
	: QToolBar(qt_(tbinfo.gui_name), &owner), owner_(owner),
	  layout_(0), command_buffer_(0)
{
	// give visual separation between adjacent toolbars
	addSeparator();

	// TODO: save toolbar position
	setMovable(true);

	ToolbarInfo::item_iterator it = tbinfo.items.begin();
	ToolbarInfo::item_iterator end = tbinfo.items.end();
	for (; it != end; ++it)
		add(*it);
}


void GuiToolbar::focusCommandBuffer()
{
	if (command_buffer_)
		command_buffer_->setFocus();
}


Action * GuiToolbar::addItem(ToolbarItem const & item)
{
	Action * act = new Action(owner_,
		getIcon(item.func_, false).c_str(),
	  toqstr(item.label_), item.func_, toqstr(item.label_));
	actions_.append(act);
	return act;
}


void GuiToolbar::add(ToolbarItem const & item)
{
	switch (item.type_) {
	case ToolbarItem::SEPARATOR:
		addSeparator();
		break;
	case ToolbarItem::LAYOUTS:
		layout_ = new GuiLayoutBox(this, owner_);
		break;
	case ToolbarItem::MINIBUFFER:
		command_buffer_ = new GuiCommandBuffer(&owner_);
		addWidget(command_buffer_);
		/// \todo find a Qt4 equivalent to setHorizontalStretchable(true);
		//setHorizontalStretchable(true);
		break;
	case ToolbarItem::TABLEINSERT: {
		QToolButton * tb = new QToolButton;
		tb->setCheckable(true);
		tb->setIcon(QPixmap(toqstr(getIcon(FuncRequest(LFUN_TABULAR_INSERT)))));
		tb->setToolTip(qt_(to_ascii(item.label_)));
		tb->setStatusTip(qt_(to_ascii(item.label_)));
		tb->setText(qt_(to_ascii(item.label_)));
		InsertTableWidget * iv = new InsertTableWidget(owner_, tb);
		connect(tb, SIGNAL(clicked(bool)), iv, SLOT(show(bool)));
		connect(iv, SIGNAL(visible(bool)), tb, SLOT(setChecked(bool)));
		connect(this, SIGNAL(updated()), iv, SLOT(updateParent()));
		addWidget(tb);
		break;
		}
	case ToolbarItem::ICONPALETTE: {
		QToolButton * tb = new QToolButton(this);
		tb->setToolTip(qt_(to_ascii(item.label_)));
		tb->setStatusTip(qt_(to_ascii(item.label_)));
		tb->setText(qt_(to_ascii(item.label_)));
		connect(this, SIGNAL(iconSizeChanged(QSize)),
			tb, SLOT(setIconSize(QSize)));
		IconPalette * panel = new IconPalette(tb);
		panel->setWindowTitle(qt_(to_ascii(item.label_)));
		connect(this, SIGNAL(updated()), panel, SLOT(updateParent()));
		ToolbarInfo const * tbinfo = toolbarbackend.getDefinedToolbarInfo(item.name_);
		if (!tbinfo) {
			lyxerr << "Unknown toolbar " << item.name_ << endl;
			break;
		}
		ToolbarInfo::item_iterator it = tbinfo->items.begin();
		ToolbarInfo::item_iterator const end = tbinfo->items.end();
		for (; it != end; ++it)
			if (!getStatus(it->func_).unknown()) {
				panel->addButton(addItem(*it));
				// use the icon of first action for the toolbar button
				if (it == tbinfo->items.begin())
					tb->setIcon(QPixmap(getIcon(it->func_).c_str()));
			}
		tb->setCheckable(true);
		connect(tb, SIGNAL(clicked(bool)), panel, SLOT(setVisible(bool)));
		connect(panel, SIGNAL(visible(bool)), tb, SLOT(setChecked(bool)));
		addWidget(tb);
		break;
		}
	case ToolbarItem::POPUPMENU: {
		QToolButton * tb = new QToolButton;
		tb->setPopupMode(QToolButton::InstantPopup);
		tb->setToolTip(qt_(to_ascii(item.label_)));
		tb->setStatusTip(qt_(to_ascii(item.label_)));
		tb->setText(qt_(to_ascii(item.label_)));
		FileName icon_path = libFileSearch("images/math", item.name_, "png");
		tb->setIcon(QIcon(toqstr(icon_path.absFilename())));
		connect(this, SIGNAL(iconSizeChanged(QSize)),
			tb, SLOT(setIconSize(QSize)));

		ButtonMenu * m = new ButtonMenu(qt_(to_ascii(item.label_)), tb);
		m->setWindowTitle(qt_(to_ascii(item.label_)));
		m->setTearOffEnabled(true);
		connect(this, SIGNAL(updated()), m, SLOT(updateParent()));
		ToolbarInfo const * tbinfo = toolbarbackend.getDefinedToolbarInfo(item.name_);
		if (!tbinfo) {
			lyxerr << "Unknown toolbar " << item.name_ << endl;
			break;
		}
		ToolbarInfo::item_iterator it = tbinfo->items.begin();
		ToolbarInfo::item_iterator const end = tbinfo->items.end();
		for (; it != end; ++it)
			if (!getStatus(it->func_).unknown())
				m->add(addItem(*it));
		tb->setMenu(m);
		addWidget(tb);
		break;
		}
	case ToolbarItem::COMMAND: {
		if (!getStatus(item.func_).unknown())
			addAction(addItem(item));
		break;
		}
	default:
		break;
	}
}


void GuiToolbar::hide(bool)
{
	QToolBar::hide();
}


void GuiToolbar::show(bool)
{
	QToolBar::show();
}


bool GuiToolbar::isVisible() const
{
	return QToolBar::isVisible();
}


void GuiToolbar::saveInfo(ToolbarSection::ToolbarInfo & tbinfo)
{
	// if tbinfo.state == auto *do not* set on/off
	if (tbinfo.state != ToolbarSection::ToolbarInfo::AUTO) {
		if (GuiToolbar::isVisible())
			tbinfo.state = ToolbarSection::ToolbarInfo::ON;
		else
			tbinfo.state = ToolbarSection::ToolbarInfo::OFF;
	}
	//
	// no need to save it here.
	Qt::ToolBarArea loc = owner_.toolBarArea(this);

	if (loc == Qt::TopToolBarArea)
		tbinfo.location = ToolbarSection::ToolbarInfo::TOP;
	else if (loc == Qt::BottomToolBarArea)
		tbinfo.location = ToolbarSection::ToolbarInfo::BOTTOM;
	else if (loc == Qt::RightToolBarArea)
		tbinfo.location = ToolbarSection::ToolbarInfo::RIGHT;
	else if (loc == Qt::LeftToolBarArea)
		tbinfo.location = ToolbarSection::ToolbarInfo::LEFT;
	else
		tbinfo.location = ToolbarSection::ToolbarInfo::NOTSET;

	// save toolbar position. They are not used to restore toolbar position
	// now because move(x,y) does not work for toolbar.
	tbinfo.posx = pos().x();
	tbinfo.posy = pos().y();
}


void GuiToolbar::update()
{
	// This is a speed bottleneck because this is called on every keypress
	// and update calls getStatus, which copies the cursor at least two times
	for (int i = 0; i < actions_.size(); ++i)
		actions_[i]->update();

	// emit signal
	updated();
}


string const getIcon(FuncRequest const & f, bool unknown)
{
	using frontend::find_png;

	string fullname;

	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty())
			fullname = find_png(to_utf8(f.argument()).substr(1));
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		fullname = find_png(to_utf8(f.argument()));
		break;
	default:
		string const name = lyxaction.getActionName(f.action);
		string png_name = name;

		if (!f.argument().empty())
			png_name = subst(name + ' ' + to_utf8(f.argument()), ' ', '_');

		fullname = libFileSearch("images", png_name, "png").absFilename();

		if (fullname.empty()) {
			// try without the argument
			fullname = libFileSearch("images", name, "png").absFilename();
		}
	}

	if (!fullname.empty()) {
		LYXERR(Debug::GUI) << "Full icon name is `"
				   << fullname << '\'' << endl;
		return fullname;
	}

	LYXERR(Debug::GUI) << "Cannot find icon for command \""
			   << lyxaction.getActionName(f.action)
			   << '(' << to_utf8(f.argument()) << ")\"" << endl;
	if (unknown)
		return libFileSearch("images", "unknown", "png").absFilename();
	else
		return string();
}


} // namespace frontend
} // namespace lyx

#include "GuiToolbar_moc.cpp"
