/**
 * \file Dialogs.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author John Levon
 */

#include <config.h>
#include FORMS_H_LOCATION

#include "Dialogs.h"

#include "ControlBibitem.h"
#include "ControlCitation.h"
#include "kdeBC.h"

#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCitation.h"
#include "citationdlg.h"
#include "FormCopyright.h"
#include "FormCredits.h"
#include "FormDocument.h"
#include "FormError.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormParagraph.h"
#include "FormPreamble.h"
#include "FormPreferences.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSplash.h"
#include "FormTabular.h"
#include "FormTabularCreate.h"
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"

#ifdef __GNUG__
#pragma implementation
#endif

/* We don't implement this, but it's needed whilst we
 * still rely on xforms
 */
Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new FormSplash(lv, this));

	add(new GUIBibitem<FormBibitem, xformsBC>(*lv, *this));
	add(new GUICitation<FormCitation, xformsBC>(*lv, *this));

	add(new FormBibtex(lv, this));
	add(new FormCharacter(lv, this));
	add(new FormCopyright(lv, this));
	add(new FormCredits(lv, this));
	add(new FormDocument(lv, this));
	add(new FormError(lv, this));
	add(new FormGraphics(lv, this));
	add(new FormInclude(lv, this));
	add(new FormIndex(lv, this));
	add(new FormLog(lv, this));
	add(new FormParagraph(lv, this));
	add(new FormPreamble(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormPrint(lv, this));
	add(new FormRef(lv, this));
	add(new FormSearch(lv, this));
	add(new FormTabular(lv, this));
	add(new FormTabularCreate(lv, this));
	add(new FormToc(lv, this));
	add(new FormUrl(lv, this));
	add(new FormVCLog(lv, this));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}
