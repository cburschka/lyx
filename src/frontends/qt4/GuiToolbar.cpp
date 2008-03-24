/**
 * \file qt4/GuiToolbar.cpp
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

#include <QAbstractItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QComboBox>
#include <QFontMetrics>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QList>
#include <QListView>
#include <QPainter>
#include <QPixmap>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QTextFrame>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>

#include <boost/assert.hpp>

#include <map>
#include <vector>

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
	QString key;
	QString value;
};


bool operator<(PngMap const & lhs, PngMap const & rhs)
{
		return lhs.key < rhs.key;
}


class CompareKey {
public:
	CompareKey(QString const & name) : name_(name) {}
	bool operator()(PngMap const & other) const { return other.key == name_; }
private:
	QString const name_;
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


QString findPng(QString const & name)
{
	PngMap const * const begin = sorted_png_map;
	PngMap const * const end = begin + nr_sorted_png_map;
	BOOST_ASSERT(sorted(begin, end));

	PngMap const * const it = find_if(begin, end, CompareKey(name));

	QString png_name;
	if (it != end) {
		png_name = it->value;
	} else {
		png_name = name;
		png_name.replace('_', "underscore");
		png_name.replace(' ', '_');

		// This way we can have "math-delim { }" on the toolbar.
		png_name.replace('(', "lparen");
		png_name.replace(')', "rparen");
		png_name.replace('[', "lbracket");
		png_name.replace(']', "rbracket");
		png_name.replace('{', "lbrace");
		png_name.replace('}', "rbrace");
		png_name.replace('|', "bars");
		png_name.replace(',', "thinspace");
		png_name.replace(':', "mediumspace");
		png_name.replace(';', "thickspace");
		png_name.replace('!', "negthinspace");
	}

	LYXERR(Debug::GUI, "findPng(" << fromqstr(name) << ")\n"
		<< "Looking for math PNG called \"" << fromqstr(png_name) << '"');
	return png_name;
}

} // namespace anon


/// return a icon for the given action
static QIcon getIcon(FuncRequest const & f, bool unknown)
{
	initializeResources();
	QPixmap pm;
	QString name1;
	QString name2;
	QString path;
	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty()) {
			path = "math/";
			name1 = findPng(toqstr(f.argument()).mid(1));
		}
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		path = "math/";
		name1 = findPng(toqstr(f.argument()));
		break;
	case LFUN_CALL:
		path = "commands/";
		name1 = toqstr(f.argument());
		break;
	default:
		name2 = toqstr(lyxaction.getActionName(f.action));
		name1 = name2;

		if (!f.argument().empty()) {
			name1 = name2 + ' ' + toqstr(f.argument());
			name1.replace(' ', '_');
		}
	}

	string fullname = libFileSearch("images/" + path, name1, "png").absFilename();
	if (pm.load(toqstr(fullname)))
		return pm;

	fullname = libFileSearch("images/" + path, name2, "png").absFilename();
	if (pm.load(toqstr(fullname)))
		return pm;

	if (pm.load(":/images/" + path + name1 + ".png"))
		return pm;

	if (pm.load(":/images/" + path + name2 + ".png"))
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

class LayoutItemDelegate : public QItemDelegate {
public:
	///
	explicit LayoutItemDelegate(QObject * parent = 0)
		: QItemDelegate(parent)
	{}
	
	///
	void paint(QPainter * painter, QStyleOptionViewItem const & option,
		QModelIndex const & index) const
	{
		QStyleOptionViewItem opt = option;
		
		// default background
		painter->fillRect(opt.rect, opt.palette.color(QPalette::Base));
		
		// category header?
		if (lyxrc.group_layouts) {
			QSortFilterProxyModel const * model
			= static_cast<QSortFilterProxyModel const *>(index.model());
			
			QString stdCat = category(*model->sourceModel(), 0);
			QString cat = category(*index.model(), index.row());
			
			// not the standard layout and not the same as in the previous line?
			if (stdCat != cat
			    && (index.row() == 0 || cat != category(*index.model(), index.row() - 1))) {
				painter->save();
				
				// draw unselected background
				QStyle::State state = opt.state;
				opt.state = opt.state & ~QStyle::State_Selected;
				drawBackground(painter, opt, index);
				opt.state = state;
				
				// draw category header
				drawCategoryHeader(painter, opt, 
					category(*index.model(), index.row()));

				// move rect down below header
				opt.rect.setTop(opt.rect.top() + headerHeight(opt));
				
				painter->restore();
			}
		}

		QItemDelegate::paint(painter, opt, index);
	}
	
	///
	void drawDisplay(QPainter * painter, QStyleOptionViewItem const & opt,
			 const QRect & /*rect*/, const QString & text ) const
	{
		QString utext = underlineFilter(text);

		// Draw the rich text.
		painter->save();
		QColor col = opt.palette.text().color();
		if (opt.state & QStyle::State_Selected)
			col = opt.palette.highlightedText().color();
		QAbstractTextDocumentLayout::PaintContext context;
		context.palette.setColor(QPalette::Text, col);
		
		QTextDocument doc;
		doc.setDefaultFont(opt.font);
		doc.setHtml(utext);
		
		QTextFrameFormat fmt = doc.rootFrame()->frameFormat();
		fmt.setMargin(0);
		doc.rootFrame()->setFrameFormat(fmt);
		
		painter->translate(opt.rect.x() + 5,
			opt.rect.y() + (opt.rect.height() - opt.fontMetrics.height()) / 2);
		doc.documentLayout()->draw(painter, context);
		painter->restore();
	}
	
	///
	QSize sizeHint(QStyleOptionViewItem const & opt,
		QModelIndex const & index) const
	{
		GuiLayoutBox * combo = static_cast<GuiLayoutBox *>(parent());
		QSortFilterProxyModel const * model
		= static_cast<QSortFilterProxyModel const *>(index.model());	
		QSize size = QItemDelegate::sizeHint(opt, index);
		
		/// QComboBox uses the first row height to estimate the
		/// complete popup height during QComboBox::showPopup().
		/// To avoid scrolling we have to sneak in space for the headers.
		/// So we tweak this value accordingly. It's not nice, but the
		/// only possible way it seems.
		if (lyxrc.group_layouts && index.row() == 0 && combo->inShowPopup_) {
			int itemHeight = size.height();
			
			// we have to show \c cats many headers:
			unsigned cats = combo->visibleCategories_;
			
			// and we have \c n items to distribute the needed space over
			unsigned n = combo->model()->rowCount();
			
			// so the needed average height (rounded upwards) is:
			size.setHeight((headerHeight(opt) * cats + itemHeight * n + n - 1) / n); 
			return size;
		}

		// Add space for the category headers here?
		// Not for the standard layout though.
		QString stdCat = category(*model->sourceModel(), 0);
		QString cat = category(*index.model(), index.row());
		if (lyxrc.group_layouts && stdCat != cat
		    && (index.row() == 0 || cat != category(*index.model(), index.row() - 1))) {
			size.setHeight(size.height() + headerHeight(opt));
		}

		return size;
	}
	
