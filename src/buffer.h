// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor 	 
*	    Copyright (C) 1995 Matthias Ettrich
*
*           This file is Copyleft (C) 1996
*           Lars Gullik Bjønnes
*
* ====================================================== */
 
// Change Log:
// =========== 
// 23/03/98   Heinrich Bauer (heinrich.bauer@t-mobil.de)
// Spots marked "changed Heinrich Bauer, 23/03/98" modified due to the
// following bug: dvi file export did not work after printing (or previewing)
// and vice versa as long as the same file was concerned. This happened
// every time the LyX-file was left unchanged between the two actions mentioned
// above.

#ifndef BUFFER_H
#define BUFFER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "undo.h"
#include "BufferView.h"
#include "lyxvc.h"
#include "bufferparams.h"
#include "texrow.h"
#include "lyxtext.h"

class LyXRC;
class TeXErrors;
class LaTeXFeatures;

extern void updateAllVisibleBufferRelatedPopups();
extern void WarnReadonly();

///
struct DEPCLEAN {
	///
	bool clean;
	///
	string master;
	///
	DEPCLEAN *next;
};

/** The buffer object.
  The is is the buffer object. It contains all the informations about
  a document loaded into LyX. I am not sure if the class is complete or
  minimal, probably not.
  */
class Buffer {
public:
	/**@name Constructors and destructor */
	//@{
	///
	Buffer(string const & file, LyXRC * lyxrc = 0, bool b = false);
	
	///
	~Buffer();
	//@}

	/**@name Methods */
	//@{

	/** save the buffer's parameters as user default
 	  This function saves a file user_lyxdir/templates/defaults.lyx 
	  which parameters are those of the current buffer. This file
	  is used as a default template when creating a new
	  file. Returns true on success.
	  */
	bool saveParamsAsDefaults();

	/// should be changed to work for a list.
	void resize()
	{
		if (users) {
			users->resize();
		} else if (text) {
			delete text;
			text = 0;
		}
	}

	/// Update window titles of all users
	void updateTitles();

	/// Reset autosave timers for all users
	void resetAutosaveTimers();

	/** Adds the BufferView to the users list.
	  Later this func will insert the BufferView into a real list,
	  not just setting a pointer.
	  */
	void addUser(BufferView * u) { users = u; }

	/** Removes the BufferView from the users list.
 	  Since we only can have one at the moment, we just reset it.
	  */
	void delUser(BufferView *){ users = 0; }

	///
	void update(signed char f);

	///
	void redraw() {
              users->redraw(); 
              users->fitCursor(); 
              users->updateScrollbar();
	}

        /// Open and lock an updatable inset
        void open_new_inset(UpdatableInset *);

	///
	void loadAutoSaveFile();
	
	/** Reads a file. 
	    Returns false if it fails.
	    If par is given, the file is inserted. */
	bool readFile(LyXLex &, LyXParagraph * par = 0);
	
	/** Reads a file without header.
	    Returns false, if file is not completely read.
	    If par is given, the file is inserted. */
	bool readLyXformat2(LyXLex &, LyXParagraph * par = 0);

	/// Inserts a lyx file at cursor position. Returns false if it fails.
	bool insertLyXFile(string const & filename);

	/// Write file. Returns false if unsuccesful.
	bool writeFile(string const &, bool);
	
	///
	void writeFileAscii(string const & , int);
	
	///
	void makeLaTeXFile(string const & filename,
			   string const & original_path,
			   bool nice, bool only_body = false);

	///
	int runLaTeX();

        ///
        int runLiterate();

        ///
        int buildProgram();

	///
	int runChktex();

	///
	void makeLinuxDocFile(string const & filename, int column);
	///
	void makeDocBookFile(string const & filename, int column);

	/// returns the main language for the buffer (document)
	string GetLanguage() const {
		return params.language;
	}
	
	///
	bool isLyxClean() const { return lyx_clean; }
	
	/// changed Heinrich Bauer, 23/03/98
	bool isDviClean();
	
        ///
        bool isNwClean() const { return nw_clean; }
       
	///
	bool isBakClean() const { return bak_clean; }
	
	///
	bool isDepClean(string const & name) const;
	
	///
	void markLyxClean() { 
		if (!lyx_clean) {
			lyx_clean = true; 
			updateTitles();
		}
		// if the .lyx file has been saved, we don't need an
		// autosave 
		bak_clean = true;
	}

	/// changed Heinrich Bauer, 23/03/98
	void markDviClean();
	
        ///
        void markNwClean() { nw_clean = true; }
       
	///
	void markBakClean() { bak_clean = true; }
	
	///
	void markDepClean(string const & name);
	
	///
	void markDviDirty();
	
        ///
        void markNwDirty() { nw_clean = false; }
       
	///
	void markDirty() {
		if (lyx_clean) {
			lyx_clean = false;
			updateTitles();
		}
		dvi_clean_tmpd = false;
		dvi_clean_orgd = false;
                nw_clean = false;
		bak_clean = false;
		DEPCLEAN* tmp = dep_clean;
		while (tmp) {
			tmp->clean = false;
			tmp = tmp->next;
		}
	}

	///
	string getFileName() const { return filename; }

	/// Change name of buffer. Updates "read-only" flag.
	void setFileName(string const & newfile);

	/// Name of the document's parent
	void setParentName(string const &);

	/// Is buffer read-only?
	bool isReadonly() { return read_only; }

	/// Set buffer read-only flag
	void setReadonly(bool flag = true) 
	{
		if (read_only != flag) {
			read_only = flag; 
			updateTitles();
			updateAllVisibleBufferRelatedPopups();
		}
		if (read_only) {
			WarnReadonly();
		}
	}

