/**
 * \file QCommandBuffer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "QtView.h"

#include "QCommandBuffer.h"
#include "QCommandEdit.h"
#include "qt_helpers.h"

#include "controllers/ControlCommandBuffer.h"

#include "support/filetools.h"

#include <qlistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qpushbutton.h>

using lyx::support::LibFileSearch;

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

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


QCommandBuffer::QCommandBuffer(QtView * view, QWidget * parent, ControlCommandBuffer & control)
	: QWidget(parent), view_(view), controller_(control)
{
	QPixmap qpup(toqstr(LibFileSearch("images", "up", "xpm")));
	QPixmap qpdown(toqstr(LibFileSearch("images", "down", "xpm")));

	QVBoxLayout * top = new QVBoxLayout(this);
	QHBoxLayout * layout = new QHBoxLayout(0);

	QPushButton * up = new QPushButton(qpup, "", this);
	QToolTip::add(up, qt_("Previous command"));
	connect(up, SIGNAL(clicked()), this, SLOT(up()));
	QPushButton * down = new QPushButton(qpdown, "", this);
	QToolTip::add(down, qt_("Next command"));
	connect(down, SIGNAL(clicked()), this, SLOT(down()));

	edit_ = new QCommandEdit(this);
	edit_->setMinimumSize(edit_->sizeHint());
	edit_->setFocusPolicy(ClickFocus);

	connect(edit_, SIGNAL(escapePressed()), this, SLOT(cancel()));
	connect(edit_, SIGNAL(returnPressed()), this, SLOT(dispatch()));
	connect(edit_, SIGNAL(tabPressed()), this, SLOT(complete()));
	connect(edit_, SIGNAL(upPressed()), this, SLOT(up()));
	connect(edit_, SIGNAL(downPressed()), this, SLOT(down()));

	layout->addWidget(up, 0);
	layout->addWidget(down, 0);
	layout->addWidget(edit_, 10);
	top->addLayout(layout);
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
	controller_.dispatch(fromqstr(edit_->text()));
	view_->centralWidget()->setFocus();
	edit_->setText("");
	edit_->clearFocus();
}


void QCommandBuffer::complete()
{
	string const input = fromqstr(edit_->text());
	string new_input;
	vector<string> comp = controller_.completions(input, new_input);

	if (comp.empty() && new_input == input) {
	//	show_info_suffix(qt_("[no match]"), input);
		return;
	}

	if (comp.empty()) {
		edit_->setText(toqstr(new_input));
	//	show_info_suffix(("[only completion]"), new_input + ' ');
		return;
	}

	edit_->setText(toqstr(new_input));

	QTempListBox * list = new QTempListBox;

	// For some reason the scrollview's contents are larger
	// than the number of actual items...
	vector<string>::const_iterator cit = comp.begin();
	vector<string>::const_iterator end = comp.end();
	for (; cit != end; ++cit) {
		list->insertItem(toqstr(*cit));
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
	QWidget const * widget = static_cast<QWidget const *>(sender());
	const_cast<QWidget *>(widget)->hide();
	edit_->setText(str + ' ');
	edit_->setFocus();
}


void QCommandBuffer::up()
{
	string const input(fromqstr(edit_->text()));
	string const h(controller_.historyUp());

	if (h.empty()) {
	//	show_info_suffix(qt_("[Beginning of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


void QCommandBuffer::down()
{
	string const input(fromqstr(edit_->text()));
	string const h(controller_.historyDown());

	if (h.empty()) {
	//	show_info_suffix(qt_("[End of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


#if 0
void XMiniBuffer::show_info_suffix(string const & suffix, string const & input)
{
	stored_input_ = input;
	info_suffix_shown_ = true;
	set_input(input + ' ' + suffix);
	suffix_timer_->start();
}


void XMiniBuffer::suffix_timeout()
{
	info_suffix_shown_ = false;
	set_input(stored_input_);
}

#endif

} // namespace frontend
} // namespace lyx
