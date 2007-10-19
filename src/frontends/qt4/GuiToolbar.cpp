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
#include "Layout.h"
#include "LyXFunc.h"
#include "ToolbarBackend.h"

#include "GuiView.h"
#include "GuiCommandBuffer.h"
#include "GuiToolbar.h"
#include "LyXAction.h"
#include "Action.h"
#include "qt_helpers.h"
#include "InsertTableWidget.h"

#include "support/lstrings.h"
#include "support/lyxalgo.h" // sorted

#include <QComboBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QPixmap>


static void initializeResources()
{
	static bool initialized = false;
	if (!initialized) {
		Q_INIT_RESOURCE(Resources); 
		initialized = true;
	}
}


namespace lyx {
namespace frontend {

using std::string;
using std::endl;

using support::subst;
using support::compare;


namespace {

struct PngMap {
	char const * key;
	char const * value;
};


bool operator<(PngMap const & lhs, PngMap const & rhs)
{
		return compare(lhs.key, rhs.key) < 0;
}


class CompareKey {
public:
	CompareKey(string const & name) : name_(name) {}
	bool operator()(PngMap const & other) const { return other.key == name_; }
private:
	string const name_;
};


PngMap sorted_png_map[] = {
	{ "Bumpeq", "bumpeq2" },
	{ "Cap", "cap2" },
	{ "Cup", "cup2" },
	{ "Delta", "delta2" },
	{ "Downarrow", "downarrow2" },
	{ "Gamma", "gamma2" },
	{ "Lambda", "lambda2" },
	{ "Leftarrow", "leftarrow2" },
	{ "Leftrightarrow", "leftrightarrow2" },
	{ "Longleftarrow", "longleftarrow2" },
	{ "Longleftrightarrow", "longleftrightarrow2" },
	{ "Longrightarrow", "longrightarrow2" },
	{ "Omega", "omega2" },
	{ "Phi", "phi2" },
	{ "Pi", "pi2" },
	{ "Psi", "psi2" },
	{ "Rightarrow", "rightarrow2" },
	{ "Sigma", "sigma2" },
	{ "Subset", "subset2" },
	{ "Supset", "supset2" },
	{ "Theta", "theta2" },
	{ "Uparrow", "uparrow2" },
	{ "Updownarrow", "updownarrow2" },
	{ "Upsilon", "upsilon2" },
	{ "Vdash", "vdash3" },
	{ "Xi", "xi2" },
	{ "nLeftarrow", "nleftarrow2" },
	{ "nLeftrightarrow", "nleftrightarrow2" },
	{ "nRightarrow", "nrightarrow2" },
	{ "nVDash", "nvdash3" },
	{ "nvDash", "nvdash2" },
	{ "textrm \\AA", "textrm_AA"},
	{ "textrm \\O", "textrm_Oe"},
	{ "vDash", "vdash2" }
};

size_t const nr_sorted_png_map = sizeof(sorted_png_map) / sizeof(PngMap);


string const find_png(string const & name)
{
	PngMap const * const begin = sorted_png_map;
	PngMap const * const end = begin + nr_sorted_png_map;
	BOOST_ASSERT(sorted(begin, end));

	PngMap const * const it = std::find_if(begin, end, CompareKey(name));

	string png_name;
	if (it != end)
		png_name = it->value;
	else {
		png_name = subst(name, "_", "underscore");
		png_name = subst(png_name, ' ', '_');

		// This way we can have "math-delim { }" on the toolbar.
		png_name = subst(png_name, "(", "lparen");
		png_name = subst(png_name, ")", "rparen");
		png_name = subst(png_name, "[", "lbracket");
		png_name = subst(png_name, "]", "rbracket");
		png_name = subst(png_name, "{", "lbrace");
		png_name = subst(png_name, "}", "rbrace");
		png_name = subst(png_name, "|", "bars");
		png_name = subst(png_name, ",", "thinspace");
		png_name = subst(png_name, ":", "mediumspace");
		png_name = subst(png_name, ";", "thickspace");
		png_name = subst(png_name, "!", "negthinspace");
	}

	LYXERR(Debug::GUI) << "find_png(" << name << ")\n"
			   << "Looking for math PNG called \""
			   << png_name << '"' << std::endl;
	return png_name;
}

} // namespace anon


/// return a icon for the given action
static QIcon getIcon(FuncRequest const & f, bool unknown)
{
	initializeResources();
	QPixmap pm;
	string name1;
	string name2;

	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty())
			name1 = "math/" + find_png(to_utf8(f.argument()).substr(1));
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		name1 = "math/" + find_png(to_utf8(f.argument()));
		break;
	default:
		name2 = lyxaction.getActionName(f.action);
		name1 = name2;

