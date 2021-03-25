#ifndef CORRLNFIELDS_AUX     // include guard.
#define CORRLNFIELDS_AUX 1

#include "definitions.hpp"
#include "gsl_aux.hpp"          // Using and reading GSL matrices.
#include "Utilities.hpp"
#include <math.h>
#include <random>
#include <complex>
#include "Cosmology.hpp" // for ran_redshift function.
#include <vec3.h>        // For xyz2ang function.
#include <pointing.h>    // For xyz2ang function.
#include <healpix_map.h> // For RandAngInPix function.
#include <alm.h>         // For Kappa2ShearEmode function.
#include <xcomplex.h>    // For Kappa2ShearEmode function.
#include <omp.h>
#include "FieldsDatabase.hpp"

// Auxiliary functions for flask program:
void OrganizeAngularCoord(int *AngularCoord, int *phi_pos, int *theta_pos, int *ra_pos, int *dec_pos, std::string & Header);
int CountLensingFields(const FZdatabase & fieldlist);
int CountGalaxyFields(const FZdatabase & fieldlist);
void TabulateKappaWeight(double **KappaWeightTable, const Cosmology & cosmo, const FZdatabase & fieldlist);
void ChangeCoord(CAT_PRECISION **catalog, int theta_pos, int phi_pos, long Ngalaxies, int coordtype);
bool ComputeShearQ(const ParameterList & config);
double MapMean(const Healpix_Map<MAP_PRECISION> & map);
double MapVariance(const Healpix_Map<MAP_PRECISION> & map, double mean);
double MapSkewness(const Healpix_Map<MAP_PRECISION> & map, double mean, double variance);
using namespace definitions;
void PrintMapsStats(Healpix_Map<MAP_PRECISION> *mapf, const FZdatabase & fieldlist, simtype dist, std::ostream *output = &std::cout);
void RecoverCls(Alm<xcomplex <ALM_PRECISION> > *bflm, const FZdatabase & fieldlist, std::string clsKey, const ParameterList & config);
void RecoverAlmCls(Healpix_Map<MAP_PRECISION> *mapf, const FZdatabase & fieldlist, 
		   std::string almKey, std::string clsKey, const ParameterList & config);
void PrepRingWeights(int col, arr<double> & weight, const ParameterList & config);
double rad2deg(double rad);
double theta2dec(double theta);
double phi2ra(double phi);
int FileFormat(std::string);
int CountWords(const std::string header);
int GetSubstrPos(const std::string field, const std::string header);
void CatalogFill(CAT_PRECISION **catalog, long row, int column, double value, char **catSet);
void Kappa2ShearEmode(Alm<xcomplex <ALM_PRECISION> > &Elm, Alm<xcomplex <ALM_PRECISION> > &Klm);
pointing xyz2ang(const vec3 & cartesian);
void CorrGauss(double **gaus1, gsl_matrix *L, double **gaus0);


// Template definitions


// Generates galaxy ellipticity from shear and convergence, including random source ellipticity:
template<class RNG>
void GenEllip(RNG& rng, double sigma, double kappa, double gamma1,
              double gamma2, double *eps1, double *eps2, bool use_shear) {
  using std::cout; using std::endl; using std::cin;

  std::complex<double> g, epsSrc, eps, k, one, gamma;
  double epsmag;

  // Set complex numbers:
  gamma.real(gamma1); gamma.imag(gamma2);
  k.real(kappa);      k.imag(0.0);
  one.real(1.0);      one.imag(0.0);

  // Compute reduced shear:
  g = gamma/(one-k);

  std::normal_distribution<> gaussian(0, sigma);

  // Generate source intrinsic ellipticity:
  if (sigma>0.0) {
    // If shear is being used, we are going to limit ourselves to the ellipticity formula for the case of abs(g)<1.
    // We then assume that the maximum value of shear (gamma) that can be encountered is 0.1.
    // Hence, we allow intrinsic elipticity (epsSrc = epsilon_s) to be only up to 0.9.
    if (use_shear==1) {
      epsmag = 1.0;         //Initial dummy value of the magnitude of intrinsic elipticity
      while (epsmag > 0.9) {
        epsSrc.real(gaussian(rng));
        epsSrc.imag(gaussian(rng));
        epsmag = std::abs(epsSrc);
      }

    }
    // If reduced shear is being used, then we are free to generate any random value for the intrinsic ellipticity:
    else {
      epsSrc.real(gaussian(rng));
      epsSrc.imag(gaussian(rng));
    }
  }
  else {
    epsSrc.real(0.0);
    epsSrc.imag(0.0);
  }
  // Compute ellipticity of the image:
  if (norm(g) <= 1.0) {
    if (use_shear==1) {eps = (epsSrc + gamma);}                 // If shear is to be used
    else {eps = (epsSrc+g) / (one + conj(g)*epsSrc);}           // If reduced shear is to be used
  }
  else {
    eps = (one + g*conj(epsSrc))/(conj(epsSrc)+conj(g));
  }
  //Return ellipticity of the image:
  (*eps1) = eps.real();
  (*eps2) = eps.imag();
}


// Randomly picks an angular position (uniformly) inside angular boundaries.
// Needed for RandAngInPix. 
template<class RNG>
pointing randang(RNG& rng, double thetamin, double thetamax,
                 double phimin, double phimax) {
  const double twopi=6.283185307179586;
  double xmin, xmax;
  pointing ang;

  std::uniform_real_distribution<> unif;

  xmin      = (1.0+cos(thetamax))/2.0;
  xmax      = (1.0+cos(thetamin))/2.0;
  ang.phi   = unif(rng)*(phimax-phimin)+phimin;
  ang.theta = acos(2*(unif(rng)*(xmax-xmin)+xmin)-1);
  return ang;
}


