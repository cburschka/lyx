// -*- C++ -*-

#ifndef _LYX_VC_H_
#define _LYX_VC_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "latexoptions.h"

class Buffer;

/** Version Control for LyX. This is the class giving the verison control
features to LyX. It is
  intended to support different kinds of version control, but at this point
  we will only support RCS. Later CVS is a likely candidate for support.
  The support in LyX is based loosely upon the version control in GNU Emacs,
  but is not as extensive as that one. See examples/VC.lyx for a simple
  tutorial and manual for the use of the version control system in LyX.

  LyXVC use this algorithm when it searches for VC files:
  for RCS it searches for <filename>,v and RCS/<filename>,v similar
  should be done for CVS. By doing this there doesn't need to be any
  special support for VC in the lyx format, and this is especially good
  when the lyx format will be a subset of LaTeX.
  */
class LyXVC {
public:
	///
	LyXVC();
	///
	~LyXVC();
	/** Not a good name perhaps. This function should be called whenever
	  LyX loads a file. This function then checks for a master VC file
	  (for RCS this is *,v or RCS/ *,v) if this file is found, the loaded
	  file is assumed to be under controll by VC (only RCS so far), and
	  the appropiate actions is taken. Returns true if the file is under
	  control by a VCS.
	  */
	bool file_found_hook(string const & fn);
	
	/** This function should be run when a file is requested for loading,
	  but it does not exist. This function will then check for a VC master
	  file with the same name (see above function). If this exists the
	  user should be asked if he/her wants to checkout a version for
	  viewing/editing. Returns true if the file is under control by a VCS
	  and the user wants to view/edit it.
	  */
	static bool file_not_found_hook(string const & fn);

	///
	void scanMaster();
	///
	void setBuffer(Buffer*);

	/// Register the document as an VC file.
	void registrer();

	/// Unlock and commit changes.
	void checkIn();

	/// Lock and prepare to edit document.
	void checkOut();

	/// Revert to last version
	void revert();

	/// Undo last check-in.
	void undoLast();

	///
	void viewLog(string const &);

	///
	void showLog();

	/// 
	void toggleReadOnly();
	
	/// Is the document under administration by RCS?
	bool inUse();

	/// Returns the version number.
	string const getVersion() const;

	/// Returns the userid of the person who has locked the doc.
	string const getLocker() const;

	///
	static void logClose(FL_OBJECT*, long);
	///
	static void logUpdate(FL_OBJECT*, long);
protected:
private:
	///
	int doVCCommand(string const&);
	
	/** The master VC file. For RCS this is *,v or RCS/ *,v. master should
	  have full path.
	  */
	string master;

	/** The version of the VC file. I am not sure if this can be a
	string of if it must be a
	  float/int. */
	string version;

	/// The user currently keeping the lock on the VC file.
	string locker;

	///
	enum VCStatus {
		///
		UNLOCKED,
		///
		LOCKED
	};

	/// The status of the VC controlled file.
	VCStatus vcstat;

	///	
	enum Backend {
		///
		UNKNOWN_VCS,
		///
		RCS_VCS
	};

	/// The VC backend used. (so far this can only be RCS)
	Backend backend;

	/// The buffer using this VC
	Buffer *_owner;

	///
	FD_LaTeXLog *browser; // FD_LaTeXLog is just a browser with a
	// close button. Unfortunately we can not use the standard callbacks.
};

#endif
