// -*- C++ -*-
/**
 * \file GSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GSENDTO_H
#define GSENDTO_H

#include "GViewBase.h"
#include <vector>

class Format;

namespace lyx {
namespace frontend {

class ControlSendto;

/** This class provides a GTK+ implementation of the Sendto Dialog.
 */
class GSendto : public GViewCB<ControlSendto, GViewGladeB> {
public:
	GSendto(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	Gtk::TreeModelColumn<Glib::ustring> stringcol_;
	Gtk::TreeModelColumn<unsigned int> indexcol_;
	Gtk::TreeModel::ColumnRecord cols_;
	Glib::RefPtr<Gtk::ListStore> formatstore_;

	Gtk::TreeView * formatview_;
	Gtk::Entry * commandentry_;

	std::vector<Format const *> all_formats_;

	void onCommandEntryChanged();
};

} // namespace frontend
} // namespace lyx

#endif // GSENDTO_H
