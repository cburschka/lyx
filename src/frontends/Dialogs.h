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
 *           Copyright 1995-2001 The LyX Team.
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
#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"

// Maybe this should be a UIFunc modelled on LyXFunc
class LyXView;

class InsetGraphics;
class InsetBibKey;
class InsetBibtex;
class InsetError;
class InsetExternal;
class InsetInclude;
class InsetInfo;
class InsetTabular;
class InsetCommand;
class InsetMinipage;
class InsetFloat;
class InsetERT;
class Paragraph;

/** Container of all dialogs and signals a LyXView needs or uses to access them
    The list of dialog signals isn't comprehensive but should be a good guide
    for any future additions.  Remember don't go overboard -- think minimal.
 */
class Dialogs : boost::noncopyable
{
public:
	///
	typedef boost::shared_ptr<DialogBase> db_ptr;
	/**@name Constructor */
	//@{
	///
	Dialogs(LyXView *);
	//@}

	/** Redraw all visible dialogs because, for example, the GUI colours
	    have been re-mapped. */
	static SigC::Signal0<void> redrawGUI;

	/**@name Global Hide and Update Signals */
	//@{
	/// Hide all visible dialogs
	SigC::Signal0<void> hideAll;
	
	/// Hide any dialogs that require a buffer for them to operate
	SigC::Signal0<void> hideBufferDependent;
	
	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else its still the same buffer.
	 */
	SigC::Signal1<void, bool> updateBufferDependent;
	//@}

	/**@name Dialog Access Signals.
	   Put into some sort of alphabetical order */
	//@{
	/// Do we really have to push this?
	SigC::Signal1<void, std::vector<string> const &> SetDocumentClassChoice;
	///
	SigC::Signal0<void> showAboutlyx;
	/// show the key and label of a bibliography entry
	SigC::Signal1<void, InsetCommand *> showBibitem;
	/// show the bibtex dialog
	SigC::Signal1<void, InsetCommand *> showBibtex;
	///
	SigC::Signal0<void> showCharacter;
	/// connected to the character dialog also
	SigC::Signal0<void> setUserFreeFont;
	///
	SigC::Signal1<void, InsetCommand *> showCitation;
	///
	SigC::Signal1<void, string const &> createCitation;
	///
	SigC::Signal0<void> showDocument;
	///
	SigC::Signal1<void, InsetError *> showError;
	/// show the external inset dialog
	SigC::Signal1<void, InsetExternal *> showExternal; 
	///
	SigC::Signal1<void, InsetGraphics *> showGraphics;
	/// show the details of a LyX file include inset
	SigC::Signal1<void, InsetInclude *> showInclude;
	///
	SigC::Signal1<void, InsetCommand *> showIndex;
	///
	SigC::Signal1<void, string const &> createIndex;
	///
	SigC::Signal1<void, InsetInfo *> showInfo;
	/// show the LaTeX log or build file
	SigC::Signal0<void> showLogFile;
 	/// display the top-level maths panel
 	SigC::Signal0<void> showMathPanel;
	///
	SigC::Signal1<void, InsetMinipage *> showMinipage;
	///
	SigC::Signal1<void, InsetMinipage *> updateMinipage;
	///
	SigC::Signal1<void, InsetERT *> showERT;
	///
	SigC::Signal1<void, InsetERT *> updateERT;
	///
	SigC::Signal1<void, InsetFloat *> showFloat;
	///
	SigC::Signal0<void> showParagraph;
	///
	SigC::Signal0<void> updateParagraph;
	///
	SigC::Signal0<void> showPreamble;
	///
	SigC::Signal0<void> showPreferences;
	///
	SigC::Signal0<void> showPrint;
	///
	SigC::Signal1<void, InsetCommand *> showRef;
	///
	SigC::Signal1<void, string const &> createRef;
	///
	SigC::Signal0<void> showSearch;
	/// bring up the spellchecker
	SigC::Signal0<void> showSpellchecker;
	/// bring up the spellchecker tab in preferences
	SigC::Signal0<void> showSpellcheckerPreferences;
	///
	SigC::Signal1<void, InsetTabular *> showTabular;
	///
	SigC::Signal1<void, InsetTabular *> updateTabular;
	///
	SigC::Signal0<void> showTabularCreate;
	/// show the thesaurus dialog
	SigC::Signal1<void, string const &> showThesaurus; 
	///
	SigC::Signal1<void, InsetCommand *> showTOC;
	///
	SigC::Signal1<void, string const &> createTOC;
	///
	SigC::Signal1<void, InsetCommand *> showUrl;
	///
	SigC::Signal1<void, string const &> createUrl;
	/// show the version control log
	SigC::Signal0<void> showVCLogFile;
	//@}
private:
	/// Add a dialog to the vector of dialogs.
	void add(DialogBase *);
	/// the dialogs being managed
	std::vector<db_ptr> dialogs_;
};

#endif
