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
#include <boost/utility.hpp>

class DialogBase;

// Maybe this should be a UIFunc modelled on LyXFunc
class LyXView;

class FormSplash;

class InsetGraphics;
class InsetBibKey;
class InsetBibtex;
class InsetError;
class InsetExternal;
class InsetInclude;
class InsetInfo;
class InsetTabular;
class InsetCommand;

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
	
	/** Redraw all visible popups because, for example, the GUI colours
	    have been re-mapped. */
	static Signal0<void> redrawGUI;

	/**@name Global Hide and Update Signals */
	//@{
	/// Hide all visible popups
	Signal0<void> hideAll;
	
	/// Hide any dialogs that require a buffer for them to operate
	Signal0<void> hideBufferDependent;
	
	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else its still the same buffer.
	 */
	Signal1<void, bool> updateBufferDependent;
	//@}

	/**@name Dialog Access Signals.
	   Put into some sort of alphabetical order */
	//@{
	/// Do we really have to push this?
	Signal1<void, std::vector<string> const &> SetDocumentClassChoice;
	/// show the key and label of a bibliography entry
	Signal1<void, InsetCommand *> showBibitem;
	/// show the bibtex dialog
	Signal1<void, InsetCommand *> showBibtex;
	///
	Signal0<void> showCharacter;
	///
	Signal1<void, InsetCommand *> showCitation;
	///
	Signal1<void, string const &> createCitation;
	///
	Signal0<void> showCopyright;
	///
	Signal0<void> showCredits;
	///
	Signal1<void, InsetError *> showError;
	/// show the external inset dialog
	Signal1<void, InsetExternal *> showExternal; 
	///
	Signal1<void, InsetGraphics *> showGraphics;
	/// show the details of a LyX file include inset
	Signal1<void, InsetInclude *> showInclude;
	///
	Signal1<void, InsetCommand *> showIndex;
	///
	Signal1<void, string const &> createIndex;
	///
	Signal1<void, InsetInfo *> showInfo;
	///
	Signal0<void> showLayoutDocument;
	///
	Signal0<void> showLayoutParagraph;
	///
	Signal0<void> showLayoutCharacter;
	///
	Signal0<void> setUserFreeFont;
	/// show the version control log
	Signal0<void> showVCLogFile;
	/// show the LaTeX log or build file
	Signal0<void> showLogFile;
	///
	Signal0<void> showPreamble;
	///
	Signal0<void> showPreferences;
	///
	Signal0<void> showPrint;
	///
	Signal1<void, InsetCommand *> showRef;
	///
	Signal1<void, string const &> createRef;
	///
	Signal0<void> showSearch;
	/// pop up the splash
	Signal0<void> showSplash;
	/// destroy the splash dialog
	void destroySplash();
	///
	Signal1<void, InsetTabular *> showTabular;
	///
	Signal1<void, InsetTabular *> updateTabular;
	///
	Signal0<void> showTabularCreate;
	///
	Signal1<void, InsetCommand *> showTOC;
	///
	Signal1<void, string const &> createTOC;
	///
	Signal1<void, InsetCommand *> showUrl;
	///
	Signal1<void, string const &> createUrl;
	///
	Signal0<void> updateCharacter;  // allow update as cursor moves
	//@}
private:
	/// the dialogs being managed
	std::vector<DialogBase *> dialogs_;
	/// the splash dialog
	FormSplash * splash_;
};

#endif