		if (!f.argument().empty())
			name1 = subst(name2 + ' ' + to_utf8(f.argument()), ' ', '_');
	}

	if (pm.load(":/images/" + toqstr(name1) + ".png"))
		return pm;

	if (pm.load(":/images/" + toqstr(name2) + ".png"))
		return pm;

	LYXERR(Debug::GUI) << "Cannot find icon for command \""
			   << lyxaction.getActionName(f.action)
			   << '(' << to_utf8(f.argument()) << ")\"" << endl;
	if (unknown)
		pm.load(":/images/unknown.png");

	return pm;
}


static TextClass const & textClass(LyXView const & lv)
{
	return lv.buffer()->params().getTextClass();
}


/////////////////////////////////////////////////////////////////////
//
// GuiLayoutBox
//
/////////////////////////////////////////////////////////////////////

GuiLayoutBox::GuiLayoutBox(GuiViewBase & owner)
	: owner_(owner)
{
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
	setFocusPolicy(Qt::ClickFocus);
	setMinimumWidth(sizeHint().width());
	setMaxVisibleItems(100);

	QObject::connect(this, SIGNAL(activated(QString)),
			 this, SLOT(selected(QString)));
}


void GuiLayoutBox::set(docstring const & layout)
{
	TextClass const & tc = textClass(owner_);

	QString const & name = toqstr(translateIfPossible(tc[layout]->name()));

	int i = 0;
	for (; i < count(); ++i) {
		if (name == itemText(i))
			break;
	}

	if (i == count()) {
		lyxerr << "Trying to select non existent layout type "
			<< fromqstr(name) << endl;
		return;
	}

	setCurrentIndex(i);
}


void GuiLayoutBox::updateContents()
{
	TextClass const & tc = textClass(owner_);

	setUpdatesEnabled(false);
	clear();

	TextClass::const_iterator it = tc.begin();
	TextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		// ignore obsolete entries
		addItem(toqstr(translateIfPossible((*it)->name())));
	}

	// needed to recalculate size hint
	hide();
	setMinimumWidth(sizeHint().width());
	show();

	setUpdatesEnabled(true);
}


void GuiLayoutBox::selected(const QString & str)
{
	owner_.setFocus();
	TextClass const & tc = owner_.buffer()->params().getTextClass();
	docstring const name = qstring_to_ucs4(str);
	TextClass::const_iterator it  = tc.begin();
	TextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		docstring const & itname = (*it)->name();
		if (translateIfPossible(itname) == name) {
			FuncRequest const func(LFUN_LAYOUT, itname,
					       FuncRequest::TOOLBAR);
			owner_.dispatch(func);
			return;
		}
	}
	lyxerr << "ERROR (layoutSelected): layout not found!" << endl;
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


Action * GuiToolbar::addItem(ToolbarItem const & item)
{
	Action * act = new Action(owner_,
		getIcon(item.func_, false),
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
		layout_ = new GuiLayoutBox(owner_);
		addWidget(layout_);
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
		tb->setIcon(getIcon(FuncRequest(LFUN_TABULAR_INSERT), true));
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
					tb->setIcon(getIcon(it->func_, true));
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
		tb->setIcon(QPixmap(":images/math/" + toqstr(item.name_) + ".png"));
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


void GuiToolbar::updateContents()
{
	// update visible toolbars only
	if (!isVisible())
		return;
	// This is a speed bottleneck because this is called on every keypress
	// and update calls getStatus, which copies the cursor at least two times
	for (int i = 0; i < actions_.size(); ++i)
		actions_[i]->update();

	// emit signal
	updated();
}


} // namespace frontend
} // namespace lyx

#include "GuiToolbar_moc.cpp"
