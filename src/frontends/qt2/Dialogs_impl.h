// -*- C++ -*-
/**
 * \file qt2/Dialogs_impl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef DIALOGS_IMPL_H
#define DIALOGS_IMP_H

#include "Dialogs.h"
#include "controllers/GUI.h"

#include "ControlDocument.h"
#include "ControlForks.h"
#include "ControlPrefs.h"
#include "ControlPrint.h"
#include "ControlSearch.h"
#include "ControlSendto.h"
#include "ControlSpellchecker.h"
#include "ControlTexinfo.h"

#include "QCharacter.h"
#include "QCharacterDialog.h"
#include "QDocument.h"
#include "QDocumentDialog.h"
//#include "QForks.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QPrefs.h"
#include "QPrefsDialog.h"
#include "QPrint.h"
#include "QLPrintDialog.h"
#include "QSearch.h"
#include "QSearchDialog.h"
#include "QSendto.h"
#include "QSendtoDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"
#include "QTexinfo.h"
#include "QTexinfoDialog.h"

#include "Qt2BC.h"



typedef GUI<ControlDocument, QDocument, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
DocumentDialog;

typedef GUI<ControlPrefs, QPrefs, OkApplyCancelPolicy, Qt2BC>
PrefsDialog;

typedef GUI<ControlPrint, QPrint, OkApplyCancelPolicy, Qt2BC>
PrintDialog;

typedef GUI<ControlSearch, QSearch, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SearchDialog;

typedef GUI<ControlSendto, QSendto, OkApplyCancelPolicy, Qt2BC>
SendtoDialog;

typedef GUI<ControlSpellchecker, QSpellchecker, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SpellcheckerDialog;

typedef GUI<ControlTexinfo, QTexinfo, OkCancelPolicy, Qt2BC>
TexinfoDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	DocumentDialog      document;
	PrefsDialog         prefs;
	PrintDialog         print;
	SearchDialog        search;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
	TexinfoDialog       texinfo;
};

#endif // DIALOGS_IMPL_H
