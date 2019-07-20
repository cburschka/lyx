// -*- C++ -*-
/**
 * \file ToolTipFormatter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOOLTIPFORMATTER_H
#define TOOLTIPFORMATTER_H

#include <QObject>

class QEvent;

namespace lyx {
namespace frontend {


/// This event filter intercepts ToolTip events, to format any tooltip
/// appropriately before display.
class ToolTipFormatter : public QObject {
	Q_OBJECT
public:
	ToolTipFormatter(QObject * parent);
protected:
	bool eventFilter(QObject * o, QEvent * e);
};


} // namespace frontend
} // namespace lyx

#endif // TOOLTIPFORMATTER_H
