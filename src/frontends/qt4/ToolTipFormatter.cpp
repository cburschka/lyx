/**
 * \file ToolTipFormatter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ToolTipFormatter.h"
#include "qt_helpers.h"

#include <QAbstractItemView>
#include <QTextDocument>
#include <QTextLayout>
#include <QToolTip>


//#include "support/debug.h"
//#include "support/lstrings.h"


namespace lyx {
namespace frontend {


ToolTipFormatter::ToolTipFormatter(QObject * parent) : QObject(parent) {}


bool ToolTipFormatter::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() != QEvent::ToolTip)
		return false;

	// Format the tooltip of the widget being considered.
	QWidget * w = qobject_cast<QWidget *>(o);
	if (!w)
		return false;
	// Unchanged if empty or already formatted
	w->setToolTip(formatToolTip(w->toolTip()));

	// Now, if the tooltip is for an item in a QListView or a QTreeView,
	// then the widget above was probably not the one with the tooltip.
	// Check if the parent is a QAbstractItemView.
	QAbstractItemView * iv = qobject_cast<QAbstractItemView *>(w->parent());
	if (!iv)
		return false;
	// In this case, the item is retrieved from the position of the QHelpEvent
	// on the screen.
	QPoint pos = static_cast<QHelpEvent *>(e)->pos();
	QModelIndex item = iv->indexAt(pos);
	QVariant data = iv->model()->data(item, Qt::ToolTipRole);
	if (data.isValid() && data.typeName() == toqstr("QString"))
		// Unchanged if empty or already formatted
		iv->model()->setData(item, formatToolTip(data.toString()),
		                     Qt::ToolTipRole);
	// We must let the tooltip event reach its destination.
	return false;
}


} // namespace frontend
} // namespace lyx

#include "moc_ToolTipFormatter.cpp"
