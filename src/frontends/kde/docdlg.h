/*
 * docdlg.h
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCDLG_H
#define DOCDLG_H

#include <config.h>
#include <gettext.h>
#include <string>

#include "dlg/docsettingsdlgdata.h"
#include "dlg/docgeometrydlgdata.h"
#include "dlg/doclanguagedlgdata.h"
#include "dlg/docextradlgdata.h"
#include "dlg/docbulletsdlgdata.h"
#include "dlg/docdlgdata.h"
 
#include "debug.h" 

class BufferParams;

// to connect apply() and close()
#include "FormDocument.h"

class DocDialog : public DocDialogData  {
   Q_OBJECT
public:
	DocDialog(FormDocument *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~DocDialog();

	/** 
	 * \fn setReadOnly
	 * \brief set widgets read only status appropriately
	 */
	void setReadOnly(bool readonly);

	/**
	 * \fn setFromParams
	 * \brief set dialog widgets from buffer parameters
	 */
	void setFromParams(BufferParams const & params);

	/**
	 * \fn updateParams
	 * \brief update the given params object based on the dialog widgets
	 *
	 * This will update the params object. It returns true if a redo
	 * of the related buffer is necessary
	 */
	bool updateParams(BufferParams & params) const;
	
	/**
	 * \fn updateParams
	 * \brief update the given params object based on the dialog widgets
	 *
	 * This will update the params object. It returns true if a redo
	 * of the related buffer is necessary
	 */
	bool updateParams(BufferParams & params);

protected:
	void closeEvent(QCloseEvent *e);
 
private:
	/// the form
	FormDocument *form_;

	/// the settings page
	DocSettingsDialogData *settings; 
	/// the geometry page
	DocGeometryDialogData *geometry;
	/// the language page
	DocLanguageDialogData *language;
	/// the extra page
	DocExtraDialogData *extra;
	/// the bullets page
	DocBulletsDialogData *bullets;

private slots:

	void skipspacingChanged(const char *sel);

	void linespacingChanged(const char *sel);

	void ok_adaptor(void) {
		apply_adaptor();
		form_->close();
		hide();
	}

	void apply_adaptor(void) {
		form_->apply();
	}

	void restore_adaptor(void) {
		// this will reset to known values
		form_->update();
	}
	
	void cancel_adaptor(void) {
		form_->close();
		hide();
	}

};

#endif
