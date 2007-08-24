/**
 * \file QCommandBuffer.cpp
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
#include "GuiView.h"

#include "QCommandBuffer.h"
#include "QCommandEdit.h"
#include "qt_helpers.h"

#include "support/filetools.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLayout>
#include <QListWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QToolTip>
#include <QVBoxLayout>

using lyx::support::libFileSearch;

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

namespace {

class QTempListBox : public QListWidget {
public:
	QTempListBox() {
		//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setWindowModality(Qt::WindowModal);
		setWindowFlags(Qt::Popup);
		setAttribute(Qt::WA_DeleteOnClose);
	}
protected:
	void mouseReleaseEvent(QMouseEvent * ev) {
		if (ev->x() < 0 || ev->y() < 0
		    || ev->x() > width() || ev->y() > height()) {
			hide();
		} else {
			// emit signal
			itemPressed(currentItem());
		}
	}

	void keyPressEvent(QKeyEvent * ev) {
		if (ev->key() == Qt::Key_Escape) {
			hide();
			return;
		} else if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Space) {
			// emit signal
			itemPressed(currentItem());
		} else
			QListWidget::keyPressEvent(ev);
	}
};

} // end of anon


QCommandBuffer::QCommandBuffer(GuiView * view)
	: view_(view), controller_(*view)
{
	QPixmap qpup(toqstr(libFileSearch("images", "up", "xpm").absFilename()));
	QPixmap qpdown(toqstr(libFileSearch("images", "down", "xpm").absFilename()));

	QVBoxLayout * top = new QVBoxLayout(this);
	QHBoxLayout * layout = new QHBoxLayout(0);

	QPushButton * up = new QPushButton(qpup, "", this);
	up->setMaximumSize(24, 24);
	up->setToolTip(qt_("Previous command"));
	connect(up, SIGNAL(clicked()), this, SLOT(up()));
	QPushButton * down = new QPushButton(qpdown, "", this);
	down->setToolTip(qt_("Next command"));
	down->setMaximumSize(24, 24);
	connect(down, SIGNAL(clicked()), this, SLOT(down()));

	edit_ = new QCommandEdit(this);
	edit_->setMinimumSize(edit_->sizeHint());
	edit_->setFocusPolicy(Qt::ClickFocus);

	connect(edit_, SIGNAL(escapePressed()), this, SLOT(cancel()));
	connect(edit_, SIGNAL(returnPressed()), this, SLOT(dispatch()));
	connect(edit_, SIGNAL(tabPressed()), this, SLOT(complete()));
	connect(edit_, SIGNAL(upPressed()), this, SLOT(up()));
	connect(edit_, SIGNAL(downPressed()), this, SLOT(down()));
	connect(edit_, SIGNAL(hidePressed()), this, SLOT(hideParent()));

	layout->addWidget(up, 0);
	layout->addWidget(down, 0);
	layout->addWidget(edit_, 10);
	layout->setMargin(0);
	top->addLayout(layout);
	top->setMargin(0);
	setFocusProxy(edit_);
}


void QCommandBuffer::cancel()
{
	view_->setFocus();
	edit_->setText(QString());
}


void QCommandBuffer::dispatch()
{
	controller_.dispatch(fromqstr(edit_->text()));
	view_->setFocus();
	edit_->setText(QString());
	edit_->clearFocus();
}


void QCommandBuffer::complete()
{
	string const input = fromqstr(edit_->text());
	string new_input;
	vector<string> comp = controller_.completions(input, new_input);

	if (comp.empty() && new_input == input) {
		// show_info_suffix(qt_("[no match]"), input);
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
	for (; cit != end; ++cit)
		list->addItem(toqstr(*cit));

	list->resize(list->sizeHint());
	QPoint const pos = edit_->mapToGlobal(QPoint(0, 0));

	int const y = std::max(0, pos.y() - list->height());

	list->move(pos.x(), y);

	connect(list, SIGNAL(itemPressed(QListWidgetItem *)),
		this, SLOT(complete_selected(QListWidgetItem *)));
	connect(list, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(complete_selected(QListWidgetItem *)));

	list->show();
	list->setFocus();
}


void QCommandBuffer::complete_selected(QListWidgetItem * item)
{
	QWidget const * widget = static_cast<QWidget const *>(sender());
	const_cast<QWidget *>(widget)->hide();
	edit_->setText(item->text() + ' ');
	edit_->activateWindow();
	edit_->setFocus();
}


void QCommandBuffer::up()
{
	string const input = fromqstr(edit_->text());
	string const h = controller_.historyUp();

	if (h.empty()) {
	//	show_info_suffix(qt_("[Beginning of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


void QCommandBuffer::down()
{
	string const input = fromqstr(edit_->text());
	string const h = controller_.historyDown();

	if (h.empty()) {
	//	show_info_suffix(qt_("[End of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


void QCommandBuffer::hideParent()
{
	view_->setFocus();
	edit_->setText(QString());
	edit_->clearFocus();
	controller_.hide();
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

#include "QCommandBuffer_moc.cpp"
