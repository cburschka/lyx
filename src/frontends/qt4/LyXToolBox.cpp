// -*- C++ -*-
/**
 * \file LyXToolBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include "LyXToolBox.h"

#include <QApplication>
#include <QLayout>

#include "support/debug.h"

namespace lyx {
namespace frontend {


QSize LyXToolBox::minimumSizeHint() const
{
	QSize s(0,0);
	// Compute the max of the minimal sizes of the pages
	QWidget * page;
	for (int i = 0; (page = widget(i)); ++i)
		s = s.expandedTo(page->minimumSizeHint());
	// Add the height of the tabs
	if (currentWidget())
		s.rheight() += height() - currentWidget()->height();
	return s;
}

void LyXToolBox::showEvent(QShowEvent * e)
{
	// Computation of the tab height might be incorrect yet (the proper sizes of
	// the pages have only been computed now).
	// It might still be incorrect after this. All this would be unnecessary if
	// QToolBox made our life easier and exposed more information; for instance
	// let us access the scroll areas enclosing the pages (from which one can
	// deduce the real tab height).
	layout()->invalidate();
	// proceed with geometry update to avoid flicker
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);
	QToolBox::showEvent(e);
}


} // namespace frontend
} // namespace lyx

#include "moc_LyXToolBox.cpp"

