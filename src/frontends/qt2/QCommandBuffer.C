/**
 * \file QCommandBuffer.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
 
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
	void keyPressEvent(QKeyEvent * e) {
		if (e->key() == Key_Escape) {
			hide();
			return;
		}
		QListBox::keyPressEvent(e);
	}
};

}
 
QCommandBuffer::QCommandBuffer(QtView * view, ControlCommandBuffer & control)
	: QToolBar(view), view_(view), controller_(control)
{
	setHorizontalStretchable(true);
 
	QPixmap qp(LibFileSearch("images", "unknown", "xpm").c_str());

	QToolButton * upb = new QToolButton(qp, _("Up"), "", this, SLOT(up()), this);
	upb->show();
 
	QToolButton * downb = new QToolButton(qp, _("Down"), "", this, SLOT(down()), this);
	downb->show();
 
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

	QTempListBox * list = new QTempListBox();
	vector<string>::const_iterator cit = comp.begin();
	vector<string>::const_iterator end = comp.end();
	for (; cit != end; ++cit) {
		list->insertItem(cit->c_str());
	}

	// For some reason we get lots of empty entries and the 
	// scrollbar is wrong as a result. No fix. Qt Sucks.
 
	// width() is not big enough by a few pixels. Qt Sucks.
	list->setMinimumWidth(list->sizeHint().width() + 10);
 
	list->resize(list->sizeHint());
	QPoint pos(edit_->mapToGlobal(QPoint(0, 0))); 
	int y = pos.y() - list->height();
	if (y < 0)
		y = 0;
	list->move(pos.x(), y);
 
	connect(list, SIGNAL(selected(const QString &)),
		this, SLOT(complete_selected(const QString &))); 
 
	// Note we *cannot* make a single click popup, because
	// events get generated for outside the popup on Qt 2.3.1
	// and even gives valid QListBoxItem *'s. We have no way
	// to work past this. Qt Sucks. 
	//connect(list, SIGNAL(clicked(QListBoxItem *)),
	//	this, SLOT(complete_selected2(QListBoxItem *)));

	list->show();
	list->setFocus();
}


void QCommandBuffer::complete_selected(const QString & str)
{
	edit_->setText(str + " ");
	QWidget const * widget = static_cast<QWidget const *>(sender());
	const_cast<QWidget *>(widget)->hide();
}

 
void QCommandBuffer::up()
{
	string const input = edit_->text().latin1();
	string const h(controller_.historyUp());
 
	if (h.empty()) {
	//	show_info_suffix(_("[Beginning of history]"), input);
	} else {
		edit_->setText(h.c_str());
	}
}


void QCommandBuffer::down()
{
	string const input = edit_->text().latin1();
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
