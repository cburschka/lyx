// -*- C++ -*-
#ifndef LYXACTION_H
#define LYXACTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "commandtags.h"
#include "LString.h"
#include <boost/utility.hpp>

/** This class encapsulates LyX action and user command operations.
 */
class LyXAction : boost::noncopyable {
private:
	///
	struct func_info {
		///
		string name;
		///
		unsigned int attrib;
		///
		string helpText;
	};

	///
	struct pseudo_func {
		///
		kb_action action;
		///
		string arg;
	};
public:
	///
	typedef std::map<string, kb_action> func_map;
	///
	typedef std::map<kb_action, func_info> info_map;
	///
	typedef std::map<unsigned int, pseudo_func> pseudo_map;
	///
	typedef std::map<string, unsigned int> arg_item;
	///
	typedef std::map<kb_action, arg_item> arg_map;

	///
	enum func_attrib {
		/// nothing special about this func
		Noop = 0,
		/// can be used in RO mode (perhaps this should change)
		ReadOnly = 1, // ,
		/// Can be used when there is no document open
		NoBuffer = 2,
		//Interactive = 2, // Is interactive (requires a GUI)
		///
		Argument = 4      // Requires argument
		//MathOnly = 8,    // Only math mode
		//EtcEtc = ...     // Or other attributes...
	};

	///
	LyXAction();

	/** Returns an pseudoaction from a string
	  If you include arguments in func_name, a new psedoaction will be
	  created if needed. */
	int LookupFunc(string const & func_name) const;

	/** Returns an action tag which name is the most similar to a string.
	    Don't include arguments, they would be ignored. */
	int getApproxFunc(string const & func) const;

	/** Returns an action name the most similar to a string.
	    Don't include arguments, they would be ignored. */
	string const getApproxFuncName(string const & func) const;

	/// Returns a pseudo-action given an action and its argument.
	int getPseudoAction(kb_action action, string const & arg) const;

	/// Retrieves the real action and its argument.
	kb_action retrieveActionArg(int i, string & arg) const;

	/// Search for an existent pseudoaction, return -1 if it doesn't exist.
	int searchActionArg(kb_action action, string const & arg) const;

	/// Return the name associated with command
	string const getActionName(int action) const;

	/// Return one line help text associated with (pseudo)action
	string const helpText(int action) const;

	/// True if the command has `flag' set
	bool funcHasFlag(kb_action action, func_attrib flag) const;

	typedef func_map::const_iterator const_func_iterator;
	const_func_iterator func_begin() const;
	const_func_iterator func_end() const;
private:
	///
	void init();
	///
	void newFunc(kb_action, string const & name,
		     string const & helpText, unsigned int attrib);

	/** This is a list of all the LyXFunc names with the
	  coresponding action number. It is usually only used by the
	  minibuffer or when assigning commands to keys during init. */
	func_map lyx_func_map;

	/** This is a mapping from action number to an object holding
	  info about this action. f.ex. helptext, command name (string),
	  command attributes (ro) */
	info_map lyx_info_map;

	/** A mapping from the automatically created pseudo action number
	  to the real action and its argument. */
	mutable pseudo_map lyx_pseudo_map;

	/** A (multi) mapping from the lyx action to all the generated
	  pseudofuncs and the arguments the action should use. */
	mutable arg_map lyx_arg_map;
};

#endif
