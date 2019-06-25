// -*- C++ -*-
/**
 * \file MathStream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MATHMLSTREAM_H
#define MATH_MATHMLSTREAM_H

#include "InsetMath.h"

#include "TexRow.h"
#include "texstream.h"

#include "support/Changer.h"
#include "support/strfwd.h"
#include "support/unique_ptr.h"


namespace lyx {

class Encoding;
class InsetMath;
class MathAtom;
class MathData;

//
// LaTeX/LyX
//

class WriteStream {
public:
	///
	enum OutputType {
		wsDefault,
		wsDryrun,
		wsPreview
	};
	///
	enum UlemCmdType {
		NONE,
		UNDERLINE,
		STRIKEOUT
	};
	///
	explicit WriteStream(otexrowstream & os, bool fragile = false,
	                     bool latex = false, OutputType output = wsDefault,
	                     Encoding const * encoding = 0);
	///
	~WriteStream();
	///
	int line() const { return line_; }
	///
	bool fragile() const { return fragile_; }
	///
	bool latex() const { return latex_; }
	///
	OutputType output() const { return output_; }
	///
	otexrowstream & os() { return os_; }
	///
	TexRow & texrow() { return os_.texrow(); }
	///
	bool & firstitem() { return firstitem_; }
	///
	void addlines(unsigned int);
	/// record whether we can write an immediately following newline char
	void canBreakLine(bool breakline) { canbreakline_ = breakline; }
	/// tell whether we can write an immediately following newline char
	bool canBreakLine() const { return canbreakline_; }
	/// record whether we have to take care for striking out display math
	void strikeoutMath(bool mathsout) { mathsout_ = mathsout; }
	/// tell whether we have to take care for striking out display math
	bool strikeoutMath() const { return mathsout_; }
	/// record which ulem command type we are inside
	void ulemCmd(UlemCmdType ulemcmd) { ulemcmd_ = ulemcmd; }
	/// tell which ulem command type we are inside
	UlemCmdType ulemCmd() const { return ulemcmd_; }
	/// writes space if next thing is isalpha()
	void pendingSpace(bool how);
	/// writes space if next thing is isalpha()
	bool pendingSpace() const { return pendingspace_; }
	/// tell whether to write the closing brace of \ensuremath
	void pendingBrace(bool brace);
	/// tell whether to write the closing brace of \ensuremath
	bool pendingBrace() const { return pendingbrace_; }
	/// tell whether we are in text mode or not when producing latex code
	void textMode(bool textmode);
	/// tell whether we are in text mode or not when producing latex code
	bool textMode() const { return textmode_; }
	/// tell whether we are allowed to switch mode when producing latex code
	void lockedMode(bool locked);
	/// tell whether we are allowed to switch mode when producing latex code
	bool lockedMode() const { return locked_; }
	/// tell whether to use only ascii chars when producing latex code
	void asciiOnly(bool ascii);
	/// tell whether to use only ascii chars when producing latex code
	bool asciiOnly() const { return ascii_; }
	/// LaTeX encoding
	Encoding const * encoding() const { return encoding_; }

	/// Temporarily change the TexRow information about the outer row entry.
	Changer changeRowEntry(TexRow::RowEntry entry);
	/// TexRow::starts the innermost outer math inset
	/// returns true if the outer row entry will appear at this line
	bool startOuterRow();
private:
	///
	otexrowstream & os_;
	/// do we have to write \\protect sometimes
	bool fragile_;
	/// are we at the beginning of an MathData?
	bool firstitem_;
	/// are we writing to .tex?
	int latex_;
	/// output type (default, source preview, instant preview)?
	OutputType output_;
	/// do we have a space pending?
	bool pendingspace_;
	/// do we have a brace pending?
	bool pendingbrace_;
	/// are we in text mode when producing latex code?
	bool textmode_;
	/// are we allowed to switch mode when producing latex code?
	bool locked_;
	/// should we use only ascii chars when producing latex code?
	bool ascii_;
	/// are we allowed to output an immediately following newline?
	bool canbreakline_;
	/// should we take care for striking out display math?
	bool mathsout_;
	/// what ulem command are we inside (none, underline, strikeout)?
	UlemCmdType ulemcmd_;
	///
	int line_;
	///
	Encoding const * encoding_;
	/// Row entry we are in
	TexRow::RowEntry row_entry_;
};

///
WriteStream & operator<<(WriteStream &, MathAtom const &);
///
WriteStream & operator<<(WriteStream &, MathData const &);
///
WriteStream & operator<<(WriteStream &, docstring const &);
///
WriteStream & operator<<(WriteStream &, char const * const);
///
WriteStream & operator<<(WriteStream &, char);
///
WriteStream & operator<<(WriteStream &, int);
///
WriteStream & operator<<(WriteStream &, unsigned int);

/// ensure correct mode, possibly by opening \ensuremath or \lyxmathsym
bool ensureMath(WriteStream & os, bool needs_mathmode = true,
                bool macro = false, bool textmode_macro = false);

/// ensure the requested mode, possibly by closing \ensuremath or \lyxmathsym
int ensureMode(WriteStream & os, InsetMath::mode_type mode, bool locked, bool ascii);


/**
 * MathEnsurer - utility class for ensuring math mode
 *
 * A local variable of this type can be used to either ensure math mode
 * or delay the writing of a pending brace when outputting LaTeX.
 * A LyX InsetMathMacro is always assumed needing a math mode environment, while
 * no assumption is made for macros defined through \newcommand or \def.
 *
 * Example 1:
 *
 *      MathEnsurer ensurer(os);
 *
 * If not already in math mode, inserts an \ensuremath command followed
 * by an open brace. This brace will be automatically closed when exiting
 * math mode. Math mode is automatically exited when writing something
 * that doesn't explicitly require math mode.
 *
 * Example 2:
 *
 *      MathEnsurer ensurer(os, false);
 *
 * Simply suspend writing a closing brace until the end of ensurer's scope.
 *
 * Example 3:
 *
 *      MathEnsurer ensurer(os, needs_mathmode, true, textmode_macro);
 *
 * This form is mainly used for math macros as they are treated specially.
 * In essence, the macros defined in the lib/symbols file and tagged as
 * textmode will be enclosed in \lyxmathsym if they appear in a math mode
 * environment, while macros defined in the preamble or ERT are left as is.
 * The third parameter must be set to true and the fourth parameter has also
 * to be specified. Only the following 3 different cases are handled.
 *
 * When the needs_mathmode parameter is true the behavior is as in Example 1.
 * This is the case for a LyX InsetMathMacro or a macro not tagged as textmode.
 *
 * When the needs_mathmode and textmode_macro parameters are both false the
 * macro is left in the same (text or math mode) environment it was entered.
 * This is because it is assumed that the macro was designed for that mode
 * and we have no way to tell the contrary.
 * This is the case for macros defined by using \newcommand or \def in ERT.
 *
 * When the needs_mathmode parameter is false while textmode_macro is true the
 * macro will be enclosed in \lyxmathsym if it appears in a math mode environment.
 * This is the case for the macros tagged as textmode in lib/symbols.
 */
