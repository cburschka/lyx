// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995 1996 Matthias Ettrich
*           and the LyX Team.
*
*======================================================*/

#ifndef _LASTFILES_H
#define _LASTFILES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"


/** The latest documents loaded
  This class takes care of the last .lyx files used by the LyX user. It
  both reads and writes this information to a file. The number of files
  kept are user defined, but defaults to four.
*/
class LastFiles 
{
public:
	/**@name Constructors and Deconstructors */
	//@{
	/**
	  Parameters are: name of file to read. Whether you want LastFiles 
	  to check for file existance, and the number of files to remember.
	  */
	LastFiles(LString const &, bool dostat = true, char num = 4);
	///
	~LastFiles();
	//@}

	/**@name Methods */
	//@{
	/**
	  This funtion inserts #file# into the last files list. If the file
	  already exist it is moved to the top of the list. If it don't
	  exist it is placed on the top of the list. If the list already is
	  full the last visited file in the list is puched out and deleted.
	 */
	void newFile(LString const &);
	/**  Writes the lastfiles table to disk. A " is placed around the
	  filenames to preserve special chars. (not all are preserved
	  anyway, but at least space is.)
	  */
	void writeFile(LString const &) const;
	//@}
private:
	/**@name const variables */
	//@{
	/// 
	enum {
		///
		DEFAULTFILES = 4
	};
	/** There is no point in keeping more than this number of files
	  at the same time. However perhaps someday someone finds use for
	  more files and wants to change it. Please do. But don't show
	  the files in a menu...
	  */
	enum {
		///
		ABSOLUTEMAXLASTFILES = 20
	};
	//@}

	/**@name Variables */
	//@{
	/// an array of lastfiles
	LString *files;
	/// number of files in the lastfiles list.
	char num_files;
	/// check for file existance or not.
	bool dostat;
	//@}
	
	/**@name Methods */
	//@{
	/** reads the .lyx_lastfiles at the beginning of the LyX session.
	  This will read the lastfiles file (usually .lyx_lastfiles). It
	  will normally discard files that don't exist anymore, unless
	  LastFiles has been initialized with dostat = false. 
	 */
	void readFile(LString const &);
	/// used by the constructor to set the number of stored last files.
        void setNumberOfFiles(char num);
	//@}

	/**@name Friends */
	//@{
	///
	friend class LastFiles_Iter;
	//@}
};


/// An Iterator class for LastFiles
class LastFiles_Iter {
public:
	///
	LastFiles_Iter(const LastFiles& la)
	{cs = &la; index = 0;}
	///
	LString operator() ()
	{
		return (index < cs->num_files)? cs->files[index++] 
					        : LString();
	}
	///
	LString operator[] (int a)
	{ return cs->files[a];}
private:
	///
	const LastFiles *cs;
	///
	char index;
};

#endif
