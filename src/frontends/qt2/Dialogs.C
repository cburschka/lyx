/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include "Dialogs.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormCopyright.h"
#include "FormDocument.h"
#include "FormError.h"
#include "FormGraphics.h"
#include "FormIndex.h"
#include "FormParagraph.h"
#include "FormPreferences.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h"
#include "FormTabular.h"
#include "FormTabularCreate.h"
#include "FormToc.h"
#include "FormUrl.h"

#ifdef __GNUG__
#pragma implementation
#endif

using std::endl;

// temporary till ported
extern void ShowCredits();


// Signal enabling all visible popups to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
	dialogs_.push_back(new FormCharacter(lv, this));
	dialogs_.push_back(new FormCitation(lv, this));
	dialogs_.push_back(new FormCopyright(lv, this));
	dialogs_.push_back(new FormDocument(lv, this));
	dialogs_.push_back(new FormError(lv, this));
	dialogs_.push_back(new FormGraphics(lv, this));
	dialogs_.push_back(new FormIndex(lv, this));
	dialogs_.push_back(new FormParagraph(lv, this));
	dialogs_.push_back(new FormPreferences(lv, this));
	dialogs_.push_back(new FormPrint(lv, this));
	dialogs_.push_back(new FormRef(lv, this));
	dialogs_.push_back(new FormSearch(lv, this));
	dialogs_.push_back(new FormTabular(lv, this));
	dialogs_.push_back(new FormTabularCreate(lv, this));
	dialogs_.push_back(new FormToc(lv, this));
	dialogs_.push_back(new FormUrl(lv, this));

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
