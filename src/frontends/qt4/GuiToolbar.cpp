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

#include "GuiView.h"
#include "GuiCommandBuffer.h"
#include "GuiToolbar.h"
#include "LyXAction.h"
#include "Action.h"
#include "qt_helpers.h"
#include "InsertTableWidget.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "IconPalette.h"
#include "Layout.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "TextClass.h"
#include "ToolbarBackend.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h" // sorted

#include <QComboBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QPixmap>

#include <boost/assert.hpp>

using namespace std;
using namespace lyx::support;

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

namespace {

struct PngMap {
	char const * key;
	char const * value;
};


bool operator<(PngMap const & lhs, PngMap const & rhs)
{
		return strcmp(lhs.key, rhs.key) < 0;
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
	{ "textrm \\O", "textrm_O"},
	{ "vDash", "vdash2" }
};

size_t const nr_sorted_png_map = sizeof(sorted_png_map) / sizeof(PngMap);


string const find_png(string const & name)
{
	PngMap const * const begin = sorted_png_map;
	PngMap const * const end = begin + nr_sorted_png_map;
	BOOST_ASSERT(sorted(begin, end));

	PngMap const * const it = find_if(begin, end, CompareKey(name));

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

	LYXERR(Debug::GUI, "find_png(" << name << ")\n"
		<< "Looking for math PNG called \"" << png_name << '"');
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
	string path;
	string fullname;

	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty()) {
			path = "math/";
			name1 = find_png(to_utf8(f.argument()).substr(1));
		}
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		path = "math/";
		name1 = find_png(to_utf8(f.argument()));
		break;
	case LFUN_CALL:
		path = "commands/";
		name1 = to_utf8(f.argument());
		break;
	default:
		name2 = lyxaction.getActionName(f.action);
		name1 = name2;

		if (!f.argument().empty())
			name1 = subst(name2 + ' ' + to_utf8(f.argument()), ' ', '_');
	}

	fullname = libFileSearch("images/" + path, name1, "png").absFilename();
	if (pm.load(toqstr(fullname)))
		return pm;

	fullname = libFileSearch("images/" + path, name2, "png").absFilename();
	if (pm.load(toqstr(fullname)))
		return pm;

	if (pm.load(":/images/" + toqstr(path + name1) + ".png"))
		return pm;

	if (pm.load(":/images/" + toqstr(path + name2) + ".png"))
		return pm;

	LYXERR(Debug::GUI, "Cannot find icon for command \""
			   << lyxaction.getActionName(f.action)
			   << '(' << to_utf8(f.argument()) << ")\"");
	if (unknown)
		pm.load(":/images/unknown.png");

	return pm;
}


/////////////////////////////////////////////////////////////////////
//
// GuiLayoutBox
//
/////////////////////////////////////////////////////////////////////

GuiLayoutBox::GuiLayoutBox(GuiView & owner)
	: owner_(owner)
{
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
	setFocusPolicy(Qt::ClickFocus);
	setMinimumWidth(sizeHint().width());
	setMaxVisibleItems(100);

	QObject::connect(this, SIGNAL(activated(QString)),
			 this, SLOT(selected(QString)));
	owner_.setLayoutDialog(this);
	updateContents(true);
}


void GuiLayoutBox::set(docstring const & layout)
{
	if (!text_class_)
		return;

	QString const & name = toqstr(translateIfPossible(
		(*text_class_)[layout]->name()));

	if (name == currentText())
		return;

	int i = findText(name);
	if (i == -1) {
		lyxerr << "Trying to select non existent layout type "
			<< fromqstr(name) << endl;
		return;
	}

	setCurrentIndex(i);
}