private:
	///
	QString category(QAbstractItemModel const & model, int row) const
	{
		return model.data(model.index(row, 2), Qt::DisplayRole).toString();
	}
		
	///
	int headerHeight(QStyleOptionViewItem const & opt) const
	{
		return opt.fontMetrics.height() * 8 / 10;
	}
	///
	void drawCategoryHeader(QPainter * painter, QStyleOptionViewItem const & opt,
		QString const & category) const
	{
		// slightly blended color
		QColor lcol = opt.palette.text().color();
		lcol.setAlpha(127);
		painter->setPen(lcol);
		
		// set 80% scaled, bold font
		QFont font = opt.font;
		font.setBold(true);
		font.setWeight(QFont::Black);
		font.setPointSize(opt.font.pointSize() * 8 / 10);
		painter->setFont(font);
		
		// draw the centered text
		QFontMetrics fm(font);
		int w = fm.width(category);
		int x = opt.rect.x() + (opt.rect.width() - w) / 2;
		int y = opt.rect.y() + fm.ascent();
		int left = x;
		int right = x + w;
		painter->drawText(x, y, category);
		
		// the vertical position of the line: middle of lower case chars
		int ymid = y - 1 - fm.xHeight() / 2; // -1 for the baseline
		
		// draw the horizontal line
		if (!category.isEmpty()) {
			painter->drawLine(opt.rect.x(), ymid, left - 1, ymid);
			painter->drawLine(right + 1, ymid, opt.rect.right(), ymid);
		} else
			painter->drawLine(opt.rect.x(), ymid, opt.rect.right(), ymid);
	}

	
	///
	QString underlineFilter(QString const & s) const
	{
		// get filter
		GuiLayoutBox * p = static_cast<GuiLayoutBox *>(parent());
		QString const & f = p->filter();
		if (f.isEmpty())
			return s;
		
		// step through data item and put "(x)" for every matching character
		QString r;
		int lastp = -1;
		p->filter();
		for (int i = 0; i < f.length(); ++i) {
			int p = s.indexOf(f[i], lastp + 1, Qt::CaseInsensitive);
			BOOST_ASSERT(p != -1);
			if (lastp == p - 1 && lastp != -1) {
				// remove ")" and append "x)"
				r = r.left(r.length() - 4) + s[p] + "</u>";
			} else {
				// append "(x)"
				r += s.mid(lastp + 1, p - lastp - 1);
				r += QString("<u>") + s[p] + "</u>";
			}
			lastp = p;
		}
		r += s.mid(lastp + 1);
		return r;
	}
};


