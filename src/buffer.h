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


class LyXRC;
class TeXErrors;
class LaTeXFeatures;
class auto_mem_buffer;

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
	explicit Buffer(string const & file, bool b = false);
	
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
	bool Dispatch(string const & command);

	/// Maybe we know the function already by number...
	bool Dispatch(int ac, string const & argument);

	/// and have an xtl buffer to work with.
	bool Dispatch(int, auto_mem_buffer &);

	/// should be changed to work for a list.
	void resize();

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
		//users->updateScrollbar();
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
					string const & token, int & pos,
					char & depth, LyXFont &,
					LyXParagraph::footnote_flag &,
					LyXParagraph::footnote_kind &);

	/** Save file
	    Takes care of auto-save files and backup file if requested.
	    Returns true if the save is successful, false otherwise.
	*/
	bool save() const;
	
	/// Write file. Returns false if unsuccesful.
	bool writeFile(string const &, bool) const;
	
	///
	void writeFileAscii(string const & , int);
	
	///
	void makeLaTeXFile(string const & filename,
			   string const & original_path,
			   bool nice, bool only_body = false);
	//
	// LaTeX all paragraphs from par to endpar,
	// if endpar == 0 then to the end
	//
	void latexParagraphs(std::ostream & os, LyXParagraph *par,
			     LyXParagraph *endpar, TexRow & texrow) const;

	///
	int runLaTeX();

        ///
        int runLiterate();

        ///
        int buildProgram();

	///
	int runChktex();

	///
	void makeLinuxDocFile(string const & filename,
			      bool nice, bool only_body = false);
	///
	void makeDocBookFile(string const & filename,
			     bool nice, bool only_body = false);

	/// returns the main language for the buffer (document)
	string GetLanguage() const {
		return params.language;
	}
	
	///
	bool isLyxClean() const { return lyx_clean; }
	
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

        ///
        void markNwClean() { nw_clean = true; }
       
	///
	void markBakClean() { bak_clean = true; }
	
	///
	void markDepClean(string const & name);
	
        ///
        void markNwDirty() { nw_clean = false; }
       
	///
	void markDirty() {
		if (lyx_clean) {
			lyx_clean = false;
			updateTitles();
		}
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
	string getLatexName(bool no_path = true) const;

	/// Change name of buffer. Updates "read-only" flag.
	void fileName(string const & newfile);

	/// Name of the document's parent
	void setParentName(string const &);

	/// Is buffer read-only?
	bool isReadonly() const { return read_only; }

	/// Set buffer read-only flag
	void setReadonly(bool flag = true);

	/// returns true if the buffer contains a LaTeX document
	bool isLatex() const;
	/// returns true if the buffer contains a LinuxDoc document
	bool isLinuxDoc() const;
	/// returns true if the buffer contains a DocBook document
	bool isDocBook() const;
	/** returns true if the buffer contains either a LinuxDoc
	    or DocBook document */
	bool isSGML() const;
        /// returns true if the buffer contains a Wed document
        bool isLiterate() const;

	///
	void setPaperStuff();

	/** Validate a buffer for LaTeX.
	    This validates the buffer, and returns a struct for use by
	    makeLaTeX and others. Its main use is to figure out what
	    commands and packages need to be included in the LaTeX file.
	    It (should) also check that the needed constructs are there
	    (i.e. that the \refs points to coresponding \labels). It
	    should perhaps inset "error" insets to help the user correct
	    obvious mistakes.
	*/
	void validate(LaTeXFeatures &) const;

	///
	string getIncludeonlyList(char delim = ',');
	///
	std::vector<std::pair<string,string> > getBibkeyList();
	///
	struct TocItem {
		LyXParagraph * par;
		int depth;
		string str;
	};
	///
	enum TocType {
		///
		TOC_TOC = 0,
		///
		TOC_LOF,
		///
		TOC_LOT,
		///
		TOC_LOA
	};
	///
	std::vector<std::vector<TocItem> > getTocList();
	///
	std::vector<string> getLabelList();

	/** This will clearly have to change later. Later we can have more
	    than one user per buffer. */
	BufferView * getUser() const { return users; }

	///
	void ChangeLanguage(Language const * from, Language const * to);
	///
	bool isMultiLingual();

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
        void linuxDocHandleFootnote(std::ostream & os,
				    LyXParagraph * & par, int const depth);
        ///
	void DocBookHandleCaption(std::ostream & os, string & inner_tag,
				  int const depth, int desc_on,
				  LyXParagraph * & par);
        ///
	void DocBookHandleFootnote(std::ostream & os,
				   LyXParagraph * & par, int const depth);
	///
        void sgmlOpenTag(std::ostream & os, int depth,
			 string const & latexname) const;
        ///
        void sgmlCloseTag(std::ostream & os, int depth,
			  string const & latexname) const;
	///
	void LinuxDocError(LyXParagraph * par, int pos, char const * message);
        ///
	void SimpleLinuxDocOnePar(std::ostream & os, LyXParagraph * par,
				  int desc_on, int const depth);
        ///
	void SimpleDocBookOnePar(std::ostream &, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int const depth);

	/// LinuxDoc.
	void push_tag(std::ostream & os, char const * tag,
		      int & pos, char stack[5][3]);
	
	/// LinuxDoc.
	void pop_tag(std::ostream & os, char const * tag,
		     int & pos, char stack[5][3]);

	/// is save needed
	mutable bool lyx_clean;
	
	/// is autosave needed
	mutable bool bak_clean;
	
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

public:
	class inset_iterator {
	public:
		inset_iterator() : par(0) /*, it(0)*/ {}
		inset_iterator(LyXParagraph * paragraph) : par(paragraph) {
			SetParagraph();
		}
		inset_iterator(LyXParagraph * paragraph, LyXParagraph::size_type pos);
		inset_iterator & operator++() {
			if (par) {
				++it;
				if (it == par->inset_iterator_end()) {
					par = par->next;
					SetParagraph();
				}
			}
			return *this;
		}
		Inset * operator*() {return *it; }
		LyXParagraph * getPar() { return par; }
		LyXParagraph::size_type getPos() {return it.getPos(); }
		friend
		bool operator==(inset_iterator const & iter1,
				inset_iterator const & iter2) {
			return iter1.par == iter2.par
				&& (iter1.par == 0 || iter1.it == iter2.it);
		}
		friend
		bool operator!=(inset_iterator const & iter1,
				inset_iterator const & iter2) {
			return !(iter1 == iter2);
		}
	private:
		void SetParagraph();
		LyXParagraph * par;
		LyXParagraph::inset_iterator it;
	};

	///
	inset_iterator inset_iterator_begin() {
		return inset_iterator(paragraph);
	}
	///
	inset_iterator inset_iterator_end() {
		return inset_iterator();
	}
};


inline  
void Buffer::setParentName(string const & name)
{
	params.parentname = name;    
}

inline
bool operator==(Buffer::TocItem const & a, Buffer::TocItem const & b) {
	return a.par == b.par && a.str == b.str;
	// No need to compare depth.
}


inline
bool operator!=(Buffer::TocItem const & a, Buffer::TocItem const & b) {
	return !(a == b);
	// No need to compare depth.
}

#endif
