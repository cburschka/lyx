#ifndef BUTTON_INSET_H
#define BUTTON_INSET_H

#include "math_nestinset.h"

// Try to implement the reference inset "natively" for mathed.

class ButtonInset: public MathNestInset {
public:
	///
	ButtonInset();
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;

protected:
	/// This should provide the text for the button
	virtual string screenLabel() const = 0;
};

#endif
