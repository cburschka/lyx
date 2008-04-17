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
	GuiIdListModel() {}
	//////////////////////////////////////////////////////////////////////
	// Methods overridden from QAbstractListModel
	//////////////////////////////////////////////////////////////////////
	///
	int rowCount(QModelIndex const & = QModelIndex()) const
		{ return userData_.size(); }

	///
	virtual QVariant data(QModelIndex const & index, 
	                      int role = Qt::DisplayRole) const;
	///
	bool insertRows(int row, int count, QModelIndex const & parent = QModelIndex());
	///
	bool removeRows(int row, int count, QModelIndex const & parent = QModelIndex());
	/// 
	void clear() { removeRows(0, rowCount()); }
	///
	virtual bool setData (QModelIndex const & index, 
			const QVariant & value, int role = Qt::EditRole );
	///
	virtual QMap<int, QVariant> itemData(QModelIndex const & index ) const;
	//////////////////////////////////////////////////////////////////////
	// New methods
	//////////////////////////////////////////////////////////////////////
	///
	void insertRow(int const i, QString const & uiString, 
			std::string const & idString, QString const & ttString);
	/// A convenience method, setting ttString to the same as uiString
	void insertRow(int const i, QString const & uiString, 
			std::string const & idString);
	/// \return the index of the (first) item with idString
	/// \return -1 if not found
	int findIDString(std::string const & idString);
	///
	virtual QString getIDString(QModelIndex const & index) const
		{ return data(index, Qt::UserRole).toString(); }
	///
	virtual std::string getIDString(int const i) const
		{ return fromqstr(getIDString(index(i))); }

private:
	/// noncopyable
	GuiIdListModel(GuiIdListModel const &);
	///
	void operator=(GuiIdListModel const &);
	///
	void setUIString(QModelIndex const & index, QString const & value)
		{ setData(index, value); }
	///
	void setUIString(int const i, QString const & value)
		{ setUIString(index(i), value); }
	///
	void setIDString(QModelIndex const & index, QString const & value)
		{ setData(index, value, Qt::UserRole); }
	///
	void setIDString(int const i, std::string const & value)
		{ setIDString(index(i), toqstr(value)); }
	///
	void setTTString(QModelIndex const & index, QString const & value)
		{ setData(index, value, Qt::ToolTipRole); }
	///
	void setTTString(int const i, QString const & value)
		{ setTTString(index(i), value); }
	struct OurData {
		/// Qt::DisplayRole and Qt::EditRole
		QVariant uiString;
		/// Qt::UserRole
		QVariant idString;
		/// Qt::ToolTipRole
		QVariant ttString;
	};
	///
	std::vector<OurData> userData_;
	///
	bool rowIsValid(int const i) const
	{
		return i >= 0 && i <= int(userData_.size());
	}
};


}
}
#endif //GUIIDLISTMODEL_H 
