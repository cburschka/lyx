#ifndef FUNCREQUEST_H
#define FUNCREQUEST_H

#include "commandtags.h"
#include "LString.h"

/** This class encapsulates LyX action and arguemnt
 *  in order to passs it around easily.
 */
struct FuncRequest {
	/// 
	FuncRequest()
		: action(LFUN_UNKNOWN_ACTION)
	{}
	/// 
	FuncRequest(kb_action act)
		: action(act)
	{}
	/// 
	FuncRequest(kb_action act, string const & arg)
		: action(act), argument(arg)
	{}

	///
	kb_action action;
	///
	string argument;
};

#endif
