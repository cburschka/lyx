/**
 * \file iconpalette.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "debug.h"
 
#include "iconpalette.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qtooltip.h>

using std::endl;
using std::make_pair;
using std::vector;
using std::max;
 
int const button_size = 40;
 
IconPalette::IconPalette(QWidget * parent, char const * name)
	: QWidget(parent, name), maxcol_(-1)
{
	QVBoxLayout * top = new QVBoxLayout(this);
	QHBoxLayout * row = new QHBoxLayout(top);
	layout_ = new QGridLayout(row);
	row->addStretch(0);
	top->addStretch(0);
}


void IconPalette::add(QPixmap const & pixmap, string name, string tooltip)
{
	QPushButton * p = new QPushButton(this);
	p->setFixedSize(button_size, button_size);
	p->setPixmap(pixmap);
	QToolTip::add(p, tooltip.c_str());
	connect(p, SIGNAL(clicked()), this, SLOT(clicked())); 
	buttons_.push_back(make_pair(p, name));
}


void IconPalette::clicked()
{
	vector<Button>::const_iterator it(buttons_.begin());
	vector<Button>::const_iterator const end(buttons_.end());
	for (; it != end; ++it) {
		if (sender() == it->first) {
			emit button_clicked(it->second);
			return;
		}
	}
}


void IconPalette::resizeEvent(QResizeEvent * e)
{
	lyxerr << "resize panel to " << e->size().width() << "," << e->size().height() << endl;
 
	int maxcol = e->size().width() / button_size;
 
	if (!layout_->isEmpty() && maxcol == maxcol_)
		return;

	lyxerr << "doing layout !" << maxcol << " " << width() << endl;
	lyxerr << "before is " << maxcol_ << endl; 
 
	setUpdatesEnabled(false);
 
	// clear layout
	QLayoutIterator lit = layout_->iterator();
	while (lit.current()) {
		lit.takeCurrent();
	}
		 
	layout_->invalidate();
 
	vector<Button>::const_iterator it(buttons_.begin());
	vector<Button>::const_iterator const end(buttons_.end());

	int row = 0;
	int col = 0;
	 
	for (; it != end; ++it) {
		layout_->addWidget(it->first, row, col++);
		if (col >= maxcol) {
			col = 0;
			++row;
		}
	}

	maxcol_ = maxcol;
 
	// this is OK because width won't change, and we have the check above
	setGeometry(x(), y(), width(), (row + 1) * button_size);
 
	repaint();
	lyxerr << "after is " << row << "," << maxcol << endl; 
	setUpdatesEnabled(true);
	update();
}
