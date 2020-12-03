// -*- C++ -*-
/**
 * \file Buffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "OutputEnums.h"

#include "support/unique_ptr.h"
#include "support/strfwd.h"
#include "support/types.h"

#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>


namespace lyx {

class BiblioInfo;
class BibTeXInfo;
class BufferParams;
class DispatchResult;
class DocIterator;
class docstring_list;
class ErrorList;
class FuncRequest;
class FuncStatus;
class Inset;
class InsetLabel;
class Lexer;
class Text;
class LyXVC;
class LaTeXFeatures;
class Language;
class MacroData;
class MacroNameSet;
class MacroSet;
class OutputParams;
class otexstream;
class ParagraphList;
class ParIterator;
class ParConstIterator;
class TeXErrors;
class TexRow;
class TocBackend;
class Undo;
class WordLangTuple;

namespace frontend {
class GuiBufferDelegate;
class WorkAreaManager;
} // namespace frontend

namespace support {
class DocFileName;
class FileName;
class FileNameList;
} // namespace support

namespace graphics {
class PreviewLoader;
class Cache;
} // namespace graphics


class Buffer;
typedef std::list<Buffer *> ListOfBuffers;
/// a list of Buffers we cloned
typedef std::set<Buffer *> CloneList;
typedef std::shared_ptr<CloneList> CloneList_ptr;


/** The buffer object.
 * This is the buffer object. It contains all the information about
 * a document loaded into LyX.
 * The buffer object owns the Text (wrapped in an InsetText), which
 * contains the individual paragraphs of the document.
 *
 *
 * I am not sure if the class is complete or
 * minimal, probably not.
 * \author Lars Gullik Bjønnes
 */

class Buffer {
public:
	/// What type of log will \c getLogName() return?
	enum LogType {
		latexlog, ///< LaTeX log
		buildlog  ///< Literate build log
	};

	/// Result of \c readFile()
	enum ReadStatus {
		ReadSuccess,
		ReadCancel,
		// failures
		ReadFailure,
		ReadWrongVersion,
		ReadFileNotFound,
		ReadVCError,
		ReadAutosaveFailure,
		ReadEmergencyFailure,
		ReadNoLyXFormat,
		ReadDocumentFailure,
		// lyx2lyx
		LyX2LyXNoTempFile,
		LyX2LyXNotFound,
		LyX2LyXOlderFormat,
		LyX2LyXNewerFormat,
		// other
		ReadOriginal
	};

	enum ExportStatus {
		// export
		ExportSuccess,
		ExportCancel,
		ExportKilled,
		ExportError,
		ExportNoPathToFormat,
		ExportTexPathHasSpaces,
		ExportConverterError,
		// preview
		// Implies ExportSuccess.
		PreviewSuccess,
		// The exported file exists but there was an error when opening
		// it in a viewer.
		PreviewError
	};

	///
	enum UpdateScope {
		UpdateMaster,
		UpdateChildOnly
	};

	/// Constructor
	explicit Buffer(std::string const & file, bool readonly = false,
		Buffer const * cloned_buffer = nullptr);

	/// Destructor
	~Buffer();

	/// Clones the entire structure of which this Buffer is part, 
	/// cloning all the children, too.
	Buffer * cloneWithChildren() const;
	/// Just clones this single Buffer. For autosave.
	Buffer * cloneBufferOnly() const;
	///
	bool isClone() const;

	/** High-level interface to buffer functionality.
	    This function parses a command string and executes it.
	*/
	void dispatch(std::string const & command, DispatchResult & result);

	/// Maybe we know the function already by number...
	void dispatch(FuncRequest const & func, DispatchResult & result);

	/// Can this function be exectued?
	/// \return true if we made a decision
	bool getStatus(FuncRequest const & cmd, FuncStatus & flag);

	///
	DocIterator getParFromID(int id) const;
	/// do we have a paragraph with this id?
	bool hasParWithID(int id) const;

	///
	frontend::WorkAreaManager & workAreaManager() const;

	/** Save file.
	    Takes care of auto-save files and backup file if requested.
	    Returns \c true if the save is successful, \c false otherwise.
	*/
	bool save() const;
	/// Renames and saves the buffer
	bool saveAs(support::FileName const & fn);

	/// Write document to stream. Returns \c false if unsuccessful.
	bool write(std::ostream &) const;
	/// Write file. Returns \c false if unsuccessful.
	bool writeFile(support::FileName const &) const;

