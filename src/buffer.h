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
class Language;

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
  \author Lars Gullik Bjønnes
  */
class Buffer {
public:
	/// What type of log will \c getLogName() return?
	enum LogType {
		latexlog, ///< LaTeX log
		buildlog  ///< Literate build log
	};

	/** Constructor
	    \param file
	    \param b  optional \c false by default
	*/
	explicit Buffer(string const & file, bool b = false);
	
	/// Destrucotr
	~Buffer();

	/** Save the buffer's parameters as user default.
	    This function saves a file \c user_lyxdir/templates/defaults.lyx
	    which parameters are those of the current buffer. This file
	    is used as a default template when creating a new
	    file. Returns \c true on success.
	*/
	bool saveParamsAsDefaults();

	/** High-level interface to buffer functionality.
	    This function parses a command string and executes it
	*/
	bool Dispatch(string const & command);

	/// Maybe we know the function already by number...
	bool Dispatch(int ac, string const & argument);

	/// Should be changed to work for a list.
	void resize();
	/// 
	void resizeInsets(BufferView *);

	/// Update window titles of all users.
	void updateTitles() const;

	/// Reset autosave timers for all users.
	void resetAutosaveTimers() const;

	/** Adds the BufferView to the users list.
	    Later this func will insert the \c BufferView into a real list,
	    not just setting a pointer.
	*/
	void addUser(BufferView * u);

	/** Removes the #BufferView# from the users list.
	    Since we only can have one at the moment, we just reset it.
	*/
	void delUser(BufferView *);
	
	///
	void redraw();

	/// Load the autosaved file.
	void loadAutoSaveFile();
	
	/** Reads a file. 
	    \param par if != 0 insert the file.
	    \return \c false if method fails.
	*/
	bool readFile(LyXLex &, LyXParagraph * par = 0);
	
	/** Reads a file without header.
	    \param par if != 0 insert the file.
	    \return \c false if file is not completely read.
	*/
	bool readLyXformat2(LyXLex &, LyXParagraph * par = 0);

	/// This parses a single LyXformat-Token.
	bool parseSingleLyXformat2Token(LyXLex &, LyXParagraph *& par,
					LyXParagraph *& return_par,
					string const & token, int & pos,
					char & depth, LyXFont &
#ifndef NEW_INSETS
					,LyXParagraph::footnote_flag &,
					LyXParagraph::footnote_kind &
#endif
		);
private:
	/// Parse a single inset.
	void readInset(LyXLex &, LyXParagraph *& par, int & pos, LyXFont &);
public:
	/** Save file.
	    Takes care of auto-save files and backup file if requested.
	    Returns \c true if the save is successful, \c false otherwise.
	*/
	bool save() const;
	
	/// Write file. Returns \c false if unsuccesful.
	bool writeFile(string const &, bool) const;
	
	///
	void writeFileAscii(string const & , int);
	///
	void writeFileAscii(std::ostream &, int);
	///
	string const asciiParagraph(LyXParagraph const *,
				    unsigned int linelen) const;
	///
	void makeLaTeXFile(string const & filename,
			   string const & original_path,
			   bool nice, bool only_body = false);
	/** LaTeX all paragraphs from par to endpar.
	    \param \a endpar if == 0 then to the end
	*/
	void latexParagraphs(std::ostream & os, LyXParagraph * par,
			     LyXParagraph * endpar, TexRow & texrow) const;

        ///
	void SimpleDocBookOnePar(std::ostream &, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int depth) const ;

	///
	int runChktex();

	///
	void makeLinuxDocFile(string const & filename,
			      bool nice, bool only_body = false);
	///
	void makeDocBookFile(string const & filename,
			     bool nice, bool only_body = false);

	/// returns the main language for the buffer (document)
	Language const * GetLanguage() const;
	///
	bool isLyxClean() const;
	///
	bool isBakClean() const;
	///
	bool isDepClean(string const & name) const;
	
	///
	void markLyxClean() const;

	///
	void markBakClean();
	
	///
	void markDepClean(string const & name);
	
	///
	void setUnnamed(bool flag=true);

	///
	bool isUnnamed();

	/// Mark this buffer as dirty.
	void markDirty();

	/// Returns the buffers filename.
	string const & fileName() const;

