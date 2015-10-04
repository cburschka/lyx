/**
 * \file GuiCompleter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCompleter.h"

#include "Buffer.h"
#include "BufferView.h"
#include "CompletionList.h"
#include "Cursor.h"
#include "Dimension.h"
#include "GuiWorkArea.h"
#include "GuiView.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "qt_helpers.h"
#include "version.h"

#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/debug.h"

#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>
#include <QItemDelegate>
#include <QTreeView>
#include <QTimer>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

class CompleterItemDelegate : public QItemDelegate
{
public:
	explicit CompleterItemDelegate(QObject * parent)
		: QItemDelegate(parent)
	{}

	~CompleterItemDelegate()
	{}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		   const QModelIndex &index) const
	{
		if (index.column() == 0) {
			QItemDelegate::paint(painter, option, index);
			return;
		}
		QStyleOptionViewItem opt = setOptions(index, option);
		QVariant value = index.data(Qt::DisplayRole);
		QPixmap pixmap = qvariant_cast<QPixmap>(value);
		
		// draw
		painter->save();
		drawBackground(painter, opt, index);
		if (!pixmap.isNull()) {
			const QSize size = pixmap.size();
			painter->drawPixmap(option.rect.left() + (16 - size.width()) / 2,
				option.rect.top() + (option.rect.height() - size.height()) / 2,
				pixmap);
		}
		drawFocus(painter, opt, option.rect);
		painter->restore();
	}
};

class GuiCompletionModel : public QAbstractListModel
{
public:
	///
	GuiCompletionModel(QObject * parent, CompletionList const * l)
		: QAbstractListModel(parent), list_(l)
	{}
	///
	~GuiCompletionModel() { delete list_; }
	///
	void setList(CompletionList const * l) {
		beginResetModel();
		delete list_;
		list_ = l;
		endResetModel();
	}
	///
	bool sorted() const
	{
		if (list_)
			return list_->sorted();
		return false;
	}
	///
	int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const
	{
		return 2;
	}
	///
	int rowCount(const QModelIndex & /*parent*/ = QModelIndex()) const
	{
		if (list_ == 0)
			return 0;
		return list_->size();
	}

	///
	QVariant data(const QModelIndex & index, int role) const
	{
		if (list_ == 0)
			return QVariant();

		if (index.row() < 0 || index.row() >= rowCount())
			return QVariant();

		if (role != Qt::DisplayRole && role != Qt::EditRole)
		    return QVariant();
		    
		if (index.column() == 0)
			return toqstr(list_->data(index.row()));

		if (index.column() != 1)
			return QVariant();
	
		// get icon from cache
		QPixmap scaled;
		QString const name = ":" + toqstr(list_->icon(index.row()));
		if (name == ":")
			return scaled;
		if (!QPixmapCache::find("completion" + name, scaled)) {
			// load icon from disk
			QPixmap p = QPixmap(name);
			if (!p.isNull()) {
				// scale it to 16x16 or smaller
				scaled = p.scaled(min(16, p.width()), min(16, p.height()), 
					Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}
			QPixmapCache::insert("completion" + name, scaled);
		}
		return scaled;
	}

private:
	/// owned by us
	CompletionList const * list_;
};


GuiCompleter::GuiCompleter(GuiWorkArea * gui, QObject * parent)
	: QCompleter(parent), gui_(gui), old_cursor_(0), updateLock_(0),
	  inlineVisible_(false), popupVisible_(false),
	  modelActive_(false)
{
	// Setup the completion popup
	model_ = new GuiCompletionModel(this, 0);
	setModel(model_);
	setCompletionMode(QCompleter::PopupCompletion);
	setCaseSensitivity(Qt::CaseSensitive);
	setWidget(gui_);
	
	// create the popup
	QTreeView *listView = new QTreeView;
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	listView->setSelectionBehavior(QAbstractItemView::SelectRows);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->header()->hide();
	listView->setIndentation(0);
	listView->setUniformRowHeights(true);
	setPopup(listView);
	
	itemDelegate_ = new CompleterItemDelegate(this);
	popup()->setItemDelegate(itemDelegate_);
	
	// create timeout timers
	popup_timer_.setSingleShot(true);
	inline_timer_.setSingleShot(true);
	connect(this, SIGNAL(highlighted(const QString &)),
		this, SLOT(popupHighlighted(const QString &)));
	connect(this, SIGNAL(activated(const QString &)),
		this, SLOT(popupActivated(const QString &)));
	connect(&popup_timer_, SIGNAL(timeout()),
		this, SLOT(showPopup()));
	connect(&inline_timer_, SIGNAL(timeout()),
		this, SLOT(showInline()));
}


