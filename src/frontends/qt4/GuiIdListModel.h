// -*- C++ -*-
/**
 * \file GuiIdListModel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Some of this code is based upon qstringlistmodel.{h,cpp}, which is
 * part of the Qt toolkit, copyright (C) 1992-2007 Trolltech ASA, and
 * released under the General Public License.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIIDLISTMODEL_H
#define GUIIDLISTMODEL_H

#include "support/qstring_helpers.h"

#include <QAbstractListModel>
#include <vector>
#include <string>

namespace lyx {
namespace frontend {

/**
 * A QAbstractListModel that associates an identifying string with
 * each item, as well as a display string. The display string is set
 * with setUIString; the identifying string, with setIDString. 
 *
 * This is intended to be used, for example, with GuiSelectionManager.
 * In that case, one needs to recover from selectedModel which items 
 * have been selected. One may not wish to do so using the string that
 * is there *displayed*, since, among other things, that string may be
 * translated. So the id can be used to identify the items in this case.
 */
class GuiIdListModel : public QAbstractListModel {
public:
	///
	explicit GuiIdListModel() {};
	//////////////////////////////////////////////////////////////////////
	// Methods overridden from QAbstractListModel
	//////////////////////////////////////////////////////////////////////
	///
	inline int rowCount(QModelIndex const & parent = QModelIndex()) const;
	///
	virtual QVariant data(QModelIndex const & index, 
	                      int role = Qt::DisplayRole) const;
	///
	bool insertRows(int row, int count, QModelIndex const & parent = QModelIndex());
	///
	bool removeRows(int row, int count, QModelIndex const & parent = QModelIndex());
	/// 
	void clear() { removeRows(0, rowCount()); };
	///
	virtual bool setData (QModelIndex const & index, 
			const QVariant & value, int role = Qt::EditRole );
	///
	virtual QMap<int, QVariant> itemData(QModelIndex const & index ) const;
	//////////////////////////////////////////////////////////////////////
	// New methods
	//////////////////////////////////////////////////////////////////////
	///
	void setUIString(QModelIndex const & index, QString const & value)
			{ setData(index, value); };
	///
	void setUIString(int const i, std::string const & value)
			{  setUIString(index(i), toqstr(value));  };
	///
	void setIDString(QModelIndex const & index, QString const & value)
			{ setData(index, value, Qt::UserRole); };
	///
	void setIDString(int const i, std::string const & value)
			{ setIDString(index(i), toqstr(value)); };
	///
	virtual QString getIDString(QModelIndex const & index) const
			{ return data(index, Qt::UserRole).toString(); };
	///
	virtual std::string getIDString(int const i) const
			{  return fromqstr(getIDString(index(i))); };
	///
	void insertRow(int const i, std::string const & uiString, 
			std::string const & idString);
	/* The following functions are currently unused but are retained here in
	   case they should at some point be useful.
	///
	void setUIString(int const i, QString const & value)
			{ setUIString(index(i), value); };
	///
	void setIDString(int const i, QString const & value)
			{ setIDString(index(i), value); };
	///
	QStringList getIDStringList() const;
	///
	void insertRow(int const i, QString const & uiString, 
			QString const & idString);
	/// Returns whether the model contains an item with the given ID
	bool containsID(QVariant const &) const;
	*/
private:
	/// noncopyable
	GuiIdListModel(GuiIdListModel const &);
	///
	struct OurData {
		QVariant uiString;
		QVariant idString;
	};
	///
	std::vector<OurData> userData_;
	///
	inline bool rowIsValid(int const i) const
	{
		return i >= 0 && i <= int(userData_.size());
	}
;
};


//definition is here to silence a warning
inline int GuiIdListModel::rowCount(QModelIndex const &) const
		{ return userData_.size(); }

}
}
#endif //GUIIDLISTMODEL_H 
