// -*- C++ -*-
/**
 * \file CategorizedCombo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CATEGORIZEDCOMBO_H
#define LYX_CATEGORIZEDCOMBO_H

#include "support/strfwd.h"

#include <QComboBox>


namespace lyx {
namespace frontend {

class CCItemDelegate;

/**
 * A combo box with categorization
 */
class CategorizedCombo : public QComboBox
{
	Q_OBJECT
public:
	CategorizedCombo(QWidget * parent);
	~CategorizedCombo();

	/// select an item in the combobox. Returns false if item does not exist
	bool set(QString const & cc);
	/// Reset the combobox.
	void reset();
	/// Reset the combobox filter.
	void resetFilter();
	/// Update combobox.
	void updateCombo();
	/// Add Item to combo according to sorting settings from preferences
	void addItemSort(QString const & item, QString const & guiname,
			 QString const & category, QString const & tooltip,
			 bool sorted, bool sortedByCat, bool sortCats,
			 bool available);
	///
	QString getData(int row) const;
	///
	void setTooltipTemplate(QString const & tooltip,
						    QString const & unavail = QString());

	///
	void showPopup() override;

	///
	bool eventFilter(QObject * o, QEvent * e) override;
	///
	QString const & filter() const;

private Q_SLOTS:
	///
	void setIconSize(QSize size);

private:
	friend class CCItemDelegate;
	///
	struct Private;
	///
	Private * const d;
};


} // namespace frontend
} // namespace lyx

#endif // LYX_CATEGORIZEDCOMBO_H
