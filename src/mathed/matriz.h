// -*- C++ -*-

#ifndef MATH_MATRIZ_H
#define MATH_MATRIZ_H

typedef float matriz_data[2][2];

class Matriz {
 public: 
   Matriz();
   void rota(int);
   void escala(float, float);
   void transf(float, float, float &, float &);
 protected:
   matriz_data m;
   void matmat(matriz_data & a);
};

#endif
