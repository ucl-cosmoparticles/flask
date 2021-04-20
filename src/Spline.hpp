#ifndef SPLINE_H    // include guard.
#define SPLINE_H 1

class Spline {
private:
  int N1;
  double *x1, *f1d, *d2fdx2;
  void clear();
  void dealloc();
public:
  Spline();
  void init(double *x, double *y, int n);
  double operator()(double x) const;
  ~Spline();
};


// Standalone functions that serve as bakcbone (from Numerical Recipes)
void spline(double x[], double y[], int n, double yp1, double ypn, double y2[]);
void splint(double xa[], double ya[], double y2a[], int n, double x, double *y);


#endif
