// -*- C++ -*-
/**
 * \file qt4/GuiSelection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SELECTION_H
#define SELECTION_H

#include "frontends/Selection.h"

#include <QObject>

namespace lyx {
namespace frontend {

/**
 * The Qt4 version of the Selection.
 */
class GuiSelection : public QObject, public Selection
{
	Q_OBJECT
public:
	GuiSelection();
	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool own);
	docstring const get() const;
	void put(docstring const & str);
	bool empty() const;
	//@}

private Q_SLOTS:
	void on_dataChanged();

private:
	bool text_selection_empty_;
	bool const selection_supported_;
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
