// -*- C++ -*-
/**
 * \file QTocDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTOCDIALOG_H
#define QTOCDIALOG_H

#include "ui/QTocDialogBase.h"

namespace lyx {
namespace frontend {

class QToc;

class QTocDialog : public QTocDialogBase {
	Q_OBJECT
public:
	QTocDialog(QToc * form);
	~QTocDialog();
	///
	void enableButtons(bool enable = true);
public slots:
	void activate_adaptor(int);
	void depth_adaptor(int);
	void select_adaptor(QListViewItem *);
	void update_adaptor();
	void moveup_adaptor();
	void movedn_adaptor();
	void movein_adaptor();
	void moveout_adaptor();
protected:
	void closeEvent(QCloseEvent * e);
private:
	QToc * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOCDIALOG_H
