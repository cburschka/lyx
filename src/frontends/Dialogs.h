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

#include "LString.h"

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/function.hpp>

#include <vector>

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
	static boost::signal0<void> redrawGUI;

	/// Toggle tooltips on/off in all dialogs.
	static boost::signal0<void> toggleTooltips;

	/// Are the tooltips on or off?
	static bool tooltipsEnabled();

	/**@name Global Hide and Update Signals */
	//@{
	/// Hide all visible dialogs
	boost::signal0<void> hideAll;

	/// Hide any dialogs that require a buffer for them to operate
	boost::signal0<void> hideBufferDependent;

	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else its still the same buffer.
	 */
	boost::signal1<void, bool> updateBufferDependent;
	//@}

	/**@name Dialog Access Signals.
	   Put into some sort of alphabetical order */
	//@{
	///
	boost::function<void> showAboutlyx;
	/// show the key and label of a bibliography entry
	boost::function<void, InsetCommand *> showBibitem;
	/// show the bibtex dialog
	boost::function<void, InsetCommand *> showBibtex;
	///
	boost::function<void> showCharacter;
	/// connected to the character dialog also
	boost::function<void> setUserFreeFont;
	///
	boost::function<void, InsetCommand *> showCitation;
	///
	boost::function<void, string const &> createCitation;
	///
	boost::function<void> showDocument;
	///
	boost::function<void, InsetError *> showError;
	/// show the external inset dialog
	boost::function<void, InsetExternal *> showExternal;
	/// show the contents of a file.
	boost::function<void, string const &> showFile;
	/// show all forked child processes
	boost::function<void> showForks;
	///
	boost::function<void, InsetGraphics *> showGraphics;
	/// show the details of a LyX file include inset
	boost::function<void, InsetInclude *> showInclude;
	///
	boost::function<void, InsetCommand *> showIndex;
	///
	boost::function<void, string const &> createIndex;
	///
	boost::function<void, InsetInfo *> showInfo;
	/// show the LaTeX log or build file
	boost::function<void> showLogFile;
	/// display the top-level maths panel
	boost::function<void> showMathPanel;
	///
	boost::function<void, InsetMinipage *> showMinipage;
	///
	boost::function<void, InsetMinipage *> updateMinipage;
	///
	boost::function<void, InsetERT *> showERT;
	///
	boost::function<void, InsetERT *> updateERT;
	///
	boost::function<void, InsetFloat *> showFloat;
	///
	boost::function<void> showParagraph;
	///
	boost::signal0<void> updateParagraph;
	///
	boost::function<void> showPreamble;
	///
	boost::function<void> showPreferences;
	///
	boost::function<void> showPrint;
	///
	boost::function<void, InsetCommand *> showRef;
	///
	boost::function<void, string const &> createRef;
	///
	boost::function<void> showSearch;
	///
	boost::function<void> showSendto;
	/// bring up the spellchecker
	boost::function<void> showSpellchecker;
	/// bring up the spellchecker tab in preferences
	boost::function<void> showSpellcheckerPreferences;
	///
	boost::function<void, InsetTabular *> showTabular;
	///
	boost::function<void, InsetTabular *> updateTabular;
	///
	boost::function<void> showTabularCreate;
	/// show the TexInfo
	boost::function<void> showTexinfo;
	/// show the thesaurus dialog
	boost::function<void, string const &> showThesaurus;
	///
	boost::function<void, InsetCommand *> showTOC;
	///
	boost::function<void, string const &> createTOC;
	///
	boost::function<void, InsetCommand *> showUrl;
	///
	boost::function<void, string const &> createUrl;
	/// show the version control log
	boost::function<void> showVCLogFile;
	//@}
private:
	/// Add a dialog to the vector of dialogs.
	void add(DialogBase *);
	/// the dialogs being managed
	std::vector<db_ptr> dialogs_;
};

#endif
