// -*- C++ -*-
/* Dialogs.h
 * Container of all dialogs and signals a LyXView needs or uses to access them.
 * Author: Allan Rae <rae@lyx.org>
 * This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <vector>
#include <sigc++/signal_system.h>

using std::vector;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
using SigC::Signal1;
#endif

#include "LString.h"

class DialogBase;

// Maybe this should be a UIFunc modelled on LyXFunc
class LyXView;

class InsetInclude;
class InsetIndex;
class InsetBibKey;
class InsetCitation;
class InsetBibtex;
class InsetInfo;

/** Container of all dialogs and signals a LyXView needs or uses to access them
    The list of dialog signals isn't comprehensive but should be a good guide
    for any future additions.  Remember don't go overboard -- think minimal.
 */
class Dialogs
{
public:
	/**@name Constructors and Deconstructors */
	//@{
	///
	Dialogs(LyXView *);
	///
	~Dialogs();
	//@}
	
	/**@name Global Hide and Update Signals */
	//@{
	/// Hide all visible popups
	Signal0<void> hideAll;
	
	/// Hide any popups that require a buffer for them to operate
	Signal0<void> hideBufferDependent;
	
	/// Update visible, buffer-dependent popups
	Signal0<void> updateBufferDependent;
	//@}

	/**@name Dialog Access Signals */
	//@{
	/// Opens the Preamble Dialog.
	Signal0<void> showPreamble;
	///
	Signal0<void> showLogFile;
	///
	Signal0<void> showTable;
	///
	Signal0<void> updateTable; // needed for InsetTabular-Update
	///
	Signal0<void> showTableNew;
	///
	Signal0<void> showCharacter;
	///
	Signal0<void> updateCharacter;  // allow update as cursor moves
	///
	Signal0<void> showParagraph;
	///
	Signal0<void> showDocument;
	/// Do we really have to push this?
	Signal1<void, vector<string> const &> SetDocumentClassChoice;
	///
	Signal0<void> showPrint;
	///
	Signal0<void> showCopyright;
	///
	Signal0<void> showCredits;
	///
	Signal0<void> showPreferences;
	///
	Signal1<void, InsetInclude *> showInclude;
	///
	Signal1<void, InsetIndex *> showIndex;
	///
	Signal1<void, InsetBibKey *> showBibkey;
	///
	Signal1<void, InsetCitation *> showCitation;
	///
	Signal1<void, string const &> createCitation;
	///
	Signal1<void, InsetBibtex *> showBibtex;
	///
	Signal1<void, InsetInfo *> showInfo;
	//@}
private:
	/// Disallow default constructor
	Dialogs() {}
	/// Disallow copy constructor
	Dialogs(Dialogs &) {}
	///
	vector<DialogBase *> dialogs_;
};

#endif








