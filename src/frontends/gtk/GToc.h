// -*- C++ -*-
/**
 * \file GToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTOC_H
#define GTOC_H

#include "GViewBase.h"
#include "toc.h"

namespace lyx {
namespace frontend {

class ControlToc;

/** This class provides a GTK+ implementation of the Toc Dialog.
 */
class GToc : public GViewCB<ControlToc, GViewGladeB> {
public:
	///
	GToc(Dialog &);
private:
	/// not needed
	virtual void apply() {}
	/// Build the dialog
	virtual void doBuild();
	/// Update dialog
	virtual void update();

	void updateType();
	void updateContents();

	void onTocViewSelected();
	void onTypeComboChanged();

	// Makes TocViewSelected ignore events
	bool changing_views_;

	Gtk::TreeView * tocview_;
	Gtk::ComboBox * typecombo_;
	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModelColumn<unsigned int> listColIndex_;
	Gtk::TreeModel::ColumnRecord listCols_;
	Glib::RefPtr<Gtk::ListStore> tocstore_;
	Glib::RefPtr<Gtk::ListStore> typestore_;
	Glib::RefPtr<Gtk::TreeSelection> listSel_;

	toc::Toc toc_;
};

} // namespace frontend
} // namespace lyx

#endif // GTOC_H
