// -*- C++ -*-
#ifndef FUNC_STATUS_H
#define FUNC_STATUS_H

/// The status of a function.
namespace func_status {

enum value_type {
		/// No problem
		OK = 0,
		///
		Unknown = 1,
		/// Command cannot be executed
		Disabled = 2,
		///
		ToggleOn = 4,
		///
		ToggleOff = 8
	};
}

///
inline
void operator|=(func_status::value_type & fs, func_status::value_type f)
{
	fs = static_cast<func_status::value_type>(fs | f);
}

#endif