	/// \name Functions involved in reading files/strings.
	//@{
	/// Loads the LyX file into the buffer. This function
	/// tries to extract the file from version control if it
	/// cannot be found. If it can be found, it will try to
	/// read an emergency save file or an autosave file.
	/// \sa loadThisLyXFile
	ReadStatus loadLyXFile();
	/// Loads the LyX file \c fn into the buffer. If you want
	/// to check for files in a version control container,
	/// emergency or autosave files, one should use \c loadLyXFile.
	/// /sa loadLyXFile
	ReadStatus loadThisLyXFile(support::FileName const & fn);
	/// import a new document from a string
	bool importString(std::string const &, docstring const &, ErrorList &);
	/// import a new file
	bool importFile(std::string const &, support::FileName const &, ErrorList &);
	/// read a new document from a string
	bool readString(std::string const &);
	/// Reloads the LyX file
	ReadStatus reload();
//FIXME: The following function should be private
//private:
	/// read the header, returns number of unknown tokens
	int readHeader(Lexer & lex);

	double fontScalingFactor() const;

private:
	///
	typedef std::map<Buffer const *, Buffer *> BufferMap;
	///
	void cloneWithChildren(BufferMap &, CloneList_ptr) const;
	/// save checksum of the given file.
	void saveCheckSum() const;
	/// read a new file
	ReadStatus readFile(support::FileName const & fn);
	/// Reads a file without header.
	/// \param par if != 0 insert the file.
	/// \return \c true if file is not completely read.
	bool readDocument(Lexer &);
	/// Try to extract the file from a version control container
	/// before reading if the file cannot be found.
	/// \sa LyXVC::file_not_found_hook
	ReadStatus extractFromVC();
	/// Reads the first tag of a LyX File and
	/// returns the file format number.
	ReadStatus parseLyXFormat(Lexer & lex, support::FileName const & fn,
		int & file_format) const;
	/// Convert the LyX file to the LYX_FORMAT using
	/// the lyx2lyx script and returns the filename
	/// of the temporary file to be read
	ReadStatus convertLyXFormat(support::FileName const & fn,
		support::FileName & tmpfile, int from_format);
	/// get appropriate name for backing up files from older versions
	support::FileName getBackupName() const;
	//@}

public:
	/// \name Functions involved in autosave and emergency files.
	//@{
	/// Save an autosave file to #filename.lyx#
	bool autoSave() const;
	/// save emergency file
	/// \return a status message towards the user.
	docstring emergencyWrite() const;

//FIXME:The following function should be private
//private:
	///
	void removeAutosaveFile() const;

private:
	/// Try to load an autosave file associated to \c fn.
	ReadStatus loadAutosave();
	/// Try to load an emergency file associated to \c fn.
	ReadStatus loadEmergency();
	/// Get the filename of the emergency file associated with the Buffer
	support::FileName getEmergencyFileName() const;
	/// Get the filename of the autosave file associated with the Buffer
	support::FileName getAutosaveFileName() const;
	///
	void moveAutosaveFile(support::FileName const & old) const;
	//@}

public:
	/// Fill in the ErrorList with the TeXErrors
	void bufferErrors(TeXErrors const &, ErrorList &) const;
	/// Fill in the Citation/Reference ErrorList from the TeXErrors
	void bufferRefs(TeXErrors const &, ErrorList &) const;

	enum OutputWhat {
		FullSource,
		OnlyBody,
		IncludedFile,
		OnlyPreamble,
		CurrentParagraph
	};

