/**
 * \file qt4/CategorizedCombo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Stefan Schimanski
 * \author Jürgen Spitzmüller
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CategorizedCombo.h"

#include "qt_helpers.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <QAbstractTextDocumentLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTextFrame>

using namespace lyx::support;

namespace lyx {
namespace frontend {


class CCItemDelegate : public QItemDelegate {
public:
	///
	explicit CCItemDelegate(CategorizedCombo * cc)
		: QItemDelegate(cc), cc_(cc)
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
	CategorizedCombo * cc_;
};


class CCFilterModel : public QSortFilterProxyModel {
public:
	///
	CCFilterModel(QObject * parent = 0)
		: QSortFilterProxyModel(parent)
	{}
};


/////////////////////////////////////////////////////////////////////
//
// CategorizedCombo::Private
//
/////////////////////////////////////////////////////////////////////

struct CategorizedCombo::Private
{
	Private(CategorizedCombo * parent) : p(parent),
		// set the model with four columns
		// 1st: translated item names
		// 2nd: raw names
		// 3rd: category
		// 4th: availability (bool)
		model_(new QStandardItemModel(0, 4, p)),
		filterModel_(new CCFilterModel(p)),
		lastSel_(-1),
		CCItemDelegate_(new CCItemDelegate(parent)),
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
	CategorizedCombo * p;
	
	/** the layout model: 
	 * 1st column: translated GUI name,
	 * 2nd column: raw item name,
	 * 3rd column: category,
	 * 4th column: availability
	**/
	QStandardItemModel * model_;
	/// the proxy model filtering \c model_
	CCFilterModel * filterModel_;
	/// the (model-) index of the last successful selection
	int lastSel_;
	/// the character filter
	QString filter_;
	///
	CCItemDelegate * CCItemDelegate_;
	///
	unsigned visibleCategories_;
	///
	bool inShowPopup_;
};


static QString categoryCC(QAbstractItemModel const & model, int row)
{
	return model.data(model.index(row, 2), Qt::DisplayRole).toString();
}


static int headerHeightCC(QStyleOptionViewItem const & opt)
{
	return opt.fontMetrics.height();
}


void CCItemDelegate::paint(QPainter * painter, QStyleOptionViewItem const & option,
			   QModelIndex const & index) const
{
	QStyleOptionViewItem opt = option;

	// default background
	painter->fillRect(opt.rect, opt.palette.color(QPalette::Base));

	QString cat = categoryCC(*index.model(), index.row());

	// not the same as in the previous line?
	if (index.row() == 0 || cat != categoryCC(*index.model(), index.row() - 1)) {
		painter->save();

		// draw unselected background
		QStyle::State state = opt.state;
		opt.state = opt.state & ~QStyle::State_Selected;
		drawBackground(painter, opt, index);
		opt.state = state;

		// draw category header
		drawCategoryHeader(painter, opt, 
			categoryCC(*index.model(), index.row()));

		// move rect down below header
		opt.rect.setTop(opt.rect.top() + headerHeightCC(opt));

		painter->restore();
	}

	QItemDelegate::paint(painter, opt, index);
}