class MathEnsurer
{
public:
	///
	explicit MathEnsurer(WriteStream & os, bool needs_mathmode = true,
	                     bool macro = false, bool textmode_macro = false)
		: os_(os), brace_(ensureMath(os, needs_mathmode, macro, textmode_macro)) {}
	///
	~MathEnsurer() { os_.pendingBrace(brace_); }
private:
	///
	WriteStream & os_;
	///
	bool brace_;
};


/**
 * ModeSpecifier - utility class for specifying a given mode (math or text)
 *
 * A local variable of this type can be used to specify that a command or
 * environment works in a given mode. For example, \mbox works in text
 * mode, but \boxed works in math mode. Note that no mode changing commands
 * are needed, but we have to track the current mode, hence this class.
 * This is only used when exporting to latex and helps determining whether
 * the mode needs being temporarily switched when a command would not work
 * in the current mode. As there are cases where this switching is to be
 * avoided, the optional third parameter can be used to lock the mode.
 * When the mode is locked, the optional fourth parameter specifies whether
 * strings are to be output by using a suitable ascii representation.
 *
 * Example 1:
 *
 *      ModeSpecifier specifier(os, TEXT_MODE);
 *
 * Sets the current mode to text mode and allows mode switching.
 *
 * Example 2:
 *
 *      ModeSpecifier specifier(os, TEXT_MODE, true);
 *
 * Sets the current mode to text mode and disallows mode switching.
 *
 * Example 3:
 *
 *      ModeSpecifier specifier(os, TEXT_MODE, true, true);
 *
 * Sets the current mode to text mode, disallows mode switching, and outputs
 * strings as ascii only.
 *
 * At the end of specifier's scope the mode is reset to its previous value.
 */