	/// Just a wrapper for writeLaTeXSource, first creating the ofstream.
	ExportStatus makeLaTeXFile(support::FileName const & filename,
			   std::string const & original_path,
			   OutputParams const &,
			   OutputWhat output = FullSource) const;
	/** Export the buffer to LaTeX.
	    If \p os is a file stream, and params().inputenc is "auto-legacy" or
	    "auto-legacy-plain", and the buffer contains text in different languages
	    with more than one encoding, then this method will change the
	    encoding associated to \p os. Therefore you must not call this
	    method with a string stream if the output is supposed to go to a
	    file. \code
	    ofdocstream ofs;
	    otexstream os(ofs);
	    ofs.open("test.tex");
	    writeLaTeXSource(os, ...);
	    ofs.close();
	    \endcode is NOT equivalent to \code
	    odocstringstream oss;
	    otexstream os(oss);
	    writeLaTeXSource(os, ...);
	    ofdocstream ofs;
	    ofs.open("test.tex");
	    ofs << oss.str();
	    ofs.close();
	    \endcode
	 */
	ExportStatus writeLaTeXSource(otexstream & os,
			   std::string const & original_path,
			   OutputParams const &,
			   OutputWhat output = FullSource) const;
	///
	ExportStatus makeDocBookFile(support::FileName const & filename,
			     OutputParams const & runparams_in,
			     OutputWhat output = FullSource) const;
	///
	ExportStatus writeDocBookSource(odocstream & os,
				 OutputParams const & runparams_in,
				 OutputWhat output = FullSource) const;
	///
	ExportStatus makeLyXHTMLFile(support::FileName const & filename,
			     OutputParams const & runparams_in) const;
	///
	ExportStatus writeLyXHTMLSource(odocstream & os,
			     OutputParams const & runparams_in,
			     OutputWhat output = FullSource) const;
	/// returns the main language for the buffer (document)
	Language const * language() const;
	/// get l10n translated to the buffers language
	docstring B_(std::string const & l10n) const;

	///
	int runChktex();
	/// return true if the main lyx file does not need saving
	bool isClean() const;
	///
	bool isDepClean(std::string const & name) const;

	/// Whether or not disk file has been externally modified. Uses a checksum
	/// which is accurate but slow, which can be a problem when it is frequently
	/// used, or used for a large file on a slow (network) file system.
	bool isChecksumModified() const;

	/// Flag set by the FileSystemWatcher.
	/// Fast but (not so) inaccurate, can be cleared by the user.
	bool notifiesExternalModification() const;
	void clearExternalModification() const;

	/// mark the main lyx file as not needing saving
	void markClean() const;

	///
	void markDepClean(std::string const & name);

	///
	void setUnnamed(bool flag = true);

	/// Whether or not a filename has been assigned to this buffer
	bool isUnnamed() const;

	/// Whether or not this buffer is internal.
	///
	/// An internal buffer does not contain a real document, but some auxiliary text segment.
	/// It is not associated with a filename, it is never saved, thus it does not need to be
	/// automatically saved, nor it needs to trigger any "do you want to save ?" question.
	bool isInternal() const;

	void setInternal(bool flag);

	/// Mark this buffer as dirty.
	void markDirty();

	/// Returns the buffer's filename. It is always an absolute path.
	support::FileName fileName() const;

	/// Returns the buffer's filename. It is always an absolute path.
	std::string absFileName() const;

	/// Returns the path where the buffer lives.
	/// It is always an absolute path.
	std::string filePath() const;

	/** Contructs a file name of a referenced file (child doc, included graphics etc).
	 *  Absolute names are returned as is. If the name is relative, it is
	 *  interpreted relative to filePath() if the file exists, otherwise
	 *  relative to the original path where the document was last saved.
	 *  The original path may be different from filePath() if the document was
	 *  later manually moved to a different location.
	 */
	support::DocFileName getReferencedFileName(std::string const & fn) const;

	/// Format a file name for LaTeX output (absolute or relative or filename only,
	/// depending on file and context)
	std::string const prepareFileNameForLaTeX(std::string const &,
					std::string const &, bool nice) const;

	/** Returns a vector of bibliography (*.bib) file paths suitable for the
	 *  output in the respective BibTeX/Biblatex macro and potential individual encoding
	 */
	std::vector<std::pair<docstring, std::string>> const prepareBibFilePaths(OutputParams const &,
				    const docstring_list & bibfilelist,
				    bool const extension = true) const;

	/** Returns the path where a local layout file lives.
	 *  An empty string is returned for standard system and user layouts.
	 *  If possible, it is always relative to the buffer path.
	 */
	std::string layoutPos() const;

	/** Set the path to a local layout file.
	 *  This must be an absolute path but, if possible, it is always
	 *  stored as relative to the buffer path.
	 */
	void setLayoutPos(std::string const & path);

	/** A transformed version of the file name, adequate for LaTeX.
	    \param no_path optional if \c true then the path is stripped.
	*/
	std::string latexName(bool no_path = true) const;

	/// Get the name and type of the log.
	std::string logName(LogType * type = nullptr) const;

	/// Set document's parent Buffer.
	void setParent(Buffer const *);
	Buffer const * parent() const;

	/** Get the document's master (or \c this if this is not a
	    child document)
	 */
	Buffer const * masterBuffer() const;

	/// \return true if \p child is a child of this \c Buffer.
	bool isChild(Buffer * child) const;