	/// returns true if the buffer contains a LaTeX document
	bool isLatex() const;
	/// returns true if the buffer contains a LinuxDoc document
	bool isLinuxDoc() const;
	/// returns true if the buffer contains a DocBook document
	bool isDocBook() const;
	/// returns true if the buffer contains either a LinuxDoc or DocBook document
	bool isSGML() const;
        /// returns true if the buffer contains a Wed document
        bool isLiterate() const;

	///
	void setPaperStuff();

#if 0
	///
	void setOldPaperStuff();
#endif
	
	/** Validate a buffer for LaTeX.
	  This validates the buffer, and returns a struct for use by
	  makeLaTeX and others. Its main use is to figure out what commands
	  and packages need to be included in the LaTeX file. It (should)
	  also check that the needed constructs are there (i.e. that the \refs
	  points to coresponding \labels). It should perhaps inset "error"
	  insets to help the user correct obvious mistakes.
	 */
	void validate(LaTeXFeatures &); //the correct parameters to be
				        //included later 

	/** Insert an inset into the buffer
	  Insert inset into buffer, placing it in a layout of lout,
	  if no_table make sure that it doesn't end up in a table. */
	void insertInset(Inset *, string const & lout = string(), 
			 bool no_table = false);

        ///
        void setCursorFromRow (int);

	///
	string getIncludeonlyList(char delim = ',');
	///
	string getReferenceList(char delim = '|');
	///
	string getBibkeyList(char delim = '|');
	///
	bool gotoLabel(string const &);

	/// removes all autodeletable insets
	bool removeAutoInsets();

	/** This will clearly have to change later. Later we can have more
	  than one user per buffer. */
	BufferView * getUser() const { return users; }

        //@}

	/// Does this mean that this is buffer local?
        UndoStack undostack;
	
	/// Does this mean that this is buffer local? 
        UndoStack redostack;
	
	///
	BufferParams params;
	
	/** is a list of paragraphs.
	 */
	LyXParagraph * paragraph;
	
	/** This holds the mapping between buffer paragraphs and screen rows.
	    Should be moved to BufferView. (Asger)
	 */
	LyXText * text;

	///
	UpdatableInset * the_locking_inset;

	/// RCS object
	LyXVC lyxvc;

	/// where the temporaries go if we want them
	string tmppath;

	///
	string filepath;

	/** While writing as LaTeX, tells whether we are
	    doing a 'nice' LaTeX file */
	bool niceFile;
protected:
	///
        void InsetUnlock();
	
	///
        inline void InsetSleep();
	
	///
        inline void InsetWakeup();
	
	///
        bool inset_slept;
	
	///
        int  slx;
	///
	int sly;
private:
	///
	void insertErrors(TeXErrors &);
	
        ///
        void linuxDocHandleFootnote(FILE * file,
				    LyXParagraph * & par, int const depth);
        ///
	void DocBookHandleCaption(FILE * file, string & inner_tag,
				  int const depth, int desc_on,
				  LyXParagraph * &par);
        ///
	void DocBookHandleFootnote(FILE * file,
				   LyXParagraph * & par, int const depth);
	///
        void sgmlOpenTag(FILE * file, int depth,
			 string const & latexname) const;
        ///
        void sgmlCloseTag(FILE * file, int depth,
			  string const & latexname) const;
	///
	void LinuxDocError(LyXParagraph * par, int pos, char const * message);
        ///
	void SimpleLinuxDocOnePar(FILE * file, LyXParagraph * par,
				  int desc_on, int const depth);
        ///
	void SimpleDocBookOnePar(string & file, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int const depth);

	/// LinuxDoc.
	void push_tag(FILE * file, char const * tag,
		      int & pos, char stack[5][3]);
	
	/// LinuxDoc.
	void pop_tag(FILE * file, char const * tag,
		     int & pos, char stack[5][3]);

	///
	void RoffAsciiTable(FILE * file, LyXParagraph * par);

	/// is save needed
	bool lyx_clean;
	
	/// is autosave needed
	bool bak_clean;
	
	/** do we need to run LaTeX, changed 23/03/98, Heinrich Bauer
	    We have to distinguish between TeX-runs executed in the original
	    directory (in which the original LyX-file resides) and TeX-runs
	    executed in a temporary directory. The first situation is valid
	    for a dvi-export, the latter one for printing or previewing. */
	bool dvi_clean_orgd;
	bool dvi_clean_tmpd;

        /// do we need to run weave/tangle
        bool nw_clean;

	/// is regenerating .tex necessary
	DEPCLEAN * dep_clean;

	/// buffer is r/o
	bool read_only;

	/// name of the file the buffer is associated with.
	string filename;

	/// Format number of buffer
	float format;
	
	/** A list of views using this buffer.
	  Why not keep a list of the BufferViews that use this buffer?

	  At least then we don't have to do a lot of magic like:
	  buffer->lyx_gui->bufferview->updateLayoutChoice. Just ask each
	  of the buffers in the list of users to do a updateLayoutChoice.
	  */
	BufferView * users;

	///
	friend class BufferList;
	///
        friend class BufferView;

	/// Used when typesetting to place errorboxes.
	TexRow texrow;
};


inline  
void Buffer::InsetSleep()
{
    if (the_locking_inset && !inset_slept) {
	the_locking_inset->GetCursorPos(slx, sly);
        the_locking_inset->InsetUnlock();
	inset_slept = true;
    }
}


inline  
void Buffer::InsetWakeup()
{
    if (the_locking_inset && inset_slept) {
	the_locking_inset->Edit(slx, sly);
	inset_slept = false;
    }	
}


inline  
void Buffer::setParentName(string const & name)
{
    params.parentname = name;    
}

#endif
