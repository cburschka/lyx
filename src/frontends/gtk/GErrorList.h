// -*- C++ -*-
/**
 * \file GErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GERRORLIST_H
#define GERRORLIST_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlErrorList;

/** This class provides a GTK+ implementation of the ErrorList Dialog.
 */
class GErrorList : public GViewCB<ControlErrorList, GViewGladeB> {
public:
	///
	GErrorList(Dialog &);
private:
	/// not needed
	virtual void apply() {}
	/// Build the dialog
	virtual void doBuild();
	/// Update dialog before showing it
	virtual void update();

	void updateContents();
	void onErrListSelection();

	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModelColumn<int> listColIndex_;
	Gtk::TreeModel::ColumnRecord listCols_;

	Glib::RefPtr<Gtk::ListStore> errliststore_;
	Glib::RefPtr<Gtk::TreeSelection> errlistsel_;
	Gtk::TreeView * errlistview_;

	Gtk::TextView * errdescview_;
};

} // namespace frontend
} // namespace lyx

#endif // GERRORLIST_H