void GuiLayoutBox::addItemSort(QString const & item, bool sorted)
{
	//FIXME 
	//Since we are only storing the text used for display, we have no choice
	//below but to compare translated strings to figure out which layout the
	//user wants. This is not ideal. A better way is the way module names are 
	//handled in GuiDocument: viz, the untranslated name can be associated 
	//with the item via QComboBox::setItemData(). It may be that this can
	//even be done by passing: addItem(item, untransName).
	int const end = count();
	if (!sorted || end < 2 || item[0].category() != QChar::Letter_Uppercase) {
		addItem(item);
		return;
	}

	// Let the default one be at the beginning
	int i = 1;
	for (setCurrentIndex(i); currentText().localeAwareCompare(item) < 0; ) {
		// e.g. --Separator--
		if (currentText()[0].category() != QChar::Letter_Uppercase)
			break;
		if (++i == end)
			break;
		setCurrentIndex(i);
	}

	insertItem(i, item);
}


void GuiLayoutBox::updateContents(bool reset)
{
	Buffer const * buffer = owner_.buffer();
	if (!buffer) {
		clear();
		setEnabled(false);
		text_class_ = 0;
		inset_ = 0;
		return;
	}

	TextClass const * text_class = &buffer->params().textClass();
	Inset const * inset = 
		owner_.view()->cursor().innerParagraph().inInset();

	// we'll only update the layout list if the text class has changed
	// or we've moved from one inset to another
	if (!reset && text_class_ == text_class && inset_ == inset) {
		set(owner_.view()->cursor().innerParagraph().layout()->name());
		return;
	}

	inset_ = inset;
	text_class_ = text_class;

	clear();

	for (size_t i = 0; i != text_class_->layoutCount(); ++i) {
		Layout const & lt = *text_class_->layout(i);
		docstring const & name = lt.name();
		// if this inset requires the empty layout, we skip the default
		// layout
		if (name == text_class_->defaultLayoutName() && inset &&
		    (inset->forceEmptyLayout() || inset->useEmptyLayout()))
			continue;
		// if it doesn't require the empty layout, we skip it
		if (name == text_class_->emptyLayoutName() && inset &&
		    !inset->forceEmptyLayout() && !inset->useEmptyLayout())
			continue;
		addItemSort(toqstr(translateIfPossible(name)), lyxrc.sort_layouts);
	}

	set(owner_.view()->cursor().innerParagraph().layout()->name());

	// needed to recalculate size hint
	hide();
	setMinimumWidth(sizeHint().width());

	setEnabled(!buffer->isReadonly());
	show();
}


