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

#include "LString.h"
#include "support/utility.hpp"

class DialogBase;

// Maybe this should be a UIFunc modelled on LyXFunc
class LyXView;

class InsetGraphics;
class InsetBibKey;
class InsetBibtex;
class InsetInclude;
class InsetInfo;
class InsetTabular;
class InsetCommand;

using std::vector;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
using SigC::Signal1;
#endif

/** Container of all dialogs and signals a LyXView needs or uses to access them
    The list of dialog signals isn't comprehensive but should be a good guide
    for any future additions.  Remember don't go overboard -- think minimal.
 */
class Dialogs : public noncopyable
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
	Signal1<void, InsetTabular *> showTabular;
	///
	Signal1<void, InsetTabular *> updateTabular;
	///
	Signal1<void, InsetTabular *> hideTabular;
	///
	Signal0<void> showTabularCreate;
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
	Signal1<void, InsetGraphics *> showGraphics;
	///
	Signal1<void, InsetInclude *> showInclude;
	///
	Signal1<void, InsetCommand *> showIndex;
	///
	Signal1<void, string const &> createIndex;
	///
	Signal1<void, InsetBibKey *> showBibkey;
	///
	Signal1<void, InsetCommand *> showCitation;
	///
	Signal1<void, string const &> createCitation;
	///
	Signal1<void, InsetCommand *> showTOC;
	///
	Signal1<void, string const &> createTOC;
	///
	Signal1<void, InsetCommand *> showUrl;
	///
	Signal1<void, string const &> createUrl;
	///
	Signal1<void, InsetBibtex *> showBibtex;
	///
	Signal1<void, InsetInfo *> showInfo;
	//@}
private:
	///
	vector<DialogBase *> dialogs_;
};

#endif