GuiCompleter::~GuiCompleter()
{
	popup()->hide();
}


bool GuiCompleter::eventFilter(QObject * watched, QEvent * e)
{
	// hijack back the tab key from the popup
	// (which stole it from the workspace before)
	if (e->type() == QEvent::KeyPress && popupVisible()) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(e);
		switch (ke->key()) {
		case Qt::Key_Tab:
			tab();
			ke->accept();
			return true;
		default: break;
		}
	}
	
	return QCompleter::eventFilter(watched, e);
}


bool GuiCompleter::popupPossible(Cursor const & cur) const
{
	return QApplication::activeWindow()
		&& gui_->hasFocus()
		&& cur.inset().completionSupported(cur);
}


bool GuiCompleter::inlinePossible(Cursor const & cur) const
{
	return cur.inset().inlineCompletionSupported(cur);
}


bool GuiCompleter::uniqueCompletionAvailable() const
{
	if (!modelActive_)
		return false;

	size_t n = popup()->model()->rowCount();
	if (n > 1 || n == 0)
		return false;

	// if there is exactly one, we have to check whether it is a 
	// real completion, i.e. longer than the current prefix.
	if (completionPrefix() == currentCompletion())
		return false;

	return true;
}


bool GuiCompleter::completionAvailable() const
{
	if (!modelActive_)
		return false;

	size_t n = popup()->model()->rowCount();

	// if there is exactly one, we have to check whether it is a 
	// real completion, i.e. longer than the current prefix.
	if (n == 1 && completionPrefix() == currentCompletion())
	    return false;

	return n > 0;
}


bool GuiCompleter::popupVisible() const
{
	return popupVisible_;
}


bool GuiCompleter::inlineVisible() const
{
	// In fact using BufferView::inlineCompletionPos.empty() should be
	// here. But unfortunately this information is not good enough
	// because destructive operations like backspace might invalidate
	// inlineCompletionPos. But then the completion should stay visible
	// (i.e. reshown on the next update). Hence be keep this information
	// in the inlineVisible_ variable.
	return inlineVisible_;
}


void GuiCompleter::updateVisibility(Cursor & cur, bool start, bool keep)
{
	// parameters which affect the completion
	bool moved = cur != old_cursor_;
	if (moved)
		old_cursor_ = cur;

	bool const possiblePopupState = popupPossible(cur);
	bool const possibleInlineState = inlinePossible(cur);

	// we moved or popup state is not ok for popup?
	if ((moved && !keep) || !possiblePopupState)
		hidePopup();

	// we moved or inline state is not ok for inline completion?
	if ((moved && !keep) || !possibleInlineState)
		hideInline(cur);

	// we inserted something and are in a possible popup state?
	if (!popupVisible() && possiblePopupState && start
		&& cur.inset().automaticPopupCompletion())
		popup_timer_.start(int(lyxrc.completion_popup_delay * 1000));

	// we inserted something and are in a possible inline completion state?
	if (!inlineVisible() && possibleInlineState && start
		&& cur.inset().automaticInlineCompletion())
		inline_timer_.start(int(lyxrc.completion_inline_delay * 1000));
	else if (cur.inMathed() && !lyxrc.completion_inline_math) {
		// no inline completion, hence a metrics update is needed
		if (!(cur.result().screenUpdate() & Update::Force))
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
	}

	// update prefix if any completion is possible
	bool modelActive = modelActive_ && model()->rowCount() > 0;
	if (possiblePopupState || possibleInlineState) {
		if (modelActive)
			updatePrefix(cur);
		else
			updateAvailability();
	}
}


void GuiCompleter::updateVisibility(bool start, bool keep)
{
	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);
	
	updateVisibility(cur, start, keep);
	
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
}


void GuiCompleter::updatePrefix(Cursor const & cur)
{
	// get new prefix. Do nothing if unchanged
	QString newPrefix = toqstr(cur.inset().completionPrefix(cur));
	if (newPrefix == completionPrefix())
		return;
	
	// value which should be kept selected
	QString old = currentCompletion();
	if (old.length() == 0)
		old = last_selection_;
	
	// update completer to new prefix
	setCompletionPrefix(newPrefix);

	// update popup because its size might have changed
	if (popupVisible())
		updatePopup(cur);

	// restore old selection
	setCurrentCompletion(old);
	
	// if popup is not empty, the new selection will
	// be our last valid one
	QString const & s = currentCompletion();
	if (popupVisible() || inlineVisible()) {
		if (s.length() > 0)
			last_selection_ = s;
		else
			last_selection_ = old;
	}

	// update inline completion because the default
	// completion string might have changed
	if (inlineVisible())
		updateInline(cur, s);
}


