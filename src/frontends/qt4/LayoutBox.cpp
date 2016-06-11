/**
 * \file qt4/LayoutBox.cpp
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

#include "LayoutBox.h"

#include "GuiView.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "DocumentClassPtr.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "TextClass.h"

#include "insets/InsetText.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTextFrame>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

	
class LayoutItemDelegate : public QItemDelegate {
public:
	///
	explicit LayoutItemDelegate(LayoutBox * layout)
		: QItemDelegate(layout), layout_(layout)
	{}
	///
	void paint(QPainter * painter, QStyleOptionViewItem const & option,
		QModelIndex const & index) const;
	///
	void drawDisplay(QPainter * painter, QStyleOptionViewItem const & opt,
		const QRect & /*rect*/, const QString & text ) const;
	///
	QSize sizeHint(QStyleOptionViewItem const & opt,
		QModelIndex const & index) const;
	
private:
	///
	void drawCategoryHeader(QPainter * painter, QStyleOptionViewItem const & opt,
		QString const & category) const;	
	///
	QString underlineFilter(QString const & s) const;
	///
	LayoutBox * layout_;
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


/////////////////////////////////////////////////////////////////////
//
// LayoutBox::Private
//
/////////////////////////////////////////////////////////////////////

class LayoutBox::Private
{
	/// noncopyable
	Private(Private const &);
	void operator=(Private const &);
public:
	Private(LayoutBox * parent, GuiView & gv) : p(parent), owner_(gv),
		inset_(0),
		// set the layout model with two columns
		// 1st: translated layout names
		// 2nd: raw layout names
		model_(new QStandardItemModel(0, 2, p)),
		filterModel_(new GuiLayoutFilterModel(p)),
		lastSel_(-1),
		layoutItemDelegate_(new LayoutItemDelegate(parent)),
		visibleCategories_(0), inShowPopup_(false)
	{
		filterModel_->setSourceModel(model_);
	}

	void resetFilter() { setFilter(QString()); }
	///
	void setFilter(QString const & s);
	///
	void countCategories();
	///
	LayoutBox * p;
	///
	GuiView & owner_;
	///
	DocumentClassConstPtr text_class_;
	///
	Inset const * inset_;
	
	/// the layout model: 1st column translated, 2nd column raw layout name
	QStandardItemModel * model_;
	/// the proxy model filtering \c model_
	GuiLayoutFilterModel * filterModel_;
	/// the (model-) index of the last successful selection
	int lastSel_;
	/// the character filter
	QString filter_;
	///
	LayoutItemDelegate * layoutItemDelegate_;
	///
	unsigned visibleCategories_;
	///
	bool inShowPopup_;
};


static QString category(QAbstractItemModel const & model, int row)
{
	return model.data(model.index(row, 2), Qt::DisplayRole).toString();
}


static int headerHeight(QStyleOptionViewItem const & opt)
{
	return opt.fontMetrics.height() * 8 / 10;
}


