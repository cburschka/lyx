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
 
#include "QtView.h"
#include "QCommandBuffer.h"
#include "QCommandEdit.h"
 
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
 
namespace {
 
class QTempComboBox : public QComboBox {
public:
	QTempComboBox(QWidget * parent) : QComboBox(parent) {
		setWFlags(WDestructiveClose);
	}

	void popup() { QComboBox::popup(); }
};

};
 
QCommandBuffer::QCommandBuffer(QtView * view, ControlCommandBuffer & control)
	: QToolBar(view), view_(view), controller_(control)
{
	setHorizontalStretchable(true);
 
	QPixmap qp(LibFileSearch("images", "unknown", "xpm").c_str());

	QToolButton * upb = new QToolButton(qp, _("Up"), "", this, SLOT(up()), this);
	upb->setMinimumSize(upb->sizeHint());
	upb->show();
 
	QToolButton * downb = new QToolButton(qp, _("Down"), "", this, SLOT(down()), this);
	downb->setMinimumSize(downb->sizeHint());
	downb->show();
 
	edit_ = new QCommandEdit(this);
	edit_->setMinimumSize(edit_->sizeHint());
	edit_->show();
	setStretchableWidget(edit_);
 
	setMinimumSize(sizeHint()); 
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

	QTempComboBox * combo = new QTempComboBox(view_);
	combo->move(edit_->x() + x(), edit_->y() + y());

	vector<string>::const_iterator cit = comp.begin();
	vector<string>::const_iterator end = comp.end();
	for (; cit != end; ++cit) {
		combo->insertItem(cit->c_str());
	}

        combo->setMinimumWidth(combo->sizeHint().width());
	combo->resize(combo->width(), edit_->height());
 
	connect(combo, SIGNAL(activated(const QString &)),
		this, SLOT(complete_selected(const QString &))); 
 
	combo->show();
	combo->setFocus();
	combo->popup();
}


void QCommandBuffer::complete_selected(const QString & str)
{
	edit_->setText(str + " ");
	// FIXME
	QWidget const * widget = static_cast<QWidget const *>(sender());
	edit_->setFocus();
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