class ModeSpecifier
{
public:
	///
	explicit ModeSpecifier(WriteStream & os, InsetMath::mode_type mode,
				bool locked = false, bool ascii = false)
		: os_(os), oldmodes_(ensureMode(os, mode, locked, ascii)) {}
	///
	~ModeSpecifier()
	{
		os_.textMode(oldmodes_ & 0x01);
		os_.lockedMode(oldmodes_ & 0x02);
		os_.asciiOnly(oldmodes_ & 0x04);
	}
private:
	///
	WriteStream & os_;
	///
	int oldmodes_;
};



//
//  MathML
//

class MTag {
public:
	///
	MTag(char const * const tag, std::string attr = "")
		: tag_(tag), attr_(attr) {}
	///
	char const * const tag_;
	///
	std::string attr_;
};

class ETag {
public:
	///
	ETag(char const * const tag) : tag_(tag) {}
	///
	char const * const tag_;
};


/// Throw MathExportException to signal that the attempt to export
/// some math in the current format did not succeed. E.g., we can't
/// export xymatrix as MathML, so that will throw, and we'll fall back
/// to images.
class MathExportException : public std::exception {};


class MathStream {
public:
	///
	explicit MathStream(odocstream & os);
	///
	void cr();
	///
	odocstream & os() { return os_; }
	///
	int line() const { return line_; }
	///
	int & tab() { return tab_; }
	///
	friend MathStream & operator<<(MathStream &, char const *);
	///
	void defer(docstring const &);
	///
	void defer(std::string const &);
	///
	docstring deferred() const;
	///
	bool inText() const { return in_text_; }
private:
	///
	void setTextMode(bool t) { in_text_ = t; }
	///
	odocstream & os_;
	///
	int tab_;
	///
	int line_;
	///
	bool in_text_;
	///
	odocstringstream deferred_;
	///
	friend class SetMode;
};

///
MathStream & operator<<(MathStream &, MathAtom const &);
///
MathStream & operator<<(MathStream &, MathData const &);
///
MathStream & operator<<(MathStream &, docstring const &);
///
MathStream & operator<<(MathStream &, char const *);
///
MathStream & operator<<(MathStream &, char);
///
MathStream & operator<<(MathStream &, char_type);
///
MathStream & operator<<(MathStream &, MTag const &);
///
MathStream & operator<<(MathStream &, ETag const &);


/// A simpler version of ModeSpecifier, for MathML
class SetMode {
public:
	///
	explicit SetMode(MathStream & os, bool text);
	///
	~SetMode();
private:
	///
	MathStream & os_;
	///
	bool was_text_;
};


class HtmlStream {
public:
	///
	explicit HtmlStream(odocstream & os);
	///
	void cr();
	///
	odocstream & os() { return os_; }
	///
	int line() const { return line_; }
	///
	int & tab() { return tab_; }
	///
	friend HtmlStream & operator<<(HtmlStream &, char const *);
	///
	void defer(docstring const &);
	///
	void defer(std::string const &);
	///
	docstring deferred() const;
	///
	bool inText() const { return in_text_; }
private:
	///
	void setTextMode(bool t) { in_text_ = t; }
	///
	odocstream & os_;
	///
	int tab_;
	///
	int line_;
	///
	bool in_text_;
	///
	odocstringstream deferred_;
	///
	friend class SetHTMLMode;
};