class GuiLayoutFilterModel : public QSortFilterProxyModel {
public:
	///
	GuiLayoutFilterModel(QObject * parent = 0)
		: QSortFilterProxyModel(parent)
	{}
	
	///
	void triggerLayoutChange()
	{
		layoutAboutToBeChanged();
		layoutChanged();
	}
};


GuiLayoutBox::GuiLayoutBox(GuiToolbar * bar, GuiView & owner)
	: owner_(owner), bar_(bar), lastSel_(-1),
	  layoutItemDelegate_(new LayoutItemDelegate(this)),
	  visibleCategories_(0), inShowPopup_(false)
{
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
	setFocusPolicy(Qt::ClickFocus);
	setMinimumWidth(sizeHint().width());
	setMaxVisibleItems(100);

	// set the layout model with two columns
	// 1st: translated layout names
	// 2nd: raw layout names
	model_ = new QStandardItemModel(0, 2, this);
	filterModel_ = new GuiLayoutFilterModel(this);
	filterModel_->setSourceModel(model_);
	setModel(filterModel_);

	// for the filtering we have to intercept characters
	view()->installEventFilter(this);
	view()->setItemDelegateForColumn(0, layoutItemDelegate_);
	
	QObject::connect(this, SIGNAL(activated(int)),
		this, SLOT(selected(int)));
	QObject::connect(bar_, SIGNAL(iconSizeChanged(QSize)),
		this, SLOT(setIconSize(QSize)));

	owner_.setLayoutDialog(this);
	updateContents(true);
}


void GuiLayoutBox::setFilter(QString const & s)
{
	bool enabled = view()->updatesEnabled();
	view()->setUpdatesEnabled(false);

	// remember old selection
	int sel = currentIndex();
	if (sel != -1)
		lastSel_ = filterModel_->mapToSource(filterModel_->index(sel, 0)).row();

	filter_ = s;
	filterModel_->setFilterRegExp(charFilterRegExp(filter_));
	countCategories();
	
	// restore old selection
	if (lastSel_ != -1) {
		QModelIndex i = filterModel_->mapFromSource(model_->index(lastSel_, 0));
		if (i.isValid())
			setCurrentIndex(i.row());
	}
	
	// Workaround to resize to content size
	// FIXME: There must be a better way. The QComboBox::AdjustToContents)
	//        does not help.
	if (view()->isVisible()) {
		// call QComboBox::showPopup. But set the inShowPopup_ flag to switch on
		// the hack in the item delegate to make space for the headers.
		// We do not call our implementation of showPopup because that
		// would reset the filter again. This is only needed if the user clicks
		// on the QComboBox.
		BOOST_ASSERT(!inShowPopup_);
		inShowPopup_ = true;
		QComboBox::showPopup();
		inShowPopup_ = false;

		// The item delegate hack is off again. So trigger a relayout of the popup.
		filterModel_->triggerLayoutChange();
		
		if (!s.isEmpty())
			owner_.message(_("Filtering layouts with \"" + fromqstr(s) + "\". "
					 "Press ESC to remove filter."));
		else
			owner_.message(_("Enter characters to filter the layout list."));
	}
	
	view()->setUpdatesEnabled(enabled);
}


