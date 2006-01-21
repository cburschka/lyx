// -*- C++ -*-
/**
 * \file GCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GCITATION_H
#define GCITATION_H

#include "GViewBase.h"

#include "ControlCitation.h"

#include "bufferparams.h"


namespace lyx {
namespace frontend {

class ControlCitation;

/** This class provides a GTK+ implementation of the Citation Dialog.
 */
class GCitation : public GViewCB<ControlCitation, GViewGladeB> {
public:
	GCitation(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	/// Update dialog before/whilst showing it.
	virtual void update_contents();

	/// fill the styles combo
	void fill_styles();

	/// set the styles combo
	void update_style();

	/// enable the apply button if applylock_ is false
	void enable_apply();

	/// Move selected inset key up one line in list
	void up();
	/// Move selected inset key down one line in list
	void down();

	/// Add bibliography key to inset key list, remove it from bibliography view
	void add();
	/// Remove inset key from list, move it back to bibliography key view
	void remove();

	/// Find previous occurence of search string in selected key view
	void previous();
	/// Find next occurence of search string in selected key view
	void next();
	/// Find function, called by previous() and next()
	void find(biblio::Direction);

	/// Called when inset key is (un)selected
	void cite_selected();
	/// Called when bibliography key is (un)selected
	void bib_selected();

	/** Set previous and next buttons according to state of the search
	    string entry, the radio buttons and the selected keys in the inset
	    and bibliography views.
	 */
 	inline void set_search_buttons();

	/** Helper function for bibFilter_; true if argument's 
	    [bibColumns.cite] is false.
	 */
	static inline bool bib_visible(const Gtk::TreeModel::const_iterator&); 

	/** apply() won't act when this is true. 
	    true if no text is selected when the citation dialog is opened 
	 */
	bool applylock_;

	/// Last selected stylecombo_ item
	int style_;

	Gtk::Button * restorebutton_;
	Gtk::Button * cancelbutton_;
	Gtk::Button * okbutton_;
	Gtk::Button * applybutton_;

	Gtk::Button * addbutton_;
	Gtk::Button * removebutton_;
	Gtk::Button * backbutton_;
	Gtk::Button * forwardbutton_;
	Gtk::Button * upbutton_;
	Gtk::Button * downbutton_;

	Gtk::TreeView * citekeysview_;
	Gtk::TreeView * bibkeysview_;

	Gtk::TextView * infoview_;

	Gtk::Entry * findentry_;
	Gtk::CheckButton * citeradio_;
	Gtk::CheckButton * bibradio_;
	Gtk::CheckButton * casecheck_;
	Gtk::CheckButton * regexpcheck_;

	Gtk::Label * stylelabel_;
	Gtk::ComboBox * stylecombo_;

	Gtk::Label * beforelabel_;
	Gtk::Entry * beforeentry_;
	Gtk::Entry * afterentry_;
	Gtk::CheckButton * authorcheck_;
	Gtk::CheckButton * uppercasecheck_;

	Glib::RefPtr<Gtk::TextBuffer> info_;  	

	Glib::RefPtr<Gtk::ListStore> allListStore_;
	Glib::RefPtr<Gtk::ListStore> styleStore_;

	Glib::RefPtr<Gtk::TreeSelection> citeselection_;
	Glib::RefPtr<Gtk::TreeSelection> bibselection_;

	Glib::RefPtr<Gtk::TreeModelFilter> citeFilter_;
	Glib::RefPtr<Gtk::TreeModelFilter> bibFilter_;
	Glib::RefPtr<Gtk::TreeModelSort> bibSort_;

};

} // namespace frontend
} // namespace lyx

#endif // GCITATION_H