///
HtmlStream & operator<<(HtmlStream &, MathAtom const &);
///
HtmlStream & operator<<(HtmlStream &, MathData const &);
///
HtmlStream & operator<<(HtmlStream &, docstring const &);
///
HtmlStream & operator<<(HtmlStream &, char const *);
///
HtmlStream & operator<<(HtmlStream &, char);
///
HtmlStream & operator<<(HtmlStream &, char_type);
///
HtmlStream & operator<<(HtmlStream &, MTag const &);
///
HtmlStream & operator<<(HtmlStream &, ETag const &);


class SetHTMLMode {
public:
	///
	explicit SetHTMLMode(HtmlStream & os, bool text);
	///
	~SetHTMLMode();
private:
	///
	HtmlStream & os_;
	///
	bool was_text_;
};


//
// Debugging
//

class NormalStream {
public:
	///
	explicit NormalStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};

///
NormalStream & operator<<(NormalStream &, MathAtom const &);
///
NormalStream & operator<<(NormalStream &, MathData const &);
///
NormalStream & operator<<(NormalStream &, docstring const &);
///
NormalStream & operator<<(NormalStream &, char const *);
///
NormalStream & operator<<(NormalStream &, char);
///
NormalStream & operator<<(NormalStream &, int);


//
// Maple
//


class MapleStream {
public:
	///
	explicit MapleStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MapleStream & operator<<(MapleStream &, MathAtom const &);
///
MapleStream & operator<<(MapleStream &, MathData const &);
///
MapleStream & operator<<(MapleStream &, docstring const &);
///
MapleStream & operator<<(MapleStream &, char_type);
///
MapleStream & operator<<(MapleStream &, char const *);
///
MapleStream & operator<<(MapleStream &, char);
///
MapleStream & operator<<(MapleStream &, int);


//
// Maxima
//


class MaximaStream {
public:
	///
	explicit MaximaStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MaximaStream & operator<<(MaximaStream &, MathAtom const &);
///
MaximaStream & operator<<(MaximaStream &, MathData const &);
///
MaximaStream & operator<<(MaximaStream &, docstring const &);
///
MaximaStream & operator<<(MaximaStream &, char_type);
///
MaximaStream & operator<<(MaximaStream &, char const *);
///
MaximaStream & operator<<(MaximaStream &, char);
///
MaximaStream & operator<<(MaximaStream &, int);


//
// Mathematica
//


class MathematicaStream {
public:
	///
	explicit MathematicaStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MathematicaStream & operator<<(MathematicaStream &, MathAtom const &);
///
MathematicaStream & operator<<(MathematicaStream &, MathData const &);
///
MathematicaStream & operator<<(MathematicaStream &, docstring const &);
///
MathematicaStream & operator<<(MathematicaStream &, char const *);
///
MathematicaStream & operator<<(MathematicaStream &, char);
///
MathematicaStream & operator<<(MathematicaStream &, int);


//
// Octave
//


class OctaveStream {
public:
	///
	explicit OctaveStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};

///
OctaveStream & operator<<(OctaveStream &, MathAtom const &);
///
OctaveStream & operator<<(OctaveStream &, MathData const &);
///
OctaveStream & operator<<(OctaveStream &, docstring const &);
///
OctaveStream & operator<<(OctaveStream &, char_type);
///
OctaveStream & operator<<(OctaveStream &, char const *);
///
OctaveStream & operator<<(OctaveStream &, char);
///
OctaveStream & operator<<(OctaveStream &, int);


docstring convertDelimToXMLEscape(docstring const & name);

} // namespace lyx

#endif
