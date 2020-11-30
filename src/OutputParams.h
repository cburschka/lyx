// -*- C++ -*-
/**
 * \file OutputParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUTPARAMS_H
#define OUTPUTPARAMS_H


#include "Changes.h"

#include <memory>
#include <set>


namespace lyx {


class Encoding;
class ExportData;
class Font;
class Language;
class InsetArgument;


enum class Flavor : int {
	DviLuaTeX,
	LaTeX,
	LuaTeX,
	PdfLaTeX,
	XeTeX,
	DocBook5,
	Html,
	Text,
	LyX
};

enum class CtObject : int {
	Normal,
	Object,
	DisplayObject,
	UDisplayObject,
	OmitObject
};


class OutputParams {
public:
	enum MathFlavor {
		NotApplicable,
		MathAsMathML,
		MathAsHTML,
		MathAsImages,
		MathAsLaTeX
	};

	enum TableCell {
		NO,
		PLAIN,
		ALIGNED
	};

	enum Float {
		NONFLOAT,
		MAINFLOAT,
		SUBFLOAT
	};

	OutputParams(Encoding const *);
	~OutputParams();

	/** The file that we export depends occasionally on what is to
	    compile the file.
	*/
	Flavor flavor = Flavor::LaTeX;
	/// is it some flavor of LaTeX?
	bool isLaTeX() const;
	/// does this flavour support full unicode?
	bool isFullUnicode() const;
	/// Do we use the bidi package (which does some reordering and stuff)?
	bool useBidiPackage() const;

	/// Same, but for math output, which only matter is XHTML output.
	MathFlavor math_flavor = NotApplicable;

	/** Are we to write a 'nice' LaTeX file or not.
	    This essentially seems to mean whether InsetInclude, InsetGraphics
	    and InsetExternal should add the absolute path to any external
	    files or not.
	    Non-nice LaTeX also includes additional safe line breaks in order to
	    increase the precision of forward/reverse search and error reporting.
	*/
	bool nice = false;

	/** Is this a real child (i.e., compiled as a child)?
	    This depends on wherefrom we export the buffer. Even children
	    that have a master can be compiled standalone.
	*/
	mutable bool is_child = false;

	/** moving_arg == true means that the environment in which the inset
	    is typeset is a moving argument. The inset should take care about
	    fragile commands by preceding the latex with \\protect.
	*/
	bool moving_arg = false;

	/** intitle == true means that the environment in which the
	    inset is typeset is part of a title (before a \\maketitle).
	    Footnotes in such environments have moving arguments.
	*/
	bool intitle = false;

	/** need_maketitle == true means that the last layout was a title layout
	 * this is to track when \maketitle needs to be output.
	*/
	mutable bool need_maketitle = false;

	/** have_maketitle == true means that \maketitle already hase been output.
	*/
	mutable bool have_maketitle = false;

	/** inbranch == true means that the environment being typeset
	    is inside an active branch inset.
	*/
	bool inbranch = false;

	/** inulemcmd > 0 means that the environment in which the
	    inset is typeset is part of a ulem or soul command (e.g., \uline,
	    \uuline, \uwave, \sout or \xout). Insets that output latex commands
	    relying on local assignments (such as \cite) should enclose such
	    commands in \mbox{} in order to avoid breakage.
	*/
	mutable int inulemcmd = 0;

	/** the font at the point where the inset is
	 */
	Font const * local_font = nullptr;

	/** Document language lyx name
	 */
	std::string document_language;

	/// main font encoding of the document
	std::string main_fontenc;

	/** The master language. Non-null only for child documents.
	    Note that this is not the language of the top level master, but
	    of the direct parent for nested includes.
	 */
	mutable Language const * master_language = nullptr;

	/// Active characters
	std::string active_chars;

	/** Current stream encoding. Only used for LaTeX.
	    This must be set to the document encoding (via the constructor)
	    before output starts. Afterwards it must be kept up to date for
	    each single character (\sa Paragraph::latex).
	    This does also mean that you need to set it back if you use a
	    copy (e.g. in insets): \code
	    int InsetFoo::latex(..., OutputParams const & runparams_in) const
	    {
		OutputParams runparams(runparams_in);
		runparams.inComment = true;
		...
		InsetBla::latex(..., runparams);
		...
		runparams_in.encoding = runparams.encoding;
	    }
	    \endcode
	 */
	mutable Encoding const * encoding;

	/** free_spacing == true means that the inset is in a free-spacing
	    paragraph.
	*/
	bool free_spacing = false;

	/** This var is set by the return value from BufferParams::writeLaTeX
	*/
	bool use_babel = false;

	/** Do we use polyglossia (instead of babel)?
	*/
	bool use_polyglossia = false;

	/** Do we use hyperref?
	*/
	bool use_hyperref = false;

	/// Do we use the CJK package?
	bool use_CJK = false;

	/** Are we generating multiple indices?
	*/
	bool use_indices = false;

	/** Are we using japanese (pLaTeX)?
	*/
	bool use_japanese = false;

	/** Customized bibtex_command
	*/
	std::string bibtex_command;

	/** Customized index_command
	*/
	std::string index_command;

	/** Hyperref driver
	*/
	std::string hyperref_driver;

	/** Line length to use with plaintext or LaTeX export.
	*/
	size_type linelen = 0;

	/** The depth of the current paragraph, set for plaintext
	 *  export and used by InsetTabular
	 */
	int depth = 0;

	/** Export data filled in by the latex(), docbook(), etc. methods.
	    This is a hack: Make it possible to add stuff to constant
	    OutputParams instances.
	*/
	std::shared_ptr<ExportData> exportdata;

	/** Store labels, index entries, etc. (in \ref post_macro)
	 *  and output them later. This is used in particular to get
	 *  labels and index entries (and potentially other fragile commands)
	 *  outside of moving arguments (bug 2154)
	 */
	bool postpone_fragile_stuff = false;

	/** Stuff to be postponed and output after the current macro
	 *  (if \ref postpone_fragile_stuff is true). Used for labels and index
	 *  entries in commands with moving arguments (\\section, \\caption etc.)
	 */
	mutable docstring post_macro;

	/** Whether we are entering a display math inset.
	 *  Needed to correctly strike out deleted math in change tracking.
	 */
	mutable bool inDisplayMath = false;

	/** Whether we are leaving a display math inset.
	 *  Needed to correctly track nested ulem commands in change tracking.
	 */
	mutable bool wasDisplayMath = false;

	/** Whether we are inside a comment inset. Insets that are including
	 *  external files like InsetGraphics, InsetInclude and InsetExternal
	 *  may only write the usual output and must not attempt to do
	 *  something with the included files (e.g. copying, converting)
	 *  if this flag is true, since they may not exist.
	 */
	bool inComment = false;

	/** Whether we are inside an include inset. 
	 */
	bool inInclude = false;

	/** Whether a btUnit (for multiple biblographies) is open.
	 */
	mutable bool openbtUnit = false;

	/** Process only the children's aux files with BibTeX.
	 *  This is necessary with chapterbib.
	 */
	bool only_childbibs = false;

	/** Whether we are in a table cell.
	 *  For newline, it matters whether its content is aligned or not.
         */
	TableCell inTableCell = NO;

	/** Whether we are inside a float or subfloat.
	 *  Needed for subfloat detection on the command line.
	 */
	Float inFloat = NONFLOAT;

	/** Whether we are inside an index inset.
	 *  ERT needs to know this, due to the active chars.
	 */
	bool inIndexEntry = false;

	/** Whether we are inside an IPA inset.
	 *  Needed for proper IPA output.
	 */
	bool inIPA = false;

	/** Whether we are inside an inset that is logically deleted.
	 *  A value > 0 indicates a deleted inset.
	*/
	int inDeletedInset = 0;

	/** The change information of the outermost logically deleted inset.
	 *  changeOfDeletedInset shall only be evaluated if inDeletedInset > 0.
	*/
	Change changeOfDeletedInset {Change::UNCHANGED};

	/** What kind of change tracking object is this?
	 * Relevant for strikeout method in output
	 */
	mutable CtObject ctObject = CtObject::Normal;

	/** allow output of only part of the top-level paragraphs
	 *  par_begin: beginning paragraph
	 */
	mutable pit_type par_begin = 0;

	/** allow output of only part of the top-level paragraphs
	 *  par_end: par_end-1 is the ending paragraph
	 *  if par_begin=par_end, output all paragraphs
	 */
	mutable pit_type par_end = 0;

	/// Id of the last paragraph before an inset
	mutable int lastid = -1;

	/// Last position in the last paragraph before an inset
	mutable pos_type lastpos = 0;

	/// is this the last paragraph in the current buffer/inset?
	bool isLastPar = false;


	/** whether or not to do actual file copying and image conversion
	 *  This mode will be used to preview the source code
	 */
	bool dryrun = false;

	/// whether to display output errors or not
	bool silent = false;

	/// Should we output verbatim or escape LaTeX's special chars?
	bool pass_thru = false;

	/// Should we output verbatim specific chars?
	docstring pass_thru_chars;

	/// A specific newline macro
	std::string newlinecmd;

	/// Should we output captions?
	bool html_disable_captions = false;

	/// Are we already in a paragraph?
	bool html_in_par = false;

	/// Does the present context even permit paragraphs?
	bool html_make_pars = true;

	/// Are we already in a paragraph?
	bool docbook_in_par = false;

	/// Does the present context even permit paragraphs?
	bool docbook_make_pars = true;

	/// Are paragraphs mandatory in this context?
	bool docbook_force_pars = false;

	/// Anchors that should not be output (LyX-side identifier, not DocBook-side).
	std::set<docstring> docbook_anchors_to_ignore;

	/// Is the current context a float (such as a table or a figure)?
	bool docbook_in_float = false;

	/// Is the current context a listing?
	bool docbook_in_listing = false;

	/// Is the current context a table?
	bool docbook_in_table = false;

	/// Should the layouts that should/must go into <info> be generated?
	bool docbook_generate_info = true;

	/// Should wrappers be ignored? Mostly useful to avoid generation of <abstract>.
	bool docbook_ignore_wrapper = false;

	/// Some parameters are output before the rest of the paragraph, they should not be generated a second time.
	std::set<InsetArgument const *> docbook_prepended_arguments = {};

	/// Are we generating this material for inclusion in a TOC-like entity?
	bool for_toc = false;

	/// Are we generating this material for inclusion in a tooltip?
	bool for_tooltip = false;

	/// Are we generating this material for use by advanced search?
	bool for_search = false;

	/// Are we generating this material for instant preview?
	bool for_preview = false;

	/// Include all children notwithstanding the use of \includeonly
	bool includeall = false;

	/// Explicit output folder, if any is desired
	std::string export_folder;
};


} // namespace lyx

#endif // NOT OUTPUTPARAMS_H
