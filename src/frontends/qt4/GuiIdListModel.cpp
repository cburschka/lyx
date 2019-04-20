/**
 * \file GuiIdListModel.cpp
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

#include <config.h>

#include "GuiIdListModel.h"

#include "support/lassert.h"

using std::vector;

namespace lyx {
namespace frontend {


// Note: Any role that is added here must also be added to setData().
QVariant GuiIdListModel::data(QModelIndex const & index, int role) const
{
	int const row = index.row();
	if (!rowIsValid(row))
		return QVariant();
	if (role == Qt::DisplayRole || role == Qt::EditRole)
		return userData_[ulong(row)].uiString;
	if (role == Qt::ToolTipRole) {
		QString const ttstr = userData_[ulong(row)].ttString.toString();
		return !ttstr.isEmpty() ? ttstr : userData_[ulong(row)].uiString;
	}
	if (role == Qt::UserRole)
		return userData_[ulong(row)].idString;
	return QVariant();
}


bool GuiIdListModel::setData(QModelIndex const & index,
		const QVariant & value, int role)
{
	int const row = index.row();
	if (!rowIsValid(row))
		return false;
	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		userData_[ulong(row)].uiString = value;
		dataChanged(index, index);
		return true;
	}
	if (role == Qt::UserRole) {
		userData_[ulong(row)].idString = value;
		dataChanged(index, index);
		return true;
	}
	if (role == Qt::ToolTipRole) {
		userData_[ulong(row)].ttString = value;
		dataChanged(index, index);
		return true;
	}
	if (role == Qt::DecorationRole)
		// nothing to do
		return true;
	// If we assert here, it's because we're trying to set an
	// unrecognized role.
	LATTEST(false);
	return false;
}


bool GuiIdListModel::insertRows(int row, int count,
		QModelIndex const & /*parent*/)
{
	if (!rowIsValid(row))
		return false;
	vector<OurData>::iterator it = userData_.begin() + row;
	beginInsertRows(QModelIndex(), row, row + count - 1);
	userData_.insert(it, ulong(count), OurData());
	endInsertRows();
	return true;
}


bool GuiIdListModel::removeRows(int row, int count,
		QModelIndex const & /*parent*/)
{
	if (!rowIsValid(row) || row + count > int(userData_.size()) ||
	    count < 0)
		return false;
	if (count == 0)
		return true;
	vector<OurData>::iterator it = userData_.begin() + row;
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	userData_.erase(it, it + count);
	endRemoveRows();
	return true;
}


void GuiIdListModel::insertRow(int const i, QString const & uiString,
		std::string const & idString)
{
	insertRow(i, uiString, idString, uiString);
}


void GuiIdListModel::insertRow(int const i, QString const & uiString,
	std::string const & idString, QString const & ttString)
{
	insertRows(i, 1);
	setUIString(i, uiString);
	setIDString(i, idString);
	setTTString(i, ttString);
}


QMap<int, QVariant> GuiIdListModel::itemData(QModelIndex const & index) const
{
	int const row = index.row();
	if (!rowIsValid(row))
		return QMap<int, QVariant>();
	QMap<int, QVariant> qm = QAbstractListModel::itemData(index);
	qm[Qt::UserRole] = userData_[ulong(row)].idString;
	return qm;
}


int GuiIdListModel::findIDString(std::string const & idString)
{
	vector<OurData>::const_iterator it  = userData_.begin();
	vector<OurData>::const_iterator end = userData_.end();
	for (; it != end; ++it)
		if (fromqstr(it->idString.toString()) == idString)
			return it - userData_.begin();
	return -1;
}

} // namespace frontend
} // namespace lyx

