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
	void showAboutlyx();
	/// show the key and label of a bibliography entry
	void showBibitem(InsetCommand * ic);
	/// show the bibtex dialog
	void showBibtex(InsetCommand * ic);
	///
	void showCharacter();
	/// connected to the character dialog also
	void setUserFreeFont();
	///
	void showCitation(InsetCommand *);
	///
	void createCitation(string const &);
	///
	void showDocument();
	///
	void showError(InsetError *);
	/// show the external inset dialog
	void showExternal(InsetExternal *);
	/// show the contents of a file.
	void showFile(string const &);
	/// show all forked child processes
	void showForks();
	///
	void showGraphics(InsetGraphics *);
	/// show the details of a LyX file include inset
	void showInclude(InsetInclude *);
	///
	void showIndex(InsetCommand *);
	///
	void createIndex();
	///
	void showInfo(InsetInfo *);
	/// show the LaTeX log or build file
	void showLogFile();
	/// display the top-level maths panel
	void showMathPanel();
	///
	void showMinipage(InsetMinipage *);
	///
	void updateMinipage(InsetMinipage *);
	///
	void showERT(InsetERT *);
	///
	void updateERT(InsetERT *);
	///
	void showFloat(InsetFloat *);
	///
	void showParagraph();
	///
	void updateParagraph();
	///
	void showPreamble();
	///
	void showPreferences();
	///
	void showPrint();
	///
	void showRef(InsetCommand *);
	///
	void createRef(string const &);
	///
	void showSearch();
	///
	void showSendto();
	/// bring up the spellchecker
	void showSpellchecker();
	/// bring up the spellchecker tab in preferences
	void showSpellcheckerPreferences();
	///
	void showTabular(InsetTabular *);
	///
	void updateTabular(InsetTabular *);
	///
	void showTabularCreate();
	/// show the TexInfo
	void showTexinfo();
	/// show the thesaurus dialog
	void showThesaurus(string const &);
	///
	void showTOC(InsetCommand *);
	///
	void createTOC(string const &);
	///
	void showUrl(InsetCommand *);
	///
	void createUrl(string const &);
	/// show the version control log
	void showVCLogFile();
	//@}
private:
	/// Add a dialog to the vector of dialogs.
	//void add(DialogBase *);
	/// the dialogs being managed
	//std::vector<db_ptr> dialogs_;
};

#endif
