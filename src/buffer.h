// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *           Copyright 1995 Matthias Ettrich
 *
 *           This file is Copyleft 1996
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

#include "LString.h"

#include "BufferView.h"
#include "lyxvc.h"
#include "bufferparams.h"
#include "texrow.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"


class LyXRC;
class TeXErrors;
class LaTeXFeatures;

extern void updateAllVisibleBufferRelatedPopups();

///
struct DEPCLEAN {
	///
	bool clean;
	///
	string master;
	///
	DEPCLEAN * next;
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

	/** high-level interface to buffer functionality
	    This function parses a command string and executes it
	*/
	void Dispatch(const string & command);

	/// Maybe we know the function already by number...
	void Dispatch(int ac, const string & argument);

	/// should be changed to work for a list.
	void resize() {
		if (users) {
			users->resize();
		}
	}

	/// Update window titles of all users
	void updateTitles() const;

	/// Reset autosave timers for all users
	void resetAutosaveTimers() const;

	/** Adds the BufferView to the users list.
	    Later this func will insert the BufferView into a real list,
	    not just setting a pointer.
	*/
	void addUser(BufferView * u) { users = u; }

	/** Removes the BufferView from the users list.
	    Since we only can have one at the moment, we just reset it.
	*/
	void delUser(BufferView *) { users = 0; }
	
	///
	void redraw() {
		users->redraw(); 
		users->fitCursor(); 
		users->updateScrollbar();
	}

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

	/* This parses a single LyXformat-Token */
	bool parseSingleLyXformat2Token(LyXLex &, LyXParagraph *& par,
					LyXParagraph *& return_par,
					const string & token, int & pos,
					char & depth, LyXFont &,
					LyXParagraph::footnote_flag &,
					LyXParagraph::footnote_kind &);

	/** Save file
	    Takes care of auto-save files and backup file if requested.
	    Returns true if the save is successful, false otherwise.
	*/
	bool save(bool makeBackup) const;
	
	/// Write file. Returns false if unsuccesful.
	bool writeFile(string const &, bool) const;
	
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
	bool isDviClean() const;
	
        ///
        bool isNwClean() const { return nw_clean; }
       
	///
	bool isBakClean() const { return bak_clean; }
	
	///
	bool isDepClean(string const & name) const;
	
	///
	void markLyxClean() const { 
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
		DEPCLEAN * tmp = dep_clean;
		while (tmp) {
			tmp->clean = false;
			tmp = tmp->next;
		}
	}

	///
	string const & fileName() const { return filename; }

	/** A transformed version of the file name, adequate for LaTeX  
	    The path is stripped if no_path is true (default) */
	string getLatexName(bool no_path = true) const {
		return ChangeExtension(MakeLatexName(filename), 
				       ".tex", no_path); 
	}

	/// Change name of buffer. Updates "read-only" flag.
	void fileName(string const & newfile);

	/// Name of the document's parent
	void setParentName(string const &);

	/// Is buffer read-only?
	bool isReadonly() const { return read_only; }

	/// Set buffer read-only flag
	void setReadonly(bool flag = true) {
		if (read_only != flag) {
			read_only = flag; 
			updateTitles();
			updateAllVisibleBufferRelatedPopups();
		}
		if (read_only) {
			WarnReadonly(filename);
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

	/** Validate a buffer for LaTeX.
	    This validates the buffer, and returns a struct for use by
	    makeLaTeX and others. Its main use is to figure out what commands
	    and packages need to be included in the LaTeX file. It (should)
	    also check that the needed constructs are there (i.e. that the \refs
	    points to coresponding \labels). It should perhaps inset "error"
	    insets to help the user correct obvious mistakes.
	*/
	void validate(LaTeXFeatures &) const;

	///
	string getIncludeonlyList(char delim = ',');
	///
	string getReferenceList(char delim = '|');
	///
	string getBibkeyList(char delim = '|');

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

	/// RCS object
	LyXVC lyxvc;

	/// where the temporaries go if we want them
	string tmppath;

	///
	string filepath;

	/** While writing as LaTeX, tells whether we are
	    doing a 'nice' LaTeX file */
	bool niceFile;

	/// Used when typesetting to place errorboxes.
	TexRow texrow;
private:
        ///
        void linuxDocHandleFootnote(ostream & os,
				    LyXParagraph * & par, int const depth);
        ///
	void DocBookHandleCaption(ostream & os, string & inner_tag,
				  int const depth, int desc_on,
				  LyXParagraph * & par);
        ///
	void DocBookHandleFootnote(ostream & os,
				   LyXParagraph * & par, int const depth);
	///
        void sgmlOpenTag(ostream & os, int depth,
			 string const & latexname) const;
        ///
        void sgmlCloseTag(ostream & os, int depth,
			  string const & latexname) const;
	///
	void LinuxDocError(LyXParagraph * par, int pos, char const * message);
        ///
	void SimpleLinuxDocOnePar(ostream & os, LyXParagraph * par,
				  int desc_on, int const depth);
        ///
	void SimpleDocBookOnePar(string & file, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int const depth);

	/// LinuxDoc.
	void push_tag(ostream & os, char const * tag,
		      int & pos, char stack[5][3]);
	
	/// LinuxDoc.
	void pop_tag(ostream & os, char const * tag,
		     int & pos, char stack[5][3]);

#if 0
	///
	void RoffAsciiTable(ostream &, LyXParagraph * par);
#endif
	
	/// is save needed
	mutable bool lyx_clean;
	
	/// is autosave needed
	mutable bool bak_clean;
	
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
};


inline  
void Buffer::setParentName(string const & name)
{
	params.parentname = name;    
}

#endif
