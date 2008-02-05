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

using namespace std;

namespace lyx {
namespace frontend {

DialogView::DialogView(GuiView & lv, string const & name,
	QString const & title)
	: QDialog(&lv), Dialog(lv, name, "LyX: " + title)
{}

} // namespace frontend
} // namespace lyx

#include "DialogView_moc.cpp"
