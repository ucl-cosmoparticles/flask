#include "Spline.hpp"
#include "Utilities.hpp"
#include <cstddef> // For NULL pointer.

// Originally the Numerical recipes functions were written for vectors x[1..n] (i.e. first element at i=1).
// The constant below controls the offset of the vector consistently across the functions in this file.
// Now vectors are x[offset..n+offset-1] 
// The comments before each Num-Rec function were not updated.
const int splOffset = 0;
const double deriv0 = 1e40; // If this is >1e30, the second derivatives at edges are set to zero.



/*************************************/
/***         Spline class          ***/
/*************************************/


/*** Empty constructor ***/
Spline::Spline() {
  clear();
}


/*** Initialize 1D spline ***/
void Spline::init(double *x, double *y, int n) {
  int i;
  
  // Clear previous settings if existent:
  if (N1!=0) dealloc();
  // Set variables and allocate memory:
  N1      = n;
  x1      = vector<double>(splOffset, N1+splOffset-1);
  f1d     = vector<double>(splOffset, N1+splOffset-1);
  d2fdx2  = vector<double>(splOffset, N1+splOffset-1);
  // Copy function sampled point to internal arrays:
  for (i=splOffset; i<N1+splOffset; i++) {
    x1 [i] = x[i];
    f1d[i] = y[i];
  }
  // Compute second derivatives that are needed for Spline:
  spline(x1, f1d, N1, deriv0, deriv0, d2fdx2);
}


/*** Returns the 1D cubic spline interpolated value ***/
double Spline::operator()(double xpt) const {
  double result;
  splint(x1, f1d, d2fdx2, N1, xpt, &result);
  return result;
}


/*** Destructor, deallocate internal memory ***/
Spline::~Spline() {
  dealloc();
}


/*** Deallocate internal memory ***/
void Spline::dealloc() {
  if (N1!=0) {
    free_vector(x1,      splOffset, N1+splOffset-1);
    free_vector(f1d,     splOffset, N1+splOffset-1);
    free_vector(d2fdx2,  splOffset, N1+splOffset-1); 
  }
}


/*** Clear internal variables ***/
void Spline::clear() {
  N1      = 0;
  x1      = NULL;
  f1d     = NULL;
  d2fdx2  = NULL;
}


/*************************************/
/*** Standalone backbone functions ***/
/*************************************/

#define NRANSI

/*
Given arrays x[1..n] and y[1..n] containing a tabulated function, i.e., y i = f ( x i ), with
x 1 < x 2 < . . . < x N , and given values yp1 and ypn for the first derivative of the interpolating
function at points 1 and n , respectively, this routine returns an array y2[1..n] that contains
the second derivatives of the interpolating function at the tabulated points x i . If yp1 and/or
ypn are equal to 1 × 10 30 or larger, the routine is signaled to set the corresponding boundary
condition for a natural spline, with zero second derivative on that boundary.
*/

void spline(double x[], double y[], int n, double yp1, double ypn, double y2[]) {
  int i,k;
  double p,qn,sig,un,*u;

  u=vector<double>(splOffset,n+splOffset-2);
  if (yp1 > 0.99e30)
    y2[splOffset]=u[splOffset]=0.0;
  else {
    y2[splOffset] = -0.5;
    u[splOffset]=(3.0/(x[splOffset+1]-x[splOffset]))*((y[splOffset+1]-y[splOffset])/(x[splOffset+1]-x[splOffset])-yp1);
  }
  for (i=splOffset+1;i<n+splOffset-1;i++) {
    sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
    p=sig*y2[i-1]+2.0;
    y2[i]=(sig-1.0)/p;
    u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
    u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
  }
  if (ypn > 0.99e30)
    qn=un=0.0;
  else {
    qn=0.5;
    un=(3.0/(x[n+splOffset-1]-x[n+splOffset-2]))*(ypn-(y[n+splOffset-1]-y[n+splOffset-2])/(x[n+splOffset-1]-x[n+splOffset-2]));
  }
  y2[n+splOffset-1]=(un-qn*u[n+splOffset-2])/(qn*y2[n+splOffset-2]+1.0);
  for (k=n+splOffset-2;k>=splOffset;k--)
    y2[k]=y2[k]*y2[k+1]+u[k];
  free_vector(u,splOffset,n+splOffset-2);
}
#undef NRANSI
/* (C) Copr. 1986-92 Numerical Recipes Software #?w,(1. */


/*
Given the arrays xa[1..n] and ya[1..n] , which tabulate a function (with the xa i ’s in order),
and given the array y2a[1..n] , which is the output from spline above, and given a value of
x , this routine returns a cubic-spline interpolated value y .
*/
void splint(double xa[], double ya[], double y2a[], int n, double x, double *y) {
  int klo,khi,k;
  double h,b,a;

  klo=splOffset;
  khi=n+splOffset-1;
  while (khi-klo > 1) {
    k=(khi+klo) >> 1;
    if (xa[k] > x) khi=k;
    else klo=k;
  }
  h=xa[khi]-xa[klo];
  if (h == 0.0) error("splint: Bad xa input to routine splint");
  a=(xa[khi]-x)/h;
  b=(x-xa[klo])/h;
  *y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
}
/* (C) Copr. 1986-92 Numerical Recipes Software #?w,(1. */
