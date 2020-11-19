// -*- C++ -*-
/**
 * \file GuiClickableLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICLICKABLELABEL_H
#define GUICLICKABLELABEL_H

#include <QLabel>

namespace lyx {
namespace frontend {

// see https://wiki.qt.io/Clickable_QLabel
class GuiClickableLabel : public QLabel {
	Q_OBJECT
public:
	explicit GuiClickableLabel(QWidget * parent);

	~GuiClickableLabel();

Q_SIGNALS:
	void clicked();

protected:
	void mouseReleaseEvent(QMouseEvent *) override;
};

}
}
#endif