void GuiLayoutBox::countCategories()
{
	int n = filterModel_->rowCount();
	visibleCategories_ = 0;
	if (n == 0 || !lyxrc.group_layouts)
		return;

	// skip the "Standard" category
	QString prevCat = model_->index(0, 2).data().toString(); 

	// count categories
	for (int i = 0; i < n; ++i) {
		QString cat = filterModel_->index(i, 2).data().toString();
		if (cat != prevCat)
			++visibleCategories_;
		prevCat = cat;
	}
}


QString GuiLayoutBox::charFilterRegExp(QString const & filter)
{
	QString re;
	for (int i = 0; i < filter.length(); ++i) {
		QChar c = filter[i];
		if (c.isLower())
			re += ".*[" + QRegExp::escape(c) + QRegExp::escape(c.toUpper()) + "]";
		else
			re += ".*" + QRegExp::escape(c);
	}
	return re;
}


void GuiLayoutBox::resetFilter()
{
	setFilter(QString());
}


void GuiLayoutBox::showPopup()
{
	owner_.message(_("Enter characters to filter the layout list."));

	bool enabled = view()->updatesEnabled();
	view()->setUpdatesEnabled(false);

	resetFilter();

	// call QComboBox::showPopup. But set the inShowPopup_ flag to switch on
	// the hack in the item delegate to make space for the headers.
	BOOST_ASSERT(!inShowPopup_);
	inShowPopup_ = true;
	QComboBox::showPopup();
	inShowPopup_ = false;
	
	// The item delegate hack is off again. So trigger a relayout of the popup.
	filterModel_->triggerLayoutChange();
	
	view()->setUpdatesEnabled(enabled);
}


bool GuiLayoutBox::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() != QEvent::KeyPress)
		return QComboBox::eventFilter(o, e);

	QKeyEvent * ke = static_cast<QKeyEvent*>(e);
	bool modified = (ke->modifiers() == Qt::ControlModifier)
		|| (ke->modifiers() == Qt::AltModifier)
		|| (ke->modifiers() == Qt::MetaModifier);
	
	switch (ke->key()) {
	case Qt::Key_Escape:
		if (!modified && !filter_.isEmpty()) {
			resetFilter();
			return true;
		}
		break;
	case Qt::Key_Backspace:
		if (!modified) {
			// cut off one character
			setFilter(filter_.left(filter_.length() - 1));
		}
		break;
	default:
		if (modified || ke->text().isEmpty())
			break;
		// find chars for the filter string
		QString s;
		for (int i = 0; i < ke->text().length(); ++i) {
			QChar c = ke->text()[i];
			if (c.isLetterOrNumber()
			    || c.isSymbol()
			    || c.isPunct()
			    || c.category() == QChar::Separator_Space) {
				s += c;
			}
		}
		if (!s.isEmpty()) {
			// append new chars to the filter string
			setFilter(filter_ + s);
			return true;
		}
		break;
	}

	return QComboBox::eventFilter(o, e);
}

	
void GuiLayoutBox::setIconSize(QSize size)
{
#ifdef Q_WS_MACX
	bool small = size.height() < 20;
	setAttribute(Qt::WA_MacSmallSize, small);
	setAttribute(Qt::WA_MacNormalSize, !small);
#endif
}


void GuiLayoutBox::set(docstring const & layout)
{
	resetFilter();
	
	if (!text_class_)
		return;

	QString const & name = toqstr((*text_class_)[layout].name());
	if (name == currentText())
		return;

	QList<QStandardItem *> r = model_->findItems(name, Qt::MatchExactly, 1);
	if (r.empty()) {
		lyxerr << "Trying to select non existent layout type "
			<< fromqstr(name) << endl;
		return;
	}

	setCurrentIndex(filterModel_->mapFromSource(r.first()->index()).row());
}