void GuiCompleter::updateInline(Cursor const & cur, QString const & completion)
{
	if (!cur.inset().inlineCompletionSupported(cur))
		return;
	
	// compute postfix
	docstring prefix = cur.inset().completionPrefix(cur);
	docstring postfix = qstring_to_ucs4(completion.mid(prefix.length()));
	
	// shorten it if necessary
	if (lyxrc.completion_inline_dots != -1)
		support::truncateWithEllipsis(postfix,
									  unsigned(lyxrc.completion_inline_dots));

	// set inline completion at cursor position
	size_t uniqueTo = max(longestUniqueCompletion().size(), prefix.size());
	gui_->bufferView().setInlineCompletion(cur, cur, postfix, uniqueTo - prefix.size());
	inlineVisible_ = true;
}


void GuiCompleter::updatePopup(Cursor const & cur)
{
	if (!cur.inset().completionSupported(cur))
		return;
	
	popupVisible_ = true;

	if (completionCount() == 0) {
		QTimer::singleShot(0, popup(), SLOT(hide()));
		return;
	}

	QTimer::singleShot(0, this, SLOT(asyncUpdatePopup()));
}


void GuiCompleter::asyncUpdatePopup()
{
	Cursor cur = gui_->bufferView().cursor();
	if (!cur.inset().completionSupported(cur)
		  || !cur.bv().paragraphVisible(cur)) {
		popupVisible_ = false;
		return;
	}

	// get dimensions of completion prefix
	Dimension dim;
	int x;
	int y;
	cur.inset().completionPosAndDim(cur, x, y, dim);
	
	// and calculate the rect of the popup
	QRect rect;
	if (popup()->layoutDirection() == Qt::RightToLeft)
		rect = QRect(x + dim.width() - 200, y - dim.ascent() - 3, 200, dim.height() + 6);
	else
		rect = QRect(x, y - dim.ascent() - 3, 200, dim.height() + 6);
	
	// Resize the columns in the popup.
	// This should really be in the constructor. But somehow the treeview
	// has a bad memory about it and we have to tell him again and again.
	QTreeView * listView = static_cast<QTreeView *>(popup());
	listView->header()->setStretchLastSection(false);
	setSectionResizeMode(listView->header(), 0, QHeaderView::Stretch);
	setSectionResizeMode(listView->header(), 1, QHeaderView::Fixed);
	listView->header()->resizeSection(1, 22);
	
	// show/update popup
	complete(rect);
}


void GuiCompleter::updateAvailability()
{
	// this should really only be of interest if no completion is
	// visible yet, i.e. especially if automatic completion is disabled.
	if (inlineVisible() || popupVisible())
		return;
	Cursor const & cur = gui_->bufferView().cursor();
	if (!popupPossible(cur) && !inlinePossible(cur))
		return;
	
	updateModel(cur, false, false);
}
	

void GuiCompleter::updateModel(Cursor const & cur, bool popupUpdate, bool inlineUpdate)
{
	// value which should be kept selected
	QString old = currentCompletion();
	if (old.length() == 0)
		old = last_selection_;

	// set whether rtl
	bool rtl = false;
	if (cur.inTexted()) {
		Paragraph const & par = cur.paragraph();
		Font const & font =
			par.getFontSettings(cur.bv().buffer().params(), cur.pos());
		rtl = font.isVisibleRightToLeft();
	}
	popup()->setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);

	// set new model
	CompletionList const * list = cur.inset().createCompletionList(cur);
	model_->setList(list);
	modelActive_ = true;
	if (list->sorted())
		setModelSorting(QCompleter::CaseSensitivelySortedModel);
	else
		setModelSorting(QCompleter::UnsortedModel);

	// set prefix
	QString newPrefix = toqstr(cur.inset().completionPrefix(cur));
	if (newPrefix != completionPrefix())
		setCompletionPrefix(newPrefix);

	// show popup
	if (popupUpdate)
		updatePopup(cur);

	// restore old selection
	setCurrentCompletion(old);
	
	// if popup is not empty, the new selection will
	// be our last valid one
	if (popupVisible() || inlineVisible()) {
		QString const & s = currentCompletion();
		if (s.length() > 0)
			last_selection_ = s;
		else
			last_selection_ = old;
	}

	// show inline completion
	if (inlineUpdate)
		updateInline(cur, currentCompletion());
}


void GuiCompleter::showPopup(Cursor const & cur)
{
	if (!popupPossible(cur))
		return;
	
	updateModel(cur, true, inlineVisible());
}


