#include <config.h>

#include "../support/Messages.h"

using namespace std;

namespace lyx {
	// Dummy LyXRC support
	class LyXRC { string icon_set; } lyxrc;

	// Dummy LyXAlignment support
	enum LyXAlignment {
		DUMMY
	};

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

	string alignmentToCSS(LyXAlignment)
	{
		return string();
	}

}
