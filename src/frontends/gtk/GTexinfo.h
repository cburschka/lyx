// -*- C++ -*-
/**
 * \file GTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTEXINFO_H
#define GTEXINFO_H

#include "ControlTexinfo.h"

#include "GViewBase.h"

#include <map>

namespace lyx {
namespace frontend {

class GTexinfo
	: public GViewCB<ControlTexinfo, GViewGladeB> {
public:

	GTexinfo(Dialog &);
private:
	// not needed
	virtual void apply() {}
	// Build the dialog.
	virtual void doBuild();
	// not needed
	virtual void update() {}

	void updateStyles();

	ControlTexinfo::texFileSuffix activeStyle;

	Gtk::TreeView * itemsview_;
	Gtk::ComboBox * typecombo_;
	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModelColumn<unsigned int> listColIndex_;
	Gtk::TreeModel::ColumnRecord listCols_;
	Glib::RefPtr<Gtk::ListStore> itemsstore_;
	Glib::RefPtr<Gtk::ListStore> typestore_;
	Glib::RefPtr<Gtk::TreeSelection> listSel_;

	Gtk::CheckButton * fullpathcheck_;

	void onTypeComboChanged();
 	void onItemActivate(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * col);
	void onRefresh();

	typedef std::vector<std::string> ContentsType;
	std::map<ControlTexinfo::texFileSuffix, ContentsType> texdata_;
};

} // namespace frontend
} // namespace lyx

#endif // GTEXINFO_H
