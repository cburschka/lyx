// -*- C++ -*-
/**
 * \file LyXToolBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTOOLBOX_H
#define LYXTOOLBOX_H

#include <QToolBox>

namespace lyx {
namespace frontend {

// The purpose of this custom widget is to allow the use of a QToolBox in a
// limited area. The stock QToolBox does not provide a minimum size hint that
// depends on the size of the pages; it assumes that there is enough room.  This
// subclass sets the minimal size of the QToolbox. Without this, the size of the
// QToolbox is only determined by values in the ui file and therefore causes
// portability and localisation issues. Note that the computation of the minimum
// size hint depends on the minimum size hints of the page widgets. Therefore
// page widgets must have a layout with layoutSizeContraint = SetMinimumSize or
// similar.
class LyXToolBox : public QToolBox
{
	Q_OBJECT

public:
	LyXToolBox(QWidget * p = 0, Qt::WindowFlags f = 0) : QToolBox(p, f) {}
	QSize minimumSizeHint() const;

protected:
	void showEvent(QShowEvent * e);
};


} // namespace frontend
} // namespace lyx


#endif // LYXTOOLBOX_H
