// -*- C++ -*-
/**
 * \file QViewSourceDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVIEWSOURCEDIALOG_H
#define QVIEWSOURCEDIALOG_H

#include "ui/QViewSourceUi.h"

#include "frontends/Application.h"

#include <QWidget>

namespace lyx {
namespace frontend {

class QViewSource;

class QViewSourceDialog : public QWidget, public Ui::QViewSourceUi {
	Q_OBJECT
public:
	QViewSourceDialog(QViewSource * form);

public Q_SLOTS:
	// update content
	void update();

private:
	QViewSource * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QVIEWSOURCEDIALOG_H
