/**
 * \file DialogView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "DialogView.h"

#include "GuiView.h"
#include "qt_helpers.h"

#include <QCloseEvent>
#include <QShowEvent>


namespace lyx {
namespace frontend {

DialogView::DialogView(GuiView & lv, std::string const & name)
	: QDialog(&lv), Dialog(lv, name)
{}


void DialogView::setViewTitle(docstring const & title)
{
	setWindowTitle("LyX: " + toqstr(title));
}

} // namespace frontend
} // namespace lyx

#include "DialogView_moc.cpp"
