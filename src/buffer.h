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

#ifndef BUFFER_H
#define BUFFER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "undo.h"
#include "undostack.h"
#include "lyxvc.h"
#include "bufferparams.h"
#include "texrow.h"
#include "paragraph.h"

class BufferView;
class LyXRC;
class TeXErrors;
class LaTeXFeatures;
class Language;
class ParIterator;

// When lyx 1.3.x starts we should enable this
// btw. we should also test this with 1.2 so that we
// do not get any surprises. (Lgb)
//#define NO_COMPABILITY 1

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
  This is the buffer object. It contains all the informations about
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

	/// Destructor
	~Buffer();

	/** High-level interface to buffer functionality.
	    This function parses a command string and executes it
	*/
	bool dispatch(string const & command);

	/// Maybe we know the function already by number...
	bool dispatch(int ac, string const & argument);

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
	bool readFile(LyXLex &, Paragraph * par = 0);

	/** Reads a file without header.
	    \param par if != 0 insert the file.
	    \return \c false if file is not completely read.
	*/
	bool readLyXformat2(LyXLex &, Paragraph * par = 0);

	/// This parses a single LyXformat-Token.
	bool parseSingleLyXformat2Token(LyXLex &, Paragraph *& par,
					Paragraph *& return_par,
					string const & token, int & pos,
					Paragraph::depth_type & depth,
					LyXFont &);
	///
	void insertStringAsLines(Paragraph *&, lyx::pos_type &,
				 LyXFont const &, string const &) const;
#ifndef NO_COMPABILITY
	///
	Inset * isErtInset(Paragraph * par, int pos) const;
	///
	void insertErtContents(Paragraph * par, int & pos,
			       bool set_inactive = true);
#endif
	///
	Paragraph * getParFromID(int id) const;
private:
	/// Parse a single inset.
	void readInset(LyXLex &, Paragraph *& par, int & pos, LyXFont &);
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
	string const asciiParagraph(Paragraph const *, unsigned int linelen,
				    bool noparbreak = false) const;
	///
	void makeLaTeXFile(string const & filename,
			   string const & original_path,
			   bool nice, bool only_body = false);
	/** LaTeX all paragraphs from par to endpar.
	    \param \a endpar if == 0 then to the end
	*/
	void latexParagraphs(std::ostream & os, Paragraph * par,
			     Paragraph * endpar, TexRow & texrow) const;
	///
	void simpleDocBookOnePar(std::ostream &,
				 Paragraph * par, int & desc_on,
				 Paragraph::depth_type depth) const ;
	///
	void simpleLinuxDocOnePar(std::ostream & os, Paragraph * par,
				  Paragraph::depth_type depth);
	///
	void makeLinuxDocFile(string const & filename,
			      bool nice, bool only_body = false);
	///
	void makeDocBookFile(string const & filename,
			     bool nice, bool only_body = false);
	/// Open SGML/XML tag.
	void sgmlOpenTag(std::ostream & os, Paragraph::depth_type depth,
		string const & latexname) const;
	/// Closes SGML/XML tag.
	void sgmlCloseTag(std::ostream & os, Paragraph::depth_type depth,
		string const & latexname) const;
	///
	void sgmlError(Paragraph * par, int pos, string const & message) const;

	/// returns the main language for the buffer (document)
	Language const * getLanguage() const;
	///
	int runChktex();
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
	void setUnnamed(bool flag = true);

	///
	bool isUnnamed();

	/// Mark this buffer as dirty.
	void markDirty();

	/// Returns the buffer's filename. It is always an absolute path.
	string const & fileName() const;

	/// Returns the the path where the buffer lives.
	/// It is always an absolute path.
	string const & filePath() const;

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
	std::vector<std::pair<string, string> > const getBibkeyList() const;
	///
	struct TocItem {
		TocItem(Paragraph * p, int d, string const & s)
			: par(p), depth(d), str(s) {}
		///
		Paragraph * par;
		///
		int depth;
		///
		string str;
	};
	///
	typedef std::vector<TocItem> SingleList;
	///
	typedef std::map<string, SingleList> Lists;
	///
	Lists const getLists() const;
	///
	std::vector<string> const getLabelList();

	/** This will clearly have to change later. Later we can have more
	    than one user per buffer. */
	BufferView * getUser() const;

	///
	void changeLanguage(Language const * from, Language const * to);
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
	Paragraph * paragraph;

	/// LyX version control object.
	LyXVC lyxvc;

	/// Where to put temporary files.
	string tmppath;

	/** If we are writing a nice LaTeX file or not.
	    While writing as LaTeX, tells whether we are
	    doing a 'nice' LaTeX file */
	bool niceFile;

	/// Used when typesetting to place errorboxes.
	TexRow texrow;
private:
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
	string filename_;

	/// The path to the document file.
	string filepath_;

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
		inset_iterator(Paragraph * paragraph) : par(paragraph) {
			setParagraph();
		}
		///
		inset_iterator(Paragraph * paragraph, lyx::pos_type pos);
		///
		inset_iterator & operator++() { // prefix ++
			if (par) {
				++it;
				if (it == par->inset_iterator_end()) {
					par = par->next();
					setParagraph();
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
					par = par->next();
					setParagraph();
				}
			}
			return tmp;
		}
		///
		Inset * operator*() { return *it; }

		///
		Paragraph * getPar() { return par; }
		///
		lyx::pos_type getPos() const { return it.getPos(); }
		///
		friend
		bool operator==(inset_iterator const & iter1,
				inset_iterator const & iter2);
	private:
		///
		void setParagraph();
		///
		Paragraph * par;
		///
		Paragraph::inset_iterator it;
	};

	///
	inset_iterator inset_iterator_begin() {
		return inset_iterator(paragraph);
	}
	///
	inset_iterator inset_iterator_end() {
		return inset_iterator();
	}
	///
	inset_iterator inset_const_iterator_begin() const {
		return inset_iterator(paragraph);
	}
	///
	inset_iterator inset_const_iterator_end() const {
		return inset_iterator();
	}

	///
	ParIterator par_iterator_begin();
	///
	ParIterator par_iterator_end();

	///
	Inset * getInsetFromID(int id_arg) const;
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
Language const * Buffer::getLanguage() const
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
	return filename_;
}


inline
string const & Buffer::filePath() const
{
	return filepath_;
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
bool operator==(Buffer::TocItem const & a, Buffer::TocItem const & b)
{
	return a.par == b.par && a.str == b.str;
	// No need to compare depth.
}


///
inline
bool operator!=(Buffer::TocItem const & a, Buffer::TocItem const & b)
{
	return !(a == b);
	// No need to compare depth.
}


///
inline
bool operator==(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return iter1.par == iter2.par
		&& (iter1.par == 0 || iter1.it == iter2.it);
}


///
inline
bool operator!=(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return !(iter1 == iter2);
}

#endif
