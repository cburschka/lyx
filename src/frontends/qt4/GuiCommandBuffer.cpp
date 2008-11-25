/**
 * \file GuiCommandBuffer.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCommandBuffer.h"

#include "GuiCommandEdit.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "BufferView.h"
#include "Cursor.h"
#include "LyXFunc.h"
#include "LyXAction.h"
#include "FuncRequest.h"

#include "support/lyxalgo.h"
#include "support/lstrings.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QToolTip>
#include <QVBoxLayout>

using namespace std;
using namespace lyx::support;

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


GuiCommandBuffer::GuiCommandBuffer(GuiView * view)
	: view_(view), history_pos_(history_.end())
{
	transform(lyxaction.func_begin(), lyxaction.func_end(),
		back_inserter(commands_), firster());

	QPixmap qpup(":/images/up.png");
	QPixmap qpdown(":/images/down.png");

	QVBoxLayout * top = new QVBoxLayout(this);
	QHBoxLayout * layout = new QHBoxLayout(0);

	QPushButton * up = new QPushButton(qpup, "", this);
	up->setMaximumSize(24, 24);
	QPushButton * down = new QPushButton(qpdown, "", this);
	down->setToolTip(qt_("Next command"));
	down->setMaximumSize(24, 24);
	connect(down, SIGNAL(clicked()), this, SLOT(down()));
	connect(up, SIGNAL(clicked()), this, SLOT(up()));

	edit_ = new GuiCommandEdit(this);
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


void GuiCommandBuffer::cancel()
{
	view_->setFocus();
	edit_->setText(QString());
}


void GuiCommandBuffer::dispatch()
{
	QString cmd = edit_->text();
	view_->setFocus();
	edit_->setText(QString());
	edit_->clearFocus();
	dispatch(fromqstr(cmd));
}


void GuiCommandBuffer::complete()
{
	string const input = fromqstr(edit_->text());
	string new_input;
	vector<string> comp = completions(input, new_input);

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

	int const y = max(0, pos.y() - list->height());

	list->move(pos.x(), y);

	connect(list, SIGNAL(itemPressed(QListWidgetItem *)),
		this, SLOT(complete_selected(QListWidgetItem *)));
	connect(list, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(complete_selected(QListWidgetItem *)));

	list->show();
	list->setFocus();
}


void GuiCommandBuffer::complete_selected(QListWidgetItem * item)
{
	QWidget const * widget = static_cast<QWidget const *>(sender());
	const_cast<QWidget *>(widget)->hide();
	edit_->setText(item->text() + ' ');
	edit_->activateWindow();
	edit_->setFocus();
}


void GuiCommandBuffer::up()
{
	string const input = fromqstr(edit_->text());
	string const h = historyUp();

	if (h.empty()) {
	//	show_info_suffix(qt_("[Beginning of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


void GuiCommandBuffer::down()
{
	string const input = fromqstr(edit_->text());
	string const h = historyDown();

	if (h.empty()) {
	//	show_info_suffix(qt_("[End of history]"), input);
	} else {
		edit_->setText(toqstr(h));
	}
}


void GuiCommandBuffer::hideParent()
{
	view_->setFocus();
	edit_->setText(QString());
	edit_->clearFocus();
	hide();
}


namespace {

class prefix_p {
public:
	string p;
	prefix_p(string const & s) : p(s) {}
	bool operator()(string const & s) const { return prefixIs(s, p); }
};

} // end of anon namespace


string const GuiCommandBuffer::historyUp()
{
	if (history_pos_ == history_.begin())
		return string();

	return *(--history_pos_);
}


string const GuiCommandBuffer::historyDown()
{
	if (history_pos_ == history_.end())
		return string();
	if (history_pos_ + 1 == history_.end())
		return string();

	return *(++history_pos_);
}


docstring const GuiCommandBuffer::getCurrentState() const
{
	return view_->view()->cursor().currentState();
}


void GuiCommandBuffer::hide() const
{
	FuncRequest cmd(LFUN_COMMAND_EXECUTE, "off");
	theLyXFunc().setLyXView(view_);
	lyx::dispatch(cmd);
}


vector<string> const
GuiCommandBuffer::completions(string const & prefix, string & new_prefix)
{
	vector<string> comp;

	copy_if(commands_.begin(), commands_.end(),
		back_inserter(comp), prefix_p(prefix));

	if (comp.empty()) {
		new_prefix = prefix;
		return comp;
	}

	if (comp.size() == 1) {
		new_prefix = comp[0];
		return vector<string>();
	}

	// find maximal available prefix
	string const tmp = comp[0];
	string test = prefix;
	if (tmp.length() > test.length())
		test += tmp[test.length()];
	while (test.length() < tmp.length()) {
		vector<string> vtmp;
		copy_if(comp.begin(), comp.end(),
			back_inserter(vtmp), prefix_p(test));
		if (vtmp.size() != comp.size()) {
			test.erase(test.length() - 1);
			break;
		}
		test += tmp[test.length()];
	}

	new_prefix = test;
	return comp;
}


void GuiCommandBuffer::dispatch(string const & str)
{
	if (str.empty())
		return;

	history_.push_back(str);
	history_pos_ = history_.end();
	FuncRequest func = lyxaction.lookupFunc(str);
	func.origin = FuncRequest::COMMANDBUFFER;
	theLyXFunc().setLyXView(view_);
	lyx::dispatch(func);
}

} // namespace frontend
} // namespace lyx

#include "GuiCommandBuffer_moc.cpp"
