// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_GUI_H
#define LYX_GUI_H

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class LyX;
class Buffer;

/**
  This class is going to be the entry point to {\em all} GUI funcionality.
  From this object will all the things going on be initiated. However I
  have not clearly figured out how this class is going to be, suggestions
  are welcome. (Lgb)
  */
class LyXGUI {
public:
	/**@name Constructor */
	//@{
	
	/** The only constructor allowed

	  If gui is false, LyX will operate in non-X mode
	  */
	LyXGUI(LyX *owner, int *argc, char *argv[], bool gui);
	///
	~LyXGUI();
	//@}
	
	/**@name Members */
	//@{
	/**
	  This functions starts the ball. For XForms it runs a loop of
	  fl_check_forms(). For QT this will probably be .exec().
	  */
	void runTime();
	/** This will take care of the initializaton done after the
	  main initialization.
	  */
	void init();

	/// Register the buffer with the first fount LyXView in lyxViews
	void regBuf(Buffer*);
	//@}
private:
	/**@name Construcor */
	//@{
	/// not allowed
	LyXGUI(); // don't allow this
	/// not allowed
	LyXGUI(const LyXGUI&); // nor this
	//@}
	
	/**@name Members */
	//@{
	///
	void setDefaults();
	///
	void create_forms();
	//@}

	/**@name Variables */
	//@{
	/// The LyX that owns this GUI.
	LyX *_owner;
	///
	LyXView *lyxViews; // or something so that several views
	// on the same time can be allowed.
	/// Do we have a gui?
	bool gui;
	//@}
};

#endif
