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
	QWidget::resizeEvent(e);
 
	lyxerr[Debug::GUI] << "resize panel to "
		<< e->size().width() << "," << e->size().height() << endl;
 
	int maxcol = e->size().width() / button_size;
 
	if (!layout_->isEmpty() && maxcol == maxcol_)
		return;

	int cols(width() / button_size);
	int rows = max(int(buttons_.size() / cols), 1);
	if (buttons_.size() % cols)
		++rows;

	lyxerr[Debug::GUI] << "Laying out " << buttons_.size() << " widgets in a "
		<< cols << "x" << rows << " grid." << endl;
 
	setUpdatesEnabled(false);
 
	// clear layout
	QLayoutIterator lit = layout_->iterator();
	while (lit.current()) {
		lit.takeCurrent();
	}
		 
	layout_->invalidate();
 
	vector<Button>::const_iterator it(buttons_.begin());
	vector<Button>::const_iterator const end(buttons_.end());

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			layout_->addWidget(it->first, i, j);
			++it;
			if (it == end)
				goto out;
		}
	}

out:
 
	resize(cols * button_size, rows * button_size);

	maxcol_ = cols;
 
	setUpdatesEnabled(true);
	update();
}