	/** A transformed version of the file name, adequate for LaTeX.
	    \param no_path optional if \c true then the path is stripped.
	*/
	string const getLatexName(bool no_path = true) const;

	/// Get the name and type of the log.
	std::pair<LogType, string> const getLogName() const;
 
	/// Change name of buffer. Updates "read-only" flag.
	void setFileName(string const & newfile);

	/// Name of the document's parent
	void setParentName(string const &);

	/// Is buffer read-only?
	bool isReadonly() const;

	/// Set buffer read-only flag
	void setReadonly(bool flag = true);

	/// returns \c true if the buffer contains a LaTeX document
	bool isLatex() const;
	/// returns \c true if the buffer contains a LinuxDoc document
	bool isLinuxDoc() const;
	/// returns \c true if the buffer contains a DocBook document
	bool isDocBook() const;
	/** returns \c true if the buffer contains either a LinuxDoc
	    or DocBook document */
	bool isSGML() const;
        /// returns \c true if the buffer contains a Wed document
        bool isLiterate() const;

	///
	void setPaperStuff();

	/** Validate a buffer for LaTeX.
	    This validates the buffer, and returns a struct for use by
	    #makeLaTeX# and others. Its main use is to figure out what
	    commands and packages need to be included in the LaTeX file.
	    It (should) also check that the needed constructs are there
	    (i.e. that the \refs points to coresponding \labels). It
	    should perhaps inset "error" insets to help the user correct
	    obvious mistakes.
	*/
	void validate(LaTeXFeatures &) const;

	///
	string const getIncludeonlyList(char delim = ',');
	///
	std::vector<std::pair<string,string> > const getBibkeyList();
	///
	struct TocItem {
		///
		LyXParagraph * par;
		///
		int depth;
		///
		string str;
	};
	/// The different content list types.
	enum TocType {
		TOC_TOC = 0, ///< Table of Contents
		TOC_LOF, ///< List of Figures
		TOC_LOT, ///< List of Tables
		TOC_LOA ///< List of Algorithms
	};
	///
	std::vector<std::vector<TocItem> > const getTocList() const;
	///
	std::vector<string> const getLabelList();

	/** This will clearly have to change later. Later we can have more
	    than one user per buffer. */
	BufferView * getUser() const;

	///
	void ChangeLanguage(Language const * from, Language const * to);
	///
	bool isMultiLingual();

	/// Does this mean that this is buffer local?
        UndoStack undostack;
	
	/// Does this mean that this is buffer local? 
        UndoStack redostack;
	
	///
	BufferParams params;
	
	/** The list of paragraphs.
	    This is a linked list of paragraph, this list holds the
	    whole contents of the document.
	 */
	LyXParagraph * paragraph;

	/// LyX version control object.
	LyXVC lyxvc;

	/// Where to put temporary files.
	string tmppath;

	/// The path to the document file.
	string filepath;

	/** If we are writing a nice LaTeX file or not.
	    While writing as LaTeX, tells whether we are
	    doing a 'nice' LaTeX file */
	bool niceFile;

	/// Used when typesetting to place errorboxes.
	TexRow texrow;
private:
#ifndef NEW_INSETS
        ///
        void linuxDocHandleFootnote(std::ostream & os,
				    LyXParagraph * & par, int depth);
#endif
        ///
	void DocBookHandleCaption(std::ostream & os, string & inner_tag,
				  int depth, int desc_on,
				  LyXParagraph * & par);
#ifndef NEW_INSETS
        ///
	void DocBookHandleFootnote(std::ostream & os,
				   LyXParagraph * & par, int depth);
#endif
	///
        void sgmlOpenTag(std::ostream & os, int depth,
			 string const & latexname) const;
        ///
        void sgmlCloseTag(std::ostream & os, int depth,
			  string const & latexname) const;
	///
	void LinuxDocError(LyXParagraph * par, int pos,
			   string const & message);
        ///
	void SimpleLinuxDocOnePar(std::ostream & os, LyXParagraph * par,
				  int desc_on, int depth);

	/// LinuxDoc.
	void push_tag(std::ostream & os, string const & tag,
		      int & pos, char stack[5][3]);
	
	/// LinuxDoc.
	void pop_tag(std::ostream & os, string const & tag,
		     int & pos, char stack[5][3]);