void GuiCompleter::asyncHidePopup()
{
	popup()->hide();
	if (!inlineVisible())
		model_->setList(0);
}


void GuiCompleter::showInline(Cursor const & cur)
{
	if (!inlinePossible(cur))
		return;
	
	updateModel(cur, popupVisible(), true);
}


void GuiCompleter::hideInline(Cursor const & cur)
{
	gui_->bufferView().setInlineCompletion(cur, DocIterator(cur.buffer()), docstring());
	inlineVisible_ = false;
	
	if (inline_timer_.isActive())
		inline_timer_.stop();
	
	// Trigger asynchronous part of hideInline. We might be
	// in a dispatcher here and the setModel call might
	// trigger focus events which is are not healthy here.
	QTimer::singleShot(0, this, SLOT(asyncHideInline()));

	// mark that the asynchronous part will reset the model
	if (!popupVisible())
		modelActive_ = false;
}


void GuiCompleter::asyncHideInline()
{
	if (!popupVisible())
		model_->setList(0);
}


void GuiCompleter::showPopup()
{
	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);
	
	showPopup(cur);

	// redraw if needed
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
}


void GuiCompleter::showInline()
{
	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);
	
	showInline(cur);

	// redraw if needed
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
}


void GuiCompleter::hidePopup()
{
	popupVisible_ = false;

	if (popup_timer_.isActive())
		popup_timer_.stop();

	// hide popup asynchronously because we might be here inside of
	// LFUN dispatchers. Hiding a popup can trigger a focus event on the
	// workarea which then redisplays the cursor. But the metrics are not
	// yet up to date such that the coord cache has not all insets yet. The
	// cursorPos methods would triggers asserts in the coord cache then.
	QTimer::singleShot(0, this, SLOT(asyncHidePopup()));
	
	// mark that the asynchronous part will reset the model
	if (!inlineVisible())
		modelActive_ = false;
}


void GuiCompleter::hideInline()
{
	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);
	
	hideInline(cur);
	
	// redraw if needed
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
}


void GuiCompleter::activate()
{
	if (!popupVisible() && !inlineVisible())
		tab();
	else
		popupActivated(currentCompletion());
}


void GuiCompleter::tab()
{
	BufferView * bv = &gui_->bufferView();
	Cursor cur = bv->cursor();
	cur.screenUpdateFlags(Update::None);
	
	// check that inline completion is active
	if (!inlineVisible() && !uniqueCompletionAvailable()) {
		// try to activate the inline completion
		if (cur.inset().inlineCompletionSupported(cur)) {
			showInline();
			
			// show popup without delay because the completion was not unique
			if (lyxrc.completion_popup_after_complete
			    && !popupVisible()
			    && popup()->model()->rowCount() > 1)
				popup_timer_.start(0);

			return;
		}
		// or try popup
		if (!popupVisible() && cur.inset().completionSupported(cur)) {
			showPopup();
			return;
		}
		
		return;
	}
	
	// Make undo possible
	cur.beginUndoGroup();
	cur.recordUndo();

	// If completion is active, at least complete by one character
	docstring prefix = cur.inset().completionPrefix(cur);
	docstring completion = qstring_to_ucs4(currentCompletion());
	if (completion.size() <= prefix.size()) {
		// finalize completion
		cur.inset().insertCompletion(cur, docstring(), true);
		
		// hide popup and inline completion
		hidePopup();
		hideInline(cur);
		updateVisibility(false, false);
		cur.endUndoGroup();
		return;
	}
	docstring nextchar = completion.substr(prefix.size(), 1);
	if (!cur.inset().insertCompletion(cur, nextchar, false)) {
		cur.endUndoGroup();
		return;
	}
	updatePrefix(cur);

	// try to complete as far as it is unique
	docstring longestCompletion = longestUniqueCompletion();
	prefix = cur.inset().completionPrefix(cur);
	docstring postfix = longestCompletion.substr(min(longestCompletion.size(), prefix.size()));
	cur.inset().insertCompletion(cur, postfix, false);
	old_cursor_ = bv->cursor();
	updatePrefix(cur);

	// show popup without delay because the completion was not unique
	if (lyxrc.completion_popup_after_complete
	    && !popupVisible()
	    && popup()->model()->rowCount() > 1)
		popup_timer_.start(0);

	// redraw if needed
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
	cur.endUndoGroup();
}


QString GuiCompleter::currentCompletion() const
{
	if (!popup()->selectionModel()->hasSelection())
		return QString();

	// Not sure if this is bug in Qt: currentIndex() always 
	// return the first element in the list.
	QModelIndex idx = popup()->currentIndex();
	return popup()->model()->data(idx, Qt::EditRole).toString();
}


