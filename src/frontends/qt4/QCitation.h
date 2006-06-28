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

#include "ControlCitation.h"

#include <QStringListModel>

namespace lyx {
namespace frontend {

class QCitation : public ControlCitation
{
public:
	///
	QCitation(Dialog &);
	/// Available keys
	QStringListModel * available()
	{ return &available_keys_; }

	/// Selected keys
	QStringListModel * selected()
	{ return &selected_keys_; }

	/// Found keys
	QStringListModel * found()
	{ return &found_keys_; }

	/// Text before cite
	QString textBefore();

	/// Text after cite
	QString textAfter();

	/// Get key description
	QString getKeyInfo(QString const &);

	/// Find keys containing the string (not case-sens)
	void findKey(QString const &);

	/// Add key to selected keys
	void addKey(QModelIndex const &);

	/// Delete key from selected keys
	void deleteKey(QModelIndex const &);

	/// Move selected key one place up
	void upKey(QModelIndex const &);

	/// Move selected key one place down
	void downKey(QModelIndex const &);

	/// List of example cite strings
	QStringList citationStyles(int);

	/// Check whether there are keys to select
	virtual bool isValid();

	/// Set the Params variable for the Controller.
	virtual void apply(int const choice, bool const full, bool const force,
					  QString before, QString after);

	/// Update dialog before/whilst showing it.
	virtual void updateModel();

private:	
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