	/// \return true if this \c Buffer has children
	bool hasChildren() const;

	/// \return a list of the direct children of this Buffer.
	/// this list has no duplicates and is in the order in which
	/// the children appear.
	ListOfBuffers getChildren() const;

	/// \return a list of all descendants of this Buffer (children,
	/// grandchildren, etc). this list has no duplicates and is in
	/// the order in which the children appear.
	ListOfBuffers getDescendants() const;

	/// Collect all relative buffers, in the order in which they appear.
	/// I.e., the "root" Buffer is first, then its first child, then any
	/// of its children, etc. However, there are no duplicates in this
	/// list.
	/// This is "stable", too, in the sense that it returns the same
	/// thing from whichever Buffer it is called.
	ListOfBuffers allRelatives() const;

	/// Is buffer read-only? True if it has either the read-only flag or the
	/// externally modified flag.
	bool isReadonly() const;

	/// Does the buffer have the read-only flag?
	bool hasReadonlyFlag() const;

	/// Set buffer read-only flag
	void setReadonly(bool flag = true);

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

	/// Bibliography information is cached in the Buffer, so we do not
	/// have to check or read things over and over.
	/// The cache exists only in the master buffer. When it is updated,
	/// the children add their information to the master's cache.
	/// Calling this method invalidates the cache and so requires a
	/// re-read.
	void invalidateBibinfoCache() const;
	/// Clear the bibfiles cache
	void clearBibFileCache() const;
	/// Updates the cached bibliography information, checking first to see
	/// whether the cache is valid. If so, we do nothing. If not, then we
	/// reload all the BibTeX info.
	/// Note that this operates on the master document.
	void reloadBibInfoCache(bool const force = false) const;
	/// \return the bibliography information for this buffer's master,
	/// or just for it, if it isn't a child.
	BiblioInfo const & masterBibInfo() const;
	/// \return this buffer's bibliography information
	BiblioInfo const & bibInfo() const;
	/// collect bibliography info from the various insets in this buffer.
	void collectBibKeys(support::FileNameList &) const;
	/// add some BiblioInfo to our cache
	void addBiblioInfo(BiblioInfo const & bi) const;
	/// add a single piece of bibliography info to our cache
	void addBibTeXInfo(docstring const & key, BibTeXInfo const & bi) const;
	///
	void makeCitationLabels() const;
	///
	void invalidateCiteLabels() const;
	///
	bool citeLabelsValid() const;
	///
	void getLabelList(std::vector<docstring> &) const;

	/// This removes the .aux and .bbl files from the temp dir.
	void removeBiblioTempFiles() const;

	///
	void changeLanguage(Language const * from, Language const * to);

	///
	bool isMultiLingual() const;
	///
	std::set<Language const *> getLanguages() const;

	///
	BufferParams & params();
	BufferParams const & params() const;
	///
	BufferParams const & masterParams() const;

	/** The list of paragraphs.
	    This is a linked list of paragraph, this list holds the
	    whole contents of the document.
	 */
	ParagraphList & paragraphs();
	ParagraphList const & paragraphs() const;

	/// LyX version control object.
	LyXVC & lyxvc();
	LyXVC const & lyxvc() const;

	/// Where to put temporary files.
	std::string const temppath() const;

	/// Used when typesetting to place errorboxes.
	TexRow const & texrow() const;
	TexRow & texrow();

	///
	ParIterator par_iterator_begin();
	///
	ParConstIterator par_iterator_begin() const;
	///
	ParIterator par_iterator_end();
	///
	ParConstIterator par_iterator_end() const;
	/// Is document empty ?
	bool empty() const;

	// Position of the child buffer where it appears first in the master.
	DocIterator firstChildPosition(Buffer const * child);

	/** \returns true only when the file is fully loaded.
	 *  Used to prevent the premature generation of previews
	 *  and by the citation inset.
	 */
	bool isFullyLoaded() const;
	/// Set by buffer_funcs' newFile.
	void setFullyLoaded(bool);

	/// FIXME: Needed by RenderPreview.
	graphics::PreviewLoader * loader() const;
	/// Update the LaTeX preview snippets associated with this buffer
	void updatePreviews() const;
	/// Remove any previewed LaTeX snippets associated with this buffer
	void removePreviews() const;
	///
	graphics::Cache & graphicsCache() const;

	/// Our main text (inside the top InsetText)
	Text & text() const;