void GuiCompleter::setCurrentCompletion(QString const & s)
{	
	QAbstractItemModel const & model = *popup()->model();
	size_t n = model.rowCount();
	if (n == 0)
		return;

	// select the first if s is empty
	if (s.length() == 0) {
		updateLock_++;
		popup()->setCurrentIndex(model.index(0, 0));
		updateLock_--;
		return;
	}

	// find old selection in model
	size_t i;
	if (modelSorting() == QCompleter::UnsortedModel) {
		// In unsorted models, iterate through list until the s is found
		for (i = 0; i < n; ++i) {
			QString const & is
			= model.data(model.index(i, 0), Qt::EditRole).toString();
			if (is == s)
				break;
		}
	} else {
		// In sorted models, do binary search for s.
		int l = 0;
		int r = n - 1;
		while (r >= l && l < int(n)) {
			size_t mid = (r + l) / 2;
			QString const & mids
			= model.data(model.index(mid, 0),
				     Qt::EditRole).toString();

			// left or right?
			// FIXME: is this really the same order that the docstring
			// from the CompletionList has?
			int c = s.compare(mids, Qt::CaseSensitive);
			if (c == 0) {
				l = mid;
				break;
			} else if (l == r) {
				l = n;
				break;
			} else if (c > 0)
				// middle is not far enough
				l = mid + 1;
			else
				// middle is too far
				r = mid - 1;
		}

		// loop was left without finding anything
		if (r < l)
			i = n;
		else
			i = l;
		// we can try to recover
		LASSERT(i <= n, i = 0);
	}

	// select the first if none was found
	if (i == n)
		i = 0;

	updateLock_++;
	popup()->setCurrentIndex(model.index(i, 0));
	updateLock_--;
}


size_t commonPrefix(QString const & s1, QString const & s2)
{
	// find common prefix
	size_t j;
	size_t n1 = s1.length();
	size_t n2 = s2.length();
	for (j = 0; j < n1 && j < n2; ++j) {
		if (s1.at(j) != s2.at(j))
			break;
	}
	return j;
}


docstring GuiCompleter::longestUniqueCompletion() const
{
	QAbstractItemModel const & model = *popup()->model();
	size_t n = model.rowCount();
	if (n == 0)
		return docstring();
	QString s = model.data(model.index(0, 0), Qt::EditRole).toString();

	if (modelSorting() == QCompleter::UnsortedModel) {
		// For unsorted model we cannot do more than iteration.
		// Iterate through the completions and cut off where s differs
		for (size_t i = 0; i < n && s.length() > 0; ++i) {
			QString const & is
			= model.data(model.index(i, 0), Qt::EditRole).toString();

			s = s.left(commonPrefix(is, s));
		}
	} else {
		// For sorted models we can do binary search multiple times,
		// each time to find the first string which has s not as prefix.
		size_t i = 0;
		while (i < n && s.length() > 0) {
			// find first string that does not have s as prefix
			// via binary search in [i,n-1]
			size_t r = n - 1;
			do {
				// get common prefix with the middle string
				size_t mid = (r + i) / 2;
				QString const & mids
				= model.data(model.index(mid, 0), 
					Qt::EditRole).toString();
				size_t oldLen = s.length();
				size_t len = commonPrefix(mids, s);
				s = s.left(len);

				// left or right?
				if (oldLen == len) {
					// middle is not far enough
					i = mid + 1;
				} else {
					// middle is maybe too far
					r = mid;
				}
			} while (r - i > 0 && i < n);
		}
	}

	return qstring_to_ucs4(s);
}


void GuiCompleter::popupActivated(const QString & completion)
{
	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);

	cur.beginUndoGroup();
	cur.recordUndo();

	docstring prefix = cur.inset().completionPrefix(cur);
	docstring postfix = qstring_to_ucs4(completion.mid(prefix.length()));
	cur.inset().insertCompletion(cur, postfix, true);
	hidePopup();
	hideInline(cur);
	
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
	cur.endUndoGroup();
}


void GuiCompleter::popupHighlighted(const QString & completion)
{
	if (updateLock_ > 0)
		return;

	Cursor cur = gui_->bufferView().cursor();
	cur.screenUpdateFlags(Update::None);
	
	if (inlineVisible())
		updateInline(cur, completion);
	
	if (cur.result().screenUpdate())
		gui_->bufferView().processUpdateFlags(cur.result().screenUpdate());
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiCompleter.cpp"
