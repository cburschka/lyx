// -*- C++ -*-

#ifndef MATH_MATRIZ_H
#define MATH_MATRIZ_H

///
class Matriz {
public:
	///
	typedef float matriz_data[2][2];
	///
	Matriz();
	///
	void rota(int);
	///
	void escala(float, float);
	///
	void transf(float, float, float &, float &);
private:
	///
	matriz_data m_;
	///
	void matmat(matriz_data & a);
	///
	static matriz_data const MATIDEN;
};

#endif
