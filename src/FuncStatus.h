// -*- C++ -*-
#ifndef FUNC_STATUS_H
#define FUNC_STATUS_H

/// The status of a function.

class FuncStatus
{
private:

	enum StatusCodes {
		///
		OK = 0,
		///
		UNKNOWN = 1,
		///
		DISABLED = 2,  // Command cannot be executed
		///
		ON = 4,
		///
		OFF = 8
	};

	unsigned int v_;

public:
	///
	FuncStatus();
	//
	FuncStatus & clear ();
	///
	void operator |= (FuncStatus const & f);
	///
	FuncStatus & unknown(bool b);
	///
	bool unknown() const;
	
	///
	FuncStatus & disabled (bool b);
	///
	bool disabled () const;

	///
	void setOnOff (bool b);
	///
	bool onoff (bool b) const;
};

#endif
