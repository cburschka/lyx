/**
 * \file QWorkArea.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
#include "LyXView.h"
#include "lyxrc.h" // lyxrc.show_banner
#include "version.h" // lyx_version

#include "support/filetools.h" // LibFileSearch
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <cmath>
#include <cctype>

#include "QWorkArea.h"
 
#include <qapplication.h>
#include <qevent.h> 
#include <qpainter.h>
#include <qmainwindow.h>
#include <qlayout.h>
 
using std::endl;
using std::abs;
using std::hex;

 
QWorkArea::QWorkArea(int, int, int, int)
	: WorkArea(), QWidget(qApp->mainWidget()), painter_(*this)
{
	// Add a splash screen to the centre of the work area
	string const splash_file = (lyxrc.show_banner) ?
		LibFileSearch("images", "banner", "xpm") : string();

	if (!splash_file.empty()) {
		// FIXME
	}

	scrollbar_ = new QScrollBar(QScrollBar::Vertical, this);
	content_ = new QContentPane(this);

	(static_cast<QMainWindow*>(qApp->mainWidget()))->setCentralWidget(this);
 
	content_->show();

	content_->setBackgroundColor(lcolor.getX11Name(LColor::background).c_str());

	QHBoxLayout * vl = new QHBoxLayout(this);
	vl->addWidget(content_, 5);
	vl->addWidget(scrollbar_, 0);
 
	show();
}


QWorkArea::~QWorkArea()
{
}


void QWorkArea::resize(int xpos, int ypos, int width, int height)
{
	setGeometry(xpos, ypos, width, height);
}


void QWorkArea::setScrollbarParams(int h, int pos, int line_h)
{
	// do what cursor movement does (some grey)
	h += height() / 4;
 
	int max = h - height();
	if (max < 0)
		max = 0;
	scrollbar_->setRange(0, max);
	scrollbar_->setValue(pos);
	scrollbar_->setLineStep(line_h);
	scrollbar_->setPageStep(height());
}


void QWorkArea::haveSelection(bool ) const
{
	// FIXME 
}

 
string const QWorkArea::getClipboard() const 
{
	// FIXME 
	return "nothing"; 
}

	
void QWorkArea::putClipboard(string const &) const
{
	// FIXME
}
