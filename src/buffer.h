// -*- C++ -*-
/**
 * \file buffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "LString.h"
#include "undo.h"
#include "support/limited_stack.h"

#include "lyxvc.h"
#include "bufferparams.h"
#include "texrow.h"
#include "ParagraphList.h"
#include "author.h"
#include "iterators.h"

#include <boost/scoped_ptr.hpp>

class BufferView;
class LyXRC;
class TeXErrors;
class LaTeXFeatures;
class LatexRunParams;
class Language;
class ParIterator;
class ParConstIterator;
class Messages;


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
	bool dispatch(string const & command, bool * result = 0);

	/// Maybe we know the function already by number...
	bool dispatch(int ac, string const & argument, bool * result = 0);

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

	/** Inserts a file into a document
	    \param par if != 0 insert the file.
	    \return \c false if method fails.
	*/
	bool readFile(LyXLex &, string const &, ParagraphList::iterator pit);

	// FIXME: it's very silly to pass a lex in here
	/// load a new file
	bool readFile(LyXLex &, string const &);

	/// read the header, returns number of unknown tokens
	int readHeader(LyXLex & lex);

	/** Reads a file without header.
	    \param par if != 0 insert the file.
	    \return \c false if file is not completely read.
	*/
	bool readBody(LyXLex &, ParagraphList::iterator pit);

	/// This parses a single token
	int readParagraph(LyXLex &, string const & token,
	              ParagraphList & pars, ParagraphList::iterator & pit,
	              Paragraph::depth_type & depth);

	///
	void insertStringAsLines(ParagraphList::iterator &, lyx::pos_type &,
				 LyXFont const &, string const &);
	///
	ParIterator getParFromID(int id) const;
	/// do we have a paragraph with this id?
	bool hasParWithID(int id) const;

public:
	/** Save file.
	    Takes care of auto-save files and backup file if requested.
	    Returns \c true if the save is successful, \c false otherwise.
	*/
	bool save() const;

	/// Write file. Returns \c false if unsuccesful.
	bool writeFile(string const &) const;

	///
	void writeFileAscii(string const & , int);
	///
	void writeFileAscii(std::ostream &, int);
	///
	string const asciiParagraph(Paragraph const &, unsigned int linelen,
				    bool noparbreak = false) const;
	/// Just a wrapper for the method below, first creating the ofstream.
	void makeLaTeXFile(string const & filename,
			   string const & original_path,
			   LatexRunParams const &,
			   bool only_body = false,
			   bool only_preamble = false);
	///
	void makeLaTeXFile(std::ostream & os,
			   string const & original_path,
			   LatexRunParams const &,
			   bool only_body = false,
			   bool only_preamble = false);
	///
	void simpleDocBookOnePar(std::ostream &,
				 ParagraphList::iterator par, int & desc_on,
				 Paragraph::depth_type depth) const;
	///
	void simpleLinuxDocOnePar(std::ostream & os,
	       ParagraphList::iterator par,
				 Paragraph::depth_type depth) const;
	///
	void makeLinuxDocFile(string const & filename,
			      bool nice, bool only_body = false);
	///
	void makeDocBookFile(string const & filename,
			     bool nice, bool only_body = false);
	///
	void sgmlError(ParagraphList::iterator par, int pos, string const & message) const;

	/// returns the main language for the buffer (document)
	Language const * getLanguage() const;
	/// get l10n translated to the buffers language
	string const B_(string const & l10n) const;

	///
	int runChktex();
	/// return true if the main lyx file does not need saving
	bool isClean() const;
	///
	bool isBakClean() const;
	///
	bool isDepClean(string const & name) const;

	/// mark the main lyx file as not needing saving
	void markClean() const;

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

	/// return all bibkeys from buffer and its childs
	void fillWithBibKeys(std::vector<std::pair<string, string> > & keys) const;
	///
	void getLabelList(std::vector<string> &) const;

	/** This will clearly have to change later. Later we can have more
	    than one user per buffer. */
	BufferView * getUser() const;

	///
	void changeLanguage(Language const * from, Language const * to);

	///
	void updateDocLang(Language const * nlang);

	///
	bool isMultiLingual();

	/// Does this mean that this is buffer local?
	limited_stack<Undo> undostack;

	/// Does this mean that this is buffer local?
	limited_stack<Undo> redostack;

	///
	BufferParams params;

	/** The list of paragraphs.
	    This is a linked list of paragraph, this list holds the
	    whole contents of the document.
	 */
	ParagraphList paragraphs;

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

	/// the author list for the document
	AuthorList & authors();

private:
	typedef std::map<string, bool> DepClean;

	/// need to regenerate .tex ?
	DepClean dep_clean_;

	/// is save needed
	mutable bool lyx_clean;

	/// is autosave needed
	mutable bool bak_clean;

	/// is this a unnamed file (New...)
	bool unnamed;

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
	///
	boost::scoped_ptr<Messages> messages_;
public:
	///
	class inset_iterator {
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef Inset value_type;
		typedef ptrdiff_t difference_type;
		typedef Inset * pointer;
		typedef Inset & reference;
		typedef ParagraphList::iterator base_type;

		///
		inset_iterator();
		///
		inset_iterator(base_type p, base_type e);
		///
		inset_iterator(base_type p, lyx::pos_type pos, base_type e);

		/// prefix ++
		inset_iterator & operator++();
		/// postfix ++
		inset_iterator operator++(int);
		///
		reference operator*();
		///
		pointer operator->();

		///
		ParagraphList::iterator getPar() const;
		///
		lyx::pos_type getPos() const;
		///
		friend
		bool operator==(inset_iterator const & iter1,
				inset_iterator const & iter2);
	private:
		///
		void setParagraph();
		///
		ParagraphList::iterator pit;
		///
		ParagraphList::iterator pend;
		///
		InsetList::iterator it;
	};

	/// return an iterator to all *top-level* insets in the buffer
	inset_iterator inset_iterator_begin();

	/// return the end of all *top-level* insets in the buffer
	inset_iterator inset_iterator_end();

	/// return a const iterator to all *top-level* insets in the buffer
	inset_iterator inset_const_iterator_begin() const;

	/// return the const end of all *top-level* insets in the buffer
	inset_iterator inset_const_iterator_end() const;

	///
	ParIterator par_iterator_begin();
	///
	ParConstIterator par_iterator_begin() const;
	///
	ParIterator par_iterator_end();
	///
	ParConstIterator par_iterator_end() const;

	///
	Inset * getInsetFromID(int id_arg) const;
};

bool operator==(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2);

bool operator!=(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2);
#endif
