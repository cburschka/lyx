// -*- C++ -*-
/**
 * \file GExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GEXTERNAL_H
#define GEXTERNAL_H

#include "GViewBase.h"

#include "GtkLengthEntry.h"

#include "insets/ExternalTemplate.h"
#include "insets/insetexternal.h"


namespace lyx {
namespace frontend {


class ControlExternal;

/** This class provides a GTK+ implementation of the External Dialog.
 */
class GExternal : public GViewCB<ControlExternal, GViewGladeB> {
public:
	GExternal(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	/** Called if a different template is selected from the templates combo
	    Updates the dialog widgets accordingly
	*/
	void update_template();

	/** Called if the "Get from file" button in the Crop tab is pressed
	    Inserts bounding box coordinates from file into coordinate entries
	*/
	void get_bb();

	/** Checks if the Aspect Ratio CheckButton is to be activated
	    True if widthlength_'s unit is not Scale% and both widthlength and
	    heightlength_ are nonzero
	*/
	bool activate_ar() const;

	/** Calls controller().bbChanged() providing it with new coordinates
	    if they any of them were changed
	*/
	void bb_changed();

	/** Calls controller().editExternal() if corresponding button is 
	    clicked
	*/
	void edit_clicked();

	/** Sets Aspect Ratio CheckButton and Height Length sensitive or greys
	    them out depending on widthlength_'s and heightlength_'s status
	    Called if either widthlength_ or heightlength_ were changed
	*/
	void size_changed();

	/// enables the apply button
	void template_changed();

	/** Sets sensitivity of the LyxView tab's other widgets depending und 
	    the 'Show in LyX' CheckButton
	*/
	void showcheck_toggled();


	/** Sets sensitivity of the Crop tab's other widgets depending und 
	    the 'Clip to bounding box' CheckButton
	*/
	void clipcheck_toggled();

	/** Sets sensitivity of the 'Edit file' and 
	    'Get [bounding box] from file' buttons if there was a file selected
	*/
	void file_changed();

	/// Holds the currently selected template
	Gtk::TreeModel::iterator currenttemplate_;


	//  Widgets & couple of corresponding models

	Gtk::Button * cancelbutton_;
	Gtk::Button * okbutton_;
	Gtk::Button * applybutton_;

	Glib::RefPtr<Gtk::ListStore> templatestore_;
	Gtk::ComboBox * templatecombo_;

	Glib::RefPtr<Gtk::TextBuffer> templatebuffer_;  	
	Gtk::TextView * templateview_;

	Gtk::Notebook * notebook_;

	// *** Start "File" Page ***
	Gtk::Label * filelabel_;
	Gtk::FileChooserButton * templatefcbutton_; //requires gtkmm>=2.6
	Gtk::Button * editfilebutton_;
	Gtk::CheckButton * draftcheck_;
	// *** End "File" Page ***

	// *** Start "LyX View" Page ***
	Gtk::CheckButton * showcheck_;
	Gtk::Label * displaylabel_;
	Glib::RefPtr<Gtk::ListStore> displaystore_;
	Gtk::ComboBox * displaycombo_;
	Gtk::Label * scalelabel_;
	GtkLengthEntry * scalelength_;
	Gtk::ComboBoxText * scalecombo_;
	Gtk::SpinButton * scalespin_;
	Gtk::Label * percentlabel_;
	// *** End "LyX View" Page ***

	// *** Start "Rotate" Page ***
	Gtk::Entry * angleentry_;
	Gtk::Label * originlabel_;
	Gtk::ComboBoxText origincombo_;
	// *** End "Rotate" Page ***

	// *** Start "Scale" Page ***
	Gtk::CheckButton * archeck_;
	Gtk::Label * widthlabel_;
	GtkLengthEntry * widthlength_;
	Gtk::Label * heightlabel_;
	GtkLengthEntry * heightlength_;

	Gtk::ComboBoxText * widthcombo_;
	// *** End "Scale" Page ***

	// *** Start "Crop" Page ***
	Gtk::CheckButton * clipcheck_;
	Gtk::Button * bbfromfilebutton_;

	Gtk::Label * xlabel_;
	Gtk::Label * ylabel_;
	Gtk::Label * rtlabel_;
	Gtk::Label * lblabel_;

	Gtk::Entry * xlentry_;
	Gtk::Entry * ybentry_;
	Gtk::Entry * xrentry_;
	Gtk::Entry * ytentry_;
	// *** End "Crop" Page ***

	// *** Start "Options" Page ***
	Glib::RefPtr<Gtk::ListStore> formatstore_;
	Gtk::TreeView * optionsview_;
	// *** End "Options" Page ***

};

} // namespace frontend
} // namespace lyx

#endif // GEXTERNAL_H
