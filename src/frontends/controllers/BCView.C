/**
 * \file BCView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "BCView.h"
#include "ButtonController.h"


CheckedWidget::~CheckedWidget()
{}


BCView::BCView(ButtonController const & p)
	: parent(p)
{}


ButtonPolicy & BCView::bp() const
{
	return parent.bp();
}


void BCView::addCheckedWidget(CheckedWidget * ptr)
{
	if (ptr)
		checked_widgets.push_back(checked_widget_ptr(ptr));
}


bool BCView::checkWidgets()
{
	bool valid = true;

	checked_widget_list::const_iterator it  = checked_widgets.begin();
	checked_widget_list::const_iterator end = checked_widgets.end();

	for (; it != end; ++it) {
		valid &= (*it)->check();
	}

	// return valid status after checking ALL widgets
	return valid;
}
