// -*- C++ -*-
/**
 * \file InsetGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H

#include "Inset.h"
#include "InsetGraphicsParams.h"

#include <set>

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
	explicit InsetGraphics(Buffer * buf);
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
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	bool canPaintChange(BufferView const &) const override { return true; }
	///
	InsetGraphics * asInsetGraphics() override { return this; }
	/// 
	InsetGraphics const * asInsetGraphics() const override { return this; }

	///
	bool isLabeled() const override { return true; }
	///
	bool hasSettings() const override { return true; }
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	/** returns the number of rows (\n's) of generated tex code.
	 #fragile == true# means, that the inset should take care about
	 fragile commands by adding a #\protect# before.
	 */
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream & os, OutputParams const &) const override;
	/** Tell LyX what the latex features you need i.e. what latex packages
	    you need to be included.
	 */
	void validate(LaTeXFeatures & features) const override;
	/// returns LyX code associated with the inset. Used for TOC, ...)
	InsetCode lyxCode() const override { return GRAPHICS_CODE; }
	///
	docstring layoutName() const override { return from_ascii("Graphics"); }
	/// Get the inset parameters, used by the GUIndependent dialog.
	InsetGraphicsParams const & params() const;

	///
	int topOffset(BufferView const *) const override { return 0; }
	///
	int bottomOffset(BufferView const *) const override { return 0; }
	///
	int leftOffset(BufferView const *) const override { return 0; }
	///
	int rightOffset(BufferView const *) const override { return 0; }

	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	bool showInsetDialog(BufferView * bv) const override;
	///
	void editGraphics(InsetGraphicsParams const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	std::string contextMenuName() const override;
	/// Force inset into LTR environment if surroundings are RTL
	bool forceLTR(OutputParams const &) const override { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override;
	/// Get the status message, depends on the image loading status.
	std::string statusMessage() const;
	/// Get the output bounding box accounting for raster formats.
	void outBoundingBox(graphics::BoundingBox &) const;
	/// Create the options for the latex command.
	std::string createLatexOptions(bool const ps) const;
	/// Create length values for docbook export.
	docstring toDocbookLength(Length const & len) const;
	/// Create the attributes for docbook export.
	docstring createDocBookAttributes() const;
	/// Convert the file if needed, and return the location of the file.
	/// This version is for use with LaTeX-style output.
	std::string prepareFile(OutputParams const &) const;
	/// Convert the file if needed, and return the location of the file.
	/// This version is for use with HTML-style output.
	/// \return the new filename, relative to the location of the HTML file,
	/// or an empty string on error.
	std::string prepareHTMLFile(OutputParams const & runparams) const;
	///
	OutputParams::CtObject CtObject(OutputParams const &) const override { return OutputParams::CT_OBJECT; }

private:
	///
	InsetGraphics(InsetGraphics const &);

	///
	InsetGraphicsParams params_;
	/// holds the entity name that defines the graphics location (SGML).
	docstring const graphic_label;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	/// The thing that actually draws the image on LyX's screen.
	RenderGraphic * graphic_;
};

namespace graphics {

	/// Saves the list of currently used groups in the document.
	void getGraphicsGroups(Buffer const &, std::set<std::string> &);

	/// how many members has the current group?
	int countGroupMembers(Buffer const &, std::string const &);

	/// Returns parameters of a given graphics group (except filename).
	std::string getGroupParams(Buffer const &, std::string const &);

	/** Synchronize all Graphics insets of the group.
	    Both groupId and params are taken from argument.
	*/
	void unifyGraphicsGroups(Buffer &, std::string const &);
	InsetGraphics * getCurrentGraphicsInset(Cursor const &);

} // namespace graphics

} // namespace lyx

#endif // INSET_GRAPHICS_H