void GuiLayoutBox::selected(const QString & str)
{
	owner_.setFocus();
	updateContents(false);
	if (!text_class_)
		return;

	docstring const name = qstring_to_ucs4(str);
	for (size_t i = 0; i != text_class_->layoutCount(); ++i) {
		docstring const & itname = text_class_->layout(i)->name();
		// FIXME: Comparing translated strings is not ideal.
		// This should be done the way module names are handled
		// in GuiDocument: viz, the untranslated name should be
		// associated with the item via QComboBox::setItemData().
		if (translateIfPossible(itname) == name) {
			FuncRequest const func(LFUN_LAYOUT, itname,
					       FuncRequest::TOOLBAR);
			theLyXFunc().setLyXView(&owner_);
			lyx::dispatch(func);
			updateContents(false);
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


GuiToolbar::GuiToolbar(ToolbarInfo const & tbinfo, GuiView & owner)
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

namespace {

class PaletteButton : public QToolButton
{
private:
	GuiToolbar * bar_;
	ToolbarItem const & tbitem_;
	bool initialized_;
public:
	PaletteButton(GuiToolbar * bar, ToolbarItem const & item)
		: QToolButton(bar), bar_(bar), tbitem_(item), initialized_(false)
	{
		QString const label = qt_(to_ascii(tbitem_.label_));
		setToolTip(label);
		setStatusTip(label);
		setText(label);
		connect(bar_, SIGNAL(iconSizeChanged(QSize)),
			this, SLOT(setIconSize(QSize)));
		setCheckable(true);
		ToolbarInfo const * tbinfo = 
			toolbarbackend.getDefinedToolbarInfo(tbitem_.name_);
		if (tbinfo)
			// use the icon of first action for the toolbar button
			setIcon(getIcon(tbinfo->items.begin()->func_, true));
	}

	void mousePressEvent(QMouseEvent * e)
	{
		if (initialized_) {
			QToolButton::mousePressEvent(e);
			return;
		}

		initialized_ = true;

		ToolbarInfo const * tbinfo = 
			toolbarbackend.getDefinedToolbarInfo(tbitem_.name_);
		if (!tbinfo) {
			lyxerr << "Unknown toolbar " << tbitem_.name_ << endl;
			return;
		}
		IconPalette * panel = new IconPalette(this);
		QString const label = qt_(to_ascii(tbitem_.label_));
		panel->setWindowTitle(label);
		connect(this, SIGNAL(clicked(bool)), panel, SLOT(setVisible(bool)));
		connect(panel, SIGNAL(visible(bool)), this, SLOT(setChecked(bool)));
		ToolbarInfo::item_iterator it = tbinfo->items.begin();
		ToolbarInfo::item_iterator const end = tbinfo->items.end();
		for (; it != end; ++it)
			if (!getStatus(it->func_).unknown())
				panel->addButton(bar_->addItem(*it));

		QToolButton::mousePressEvent(e);
	}
};

class MenuButton : public QToolButton
{
private:
	GuiToolbar * bar_;
	ToolbarItem const & tbitem_;
	bool initialized_;
public:
	MenuButton(GuiToolbar * bar, ToolbarItem const & item)
		: QToolButton(bar), bar_(bar), tbitem_(item), initialized_(false)
	{
		setPopupMode(QToolButton::InstantPopup);
		QString const label = qt_(to_ascii(tbitem_.label_));
		setToolTip(label);
		setStatusTip(label);
		setText(label);
		setIcon(QPixmap(":images/math/" + toqstr(tbitem_.name_) + ".png"));
		connect(bar, SIGNAL(iconSizeChanged(QSize)),
			this, SLOT(setIconSize(QSize)));
	}

	void mousePressEvent(QMouseEvent * e)
	{
		if (initialized_) {
			QToolButton::mousePressEvent(e);
			return;
		}

		initialized_ = true;

		QString const label = qt_(to_ascii(tbitem_.label_));
		ButtonMenu * m = new ButtonMenu(label, this);
		m->setWindowTitle(label);
		m->setTearOffEnabled(true);
		connect(bar_, SIGNAL(updated()), m, SLOT(updateParent()));
		ToolbarInfo const * tbinfo = 
			toolbarbackend.getDefinedToolbarInfo(tbitem_.name_);
		if (!tbinfo) {
			lyxerr << "Unknown toolbar " << tbitem_.name_ << endl;
			return;
		}
		ToolbarInfo::item_iterator it = tbinfo->items.begin();
		ToolbarInfo::item_iterator const end = tbinfo->items.end();
		for (; it != end; ++it)
			if (!getStatus(it->func_).unknown())
				m->add(bar_->addItem(*it));
		setMenu(m);

		QToolButton::mousePressEvent(e);
	}
};

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
		QString const label = qt_(to_ascii(item.label_));
		tb->setToolTip(label);
		tb->setStatusTip(label);
		tb->setText(label);
		InsertTableWidget * iv = new InsertTableWidget(owner_, tb);
		connect(tb, SIGNAL(clicked(bool)), iv, SLOT(show(bool)));
		connect(iv, SIGNAL(visible(bool)), tb, SLOT(setChecked(bool)));
		connect(this, SIGNAL(updated()), iv, SLOT(updateParent()));
		addWidget(tb);
		break;
		}
	case ToolbarItem::ICONPALETTE:
		addWidget(new PaletteButton(this, item));
		break;

	case ToolbarItem::POPUPMENU: {
		addWidget(new MenuButton(this, item));
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

	if (layout_)
		layout_->setEnabled(lyx::getStatus(FuncRequest(LFUN_LAYOUT)).enabled());

	// emit signal
	updated();
}


} // namespace frontend
} // namespace lyx

#include "GuiToolbar_moc.cpp"