void LayoutItemDelegate::paint(QPainter * painter, QStyleOptionViewItem const & option,
							   QModelIndex const & index) const
{
	QStyleOptionViewItem opt = option;

	// default background
	painter->fillRect(opt.rect, opt.palette.color(QPalette::Base));

	// category header?
	if (lyxrc.group_layouts) {
		QSortFilterProxyModel const * model =
			static_cast<QSortFilterProxyModel const *>(index.model());

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


void LayoutItemDelegate::drawDisplay(QPainter * painter, QStyleOptionViewItem const & opt,
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


QSize LayoutItemDelegate::sizeHint(QStyleOptionViewItem const & opt,
								   QModelIndex const & index) const
{
	QSortFilterProxyModel const * model =
		static_cast<QSortFilterProxyModel const *>(index.model());	
	QSize size = QItemDelegate::sizeHint(opt, index);

	/// QComboBox uses the first row height to estimate the
	/// complete popup height during QComboBox::showPopup().
	/// To avoid scrolling we have to sneak in space for the headers.
	/// So we tweak this value accordingly. It's not nice, but the
	/// only possible way it seems.
	if (lyxrc.group_layouts && index.row() == 0 && layout_->d->inShowPopup_) {
		int itemHeight = size.height();

		// we have to show \c cats many headers:
		unsigned cats = layout_->d->visibleCategories_;

		// and we have \c n items to distribute the needed space over
		unsigned n = layout_->model()->rowCount();

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


void LayoutItemDelegate::drawCategoryHeader(QPainter * painter, QStyleOptionViewItem const & opt,
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


QString LayoutItemDelegate::underlineFilter(QString const & s) const
{
	QString const & f = layout_->filter();
	if (f.isEmpty())
		return s;

	// step through data item and put "(x)" for every matching character
	QString r;
	int lastp = -1;
	layout_->filter();
	for (int i = 0; i < f.length(); ++i) {
		int p = s.indexOf(f[i], lastp + 1, Qt::CaseInsensitive);
		LASSERT(p != -1, continue);
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


static QString charFilterRegExp(QString const & filter)
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


void LayoutBox::Private::setFilter(QString const & s)
{
	// exit early if nothing has to be done
	if (filter_ == s)
		return;

	bool enabled = p->view()->updatesEnabled();
	p->view()->setUpdatesEnabled(false);

	// remember old selection
	int sel = p->currentIndex();
	if (sel != -1)
		lastSel_ = filterModel_->mapToSource(filterModel_->index(sel, 0)).row();

	filter_ = s;
	filterModel_->setFilterRegExp(charFilterRegExp(filter_));
	countCategories();
	
	// restore old selection
	if (lastSel_ != -1) {
		QModelIndex i = filterModel_->mapFromSource(model_->index(lastSel_, 0));
		if (i.isValid())
			p->setCurrentIndex(i.row());
	}
	
	// Workaround to resize to content size
	// FIXME: There must be a better way. The QComboBox::AdjustToContents)
	//        does not help.
	if (p->view()->isVisible()) {
		// call QComboBox::showPopup. But set the inShowPopup_ flag to switch on
		// the hack in the item delegate to make space for the headers.
		// We do not call our implementation of showPopup because that
		// would reset the filter again. This is only needed if the user clicks
		// on the QComboBox.
		LATTEST(!inShowPopup_);
		inShowPopup_ = true;
		p->QComboBox::showPopup();
		inShowPopup_ = false;

		// The item delegate hack is off again. So trigger a relayout of the popup.
		filterModel_->triggerLayoutChange();
		
		if (!s.isEmpty())
			owner_.message(bformat(_("Filtering layouts with \"%1$s\". "
						 "Press ESC to remove filter."),
					       qstring_to_ucs4(s)));
		else
			owner_.message(_("Enter characters to filter the layout list."));
	}
	
	p->view()->setUpdatesEnabled(enabled);
}


LayoutBox::LayoutBox(GuiView & owner)
	: d(new Private(this, owner))
{
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
	setFocusPolicy(Qt::ClickFocus);
	setMinimumWidth(sizeHint().width());
	setMaxVisibleItems(100);

	setModel(d->filterModel_);

	// for the filtering we have to intercept characters
	view()->installEventFilter(this);
	view()->setItemDelegateForColumn(0, d->layoutItemDelegate_);
	
	QObject::connect(this, SIGNAL(activated(int)),
		this, SLOT(selected(int)));

	updateContents(true);
}


LayoutBox::~LayoutBox() {
	delete d;
}


void LayoutBox::Private::countCategories()
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


void LayoutBox::showPopup()
{
	d->owner_.message(_("Enter characters to filter the layout list."));

	bool enabled = view()->updatesEnabled();
	view()->setUpdatesEnabled(false);

	d->resetFilter();

	// call QComboBox::showPopup. But set the inShowPopup_ flag to switch on
	// the hack in the item delegate to make space for the headers.
	LATTEST(!d->inShowPopup_);
	d->inShowPopup_ = true;
	QComboBox::showPopup();
	d->inShowPopup_ = false;
	
	// The item delegate hack is off again. So trigger a relayout of the popup.
	d->filterModel_->triggerLayoutChange();
	
	view()->setUpdatesEnabled(enabled);
}


bool LayoutBox::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() != QEvent::KeyPress)
		return QComboBox::eventFilter(o, e);

	QKeyEvent * ke = static_cast<QKeyEvent*>(e);
	bool modified = (ke->modifiers() == Qt::ControlModifier)
		|| (ke->modifiers() == Qt::AltModifier)
		|| (ke->modifiers() == Qt::MetaModifier);
	
	switch (ke->key()) {
	case Qt::Key_Escape:
		if (!modified && !d->filter_.isEmpty()) {
			d->resetFilter();
			return true;
		}
		break;
	case Qt::Key_Backspace:
		if (!modified) {
			// cut off one character
			d->setFilter(d->filter_.left(d->filter_.length() - 1));
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
			d->setFilter(d->filter_ + s);
			return true;
		}
		break;
	}

	return QComboBox::eventFilter(o, e);
}

	
void LayoutBox::setIconSize(QSize size)
{
#ifdef Q_OS_MAC
	bool small = size.height() < 20;
	setAttribute(Qt::WA_MacSmallSize, small);
	setAttribute(Qt::WA_MacNormalSize, !small);
#else
	(void)size; // suppress warning
#endif
}


void LayoutBox::set(docstring const & layout)
{
	d->resetFilter();

	if (!d->text_class_)
		return;

	if (!d->text_class_->hasLayout(layout))
		return;

	Layout const & lay = (*d->text_class_)[layout];
	QString newLayout = toqstr(lay.name());

	// If the layout is obsolete, use the new one instead.
	docstring const & obs = lay.obsoleted_by();
	if (!obs.empty())
		newLayout = toqstr(obs);

	int const curItem = currentIndex();
	QModelIndex const mindex =
		d->filterModel_->mapToSource(d->filterModel_->index(curItem, 1));
	QString const & currentLayout = d->model_->itemFromIndex(mindex)->text();
	if (newLayout == currentLayout) {
		LYXERR(Debug::GUI, "Already had " << newLayout << " selected.");
		return;
	}

	QList<QStandardItem *> r = d->model_->findItems(newLayout, Qt::MatchExactly, 1);
	if (r.empty()) {
		LYXERR0("Trying to select non existent layout type " << newLayout);
		return;
	}

	setCurrentIndex(d->filterModel_->mapFromSource(r.first()->index()).row());
}


void LayoutBox::addItemSort(docstring const & item, docstring const & category,
	bool sorted, bool sortedByCat, bool unknown)
{
	QString qitem = toqstr(item);
	docstring const loc_item = translateIfPossible(item);
	QString titem = unknown ? toqstr(bformat(_("%1$s (unknown)"), loc_item))
				: toqstr(loc_item);
	QString qcat = toqstr(translateIfPossible(category));

	QList<QStandardItem *> row;
	row.append(new QStandardItem(titem));
	row.append(new QStandardItem(qitem));
	row.append(new QStandardItem(qcat));

	// the first entry is easy
	int const end = d->model_->rowCount();
	if (end == 0) {
		d->model_->appendRow(row);
		return;
	}

	// find category
	int i = 0;
	if (sortedByCat) {
		while (i < end && d->model_->item(i, 2)->text() != qcat)
			++i;
	}

	// skip the Standard layout
	if (i == 0)
		++i;
	
	// the simple unsorted case
	if (!sorted) {
		if (sortedByCat) {
			// jump to the end of the category group
			while (i < end && d->model_->item(i, 2)->text() == qcat)
				++i;
			d->model_->insertRow(i, row);
		} else
			d->model_->appendRow(row);
		return;
	}

	// find row to insert the item, after the separator if it exists
	if (i < end) {
		// find alphabetic position
		while (i != end
		       && d->model_->item(i, 0)->text().localeAwareCompare(titem) < 0 
		       && (!sortedByCat || d->model_->item(i, 2)->text() == qcat))
			++i;
	}

	d->model_->insertRow(i, row);
}


void LayoutBox::updateContents(bool reset)
{
	d->resetFilter();
	BufferView const * bv = d->owner_.currentBufferView();
	if (!bv) {
		d->model_->clear();
		setEnabled(false);
		setMinimumWidth(sizeHint().width());
		d->text_class_.reset();
		d->inset_ = 0;
		return;
	}
	// we'll only update the layout list if the text class has changed
	// or we've moved from one inset to another
	DocumentClassConstPtr text_class = bv->buffer().params().documentClassPtr();
	Inset const * inset = &(bv->cursor().innerText()->inset());
	if (!reset && d->text_class_ == text_class && d->inset_ == inset) {
		set(bv->cursor().innerParagraph().layout().name());
		return;
	}

	d->inset_ = inset;
	d->text_class_ = text_class;

	d->model_->clear();
	DocumentClass::const_iterator lit = d->text_class_->begin();
	DocumentClass::const_iterator len = d->text_class_->end();

	for (; lit != len; ++lit) {
		docstring const & name = lit->name();
		bool const useEmpty = d->inset_->forcePlainLayout() || d->inset_->usePlainLayout();
		// if this inset requires the empty layout, we skip the default
		// layout
		if (name == d->text_class_->defaultLayoutName() && d->inset_ && useEmpty)
			continue;
		// if it doesn't require the empty layout, we skip it
		if (name == d->text_class_->plainLayoutName() && d->inset_ && !useEmpty)
			continue;
		// obsoleted layouts are skipped as well
		if (!lit->obsoleted_by().empty())
			continue;
		addItemSort(name, lit->category(), lyxrc.sort_layouts, 
				lyxrc.group_layouts, lit->isUnknown());
	}

	set(d->owner_.currentBufferView()->cursor().innerParagraph().layout().name());
	d->countCategories();

	setMinimumWidth(sizeHint().width());
	setEnabled(!bv->buffer().isReadonly() &&
		lyx::getStatus(FuncRequest(LFUN_LAYOUT)).enabled());
}


void LayoutBox::selected(int index)
{
	// get selection
	QModelIndex mindex = d->filterModel_->mapToSource(
		d->filterModel_->index(index, 1));
	docstring layoutName = qstring_to_ucs4(
		d->model_->itemFromIndex(mindex)->text());
	d->owner_.setFocus();

	if (!d->text_class_) {
		updateContents(false);
		d->resetFilter();
		return;
	}

	// find corresponding text class
	if (d->text_class_->hasLayout(layoutName)) {
		FuncRequest const func(LFUN_LAYOUT, layoutName, FuncRequest::TOOLBAR);
		lyx::dispatch(func);
		updateContents(false);
		d->resetFilter();
		return;
	}
	LYXERR0("ERROR (layoutSelected): layout " << layoutName << " not found!");
}


QString const & LayoutBox::filter() const
{
	return d->filter_;
}

} // namespace frontend
} // namespace lyx

#include "moc_LayoutBox.cpp"
