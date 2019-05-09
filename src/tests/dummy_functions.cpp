#include <config.h>

#include "Format.h"
#include "LayoutEnums.h"
#include "LyXRC.h"

#include "support/Messages.h"
#include "xml.h"

using namespace std;

namespace lyx {

// Dummy verbose support
bool verbose = false;

// Dummy LyXRC support
LyXRC lyxrc;

// Keep the linker happy on Windows
void lyx_exit(int) {}

// Dummy language support
Messages const & getGuiMessages()
{
	static Messages lyx_messages;

	return lyx_messages;
}


Messages const & getMessages(string const &)
{
	static Messages lyx_messages;

	return lyx_messages;
}


// Dummy formats support (needed by Lexer)
Formats & theFormats()
{
	static Formats dummy_formats;
	return dummy_formats;
}


string alignmentToCSS(LyXAlignment)
{
	return string();
}

namespace xml {
docstring StartTag::writeTag() const { return docstring(); }
docstring StartTag::writeEndTag() const { return docstring(); }
bool StartTag::operator==(FontTag const & rhs) const { return rhs == *this; }
}

} // namespace lyx
