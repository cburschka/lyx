// -*- C++ -*-
/**
 * \file Dialogs.h
 * Copyright 1995-2002 The LyX Team.
 * See the file COPYING.
 * \author Allan Rae, rae@lyx.org
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>

// Maybe this should be a UIFunc modelled on LyXFunc
class LyXView;

class InsetBibKey;
class InsetBibtex;
class InsetCommand;
class InsetError;
class InsetERT;
class InsetExternal;
class InsetFloat;
class InsetGraphics;
class InsetInclude;
class InsetInfo;
class InsetMinipage;
class Paragraph;
class InsetTabular;

/** Container of all dialogs and signals a LyXView needs or uses to access them
    The list of dialog signals isn't comprehensive but should be a good guide
    for any future additions.  Remember don't go overboard -- think minimal.
 */
class Dialogs : boost::noncopyable
{
public:
	///
	typedef boost::shared_ptr<boost::noncopyable> db_ptr;
	///
	Dialogs(LyXView *);

	/** Redraw all visible dialogs because, for example, the GUI colours
	    have been re-mapped. */
	boost::signal0<void> redrawGUI;

	/// Toggle tooltips on/off in all dialogs.
	boost::signal0<void> toggleTooltips;

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
	///
	void showERT(InsetERT *);
	///
	void updateERT(InsetERT *);
	/// show the external inset dialog
	void showExternal(InsetExternal *);
	/// show the contents of a file.
	void showFile(string const &);
	///
	void showFloat(InsetFloat *);
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
	/// show the LaTeX log or build file
	void showLogFile();
	/// display the top-level maths panel
	void showMathPanel();
	///
	void showMinipage(InsetMinipage *);
	///
	void updateMinipage(InsetMinipage *);
	///
	void showParagraph();
	///
	boost::signal0<void>  updateParagraph;
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
};

#endif
