#include <config.h>

#include "../Messages.h"

using namespace std;

namespace lyx {
	// Dummy LyXRC support
	class LyXRC { string icon_set; } lyxrc;

	// Keep the linker happy on Windows
	void lyx_exit(int) {}

	docstring const _(string const & s) { return from_ascii(s); }

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
}
