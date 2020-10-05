/**
 * \file qt/GuiToolbar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Stefan Schimanski
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiToolbar.h"

#include "Action.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "GuiApplication.h"
#include "GuiCommandBuffer.h"
#include "GuiView.h"
#include "IconPalette.h"
#include "InsertTableWidget.h"
#include "KeyMap.h"
#include "LayoutBox.h"
#include "LyX.h"
#include "LyXRC.h"
#include "qt_helpers.h"
#include "Session.h"
#include "Text.h"
#include "TextClass.h"
#include "Toolbars.h"

#include "insets/InsetText.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QSettings>
#include <QShowEvent>
#include <QString>
#include <QToolBar>
#include <QToolButton>

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiToolbar::GuiToolbar(ToolbarInfo const & tbinfo, GuiView & owner)
	: QToolBar(toqstr(tbinfo.gui_name), &owner), visibility_(0),
	  owner_(owner), command_buffer_(nullptr), tbinfo_(tbinfo), filled_(false),
	  restored_(false)
{
	setIconSize(owner.iconSize());
	connect(&owner, SIGNAL(iconSizeChanged(QSize)), this,
		SLOT(setIconSize(QSize)));

	// This is used by QMainWindow::restoreState for proper main window state
	// restoration.
	setObjectName(toqstr(tbinfo.name));
	restoreSession();
}


void GuiToolbar::setVisible(bool visible)
{
	// This is a hack to find out which toolbars have been restored by
	// MainWindow::restoreState and which toolbars should be initialized
	// by us (i.e., new toolbars)
	restored_ = true;
	// Record the actual visibility in toolbar state visibility_.
	// This is useful to restore the visibility of toolbars when
	// returning from full-screen. Therefore the recording is disabled
	// while LyX is in full-screen state.
	if (!owner_.isFullScreen()) {
		if (visible)
			visibility_ |= Toolbars::ON;
		else
			visibility_ &= ~Toolbars::ON;
	}
	QToolBar::setVisible(visible);
}


bool GuiToolbar::isRestored() const
{
	return restored_;
}


void GuiToolbar::fill()
{
	if (filled_)
		return;
	ToolbarInfo::item_iterator it = tbinfo_.items.begin();
	ToolbarInfo::item_iterator end = tbinfo_.items.end();
	for (; it != end; ++it)
		add(*it);
	filled_ = true;
}


void GuiToolbar::showEvent(QShowEvent * ev)
{
	fill();
	ev->accept();
}


void GuiToolbar::setVisibility(int visibility)
{
	visibility_ = visibility;
}


Action * GuiToolbar::addItem(ToolbarItem const & item)
{
	QString text = toqstr(item.label);
	// Get the keys bound to this action, but keep only the
	// first one later
	KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(*item.func);
	if (!bindings.empty())
		text += " [" + toqstr(bindings.begin()->print(KeySequence::ForGui)) + "]";

	Action * act = new Action(item.func, getIcon(*item.func, false), text,
							  text, this);
	if (item.type == ToolbarItem::BIDICOMMAND)
		act->setRtlIcon(getIcon(*item.func, false, true));

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
		QString const label = qt_(to_ascii(tbitem_.label));
		setToolTip(label);
		setStatusTip(label);
		setText(label);
		connect(bar_, SIGNAL(iconSizeChanged(QSize)),
			this, SLOT(setIconSize(QSize)));
		setCheckable(true);
		ToolbarInfo const * tbinfo = guiApp->toolbars().info(tbitem_.name);
		if (tbinfo)
			// use the icon of first action for the toolbar button
			setIcon(getIcon(*tbinfo->items.begin()->func, true));
	}

	void mousePressEvent(QMouseEvent * e) override
	{
		if (initialized_) {
			QToolButton::mousePressEvent(e);
			return;
		}

		initialized_ = true;

		ToolbarInfo const * tbinfo = guiApp->toolbars().info(tbitem_.name);
		if (!tbinfo) {
			LYXERR0("Unknown toolbar " << tbitem_.name);
			return;
		}
		IconPalette * panel = new IconPalette(this);
		QString const label = qt_(to_ascii(tbitem_.label));
		panel->setWindowTitle(label);
		connect(this, SIGNAL(clicked(bool)), panel, SLOT(setVisible(bool)));
		connect(panel, SIGNAL(visible(bool)), this, SLOT(setChecked(bool)));
		ToolbarInfo::item_iterator it = tbinfo->items.begin();
		ToolbarInfo::item_iterator const end = tbinfo->items.end();
		for (; it != end; ++it)
			if (!getStatus(*it->func).unknown())
				panel->addButton(bar_->addItem(*it));

		QToolButton::mousePressEvent(e);
	}
};

} // namespace


MenuButtonBase::MenuButtonBase(GuiToolbar * bar, ToolbarItem const & item)
	: QToolButton(bar), bar_(bar), tbitem_(item)
{
	setPopupMode(QToolButton::InstantPopup);
	QString const label = qt_(to_ascii(tbitem_.label));
	setToolTip(label);
	setStatusTip(label);
	setText(label);
	QString const name = toqstr(tbitem_.name);
	QStringList imagedirs;
	imagedirs << "images/math/" << "images/";
	for (int i = 0; i < imagedirs.size(); ++i) {
		QString imagedir = imagedirs.at(i);
		FileName const fname = imageLibFileSearch(imagedir, name, "svgz,png",
			theGuiApp()->imageSearchMode());
		if (fname.exists()) {
			setIcon(QIcon(getPixmap(imagedir, name, "svgz,png")));
			break;
		}
	}
}


void MenuButtonBase::actionTriggered(QAction * action)
{
	QToolButton::setDefaultAction(action);
	setPopupMode(QToolButton::DelayedPopup);
}


StaticMenuButton::StaticMenuButton(
    GuiToolbar * bar, ToolbarItem const & item, bool const sticky)
	: MenuButtonBase(bar, item)
{
	if (sticky)
		connect(this, SIGNAL(triggered(QAction *)),
			this, SLOT(actionTriggered(QAction *)));
	connect(bar, SIGNAL(iconSizeChanged(QSize)),
		this, SLOT(setIconSize(QSize)));
	initialize();
}


void StaticMenuButton::initialize()
{
	QString const label = qt_(to_ascii(tbitem_.label));
	ButtonMenu * m = new ButtonMenu(label, this);
	m->setWindowTitle(label);
	m->setTearOffEnabled(true);
	connect(bar_, SIGNAL(updated()), m, SLOT(updateParent()));
	connect(bar_, SIGNAL(updated()), this, SLOT(updateTriggered()));
	ToolbarInfo const * tbinfo = guiApp->toolbars().info(tbitem_.name);
	if (!tbinfo) {
		LYXERR0("Unknown toolbar " << tbitem_.name);
		return;
	}
	ToolbarInfo::item_iterator it = tbinfo->items.begin();
	ToolbarInfo::item_iterator const end = tbinfo->items.end();
	for (; it != end; ++it)
		if (!getStatus(*it->func).unknown())
			m->add(bar_->addItem(*it));
	setMenu(m);
}


void StaticMenuButton::updateTriggered()
{
	if (!menu())
		return;

	bool enabled = false;
	QList<QAction *> acts = menu()->actions();
	for (int i = 0; i < acts.size(); ++i)
		if (acts[i]->isEnabled()) {
			enabled = true;
			break;
		}
	// Enable the MenuButton if at least one menu item is enabled
	setEnabled(enabled);
	// If a disabled item is default, switch to InstantPopup
	// (this can happen if a user selects e.g. DVI and then
	// turns non-TeX fonts on)
	if (defaultAction() && !defaultAction()->isEnabled())
		setPopupMode(QToolButton::InstantPopup);
}


class DynamicMenuButton::Private
{
	/// noncopyable
	Private(Private const &);
	void operator=(Private const &);
public:
	Private() : inset_(nullptr) {}
	///
	DocumentClassConstPtr text_class_;
	///
	InsetText const * inset_;
};


DynamicMenuButton::DynamicMenuButton(GuiToolbar * bar, ToolbarItem const & item)
	: MenuButtonBase(bar, item), d(new Private())
{
	initialize();
}


DynamicMenuButton::~DynamicMenuButton() 
{ 
	delete d;
}


void DynamicMenuButton::initialize()
{
	QString const label = qt_(to_ascii(tbitem_.label));
	ButtonMenu * m = new ButtonMenu(label, this);
	m->setWindowTitle(label);
	m->setTearOffEnabled(true);
	connect(bar_, SIGNAL(updated()), m, SLOT(updateParent()));
	connect(bar_, SIGNAL(updated()), this, SLOT(updateTriggered()));
	connect(bar_, SIGNAL(iconSizeChanged(QSize)),
		this, SLOT(setIconSize(QSize)));
	setMenu(m);
}


bool DynamicMenuButton::isMenuType(string const & s)
{
	return s == "dynamic-custom-insets"
		|| s == "dynamic-char-styles"
		|| s == "textstyle-apply"
		|| s == "paste";
}


void DynamicMenuButton::updateTriggered()
{
	QMenu * m = menu();
	// the menu should exist by this point
	// if not, we can at least avoid crashing in release mode
	LASSERT(m, return);
	GuiView const & owner = bar_->owner();
	BufferView const * bv = owner.currentBufferView();

	string const & menutype = tbitem_.name;
	if (menutype == "dynamic-custom-insets" || menutype == "dynamic-char-styles") {
		if (!bv) {
			m->clear();
			setEnabled(false);
			setMinimumWidth(sizeHint().width());
			d->text_class_.reset();
			d->inset_ = nullptr;
			return;
		}
		DocumentClassConstPtr text_class =
				bv->buffer().params().documentClassPtr();
		InsetText const * inset = &(bv->cursor().innerText()->inset());
		// if the text class has changed, then we need to reload the menu
		if (d->text_class_ != text_class) {
			d->text_class_ = text_class;
			// at the moment, we can just call loadFlexInsets, and it will
			// handle both types. if there were more types of menus, then we
			// might need to have other options.
			loadFlexInsets();
		}
		// remember where we are
		d->inset_ = inset;
		// note that enabling here might need to be more subtle if there
		// were other kinds of menus.
		setEnabled(!bv->buffer().isReadonly()
			   && !m->isEmpty()
			   && inset->insetAllowed(FLEX_CODE));
	} else if (menutype == "textstyle-apply") {
		m->clear();
		setPopupMode(QToolButton::MenuButtonPopup);
		if (!bv) {
			QToolButton::setIcon(getIcon(FuncRequest(LFUN_TEXTSTYLE_APPLY), false));
			setEnabled(false);
			return;
		}
		vector<docstring> ffList = bv->cursor().innerText()->getFreeFonts();
		unsigned int i = 0;
		Action * default_act = nullptr;
		for (auto const & f : ffList) {
			FuncRequest func(LFUN_TEXTSTYLE_APPLY, convert<docstring>(i),
					 FuncRequest::TOOLBAR);
			docstring const lb = char_type('&') + convert<docstring>(i)
				+ from_ascii(". ") + f ;
			Action * act = new Action(func, QIcon(), toqstr(lb), toqstr(f), this);
			m->addAction(act);
			// The most recent one is the default
			if (i == 0)
				default_act = act;
			++i;
		}
		// Add item to reset to defaults
		Action * reset_act = new Action(FuncRequest(LFUN_FONT_DEFAULT, FuncRequest::TOOLBAR),
						getIcon(FuncRequest(LFUN_UNDO), false),
						qt_("&Reset to default"),
						qt_("Reset all font settings to their defaults"), this);
		m->addAction(reset_act);
		if (default_act)
			QToolButton::setDefaultAction(default_act);
		QToolButton::setIcon(getIcon(FuncRequest(LFUN_TEXTSTYLE_APPLY), false));
		setEnabled(lyx::getStatus(FuncRequest(LFUN_TEXTSTYLE_APPLY)).enabled()
			   || lyx::getStatus(FuncRequest(LFUN_FONT_DEFAULT)).enabled());
	} else if (menutype == "paste") {
		m->clear();
		setPopupMode(QToolButton::MenuButtonPopup);
		Action * default_action = new Action(FuncRequest(LFUN_PASTE),
						     getIcon(FuncRequest(LFUN_PASTE), false),
						     qt_("Paste"), qt_("Paste"), this);
		if (!bv) {
			setEnabled(false);
			QToolButton::setDefaultAction(default_action);
			return;
		}
		docstring_list const sel = cap::availableSelections(&bv->buffer());

		docstring_list::const_iterator cit = sel.begin();
		docstring_list::const_iterator end = sel.end();

		for (unsigned int index = 0; cit != end; ++cit, ++index) {
			docstring const s = *cit;
			FuncRequest func(LFUN_PASTE, convert<docstring>(index),
					 FuncRequest::TOOLBAR);
			docstring const lb = char_type('&') + convert<docstring>(index)
				+ from_ascii(". ") + s ;
			Action * act = new Action(func, QIcon(), toqstr(lb), toqstr(s), this);
			m->addAction(act);
		}
		QToolButton::setDefaultAction(default_action);
		setEnabled(lyx::getStatus(FuncRequest(LFUN_PASTE)).enabled());
	}
}


void DynamicMenuButton::loadFlexInsets()
{
	QMenu * m = menu();
	m->clear();
	string const & menutype = tbitem_.name;
	InsetLayout::InsetLyXType ftype;
	if (menutype == "dynamic-custom-insets")
		ftype = InsetLayout::CUSTOM;
	else if (menutype == "dynamic-char-styles")
		ftype = InsetLayout::CHARSTYLE;
	else {
		// this should have been taken care of earlier
		LASSERT(false, return);
	}

	TextClass::InsetLayouts const & inset_layouts = 
			d->text_class_->insetLayouts();
	for (auto const & iit : inset_layouts) {
		InsetLayout const & il = iit.second;
		if (il.lyxtype() != ftype)
			continue;
		docstring const name = iit.first;
		QString const loc_item = toqstr(translateIfPossible(
				prefixIs(name, from_ascii("Flex:")) ? 
				name.substr(5) : name));
		FuncRequest func(LFUN_FLEX_INSERT, 
				from_ascii("\"") + name + from_ascii("\""), FuncRequest::TOOLBAR);
		Action * act = 
				new Action(func, getIcon(func, false), loc_item, loc_item, this);
		m->addAction(act);
	}
}


void GuiToolbar::add(ToolbarItem const & item)
{
	switch (item.type) {
	case ToolbarItem::SEPARATOR:
		addSeparator();
		break;
	case ToolbarItem::LAYOUTS: {
		LayoutBox * layout = owner_.getLayoutDialog();
		QObject::connect(this, SIGNAL(iconSizeChanged(QSize)),
			layout, SLOT(setIconSize(QSize)));
		QAction * action = addWidget(layout);
		action->setVisible(true);
		break;
	}
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
		QString const label = qt_(to_ascii(item.label));
		tb->setToolTip(label);
		tb->setStatusTip(label);
		tb->setText(label);
		InsertTableWidget * iv = new InsertTableWidget(tb);
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
		addWidget(new StaticMenuButton(this, item, false));
		break;
		}
	case ToolbarItem::STICKYPOPUPMENU: {
		addWidget(new StaticMenuButton(this, item, true));
		break;
		}
	case ToolbarItem::DYNAMICMENU: {
		// we only handle certain things
		if (DynamicMenuButton::isMenuType(item.name))
			addWidget(new DynamicMenuButton(this, item));
		else
			LYXERR0("Unknown dynamic menu type: " << item.name);
		break;
	}
	case ToolbarItem::BIDICOMMAND: {
		if (!getStatus(*item.func).unknown())
			addAction(addItem(item));
		break;
		}
	case ToolbarItem::COMMAND: {
		if (!getStatus(*item.func).unknown())
			addAction(addItem(item));
		break;
		}
	default:
		break;
	}
}


void GuiToolbar::update(int context)
{
	if (visibility_ & Toolbars::AUTO) {
		setVisible(visibility_ & context & Toolbars::ALLOWAUTO);
		if (isVisible() && commandBuffer() && (context & Toolbars::MINIBUFFER_FOCUS))
			commandBuffer()->setFocus();
	}

	// update visible toolbars only
	if (!isVisible())
		return;

	// This is a speed bottleneck because this is called on every keypress
	// and update calls getStatus, which copies the cursor at least two times
	for (int i = 0; i < actions_.size(); ++i)
		actions_[i]->update();

	LayoutBox * layout = owner_.getLayoutDialog();
	if (layout)
		layout->setEnabled(lyx::getStatus(FuncRequest(LFUN_LAYOUT)).enabled());

	// emit signal
	updated();
}


QString GuiToolbar::sessionKey() const
{
	return "views/" + QString::number(owner_.id()) + "/" + objectName();
}


void GuiToolbar::saveSession(QSettings & settings) const
{
	settings.setValue(sessionKey() + "/visibility", visibility_);
	settings.setValue(sessionKey() + "/movability", isMovable());
}


void GuiToolbar::restoreSession()
{
	QSettings settings;
	int const error_val = -1;
	int visibility =
		settings.value(sessionKey() + "/visibility", error_val).toInt();
	if (visibility == error_val || visibility == 0) {
		// This should not happen, but in case we use the defaults
		LYXERR(Debug::GUI, "Session settings could not be found! Defaults are used instead.");
		visibility =
			guiApp->toolbars().defaultVisibility(fromqstr(objectName()));
	}
	setVisibility(visibility);

	int movability = settings.value(sessionKey() + "/movability", true).toBool();
	setMovable(movability);
}


bool GuiToolbar::isVisibiltyOn() const
{
	return visibility_ & Toolbars::ON;
}


void GuiToolbar::toggle()
{
	docstring state;
	if (visibility_ & Toolbars::ALLOWAUTO) {
		if (!(visibility_ & Toolbars::AUTO)) {
			visibility_ |= Toolbars::AUTO;
			hide();
			state = _("auto");
		} else {
			visibility_ &= ~Toolbars::AUTO;
			if (isVisible()) {
				hide();
				state = _("off");
			} else {
				show();
				state = _("on");
			}
		}
	} else {
		if (isVisible()) {
			hide();
			state = _("off");
		} else {
			show();
			state = _("on");
		}
	}

	owner_.message(bformat(_("Toolbar \"%1$s\" state set to %2$s"),
		qstring_to_ucs4(windowTitle()), state));
}

void GuiToolbar::movable(bool silent)
{
	// toggle movability
	setMovable(!isMovable());

	// manual update avoids bug in qt that the drag handle is not removed
	// properly, e.g. in Windows
	Q_EMIT update();

	// silence for toggling of many toolbars for performance
	if (!silent) {
		docstring state;
		if (isMovable())
			state = _("movable");
		else
			state = _("immovable");
		owner_.message(bformat(_("Toolbar \"%1$s\" state set to %2$s"),
			qstring_to_ucs4(windowTitle()), state));
	}
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiToolbar.cpp"