	/// Our top InsetText
	Inset & inset() const;

	//
	// Macro handling
	//
	/// Collect macro definitions in paragraphs
	void updateMacros() const;
	/// Iterate through the whole buffer and try to resolve macros
	void updateMacroInstances(UpdateType) const;

	/// List macro names of this buffer, the parent and the children
	void listMacroNames(MacroNameSet & macros) const;
	/// Collect macros of the parent and its children in front of this buffer.
	void listParentMacros(MacroSet & macros, LaTeXFeatures & features) const;

	/// Return macro defined before pos (or in the master buffer)
	MacroData const * getMacro(docstring const & name, DocIterator const & pos, bool global = true) const;
	/// Return macro defined anywhere in the buffer (or in the master buffer)
	MacroData const * getMacro(docstring const & name, bool global = true) const;
	/// Return macro defined before the inclusion of the child
	MacroData const * getMacro(docstring const & name, Buffer const & child, bool global = true) const;

	/// Collect user macro names at loading time
	typedef std::set<docstring> UserMacroSet;
	mutable UserMacroSet usermacros;

	/// Replace the inset contents for insets which InsetCode is equal
	/// to the passed \p inset_code. Handles undo.
	void changeRefsIfUnique(docstring const & from, docstring const & to);

	/// get source code (latex/docbook) for some paragraphs, or all paragraphs
	/// including preamble
	/// returns nullptr if Id to Row conversion is unsupported
	unique_ptr<TexRow> getSourceCode(odocstream & os,
			std::string const & format, pit_type par_begin,
			pit_type par_end, OutputWhat output, bool master) const;

	/// Access to error list.
	/// This method is used only for GUI visualisation of Buffer related
	/// errors (like parsing or LateX compilation). This method is const
	/// because modifying the returned ErrorList does not touch the document
	/// contents.
	ErrorList & errorList(std::string const & type) const;

	/// The Toc backend.
	/// This is useful only for screen visualisation of the Buffer. This
	/// method is const because modifying this backend does not touch
	/// the document contents.
	TocBackend & tocBackend() const;

	///
	Undo & undo();

	/// This function is called when the buffer is changed.
	void changed(bool update_metrics) const;
	///
	void setChild(DocIterator const & dit, Buffer * child);
	///
	void updateTocItem(std::string const &, DocIterator const &) const;
	/// This function is called when the buffer structure is changed.
	void structureChanged() const;
	/// This function is called when some parsing error shows up.
	void errors(std::string const & err, bool from_master = false) const;
	/// This function is called when the buffer busy status change.
	void setBusy(bool on) const;
	/// Update window titles of all users.
	void updateTitles() const;
	/// Reset autosave timers for all users.
	void resetAutosaveTimers() const;
	///
	void message(docstring const & msg) const;

	///
	void setGuiDelegate(frontend::GuiBufferDelegate * gui);
	///
	bool hasGuiDelegate() const;

	///
	ExportStatus doExport(std::string const & target, bool put_in_tempdir) const;
	/// Export buffer to format \p format and open the result in a suitable viewer.
	/// Note: This has nothing to do with preview of graphics or math formulas.
	ExportStatus preview(std::string const & format) const;
	/// true if there was a previous preview this session of this buffer and
	/// there was an error on the previous preview of this buffer.
	bool freshStartRequired() const;
	///
	void requireFreshStart(bool const b) const;

private:
	///
	ExportStatus doExport(std::string const & target, bool put_in_tempdir,
		std::string & result_file) const;
	/// target is a format name optionally followed by a space
	/// and a destination file-name
	ExportStatus doExport(std::string const & target, bool put_in_tempdir,
		bool includeall, std::string & result_file) const;
	///
	ExportStatus preview(std::string const & format, bool includeall) const;
	///
	void setMathFlavor(OutputParams & op) const;

public:
	///
	bool isExporting() const;

	/// A collection of InsetRef insets and their position in the buffer
	typedef std::vector<std::pair<Inset *, ParIterator> > References;
	/// Get all InsetRef insets and their positions associated with
	/// the InsetLabel with the label string \p label
	References const & references(docstring const & label) const;
	/// Add an InsetRef at position \p it to the Insetlabel
	/// with the label string \p label
	void addReference(docstring const & label, Inset * inset, ParIterator it);
	/// Clear the whole reference cache
	void clearReferenceCache() const;
	/// Set the InsetLabel for a given \p label string. \p active
	/// determines whether this is an active label (see @ref activeLabel)
	void setInsetLabel(docstring const & label, InsetLabel const * il,
			   bool const active);
	/// \return the InsetLabel associated with this \p label string
	/// If \p active is true we only return active labels
	/// (see @ref activeLabel)
	InsetLabel const * insetLabel(docstring const & label,
				      bool const active = false) const;
	/// \return true if this \param label is an active label.
	/// Inactive labels are currently deleted labels (in ct mode)
	bool activeLabel(docstring const & label) const;

