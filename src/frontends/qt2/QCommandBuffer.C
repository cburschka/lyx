/**
 * \file QCommandBuffer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/filetools.h"
#include "controllers/ControlCommandBuffer.h"
#include "gettext.h"
#include "debug.h"

#include "QtView.h"
#include "QCommandBuffer.h"
#include "QCommandEdit.h"

#include <qcombobox.h>
#include <qlistbox.h>
#include <qtoolbutton.h>
#include <qpixmap.h>

#include "LString.h"

using std::vector;

namespace {

class QTempListBox : public QListBox {
public:
	QTempListBox()
		: QListBox(0, 0,
			   WType_Modal | WType_Popup | WDestructiveClose) {
		setHScrollBarMode(AlwaysOff);
	}
protected:
	void mouseReleaseEvent(QMouseEvent * e) {
		if (e->x() < 0 || e->y() < 0
		    || e->x() > width() || e->y() > height()) {
			hide();
		} else {
			emit selected(currentText());
		}
	}

	void keyPressEvent(QKeyEvent * e) {
		if (e->key() == Key_Escape) {
			hide();
			return;
		}
		QListBox::keyPressEvent(e);
	}
};

} // end of anon


QCommandBuffer::QCommandBuffer(QtView * view, ControlCommandBuffer & control)
	: QToolBar(view), view_(view), controller_(control)
{
	setHorizontalStretchable(true);

	QPixmap qp(LibFileSearch("images", "unknown", "xpm").c_str());

	(new QToolButton(qp, _("Up"), "", this, SLOT(up()), this))->show();
	(new QToolButton(qp, _("Down"), "", this, SLOT(down()), this))->show();

	edit_ = new QCommandEdit(this);
	edit_->setMinimumSize(edit_->sizeHint());
	edit_->show();
	setStretchableWidget(edit_);

	show();

	connect(edit_, SIGNAL(escapePressed()), this, SLOT(cancel()));
	connect(edit_, SIGNAL(returnPressed()), this, SLOT(dispatch()));
	connect(edit_, SIGNAL(rightPressed()), this, SLOT(complete()));
	connect(edit_, SIGNAL(upPressed()), this, SLOT(up()));
	connect(edit_, SIGNAL(downPressed()), this, SLOT(down()));
}



void QCommandBuffer::focus_command()
{
	edit_->setFocus();
}


void QCommandBuffer::cancel()
{
	view_->centralWidget()->setFocus();
	edit_->setText("");
}


void QCommandBuffer::dispatch()
{
	controller_.dispatch(edit_->text().latin1());
	view_->centralWidget()->setFocus();
	edit_->setText("");
}


void QCommandBuffer::complete()
{
	string const input = edit_->text().latin1();
	string new_input;
	vector<string> comp = controller_.completions(input, new_input);

	if (comp.empty() && new_input == input) {
	//	show_info_suffix(_("[no match]"), input);
		return;
	}

	if (comp.empty()) {
		edit_->setText(new_input.c_str());
	//	show_info_suffix(("[only completion]"), new_input + " ");
		return;
	}

	edit_->setText(new_input.c_str());

	QTempListBox * list = new QTempListBox;

	// For some reason the scrollview's contents are larger
	// than the number of actual items...
	vector<string>::const_iterator cit = comp.begin();
	vector<string>::const_iterator end = comp.end();
	for (; cit != end; ++cit) {
		list->insertItem(cit->c_str());
	}

	// width() is not big enough by a few pixels. Qt Sucks.
	list->setMinimumWidth(list->sizeHint().width() + 10);

	list->resize(list->sizeHint());
	QPoint pos(edit_->mapToGlobal(QPoint(0, 0)));

	int y = std::max(0, pos.y() - list->height());

	list->move(pos.x(), y);

	connect(list, SIGNAL(selected(const QString &)),
		this, SLOT(complete_selected(const QString &)));

	list->show();
	list->setFocus();
}


void QCommandBuffer::complete_selected(QString const & str)
{
	edit_->setText(str + " ");
	QWidget const * widget = static_cast<QWidget const *>(sender());
	const_cast<QWidget *>(widget)->hide();
}


void QCommandBuffer::up()
{
	string const input(edit_->text().latin1());
	string const h(controller_.historyUp());

	if (h.empty()) {
	//	show_info_suffix(_("[Beginning of history]"), input);
	} else {
		edit_->setText(h.c_str());
	}
}


void QCommandBuffer::down()
{
	string const input(edit_->text().latin1());
	string const h(controller_.historyDown());

	if (h.empty()) {
	//	show_info_suffix(_("[End of history]"), input);
	} else {
		edit_->setText(h.c_str());
	}
}


#if 0
void XMiniBuffer::show_info_suffix(string const & suffix, string const & input)
{
	stored_input_ = input;
	info_suffix_shown_ = true;
	set_input(input + " " + suffix);
	suffix_timer_->start();
}


void XMiniBuffer::suffix_timeout()
{
	info_suffix_shown_ = false;
	set_input(stored_input_);
}

#endif
