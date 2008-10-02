// -*- C++ -*-
/**
 * \file Mover.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MOVER_H
#define MOVER_H

#include <map>
#include <string>


namespace lyx {

namespace support { class FileName; }

/**
 *  Utility to copy a file of a specified format from one place to another.
 *  This base class simply invokes the command support::copy().
 */
class Mover
{
public:
	virtual ~Mover() {}

	/** Copy file @c from to @c to.
	 *  This version should be used to copy files from the original
	 *  location to the temporary directory, since @c to and @c latex
	 *  would be equal in this case.
	 *  \returns true if successful.
	 */
	bool
	copy(support::FileName const & from, support::FileName const & to) const;

	/** Rename file @c from as @c to.
	 *  This version should be used to move files from the original
	 *  location to the temporary directory, since @c to and @c latex
	 *  would be equal in this case.
	 *  \returns true if successful.
	 */
	bool
	rename(support::FileName const & from, support::FileName const & to) const;

	/** Rename file @c from as @c to.
	 *  \see SpecialisedMover::SpecialisedMover() for an explanation of
	 *  @c latex.
	 *  This version should be used to move files from the temporary
	 *  directory to the export location, since @c to and @c latex may
	 *  not be equal in this case.
	 *  \returns true if successful.
	 */
	bool
	rename(support::FileName const & from, support::FileName const & to,
	       std::string const & latex) const
	{
		return do_rename(from, to, latex);
	}

protected:
	virtual bool
	do_rename(support::FileName const & from, support::FileName const & to,
		  std::string const &) const;
};


/**
 *  Specialisation of the Mover concept that uses an external command
 *  to copy a file.
 *
 *  For example, an Xfig .fig file can contain references to external
 *  picture files. If such a reference has a relative path, then the
 *  copied .fig file will require a transformation of the picture file
 *  reference if it is to be found by Xfig.
 */
class SpecialisedMover : public Mover
{
public:
	SpecialisedMover() {}

	virtual ~SpecialisedMover() {}

	/** @c command should be of the form
	 *  <code>
	 *      python $$s/scripts/fig_copy.py $$i $$o $$l
	 *  </code>
	 *  where $$s is a placeholder for the lyx support directory,
	 *        $$i is a placeholder for the name of the file to be moved,
	 *        $$o is a placeholder for the name of the file after moving,
	 *        $$l is a placeholder for the name of the file after moving,
	 *        suitable as argument to a latex include command. This is
	 *        either an absolute filename or relative to the master
	 *        document.
	 *        $$o and $$l can only differ if the file is copied from the
	 *        temporary directory to the export location. If it is copied
	 *        from the original location to the temporary directory, they
	 *        are the same, so $$l may be ommitted in this case.
	 */
	SpecialisedMover(std::string const & command)
		: command_(command) {}

	/// The template used to launch the external command.
	std::string const & command() const { return command_; }

private:
	virtual bool
	do_copy(support::FileName const & from, support::FileName const & to,
		std::string const & latex) const;

	virtual bool
	do_rename(support::FileName const & from, support::FileName const & to,
		  std::string const & latex) const;

	std::string command_;
};


/**
 *  Manage the store of (Mover)s.
 */
class Movers
{
public:
	/** Register a specialised @c command to be used to copy a file
	 *  of format @c fmt.
	 */
	void set(std::string const & fmt, std::string const & command);

	/// @c returns the Mover registered for format @c fmt.
	Mover const & operator()(std::string  const & fmt) const;

	/** @returns the command template if @c fmt 'finds' a
	 *  SpecialisedMover. Otherwise, returns an empty string.
	 */
	std::string const command(std::string  const & fmt) const;

private:
	typedef std::map<std::string, SpecialisedMover> SpecialsMap;

public:
	typedef SpecialsMap::const_iterator const_iterator;
	const_iterator begin() const { return specials_.begin(); }
	const_iterator end() const { return specials_.end(); }

private:
	Mover default_;
	SpecialsMap specials_;
};


extern Movers & theMovers();
/// @c returns the Mover registered for format @c fmt.
extern Mover const & getMover(std::string  const & fmt);
/** Register a specialised @c command to be used to copy a file
 *  of format @c fmt.
 */
extern void setMover(std::string const & fmt, std::string const & command);
extern Movers & theSystemMovers();


} // namespace lyx

#endif // MOVER_H
