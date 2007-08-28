// -*- C++ -*-
/**
 * \file QCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 * \author Richard Heck (adapted to QSelectionManager)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATION_H
#define QCITATION_H

#include "frontends/controllers/ControlCitation.h"
#include "support/docstring.h"

#include <QStringList>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class QCitation : public ControlCitation
{
public:
	///
	QCitation(Dialog &);
	virtual ~QCitation() {}
	virtual bool initialiseParams(std::string const & data);

	///
	void init();

	/// Available keys
	QStringListModel * available() { return &available_model_; }

	/// Selected keys
	QStringListModel * selected() { return &selected_model_; }

	/// Text before cite
	QString textBefore();

	/// Text after cite
	QString textAfter();

	/// Get key description
	QString getKeyInfo(QString const &);

	/// Clear selected keys
	void clearSelection();
	
	/// Return a list of available fields 
	QStringList getFieldsAsQStringList();
	
	/// Return a list of available fields 
	QStringList getEntriesAsQStringList();
	
	/// Find keys containing a string.
	void findKey(
		QString const & str, //< string expression
		bool only_keys, //< set to true if only keys shall be searched.
		docstring field, //<field to search, empty for all fields
		docstring entryType, //<entry type to display, empty for all
		bool case_sensitive, //< set to true for case sensitive search.
		bool reg_exp, //< set to true if \c str is a regular expression.
		bool reset = false //< whether to reset and search all keys
		);

	/// List of example cite strings
	QStringList citationStyles(int);

	/// Set the Params variable for the Controller.
	virtual void apply(int const choice, bool const full, bool const force,
					  QString before, QString after);
	
	void setCitedKeys();

private:
	/// available keys.
	QStringListModel available_model_;

	/// selected keys.
	QStringListModel selected_model_;

	/// All keys.
	QStringList all_keys_;
	
	/// Cited keys.
	QStringList cited_keys_;
};


} // namespace frontend
} // namespace lyx

#endif // QCITATION_H
