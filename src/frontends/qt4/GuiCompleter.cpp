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

#include "GuiWorkArea.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Dimension.h"
#include "FuncRequest.h"
#include "GuiView.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "version.h"

#include "support/debug.h"

#include <QApplication>
#include <QAbstractListModel>
#include <QHeaderView>
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


class PixmapItemDelegate : public QItemDelegate {
public:
	explicit PixmapItemDelegate(QObject *parent = 0)
	: QItemDelegate(parent) {}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		   const QModelIndex &index) const
	{
		QStyleOptionViewItemV3 opt = setOptions(index, option);
		QVariant value = index.data(Qt::DisplayRole);
		QPixmap pixmap = qvariant_cast<QPixmap>(value);
		const QSize size = pixmap.size();

		// draw
		painter->save();
		drawBackground(painter, opt, index);
		painter->drawPixmap(option.rect.left() + (16 - size.width()) / 2,
			option.rect.top() + (option.rect.height() - size.height()) / 2,
			pixmap);
		drawFocus(painter, opt, option.rect);
		painter->restore();
	}
};


class GuiCompletionModel : public QAbstractListModel {
public:
	///
	GuiCompletionModel(QObject * parent, Inset::CompletionListPtr l)
	: QAbstractListModel(parent), list(l) {}
	///
	int columnCount(const QModelIndex & parent = QModelIndex()) const
	{
		return 2;
	}
	///
	int rowCount(const QModelIndex & parent = QModelIndex()) const
	{
		if (list.get() == 0)
			return 0;
		else
			return list->size();
	}

	///
	QVariant data(const QModelIndex & index, int role) const
	{
		if (list.get() == 0)
			return QVariant();

		if (index.row() < 0 || index.row() >= rowCount())
			return QVariant();

		if (role != Qt::DisplayRole && role != Qt::EditRole)
		    return QVariant();
		    
		if (index.column() == 0)
			return toqstr(list->data(index.row()));
		else if (index.column() == 1) {
			// get icon from cache
			QPixmap scaled;
			QString const name = ":" + toqstr(list->icon(index.row()));
			if (!QPixmapCache::find("completion" + name, scaled)) {
				// load icon from disk
				QPixmap p = QPixmap(name);

				// scale it to 16x16 or smaller
				scaled = p.scaled(min(16, p.width()), min(16, p.height()), 
					Qt::KeepAspectRatio, Qt::SmoothTransformation);
				QPixmapCache::insert("completion" + name, scaled);
			}
			return scaled;
		}
		return QVariant();
	}

private:
	Inset::CompletionListPtr list;
};


