/**
 * \file FormExternal.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon
 */

#ifndef FORMEXTERNAL_H
#define FORMEXTERNAL_H

#include "FormBase.h"
#include "insets/insetexternal.h"

#include "form_external.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The class for editing External insets via a dialog
class FormExternal : public FormBaseBD {
public:
	FormExternal(LyXView *, Dialogs *);

	~FormExternal();

	/// Connect signals. Also perform any necessary initialisation.
	virtual void connect();

	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Slot launching dialog to an existing inset
	void showInset(InsetExternal *);

	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool);

	/// Callback function for the template drop-down
	static void templateCB(FL_OBJECT *, long);

	/// Callback function for the browse button
	static void browseCB(FL_OBJECT *, long);

	/// Callback function for the edit button
	static void editCB(FL_OBJECT *, long);

	/// Callback function for the view button
	static void viewCB(FL_OBJECT *, long);

	/// Callback function for the update production button
	static void updateCB(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;

private:
	/// calculate the string to set the combo box
	string const getTemplatesComboString() const;

	/// get the position in the combo for a given name
	int getTemplateComboNumber(string const & name) const;

	/// get a template given its combo position
	ExternalTemplate getTemplate(int i) const;

	/// change widgets on change of chosen template
	void updateComboChange();
 
	/// build the dialog
	void build();

	/// the inset we're modifying
	InsetExternal * inset_;

	/// the parameters
	InsetExternal::InsetExternalParams params_;

	/// update the dialog
	void update();

	/// apply changes
	void apply();

	bool input(FL_OBJECT * obj, long data);

	/// inset::hide connection.
	Connection ih_;

	/// build the dialog
	FD_form_external * build_external();

	/// the dialog implementation
	FD_form_external * dialog_;
};

#endif // FORMEXTERNAL_H
