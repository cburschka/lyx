// -*- C++ -*-
/**
 * \file GuiSourceEdit.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISOURCEEDIT_H
#define GUISOURCEEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextBlock>

namespace lyx {
namespace frontend {

class GuiSourceEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit GuiSourceEdit(QWidget *parent = nullptr);
	// Set and get tab stop in number of spaces
	void setTabStop(int spaces);
	int tabStop() const {return tabStop_; };
	// Set and get line marker, e.g. "//"
	void setCommentMarker(QString marker);
	QString commentMarker() const { return commentMarker_; };
	// Set and get whether a space is added after the line marker
	void setAddedSpaceAfterComment(int spaces);
	bool addedSpaceAfterComment() const { return addedSpaceAfterComment_; };
	// Set font and update tab stop
	void setFont(const QFont & font);

private Q_SLOTS:
	void showMenu(const QPoint& pos);
	void toggleComment();
	void toggleShowTabsAndSpaces();

protected:
	void keyPressEvent(QKeyEvent *event) override;

private:
	enum Modification { INSERT, REMOVE };
	enum Position { START, END };

	// Get block at selection start/end
	QTextBlock blockAtSelPos(Position position) const;
	// Whether all blocks start with marker
	bool selBlocksStartWith(QString marker) const;
	// Wehther all blocks are white space
	bool selBlocksWhiteSpace() const;
	// Get length of string substituting tabs for spaces
	int getLengthInSpaces(QString const & text) const;
	// Minimum tab indentation the paragraphs selected start with
	int selMinIndentation(bool allEmpty) const;
	// Number of tabs as indentation
	int getIndentation(QString text) const;
	// String of white space
	QString getIndentationString(QString text) const;
	// Copy of textCursor() [at position]
	QTextCursor cursorAt(int position) const;

	// From positionStart remove marker from the line
	void removeMarker(int positionStart, QString marker,
	                  bool addedSpace = false);
	// From positionStart insert marker at indentation
	void insertMarkerAtIndentation(int positionStart, QString marker,
	                               bool addedSpace = false,
	                               int indentation = 0);
	// Modify (insert/remove) marker
	void modifyMarkerInSel(QString marker,
	                       Modification modification = INSERT,
	                       bool allEmpty = false, bool addedSpace = false);
	// Create a new line at cursor same indentation
	void newLineWithInheritedIndentation();

	// The comment marker
	QString commentMarker_ = "#";
	// The tab stop in spaces
	int tabStop_ = 4;
	// Whether a space gets added after the comment marker
	bool addedSpaceAfterComment_ = true;
};


} // namespace frontend
} // namespace lyx

#endif // GUISOURCEEDIT_H