GuiCompleter::GuiCompleter(GuiWorkArea * gui, QObject * parent)
	: QCompleter(parent), gui_(gui)
{
	// Setup the completion popup
	setModel(new GuiCompletionModel(this, Inset::CompletionListPtr()));
	setCompletionMode(QCompleter::PopupCompletion);
	setWidget(gui_);
	
	// create the popup
	QTreeView *listView = new QTreeView;
        listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listView->setSelectionBehavior(QAbstractItemView::SelectRows);
        listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->header()->hide();
	listView->setIndentation(0);
	setPopup(listView);
	popup()->setItemDelegateForColumn(1, new PixmapItemDelegate(popup()));
	
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


bool GuiCompleter::popupVisible() const
{
	return popup()->isVisible();
}


bool GuiCompleter::inlineVisible() const
{
	return !gui_->bufferView().inlineCompletionPos().empty();
}


void GuiCompleter::updateVisibility(Cursor & cur, bool start, bool keep, bool cursorInView)
{
	// parameters which affect the completion
	bool moved = cur != old_cursor_;
	if (moved)
		old_cursor_ = cur;

	bool possiblePopupState = popupPossible(cur) && cursorInView;
	bool possibleInlineState = inlinePossible(cur) && cursorInView;

	// we moved or popup state is not ok for popup?
	if ((moved && !keep) || !possiblePopupState) {
		// stop an old completion timer
		if (popup_timer_.isActive())
			popup_timer_.stop();

		// hide old popup
		if (popupVisible())
			popup()->hide();
	}

	// we moved or inline state is not ok for inline completion?
	if ((moved && !keep) || !possibleInlineState) {
		// stop an old completion timer
		if (inline_timer_.isActive())
			inline_timer_.stop();

		// hide old inline completion
		if (inlineVisible()) {
			gui_->bufferView().setInlineCompletion(DocIterator(), docstring());
			cur.updateFlags(Update::Force | Update::SinglePar);
		}
	}

	// we inserted something and are in a possible popup state?
	if (!popupVisible() && possiblePopupState && start
		&& cur.inset().automaticPopupCompletion())
		popup_timer_.start(lyxrc.completion_popup_delay * 1000.0);

	// we inserted something and are in a possible inline completion state?
	if (!inlineVisible() && possibleInlineState && start
		&& cur.inset().automaticInlineCompletion())
		inline_timer_.start(lyxrc.completion_inline_delay * 1000.0);

	// update prefix if popup is visible or if it will be visible soon
	if (popupVisible() || inlineVisible()
	    || popup_timer_.isActive() || inline_timer_.isActive())
		updatePrefix(cur);
}


void GuiCompleter::updateVisibility(bool start, bool keep)
{
	Cursor cur = gui_->bufferView().cursor();
	updateVisibility(cur, start, keep);
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
}


void GuiCompleter::updatePrefix(Cursor & cur)
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
	if (s.length() > 0)
		last_selection_ = s;
	else
		last_selection_ = old;
	
	// update inline completion because the default
	// completion string might have changed
	if (inlineVisible())
		updateInline(cur, s);
}


void GuiCompleter::updateInline(Cursor & cur, QString const & completion)
{
	if (!cur.inset().inlineCompletionSupported(cur))
		return;
	
	// compute postfix
	docstring prefix = cur.inset().completionPrefix(cur);
	docstring postfix = from_utf8(fromqstr(completion.mid(prefix.length())));
	
	// shorten it if necessary
	if (lyxrc.completion_inline_dots != -1
	    && postfix.size() > unsigned(lyxrc.completion_inline_dots))
		postfix = postfix.substr(0, lyxrc.completion_inline_dots - 1) + "...";

	// set inline completion at cursor position
	size_t uniqueTo = max(longestUniqueCompletion().size(), prefix.size());
	gui_->bufferView().setInlineCompletion(cur, postfix, uniqueTo - prefix.size());
	cur.updateFlags(Update::Force | Update::SinglePar);
}


void GuiCompleter::updatePopup(Cursor & cur)
{
	if (!cur.inset().completionSupported(cur))
		return;
	
	if (completionCount() == 0)
		return;
	
	// get dimensions of completion prefix
	Dimension dim;
	int x;
	int y;
	cur.inset().completionPosAndDim(cur, x, y, dim);
	QRect insetRect = QRect(x, y - dim.ascent() - 3, 200, dim.height() + 6);
	
	// show/update popup
	complete(insetRect);
	QTreeView * p = static_cast<QTreeView *>(popup());
	p->setColumnWidth(0, popup()->width() - 22 - p->verticalScrollBar()->width());
	
	// update highlight
	updateInline(cur, currentCompletion());
}


void GuiCompleter::updateModel(Cursor & cur, bool popupUpdate, bool inlineUpdate)
{
	// value which should be kept selected
	QString old = currentCompletion();
	if (old.length() == 0)
		old = last_selection_;
	
	// set new model
	setModel(new GuiCompletionModel(this, cur.inset().completionList(cur)));
	
	// show popup
	if (popupUpdate)
		updatePopup(cur);

	// restore old selection
	setCurrentCompletion(old);
	
	// if popup is not empty, the new selection will
	// be our last valid one
	QString const & s = currentCompletion();
	if (s.length() > 0)
		last_selection_ = s;
	else
		last_selection_ = old;
	
	// show inline completion
	if (inlineUpdate)
		updateInline(cur, currentCompletion());
}