	/// return a list of all used branches (also in children)
	void getUsedBranches(std::list<docstring> &, bool const from_master = false) const;

	/// Updates screen labels and some other information associated with
	/// insets and paragraphs. Actually, it's more like a general "recurse
	/// through the Buffer" routine, that visits all the insets and paragraphs.
	void updateBuffer() const { updateBuffer(UpdateMaster, InternalUpdate); }
	/// \param scope: whether to start with the master document or just
	/// do this one.
	/// \param output: whether we are preparing for output.
	void updateBuffer(UpdateScope scope, UpdateType utype) const;
	///
	void updateBuffer(ParIterator & parit, UpdateType utype, bool const deleted = false) const;

	/// Spellcheck starting from \p from.
	/// \p from initial position, will then points to the next misspelled
	///    word.
	/// \p to will points to the end of the next misspelled word.
	/// \p word_lang will contain the found misspelled word.
	/// \return progress if a new word was found.
	int spellCheck(DocIterator & from, DocIterator & to,
		WordLangTuple & word_lang, docstring_list & suggestions) const;
	///
	void checkChildBuffers();
	///
	void checkMasterBuffer();

	/// If the document is being saved to a new location and the named file
	/// exists at the old location, return its updated path relative to the
	/// new buffer path if possible, otherwise return its absolute path.
	/// In all other cases, this is a no-op and name is returned unchanged.
	/// If a non-empty ext is given, the existence of name.ext is checked
	/// but the returned path will not contain this extension.
	/// Similarly, when loading a document that was moved from the location
	/// where it was saved, return the correct path relative to the new
	/// location.
	std::string includedFilePath(std::string const & name,
				std::string const & ext = empty_string()) const;

	/// compute statistics between \p from and \p to
	/// \p from initial position
	/// \p to points to the end position
	/// \p skipNoOutput if notes etc. should be ignored
	void updateStatistics(DocIterator & from, DocIterator & to,
						  bool skipNoOutput = true) const;
	/// statistics accessor functions
	int wordCount() const;
	int charCount(bool with_blanks) const;

	/// FIXME: dummy function for now
	bool areChangesPresent() const;

	///
	void registerBibfiles(docstring_list const & bf) const;
	///
	support::FileName getBibfilePath(docstring const & bibid) const;

	/// routines for dealing with possible self-inclusion
	void pushIncludedBuffer(Buffer const * buf) const;
	void popIncludedBuffer() const;
	bool isBufferIncluded(Buffer const * buf) const;
private:
	void clearIncludeList() const;

private:
	friend class MarkAsExporting;
	/// mark the buffer as busy exporting something, or not
	void setExportStatus(bool e) const;

	///
	References & getReferenceCache(docstring const & label);
	/// Change name of buffer. Updates "read-only" flag.
	void setFileName(support::FileName const & fname);
	///
	void getLanguages(std::set<Language const *> &) const;
	/// Checks whether any of the referenced bibfiles have changed since the
	/// last time we loaded the cache. Note that this does NOT update the
	/// cached information.
	void checkIfBibInfoCacheIsValid() const;
	/// Return the list with all bibfiles in use (including bibfiles
	/// of loaded child documents).
	docstring_list const &
		getBibfiles(UpdateScope scope = UpdateMaster) const;
	///
	void collectChildren(ListOfBuffers & children, bool grand_children) const;

	/// noncopyable
	Buffer(Buffer const &);
	void operator=(Buffer const &);

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * const d;
};


/// Helper class, to guarantee that the export status
/// gets reset properly. To use, simply create a local variable:
///    MarkAsExporting mex(bufptr);
/// and leave the rest to us.
class MarkAsExporting {
public:
	MarkAsExporting(Buffer const * buf) : buf_(buf)
	{
		buf_->setExportStatus(true);
	}
	~MarkAsExporting()
	{
		buf_->setExportStatus(false);
	}
private:
	Buffer const * const buf_;
};


} // namespace lyx

#endif
