// -*- C++ -*-
/**
 * \file InsetGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H

#include <set>
#include "Inset.h"
#include "InsetGraphicsParams.h"

namespace lyx {

class RenderGraphic;
class LaTeXFeatures;

/////////////////////////////////////////////////////////////////////////
//
// InsetGraphics
//
/////////////////////////////////////////////////////////////////////////

/// Used for images etc.
class InsetGraphics : public Inset
{
public:
	///
	InsetGraphics(Buffer & buf);
	///
	~InsetGraphics();

	///
	static void string2params(std::string const & data,
				  Buffer const & buffer,
				  InsetGraphicsParams &);
	///
	static std::string params2string(InsetGraphicsParams const &,
					  Buffer const &);

	/** Set the inset parameters, used by the GUIndependent dialog.
	    Return true of new params are different from what was so far.
	*/
	bool setParams(InsetGraphicsParams const & params);

	InsetGraphicsParams getParams() const { return params_;}

private:
	///
	InsetGraphics(InsetGraphics const &);

	///
	bool isLabeled() const { return true; }
	void metrics(MetricsInfo &, Dimension &) const;
	///
	EDITABLE editable() const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void validate(LaTeXFeatures & features) const;
	/// returns LyX code associated with the inset. Used for TOC, ...)
	InsetCode lyxCode() const { return GRAPHICS_CODE; }
	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams const & params() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	void editGraphics(InsetGraphicsParams const &, Buffer const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void addToToc(DocIterator const &);
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	/// Force inset into LTR environment if surroundings are RTL?
	bool forceLTR() const { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const;
	/// Get the status message, depends on the image loading status.
	std::string statusMessage() const;
	/// Create the options for the latex command.
	std::string createLatexOptions() const;
	/// Create length values for docbook export.
	docstring toDocbookLength(Length const & len) const;
	/// Create the atributes for docbook export.
	docstring createDocBookAttributes() const;
	/// Convert the file if needed, and return the location of the file.
	std::string prepareFile(OutputParams const &) const;

	///
	InsetGraphicsParams params_;
	/// holds the entity name that defines the graphics location (SGML).
	docstring const graphic_label;
	/// The thing that actually draws the image on LyX's screen.
	RenderGraphic * graphic_;
};

namespace graphics {

	/// Saves the list of currently used groups in the document.
	void getGraphicsGroups(Buffer const &, std::set<std::string> &);

	/// Returns parameters of a given graphics group (except filename).
	std::string getGroupParams(Buffer const &, std::string const &);

	/** Synchronize all Graphics insets of the group.
	    Both groupId and params are taken from argument.
	*/
	void unifyGraphicsGroups(Buffer &, std::string const &);
	InsetGraphics * getCurrentGraphicsInset(Cursor const &);

}

} // namespace lyx

#endif // INSET_GRAPHICS_H