void GuiCompleter::showPopup(Cursor & cur)
{
	if (!popupPossible(cur))
		return;
	
	updateModel(cur, true, inlineVisible());
	updatePrefix(cur);
}
	

void GuiCompleter::showInline(Cursor & cur)
{
	if (!inlinePossible(cur))
		return;
	
	updateModel(cur, popupVisible(), true);
	updatePrefix(cur);
}


void GuiCompleter::showPopup()
{
	Cursor cur = gui_->bufferView().cursor();
	showPopup(cur);

	// redraw if needed
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
}


void GuiCompleter::showInline()
{
	Cursor cur = gui_->bufferView().cursor();
	showInline(cur);

	// redraw if needed
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
}


void GuiCompleter::activate()
{
	if (!popupVisible() && !inlineVisible())
		return;

	// Complete with current selection in the popup.
	QString s = currentCompletion();
	popup()->hide();
	popupActivated(s);
}


void GuiCompleter::tab()
{
	BufferView * bv = &gui_->bufferView();
	Cursor & cur = bv->cursor();

	// check that inline completion is active
	if (!inlineVisible()) {
		// try to activate the inline completion
		if (cur.inset().inlineCompletionSupported(cur)) {
			showInline();
			return;
		}
		// or try popup
		if (!popupVisible() && cur.inset().completionSupported(cur)) {
			showPopup();
			return;
		}
		
		return;
	}
	
	// If completion is active, at least complete by one character
	docstring prefix = cur.inset().completionPrefix(cur);
	docstring completion = from_utf8(fromqstr(currentCompletion()));
	if (completion.size() <= prefix.size()) {
		// finalize completion
		cur.inset().insertCompletion(cur, docstring(), true);
		popup()->hide();
		updateVisibility(false, false);
		return;
	}
	docstring nextchar = completion.substr(prefix.size(), 1);
	if (!cur.inset().insertCompletion(cur, nextchar, false))
		return;
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
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
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
		popup()->setCurrentIndex(model.index(0, 0));
		return;
	}

	// iterate through list until the s is found
	// FIXME: there must be a better way than this iteration
	size_t i;
	for (i = 0; i < n; ++i) {
		QString const & is
		= model.data(model.index(i, 0), Qt::EditRole).toString();
		if (is == s)
			break;
	}

	// select the first if none was found
	if (i == n)
		i = 0;

	popup()->setCurrentIndex(model.index(i, 0));
}


docstring GuiCompleter::longestUniqueCompletion() const {
	QAbstractItemModel const & model = *popup()->model();
	QString s = currentCompletion();
	size_t n = model.rowCount();

	// iterate through the completions and cut off where s differs
	for (size_t i = 0; i < n && s.length() > 0; ++i) {
		QString const & is
		= model.data(model.index(i, 0), Qt::EditRole).toString();

		// find common prefix
		size_t j;
		size_t isn = is.length();
		size_t sn = s.length();
		for (j = 0; j < isn && j < sn; ++j) {
			if (s.at(j) != is.at(j))
				break;
		}
		s = s.left(j);
	}

	return from_utf8(fromqstr(s));
}


void GuiCompleter::popupActivated(const QString & completion)
{
	Cursor & cur = gui_->bufferView().cursor();
	docstring prefix = cur.inset().completionPrefix(cur);
	docstring postfix = from_utf8(fromqstr(completion.mid(prefix.length())));
	cur.inset().insertCompletion(cur, postfix, true);
	updateVisibility(cur, false);
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
}


void GuiCompleter::popupHighlighted(const QString & completion)
{
	Cursor cur = gui_->bufferView().cursor();
	updateInline(cur, completion);
	if (cur.disp_.update())
		gui_->bufferView().processUpdateFlags(cur.disp_.update());
}

} // namespace frontend
} // namespace lyx

#include "GuiCompleter_moc.cpp"
