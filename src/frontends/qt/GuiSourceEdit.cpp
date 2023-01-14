// -*- C++ -*-
/**
 * \file GuiSourceEdit.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSourceEdit.h"

#include "qt_helpers.h"

#include <QMenu>
#include <QRegularExpression>

namespace lyx {
namespace frontend {

GuiSourceEdit::GuiSourceEdit(QWidget *parent) : QTextEdit(parent)
{
	setWordWrapMode(QTextOption::NoWrap);
	// Set the default tab stop
	setTabStop(tabStop_);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(showMenu(QPoint)));
}

void GuiSourceEdit::showMenu(const QPoint& pos)
{
	// Move the cursor to the click position unless clicked within selection
	QTextCursor cursor = textCursor();
	int const textPos = cursorForPosition(pos).position();
	bool const textPosInSel = textCursor().selectionStart() <= textPos &&
	                          textPos <= textCursor().selectionEnd();
	if (!textCursor().hasSelection() || !textPosInSel) {
		cursor.setPosition(textPos);
		setTextCursor(cursor);
	}
	// The standard menu
	QMenu * menu = QTextEdit::createStandardContextMenu();
	QAction * firstAction = menu->actions().at(0);
	// Insert toggle comment entry at the top
	QKeySequence keySeq = QKeySequence(Qt::ControlModifier | Qt::Key_Slash);
	QAction * toggleComment = new QAction(qt_("Toggle Comment") + "\t" +
		keySeq.toString(QKeySequence::NativeText), menu);
	connect(toggleComment, SIGNAL(triggered()), this, SLOT(toggleComment()));
	menu->insertAction(firstAction, toggleComment);
	// Insert toggle spaces and tabs entry at the top
	QAction * showTabsAndSpaces = new QAction(qt_("Show Tabs and Spaces"), menu);
	showTabsAndSpaces->setCheckable(true);
	auto currentFlags = document()->defaultTextOption().flags();
	showTabsAndSpaces->setChecked(currentFlags &
	                              QTextOption::ShowTabsAndSpaces);
	connect(showTabsAndSpaces, SIGNAL(triggered()), this,
	        SLOT(toggleShowTabsAndSpaces()));
	menu->insertAction(firstAction, showTabsAndSpaces);
	// Add separator to default entries
	menu->insertSeparator(firstAction);
	menu->exec(mapToGlobal(pos));
}

void GuiSourceEdit::setCommentMarker(QString marker)
{
	commentMarker_ = marker;
}

void GuiSourceEdit::setTabStop(int spaces)
{
	tabStop_ = spaces;
	QFontMetrics metrics(currentFont());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	// horizontalAdvance() is available starting in 5.11.0
	// setTabStopDistance() is available starting in 5.10.0
	setTabStopDistance(tabStop_ * metrics.horizontalAdvance(' '));
#else
	setTabStopWidth(tabStop_ * metrics.width(' '));
#endif
}

void GuiSourceEdit::setFont(const QFont & font)
{
	QTextEdit::setFont(font);
	// Re-calculate tabstop width based on new font
	setTabStop(tabStop_);
}

QTextBlock GuiSourceEdit::blockAtSelPos(Position position) const
{
	QTextCursor cursor = textCursor();
	QTextDocument * doc = cursor.document();
	int pos = position == START ? qMin(cursor.anchor(), cursor.position()) :
	                              qMax(cursor.anchor(), cursor.position());
	return doc->findBlock(pos);
}

QTextCursor GuiSourceEdit::cursorAt(int position) const {
	// Create text cursor
	QTextCursor cursor = textCursor();
	// Move the cursor to position
	cursor.setPosition(position);
	return cursor;
}

void GuiSourceEdit::removeMarker(int positionStart, QString marker,
                                 bool addedSpace)
{
	// Create text cursor
	QTextCursor cursor = cursorAt(positionStart);
	QTextBlock const block = cursor.block();
	QString blockText = block.text();
	int const index = blockText.indexOf(marker);
	int length = marker.length();
	if (index != -1) {
		// Check for extra space after marker
		if (addedSpace && blockText.remove(0, index + length).startsWith(" "))
			++length;
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
		                    index);
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
		                    length);
		cursor.deleteChar();
	}
}

void GuiSourceEdit::insertMarkerAtIndentation(int positionStart, QString marker,
                                              bool addedSpace, int indentation)
{
	// Create text cursor
	QTextCursor cursor = cursorAt(positionStart);
	// Move the cursor to the given indentation
	int i = 0;
	while (i < indentation) {
		if (toPlainText().at(cursor.position()) == '\t')
			i += tabStop_;
		else
			++i;
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
	}
	// Add the comment marker
	cursor.insertText(marker + (addedSpace ? " " : ""), QTextCharFormat());
}

void GuiSourceEdit::modifyMarkerInSel(QString marker, Modification modification,
                                      bool allEmpty, bool addedSpace)
{
	QTextBlock const startBlock = blockAtSelPos(START);
	QTextBlock const endBlock = blockAtSelPos(END);
	QTextBlock const endBlockNext = endBlock.next();
	// Create text cursor
	QTextCursor cursor = textCursor();
	cursor.beginEditBlock();
	for (QTextBlock block = startBlock; block != endBlockNext;
	     block = block.next()) {
		if (modification == REMOVE)
			removeMarker(block.position(), marker, addedSpace);
		// Disregard white space blocks
		else if (allEmpty || !block.text().trimmed().isEmpty()) {
			insertMarkerAtIndentation(block.position(), marker, addedSpace,
			                          selMinIndentation(allEmpty));
		}
	}
	cursor.endEditBlock();
}

int GuiSourceEdit::getIndentation(QString text) const
{
	int tabs = 0;
	for (QChar c : text) {
		if (c == QChar::Tabulation)
			++tabs;
		if (!c.isSpace())
			break;
	}
	return tabs;
}

QString GuiSourceEdit::getIndentationString(QString text) const
{
	// Regex to capture indentation string (i.e. spaces and tab stops)
	QRegularExpression static re("^( |\t)+");
	return re.match(text).captured(0);
}

void GuiSourceEdit::newLineWithInheritedIndentation()
{
	QTextCursor cursor = textCursor();
	cursor.beginEditBlock();
	// Start new line
	cursor.insertText("\n", QTextCharFormat());
	// Insert as many tabstops as on the previous block
	QTextBlock const previousBlock = blockAtSelPos(START).previous();
	QString const indentation = getIndentationString(previousBlock.text());
	cursor.insertText(indentation);
	cursor.endEditBlock();
}

int GuiSourceEdit::getLengthInSpaces(QString const & text) const
{
	// Replace tab stops by spaces and return length
	return QString(text).replace("\t", QString(" ").repeated(tabStop_))
			.length();
}

int GuiSourceEdit::selMinIndentation(bool allEmpty) const
{
	QTextBlock const startBlock = blockAtSelPos(START);
	QTextBlock const endBlock = blockAtSelPos(END);
	QTextBlock const endBlockNext = endBlock.next();
	QString minIndentationString = getIndentationString(startBlock.text());
	for (QTextBlock block = startBlock; block != endBlockNext;
	     block = block.next()) {
		QString text = block.text();
		if (allEmpty || !text.trimmed().isEmpty()) {
			QString curIndentationString = getIndentationString(text);
			// Chop off indentation until they have the same length
			while (!minIndentationString.isEmpty()) {
				int minLen = getLengthInSpaces(minIndentationString);
				int curLen = getLengthInSpaces(curIndentationString);
				if (minLen == curLen)
					break;
				else if (minLen > curLen)
					minIndentationString.chop(1);
				else
					curIndentationString.chop(1);
			}
		}
	}
	return getLengthInSpaces(minIndentationString);
}

bool GuiSourceEdit::selBlocksStartWith(QString marker) const
{
	QTextBlock const startBlock = blockAtSelPos(START);
	QTextBlock const endBlock = blockAtSelPos(END);
	QTextBlock const endBlockNext = endBlock.next();
	for (QTextBlock block = startBlock; block != endBlockNext;
	     block = block.next()) {
		QString const blockText = block.text();
		QString trimmedText = blockText.trimmed();
		// Disregard white space blocks
		if (trimmedText.isEmpty())
			continue;
		else if (!trimmedText.startsWith(marker))
			return false;
	}
	return true;
}

bool GuiSourceEdit::selBlocksWhiteSpace() const
{
	QTextBlock const startBlock = blockAtSelPos(START);
	QTextBlock const endBlockNext = blockAtSelPos(END).next();
	for (QTextBlock block = startBlock; block != endBlockNext;
	     block = block.next()) {
		QString const blockText = block.text();
		QString const trimmedText = blockText.trimmed();
		// Disregard white space blocks
		if (trimmedText.isEmpty())
			continue;
		else
			return false;
	}
	return true;
}

void GuiSourceEdit::toggleComment()
{
	bool const allEmpty = selBlocksWhiteSpace();
	modifyMarkerInSel(commentMarker_,
	                  !allEmpty && selBlocksStartWith(commentMarker_) ?
	                      REMOVE : INSERT,
	                  allEmpty, addedSpaceAfterComment_);
}

void GuiSourceEdit::toggleShowTabsAndSpaces()
{
	QTextOption option = document()->defaultTextOption();
	auto currentFlags = document()->defaultTextOption().flags();
	if (currentFlags & QTextOption::ShowTabsAndSpaces)
		currentFlags &= ~QTextOption::ShowTabsAndSpaces;
	else
		currentFlags |= QTextOption::ShowTabsAndSpaces;
	option.setFlags(currentFlags);
	document()->setDefaultTextOption(option);
}

void GuiSourceEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->modifiers() == Qt::ControlModifier &&
	    event->key() == Qt::Key_Slash)
		toggleComment();
	else if (event->key() == Qt::Key_Tab &&
	         blockAtSelPos(START) != blockAtSelPos(END))
		modifyMarkerInSel("\t", INSERT, selBlocksWhiteSpace(), false);
	else if (event->key() == Qt::Key_Backtab)
		modifyMarkerInSel("\t", REMOVE);
	else if (event->key() == Qt::Key_Return)
		newLineWithInheritedIndentation();
	else
		// Call base class for other events
		QTextEdit::keyPressEvent(event);
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiSourceEdit.cpp"