void GuiLayoutBox::addItemSort(docstring const & item, docstring const & category,
	bool sorted, bool sortedByCat)
{
	QString qitem = toqstr(item);
	QString titem = toqstr(translateIfPossible(item));
	QString qcat = toqstr(translateIfPossible(category));

	QList<QStandardItem *> row;
	row.append(new QStandardItem(titem));
	row.append(new QStandardItem(qitem));
	row.append(new QStandardItem(qcat));

	// the first entry is easy
	int const end = model_->rowCount();
	if (end == 0) {
		model_->appendRow(row);
		return;
	}

	// find category
	int i = 0;
	if (sortedByCat) {
		while (i < end && model_->item(i, 2)->text() != qcat)
			++i;
	}

	// skip the Standard layout
	if (i == 0)
		++i;
	
	// the simple unsorted case
	if (!sorted) {
		if (sortedByCat) {
			// jump to the end of the category group
			while (i < end && model_->item(i, 2)->text() == qcat)
				++i;
			model_->insertRow(i, row);
		} else
			model_->appendRow(row);
		return;
	}

	// find row to insert the item, after the separator if it exists
	if (i < end) {
		// find alphabetic position
		while (i != end
		       && model_->item(i, 0)->text().compare(titem) < 0 
		       && (!sortedByCat || model_->item(i, 2)->text() == qcat))
			++i;
	}

	model_->insertRow(i, row);
}


void GuiLayoutBox::updateContents(bool reset)
{
	resetFilter();
	
	Buffer const * buffer = owner_.buffer();
	if (!buffer) {
		model_->clear();
		setEnabled(false);
		text_class_ = 0;
		inset_ = 0;
		return;
	}

	// we'll only update the layout list if the text class has changed
	// or we've moved from one inset to another
	DocumentClass const * text_class = &buffer->params().documentClass();
	Inset const * inset = 
		owner_.view()->cursor().innerParagraph().inInset();
	if (!reset && text_class_ == text_class && inset_ == inset) {
		set(owner_.view()->cursor().innerParagraph().layout().name());
		return;
	}

	inset_ = inset;
	text_class_ = text_class;

	model_->clear();
	DocumentClass::const_iterator lit = text_class_->begin();
	DocumentClass::const_iterator len = text_class_->end();

	for (; lit != len; ++lit) {
		docstring const & name = lit->name();
		bool const useEmpty = inset_->forceEmptyLayout() || inset_->useEmptyLayout();
		// if this inset requires the empty layout, we skip the default
		// layout
		if (name == text_class_->defaultLayoutName() && inset_ && useEmpty)
			continue;
		// if it doesn't require the empty layout, we skip it
		if (name == text_class_->emptyLayoutName() && inset_ && !useEmpty)
			continue;
		addItemSort(name, lit->category(), lyxrc.sort_layouts, lyxrc.group_layouts);
	}

	set(owner_.view()->cursor().innerParagraph().layout().name());
	countCategories();
	
	// needed to recalculate size hint
	hide();
	setMinimumWidth(sizeHint().width());
	setEnabled(!buffer->isReadonly());
	show();
}


void GuiLayoutBox::selected(int index)
{
	// get selection
	QModelIndex mindex = filterModel_->mapToSource(filterModel_->index(index, 1));
	docstring const layoutName = 
		qstring_to_ucs4(model_->itemFromIndex(mindex)->text());

	owner_.setFocus();

	if (!text_class_) {
		updateContents(false);
		resetFilter();
		return;
	}

	// find corresponding text class
	if (text_class_->hasLayout(layoutName)) {
		FuncRequest const func(LFUN_LAYOUT, layoutName, FuncRequest::TOOLBAR);
		theLyXFunc().setLyXView(&owner_);
		lyx::dispatch(func);
		updateContents(false);
		resetFilter();
		return;
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
	Action * act = new Action(&owner_, getIcon(item.func_, false),
		toqstr(item.label_), item.func_, toqstr(item.label_), this);
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
		layout_ = new GuiLayoutBox(this, owner_);
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
