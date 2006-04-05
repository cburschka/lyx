// -*- C++ -*-
/**
 * \file QCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATION_H
#define QCITATION_H

#include "QDialogView.h"

#include <QStringListModel>

namespace lyx {
namespace frontend {

class ControlCitation;
class QCitationDialog;


class QCitation : public QController<ControlCitation, QView<QCitationDialog> >
{
public:
	friend class QCitationDialog;
	///
	QCitation(Dialog &);

	QStringListModel * available()
	{ return &available_keys_; }

	QStringListModel * selected()
	{ return &selected_keys_; }

	QStringListModel * found()
	{ return &found_keys_; }

	QModelIndex findKey(QString const & str, QModelIndex const & index) const;
	QModelIndex findKey(QString const & str) const;

	void addKeys(QModelIndexList const & indexes);
	void deleteKeys(QModelIndexList const & indexes);
	void upKey(QModelIndexList const & indexes);
	void downKey(QModelIndexList const & indexes);

protected:
	virtual bool isValid();

private:

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build_dialog();
	/// Hide the dialog.
	virtual void hide();
	/// Update dialog before/whilst showing it.
	virtual void update_contents();

	/// available keys
	QStringListModel available_keys_;

	/// selected keys
	QStringListModel selected_keys_;

	/// found keys
	QStringListModel found_keys_;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATION_H
