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
 
IconPalette::IconPalette(QWidget * parent, char const * name)
	: QWidget(parent, name), crow_(0), ccol_(0)
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
	p->setFixedSize(40, 40);
	p->setPixmap(pixmap);
	QToolTip::add(p, tooltip.c_str());
	layout_->addWidget(p, crow_, ccol_);
	if (++ccol_ == 5) {
		ccol_ = 0;
		++crow_;
	}
	resize(5 * 40, crow_ * 40);
	button_map_[p] = name;
	connect(p, SIGNAL(clicked()), this, SLOT(clicked())); 
}


void IconPalette::clicked()
{
	string name = button_map_[(QPushButton*)(sender())];
	emit button_clicked(name);
}


void IconPalette::resizeEvent(QResizeEvent * e)
{
	lyxerr << "resize panel to " << e->size().width() << "," << e->size().height() << endl;
}