	/// is save needed
	mutable bool lyx_clean;
	
	/// is autosave needed
	mutable bool bak_clean;
	
	/// is this a unnamed file (New...)
	bool unnamed;

	/// is regenerating #.tex# necessary
	DEPCLEAN * dep_clean;

	/// buffer is r/o
	bool read_only;

	/// name of the file the buffer is associated with.
	string filename;

	/// Format number of buffer
	int file_format;
	/** A list of views using this buffer.
	    Why not keep a list of the BufferViews that use this buffer?

	    At least then we don't have to do a lot of magic like:
	    #buffer->lyx_gui->bufferview->updateLayoutChoice#. Just ask each
	    of the buffers in the list of users to do a #updateLayoutChoice#.
	*/
	BufferView * users;

public:
	///
	class inset_iterator {
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef Inset value_type;
		typedef ptrdiff_t difference_type;
		typedef Inset * pointer;
		typedef Inset & reference;
		
		
		///
		inset_iterator() : par(0) /*, it(0)*/ {}
		//
		inset_iterator(LyXParagraph * paragraph) : par(paragraph) {
			SetParagraph();
		}
		///
		inset_iterator(LyXParagraph * paragraph,
			       LyXParagraph::size_type pos);
		///
		inset_iterator & operator++() { // prefix ++
			if (par) {
				++it;
				if (it == par->inset_iterator_end()) {
					par = par->next;
					SetParagraph();
				}
			}
			return *this;
		}
		///
		inset_iterator operator++(int) { // postfix ++
			inset_iterator tmp(par, it.getPos());
			if (par) {
				++it;
				if (it == par->inset_iterator_end()) {
					par = par->next;
					SetParagraph();
				}
			}
			return tmp;
		}
		///
		Inset * operator*() { return *it; }
		
		///
		LyXParagraph * getPar() { return par; }
		///
		LyXParagraph::size_type getPos() const { return it.getPos(); }
		///
		friend
		bool operator==(inset_iterator const & iter1,
				inset_iterator const & iter2);
	private:
		///
		void SetParagraph();
		///
		LyXParagraph * par;
		///
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
void Buffer::addUser(BufferView * u)
{
	users = u;
}


inline
void Buffer::delUser(BufferView *)
{
	users = 0;
}
	

inline
void Buffer::redraw()
{
	users->redraw(); 
	users->fitCursor(users->text); 
}


inline
Language const * Buffer::GetLanguage() const
{
	return params.language;
}
	

inline
bool Buffer::isLyxClean() const
{
	return lyx_clean;
}
	

inline
bool Buffer::isBakClean() const
{
	return bak_clean;
}


inline
void Buffer::markLyxClean() const
{ 
	if (!lyx_clean) {
		lyx_clean = true; 
		updateTitles();
	}
	// if the .lyx file has been saved, we don't need an
	// autosave 
	bak_clean = true;
}


inline
void Buffer::markBakClean()
{
	bak_clean = true;
}


inline
void Buffer::setUnnamed(bool flag)
{
	unnamed = flag;
}


inline
bool Buffer::isUnnamed()
{
	return unnamed;
}


inline
void Buffer::markDirty()
{
	if (lyx_clean) {
		lyx_clean = false;
		updateTitles();
	}
	bak_clean = false;
	DEPCLEAN * tmp = dep_clean;
	while (tmp) {
		tmp->clean = false;
		tmp = tmp->next;
	}
}


inline
string const & Buffer::fileName() const
{
	return filename;
}


inline
bool Buffer::isReadonly() const
{
	return read_only;
}


inline
BufferView * Buffer::getUser() const
{
	return users;
}


inline  
void Buffer::setParentName(string const & name)
{
	params.parentname = name;    
}


///
inline
bool operator==(Buffer::TocItem const & a, Buffer::TocItem const & b) {
	return a.par == b.par && a.str == b.str;
	// No need to compare depth.
}


///
inline
bool operator!=(Buffer::TocItem const & a, Buffer::TocItem const & b) {
	return !(a == b);
	// No need to compare depth.
}

///
inline
bool operator==(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2) {
	return iter1.par == iter2.par
		&& (iter1.par == 0 || iter1.it == iter2.it);
}

///
inline
bool operator!=(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2) {
	return !(iter1 == iter2);
}
#endif

