/**
 * \file Dialogs.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

// the dialog definitions
#include "QCitationDialog.h"
#include "QAboutDialog.h"
 
// the controller interface
#include "QAbout.h"
#include "QCharacter.h"
#include "QCitation.h"
#include "QParagraph.h"
#include "QPrint.h"
#include "QSearch.h"
#include "QSplash.h"
#include "QTabularCreate.h"

#include "QtLyXView.h" 
#include "Dialogs.h"
#include "BufferView.h"
#include "buffer.h"
#include "qt2BC.h"

// xforms implementations
#include "../xforms/FormError.h"
#include "../xforms/FormGraphics.h"
#include "../xforms/FormPreferences.h"
#include "../xforms/FormTabular.h"

// the controllers
#include "controllers/ControlCitation.h"
#include "controllers/ControlAboutlyx.h"
#include "controllers/ControlSplash.h"
#if 0
#include "controllers/ControlCopyright.h"
#include "controllers/ControlCredits.h"
#include "controllers/ControlBibitem.h"
#include "controllers/ControlBibtex.h"
#include "controllers/ControlButtons.h"
#include "controllers/ControlCharacter.h"
#include "controllers/ControlCitation.h"
#include "controllers/ControlCommand.h"
#include "controllers/ControlERT.h"
#include "controllers/ControlError.h"
#include "controllers/ControlExternal.h"
#include "controllers/ControlFloat.h"
#include "controllers/ControlGraphics.h"
#include "controllers/ControlInclude.h"
#include "controllers/ControlIndex.h"
#include "controllers/ControlLabel.h"
#include "controllers/ControlLog.h"
#include "controllers/ControlMinipage.h"
#include "controllers/ControlPreamble.h"
#include "controllers/ControlPrint.h"
#include "controllers/ControlRef.h"
#include "controllers/ControlSearch.h"
#include "controllers/ControlSpellchecker.h"
#include "controllers/ControlTabularCreate.h"
#include "controllers/ControlThesaurus.h"
#include "controllers/ControlToc.h"
#include "controllers/ControlUrl.h"
#include "controllers/ControlVCLog.h"
#endif

#include "GUI.h"
 
// this makes no real sense for Qt2
SigC::Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new QSplash(lv, this));

	// dialogs that have been converted to new scheme
	add(new GUICitation<QCitation, qt2BC>(*lv, *this));
	add(new GUIAboutlyx<QAbout, qt2BC>(*lv, *this));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}
