/**
 * \file qt4/QLToolbar.cpp
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
#include "LyXFunc.h"
#include "IconPalette.h"

#include "GuiView.h"
#include "QLToolbar.h"
#include "LyXAction.h"
#include "Action.h"
#include "qt_helpers.h"
#include "InsertTableWidget.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "controllers/ControlMath.h"
#include "ToolbarBackend.h"

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


using support::libFileSearch;
using support::subst;

namespace frontend {

namespace {

TextClass const & getTextClass(LyXView const & lv)
{
	return lv.buffer()->params().getTextClass();
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


void QLayoutBox::set(docstring const & layout)
{
	TextClass const & tc = getTextClass(owner_);

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


void QLayoutBox::update()
{
	TextClass const & tc = getTextClass(owner_);

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
	owner_.setFocus();

	layoutSelected(owner_, qstring_to_ucs4(str));
}


QLToolbar::QLToolbar(ToolbarInfo const & tbinfo, GuiView & owner)
	: QToolBar(qt_(tbinfo.gui_name), &owner), owner_(owner)
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


void QLToolbar::add(ToolbarItem const & item)
{
	switch (item.type_) {
	case ToolbarItem::SEPARATOR:
		addSeparator();
		break;
	case ToolbarItem::LAYOUTS:
		layout_.reset(new QLayoutBox(this, owner_));
		break;
	case ToolbarItem::MINIBUFFER:
		owner_.addCommandBuffer(this);
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
		connect(this, SIGNAL(iconSizeChanged(const QSize &)),
			tb, SLOT(setIconSize(const QSize &)));
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
				Action * action = new Action(owner_,
					getIcon(it->func_),
					it->label_,
					it->func_,
					it->label_);
				panel->addButton(action);
				ActionVector.push_back(action);
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
		FileName icon_path = libFileSearch("images/math", item.name_, "xpm");
		tb->setIcon(QIcon(toqstr(icon_path.absFilename())));
		connect(this, SIGNAL(iconSizeChanged(const QSize &)),
			tb, SLOT(setIconSize(const QSize &)));

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
			if (!getStatus(it->func_).unknown()) {
				Action * action = new Action(owner_,
					getIcon(it->func_, false),
					it->label_,
					it->func_,
					it->label_);
				m->add(action);
				ActionVector.push_back(action);
			}
		tb->setMenu(m);
		addWidget(tb);
		break;
		}
	case ToolbarItem::COMMAND: {
		if (getStatus(item.func_).unknown())
			break;

		Action * action = new Action(owner_,
			getIcon(item.func_),
			item.label_,
			item.func_,
			item.label_);
		addAction(action);
		ActionVector.push_back(action);
		break;
		}
	default:
		break;
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


bool QLToolbar::isVisible() const
{
	return QToolBar::isVisible();
}


void QLToolbar::saveInfo(ToolbarSection::ToolbarInfo & tbinfo)
{
	// if tbinfo.state == auto *do not* set on/off
	if (tbinfo.state != ToolbarSection::ToolbarInfo::AUTO) {
		if (QLToolbar::isVisible())
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


void QLToolbar::update()
{
	// update visible toolbars only
	if (!isVisible())
		return;
	// This is a speed bottleneck because this is called on every keypress
	// and update calls getStatus, which copies the cursor at least two times
	for (size_t i = 0; i < ActionVector.size(); ++i)
		ActionVector[i]->update();

	// emit signal
	updated();
}


string const getIcon(FuncRequest const & f, bool unknown)
{
	using frontend::find_xpm;

	string fullname;

	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty())
			fullname = find_xpm(to_utf8(f.argument()).substr(1));
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		fullname = find_xpm(to_utf8(f.argument()));
		break;
	default:
		string const name = lyxaction.getActionName(f.action);
		string xpm_name(name);

		if (!f.argument().empty())
			xpm_name = subst(name + ' ' + to_utf8(f.argument()), ' ', '_');

		fullname = libFileSearch("images", xpm_name, "xpm").absFilename();

		if (fullname.empty()) {
			// try without the argument
			fullname = libFileSearch("images", name, "xpm").absFilename();
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
		return libFileSearch("images", "unknown", "xpm").absFilename();
	else
		return string();
}


} // namespace frontend
} // namespace lyx

#include "QLToolbar_moc.cpp"