void CCItemDelegate::drawDisplay(QPainter * painter, QStyleOptionViewItem const & opt,
				 const QRect & /*rect*/, const QString & text) const
{
	QString utext = underlineFilter(text);

	// Draw the rich text.
	painter->save();
	QColor col = opt.palette.text().color();
	// grey out unavailable items
	if (text.startsWith(qt_("Unavailable:")))
		col = opt.palette.color(QPalette::Disabled, QPalette::Text);
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


QSize CCItemDelegate::sizeHint(QStyleOptionViewItem const & opt,
			       QModelIndex const & index) const
{
	QSize size = QItemDelegate::sizeHint(opt, index);

	/// QComboBox uses the first row height to estimate the
	/// complete popup height during QComboBox::showPopup().
	/// To avoid scrolling we have to sneak in space for the headers.
	/// So we tweak this value accordingly. It's not nice, but the
	/// only possible way it seems.
	// Add space for the category headers here
	QString cat = categoryCC(*index.model(), index.row());
	if (index.row() == 0 || cat != categoryCC(*index.model(), index.row() - 1)) {
		size.setHeight(size.height() + headerHeightCC(opt));
	}

	return size;
}


void CCItemDelegate::drawCategoryHeader(QPainter * painter, QStyleOptionViewItem const & opt,
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
	int y = opt.rect.y() + 3 * fm.ascent() / 2;
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


QString CCItemDelegate::underlineFilter(QString const & s) const
{
	QString const & f = cc_->filter();
	if (f.isEmpty())
		return s;

	// step through data item and put "(x)" for every matching character
	QString r;
	int lastp = -1;
	cc_->filter();
	for (int i = 0; i < f.length(); ++i) {
		int p = s.indexOf(f[i], lastp + 1, Qt::CaseInsensitive);
		LASSERT(p != -1, /**/);
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


static QString charFilterRegExpCC(QString const & filter)
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


void CategorizedCombo::Private::setFilter(QString const & s)
{
	bool enabled = p->view()->updatesEnabled();
	p->view()->setUpdatesEnabled(false);

	// remember old selection
	int sel = p->currentIndex();
	if (sel != -1)
		lastSel_ = filterModel_->mapToSource(filterModel_->index(sel, 0)).row();

	filter_ = s;
	filterModel_->setFilterRegExp(charFilterRegExpCC(filter_));
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
		LASSERT(!inShowPopup_, /**/);
		inShowPopup_ = true;
		p->QComboBox::showPopup();
		inShowPopup_ = false;
	}
	
	p->view()->setUpdatesEnabled(enabled);
}


CategorizedCombo::CategorizedCombo(QWidget * parent)
	: QComboBox(parent), d(new Private(this))
{
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
	setFocusPolicy(Qt::ClickFocus);
	setMinimumWidth(sizeHint().width());
	setMaxVisibleItems(100);

	setModel(d->filterModel_);

	// for the filtering we have to intercept characters
	view()->installEventFilter(this);
	view()->setItemDelegateForColumn(0, d->CCItemDelegate_);
	
	updateCombo();
}


CategorizedCombo::~CategorizedCombo() {
	delete d;
}


void CategorizedCombo::Private::countCategories()
{
	int n = filterModel_->rowCount();
	visibleCategories_ = 0;
	if (n == 0)
		return;

	QString prevCat = model_->index(0, 2).data().toString(); 

	// count categories
	for (int i = 1; i < n; ++i) {
		QString cat = filterModel_->index(i, 2).data().toString();
		if (cat != prevCat)
			++visibleCategories_;
		prevCat = cat;
	}
}


void CategorizedCombo::showPopup()
{
	bool enabled = view()->updatesEnabled();
	view()->setUpdatesEnabled(false);

	d->resetFilter();

	// call QComboBox::showPopup. But set the inShowPopup_ flag to switch on
	// the hack in the item delegate to make space for the headers.
	LASSERT(!d->inShowPopup_, /**/);
	d->inShowPopup_ = true;
	QComboBox::showPopup();
	d->inShowPopup_ = false;

	view()->setUpdatesEnabled(enabled);
}


bool CategorizedCombo::eventFilter(QObject * o, QEvent * e)
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


void CategorizedCombo::setIconSize(QSize size)
{
#ifdef Q_OS_MAC
	bool small = size.height() < 20;
	setAttribute(Qt::WA_MacSmallSize, small);
	setAttribute(Qt::WA_MacNormalSize, !small);
#else
	(void)size; // suppress warning
#endif
}


bool CategorizedCombo::set(QString const & item)
{
	d->resetFilter();

	int const curItem = currentIndex();
	QModelIndex const mindex =
		d->filterModel_->mapToSource(d->filterModel_->index(curItem, 1));
	QString const & currentItem = d->model_->itemFromIndex(mindex)->text();
	if (item == currentItem) {
		LYXERR(Debug::GUI, "Already had " << item << " selected.");
		return true;
	}

	QList<QStandardItem *> r = d->model_->findItems(item, Qt::MatchExactly, 1);
	if (r.empty()) {
		LYXERR0("Trying to select non existent layout type " << item);
		return false;
	}

	setCurrentIndex(d->filterModel_->mapFromSource(r.first()->index()).row());
	return true;
}


void CategorizedCombo::addItemSort(QString const & item, QString const & guiname,
				   QString const & category, QString const & tooltip,
				   bool sorted, bool sortedByCat, bool sortCats,
				   bool available)
{
	QString titem = available ? guiname
				  : toqstr(bformat(_("Unavailable: %1$s"),
						   qstring_to_ucs4(guiname)));
	bool const uncategorized = category.isEmpty();
	QString qcat = uncategorized ? qt_("Uncategorized") : category;

	QList<QStandardItem *> row;
	QStandardItem * gui = new QStandardItem(titem);
	if (!tooltip.isEmpty())
		gui->setToolTip(tooltip);
	row.append(gui);
	row.append(new QStandardItem(item));
	row.append(new QStandardItem(qcat));
	row.append(new QStandardItem(available));

	// the first entry is easy
	int const end = d->model_->rowCount();
	if (end == 0) {
		d->model_->appendRow(row);
		return;
	}

	// find category
	int i = 0;
	if (sortedByCat) {
		// If sortCats == true, sort categories alphabetically, uncategorized at the end.
		while (i < end && d->model_->item(i, 2)->text() != qcat
		       && (!sortCats 
			   || (!uncategorized && d->model_->item(i, 2)->text().localeAwareCompare(qcat) < 0
			       && d->model_->item(i, 2)->text() != qt_("Uncategorized"))
			   || (uncategorized && d->model_->item(i, 2)->text() != qt_("Uncategorized"))))
			++i;
	}

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
		// find alphabetic position, unavailable at the end
		while (i != end
		       && ((available && d->model_->item(i, 0)->text().localeAwareCompare(titem) < 0)
			   || ((!available && d->model_->item(i, 3))
			       || d->model_->item(i, 0)->text().localeAwareCompare(titem) < 0))
		       && (!sortedByCat || d->model_->item(i, 2)->text() == qcat))
			++i;
	}

	d->model_->insertRow(i, row);
}


QString CategorizedCombo::getData(int row) const
{
	int srow = d->filterModel_->mapToSource(d->filterModel_->index(row, 1)).row();
	return d->model_->data(d->model_->index(srow, 1), Qt::DisplayRole).toString();
}


void CategorizedCombo::reset()
{
	d->resetFilter();
	d->model_->clear();
}


void CategorizedCombo::updateCombo()
{
	d->countCategories();
	
	// needed to recalculate size hint
	hide();
	setMinimumWidth(sizeHint().width());
	show();
}


QString const & CategorizedCombo::filter() const
{
	return d->filter_;
}

}  // namespace frontend
}  // namespace lyx


#include "moc_CategorizedCombo.cpp"