// Uniformly randomly selects an angular position inside a pixel.
template<class RNG, class Map>
pointing RandAngInPix(RNG& rng, const Map& map, int pixel) {
  const double twopi=6.283185307179586;
  std::vector<vec3> corner;
  double thetamin, thetamax, phimin, phimax;
  pointing ang;
  // Find pixel limits for random sampling the angles:
  map.boundaries(pixel, 1, corner);
  thetamin = xyz2ang(corner[0]).theta; // N corner.
  thetamax = xyz2ang(corner[2]).theta; // S corner.
  phimin   = xyz2ang(corner[1]).phi;   // W corner.
  phimax   = xyz2ang(corner[3]).phi;   // E corner.
  if (phimin>phimax)
    phimin = phimin-twopi;
  // Randomly pick angle inside the pixel. 
  do
    ang = randang(rng, thetamin, thetamax, phimin, phimax);
  while (map.ang2pix(ang) != pixel);
  // Return pointing:
  return ang;
}


// Returns the minimum of matrix[offset...offset+length][index], keeping index fixed.
template<typename type>
type Minimum(type **matrix, int index, int length, int offset=0) {
  int i;
  type min;
  min = matrix[offset][index];
  for(i=offset+1; i<offset+length; i++) if (matrix[i][index]<min) min=matrix[i][index];
  return min;
}

// Returns the maximum of matrix[offset...offset+length][index], keeping index fixed.
template<typename type>
type Maximum(type **matrix, int index, int length, int offset=0) {
  int i;
  type max;
  max = matrix[offset][index];
  for(i=offset+1; i<offset+length; i++) if (matrix[i][index]>max) max=matrix[i][index];
  return max;
}


// pseudo-random distribution that returns a fixed value
template<class RealType = double>
class DeltaDistribution
{
  public:
    typedef RealType result_type;

    class param_type
    {
      public:
        typedef DeltaDistribution distribution_type;

        explicit param_type(result_type mean = 0) : x(mean) {}

        result_type mean() const { return x; }

      private:
        result_type x;
    };

    explicit DeltaDistribution(result_type mean = 0) : p(mean) {}

    template<class RNG>
    result_type operator()(RNG&) { return p.mean(); }
    template<class RNG>
    result_type operator()(RNG&, const param_type& params)
    {
      return params.mean();
    }

    result_type mean() const { return p.mean(); }

    param_type param() const { return p; }
    void param(const param_type& params) { p = params; }

  private:
    param_type p;
};


// mean and variance parameters for std::normal_distribution
template<class RealType>
typename std::normal_distribution<RealType>::param_type
    make_param(const std::normal_distribution<RealType>&,
               const RealType& mean, const RealType& variance)
{
  using param_type = typename std::normal_distribution<RealType>::param_type;
  return param_type(mean, sqrt(variance));
}


// mean and variance parameters for std::poisson_distribution
template<class IntType, class RealType>
typename std::poisson_distribution<IntType>::param_type
    make_param(const std::poisson_distribution<IntType>&,
               const RealType& mean, const RealType& variance)
{
  using param_type = typename std::poisson_distribution<IntType>::param_type;
  return param_type(mean);
}


// mean and variance parameters for DeltaDistribution
template<class RealType>
typename DeltaDistribution<RealType>::param_type
    make_param(const DeltaDistribution<RealType>&,
               const RealType& mean, const RealType& variance)
{
  using param_type = typename DeltaDistribution<RealType>::param_type;
  return param_type(mean);
}


// sample galaxies from a random number distribution
template<class Distribution, class RngVec, class MapType, class SelectionType,
         class MaskType, class FieldsType>
void SampleGalaxies(RngVec& rng, MapType& mapf, const SelectionType& selection,
                    const MaskType& maskval, const FieldsType& fieldlist,
                    bool clamp_density) {
  std::vector<int> counter(omp_get_max_threads());

  for (int i = 0, n = fieldlist.Nfields(); i < n; ++i) {
    if (fieldlist.ftype(i) == fgalaxies) {
      int f, z;
      fieldlist.Index2Name(i, &f, &z);
      Announce("    sampling f" + std::to_string(f) + "z" + std::to_string(z)
               + "...");

      const int npix = mapf[i].Npix();

      // pixel solid angle in arcmin^2
      const double dw = 1.4851066049791e8/npix;

      std::fill(counter.begin(), counter.end(), 0);

      #pragma omp parallel default(none) shared(i, npix, clamp_density, mapf, \
          counter, selection, maskval, dw, rng)
      {
        Distribution d;

        const int k = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (int j = 0; j < npix; ++j) {
          if (clamp_density && mapf[i][j] < -1.0) {
            // If density is negative, set it to zero.
            mapf[i][j] = -1.0;
            ++counter[k];
          }

          if (selection.MaskBit(i, j) == 1 || selection.MaskBit(i, j) == 2 ||
              selection.MaskBit(i, j) == 3) {
            mapf[i][j] = maskval;
          } else {
            const double s = selection(i, j);

            const double mean = s*(1.0+mapf[i][j])*dw;
            const double var = s*dw;

            mapf[i][j] = d(rng[k], make_param(d, mean, var));

            // Count pixels with negative number of galaxies after sampling.
            if (!clamp_density && mapf[i][j] < 0)
              ++counter[k];
          }
        }
      }

      Announce();
      int count = std::accumulate(counter.begin(), counter.end(), 0);
      std::cout << "    negative fraction: " << .01*count/npix << "%\n";
    }
  }
}

#endif
