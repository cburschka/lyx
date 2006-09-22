// -*- C++ -*-
/**
 * \file gtk/Selection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SELECTION_H
#define SELECTION_H

#include "frontends/Selection.h"

#include <gtkmm.h>
#include <gtk/gtk.h>

namespace lyx {
namespace frontend {

/**
 * The GTK version of the Selection.
 */
class GuiSelection: public lyx::frontend::Selection
{
public:
	GuiSelection() {}

	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool own);

	docstring const get() const;

	void put(docstring const & str);
	//@}
private:
	void onClipboardGet(Gtk::SelectionData & selection_data, guint info);
	void onClipboardClear();
};

} // namespace frontend
} // namespace lyx

#endif // SELECTION_H
