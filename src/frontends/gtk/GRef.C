/**
 * \file GRef.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 * \author Andreas Klostermann
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GRef.h"
#include "ControlRef.h"
#include "ghelpers.h"
#include "insets/insetref.h"
#include "debug.h"
#include "buffer.h"
#include "insets/insetnote.h"

#include <libglademm.h>

using std::string;
using std::vector;


namespace lyx {
namespace frontend {

class refModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

  refModelColumns()
    { add(name);}

  Gtk::TreeModelColumn<Glib::ustring> name;
};

refModelColumns refColumns;


class bufferModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

  bufferModelColumns()
    { add(name);}

  Gtk::TreeModelColumn<Glib::ustring> name;
};

bufferModelColumns bufferColumns;


GRef::GRef(Dialog & parent)
	: GViewCB<ControlRef, GViewGladeB>(parent, _("Cross-reference"), false)
{}


void GRef::doBuild()
{
	string const gladeName = findGladeFile("ref");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	xml_->get_widget("Cancel", cancelbutton_);
	setCancel(cancelbutton_);
	xml_->get_widget("Apply", applybutton_);
	setApply(applybutton_);
	xml_->get_widget("OK", okbutton_);
	setOK(okbutton_);

	xml_->get_widget("Labels", refview_);
	xml_->get_widget("Label", labelentry_);
	xml_->get_widget("Name", nameentry_);
	xml_->get_widget("Format", formatcombo_);
	xml_->get_widget("Buffer", buffercombo_ );
	xml_->get_widget("JumpTo", jumptobutton_);
	xml_->get_widget("Back", backbutton_);
	xml_->get_widget("Refresh", refreshbutton_);

	refview_->append_column(_("Label"), refColumns.name);

	buffercombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GRef::buffer_changed));
	refview_->signal_cursor_changed().connect(
		sigc::mem_fun(*this, &GRef::selection_changed));
	refview_->signal_row_activated().connect(
		sigc::mem_fun(*this, &GRef::refview_activated));
	jumptobutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GRef::jumpto));
	backbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GRef::back));
	refreshbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GRef::update_labels));
	
	labelentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GRef::update_validity));
	formatcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GRef::update_validity));
	nameentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GRef::update_validity));
	
	applylock_ = false;
	bc().valid(false);
}


void GRef::selection_changed ()
{
	if (applylock_)
		return;

	Gtk::TreeModel::iterator iter = refview_->get_selection()->get_selected();
	if(iter) {
		Gtk::TreeModel::Row row = *iter;
		labelentry_->set_text(row[refColumns.name]);		
	}
}


void GRef::jumpto()
{
	
	if (backbutton_->is_sensitive()) {
//		controller().gotoAnotherRef(labelentry_->get_text());
//		OR
//		kernel().dispatch(FuncRequest(LFUN_LABEL_GOTO, ref));
//		OR
		controller().gotoBookmark();
		controller().gotoRef(labelentry_->get_text());
	} else {
		controller().gotoRef(labelentry_->get_text());
	}
	backbutton_->set_sensitive(true);
}


void GRef::back()
{
	controller().gotoBookmark();	
	backbutton_->set_sensitive(false);
	jumptobutton_->set_sensitive(true);
}


void GRef::buffer_changed()
{
	if (applylock_)
		return;
	update_labels();
}


void GRef::update()
{
	applylock_ = true;
	
	bc().refreshReadOnly();
	jumptobutton_->set_sensitive(true);
	backbutton_->set_sensitive(false);
	labelentry_->set_text(controller().params().getContents());
	nameentry_->set_text(controller().params().getOptions());

	// Name is irrelevant to LaTeX/Literate documents
	Kernel::DocType doctype = kernel().docType();
	if (doctype == Kernel::LATEX || doctype == Kernel::LITERATE) {
		nameentry_->set_sensitive(false);
	} else {
		nameentry_->set_sensitive(true);
	}

	// Format is irrelevant to LinuxDoc/DocBook.
	if (doctype == Kernel::LINUXDOC || doctype == Kernel::DOCBOOK) {
		formatcombo_->set_active(0);
		formatcombo_->set_sensitive(false);
		
	} else {
		formatcombo_->set_active(InsetRef::getType(controller().params().getCmdName()));
		formatcombo_->set_sensitive(true);
	}
	
	bufferstore_ = Gtk::ListStore::create(bufferColumns);
	vector<string> const buffers = controller().getBufferList();
	buffercombo_->set_model(bufferstore_);

	vector<string>::const_iterator it = buffers.begin();
	vector<string>::const_iterator const end = buffers.end();
	for (; it != end; ++it) {
	 	Gtk::TreeModel::iterator iter = bufferstore_->append();
	 	(*iter)[bufferColumns.name]  = *it;
	}

	buffercombo_->set_active(controller().getBufferNum());

	update_labels();
	applylock_ = false;
	bc().valid(false);
}


void GRef::update_labels()
{
	int buffernum = buffercombo_->get_active_row_number();
	if (buffernum < 0)
		buffernum=0;

	string const name = controller().getBufferName(buffernum);
	vector<string> keys = controller().getLabelList(name);
	refListStore_ = Gtk::ListStore::create(refColumns);

	if (!keys.empty()) {
		vector<string>::const_iterator it = keys.begin();
		vector<string>::const_iterator end = keys.end();
		for (;it != keys.end(); ++it) {
			Gtk::TreeModel::iterator iter =refListStore_->append();
			(*iter)[refColumns.name] = *it;
		}
		refview_->set_sensitive(true);
	} else {
		Gtk::TreeModel::iterator iter =refListStore_->append();
		(*iter)[refColumns.name] = _("No labels found.");
		refview_->set_sensitive(false);
	}
	refview_->set_model(refListStore_);
}


void GRef::apply()
{
	if (applylock_)
		return;

	controller().params().setContents(labelentry_->get_text());
	controller().params().setOptions(nameentry_->get_text());
	int const type = formatcombo_->get_active_row_number();
	controller().params().setCmdName(InsetRef::getName(type));
}


void GRef::update_validity()
{
	bc().valid(!labelentry_->get_text().empty());
}


void GRef::refview_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)
{
	if (!labelentry_->get_text().empty())
		okbutton_->clicked();
}


} // namespace frontend
} // namespace lyx
