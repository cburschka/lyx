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

	QStringListModel * available()
	{ return &available_keys_; }

	QStringListModel * selected()
	{ return &selected_keys_; }

	QStringListModel * found()
	{ return &found_keys_; }

	QString textBefore();
	QString textAfter();

	QModelIndex findKey(QString const & str, QModelIndex const & index) const;
	QModelIndex findKey(QString const & str) const;

	void addKeys(QModelIndexList const & indexes);
	void deleteKeys(QModelIndexList const & indexes);
	void upKey(QModelIndexList const & indexes);
	void downKey(QModelIndexList const & indexes);

	QStringList citationStyles(int sel);


	virtual bool isValid();

	/// Set the Params variable for the Controller.
	virtual void apply(int const choice, bool const full, bool const force,
					  QString before, QString After);

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


#if 0
/** A controller for Citation dialogs.
 */
class Citation {
public:
	///
	Citation();

	///
	virtual bool initialiseParams(std::string const & data);

	/// clean-up on hide.
	virtual void clearParams();

	/** Disconnect from the inset when the Apply button is pressed.
	 *  Allows easy insertion of multiple citations.
	 */
	virtual bool disconnectOnApply() const { return true; }

	/// Returns a reference to the map of stored keys
	biblio::InfoMap const & bibkeysInfo() const;

	///
	biblio::CiteEngine_enum getEngine() const;

	/// Possible citations based on this key
	std::vector<std::string> const getCiteStrings(std::string const & key) const;

	/// available CiteStyle-s (depends on availability of Natbib/Jurabib)
	static std::vector<biblio::CiteStyle> const & getCiteStyles() {
		return citeStyles_;
	}
private:
	/// The info associated with each key
	biblio::InfoMap bibkeysInfo_;

	///
	static std::vector<biblio::CiteStyle> citeStyles_;
};
#endif
} // namespace frontend
} // namespace lyx

#endif // QCITATION_H
