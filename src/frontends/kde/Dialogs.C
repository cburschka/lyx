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
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCitation.h"
#include "FormCopyright.h"
#include "FormDocument.h"
#include "FormError.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormParagraph.h"
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

// temporary till ported
extern void ShowCredits();

/* We don't implement this, but it's needed whilst we
 * still rely on xforms
 */
Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	dialogs_.push_back(new FormBibitem(lv, this));
	dialogs_.push_back(new FormBibtex(lv, this));
	dialogs_.push_back(new FormCitation(lv, this));
	dialogs_.push_back(new FormCopyright(lv, this));
	dialogs_.push_back(new FormDocument(lv, this));
	dialogs_.push_back(new FormError(lv, this));
	dialogs_.push_back(new FormGraphics(lv, this));
	dialogs_.push_back(new FormInclude(lv, this));
	dialogs_.push_back(new FormIndex(lv, this));
	dialogs_.push_back(new FormLog(lv, this));
	dialogs_.push_back(new FormParagraph(lv, this));
	dialogs_.push_back(new FormPreferences(lv, this));
	dialogs_.push_back(new FormPrint(lv, this));
	dialogs_.push_back(new FormRef(lv, this));
	dialogs_.push_back(new FormSplash(lv, this));
	dialogs_.push_back(new FormTabular(lv, this));
	dialogs_.push_back(new FormTabularCreate(lv, this));
	dialogs_.push_back(new FormToc(lv, this));
	dialogs_.push_back(new FormUrl(lv, this));
	dialogs_.push_back(new FormVCLog(lv, this));

	showCredits.connect(slot(ShowCredits));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}


Dialogs::~Dialogs()
{
	for (vector<DialogBase *>::iterator iter = dialogs_.begin();
	     iter != dialogs_.end();
	     ++iter) {
		delete *iter;
	}
}
