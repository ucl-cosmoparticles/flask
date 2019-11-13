#include <cstring>
#include <fitsio.h>
#include "../src/definitions.hpp"
#include "../src/Utilities.hpp"
#include "../src/flask_aux.hpp"

// The definition below will be changed by make and the result will be written to a new file:
#define HEALPIX_DATA "/path/to/healpix/dir/data"

int WriteCatalog2Fits(std::string filename, CAT_PRECISION **table, long Nentries, const ParameterList & config, std::string headerline) {
  const int COLNAMELENGTH=20;
  fitsfile *fpointer;
  std::stringstream ss, ss2, ss3;
  std::string colwrite, header, word, word2, word3;
  char **columnNames, **columnTypes, **columnUnits;
  int status=0, Ncols, i, datatype, *integer, fits_precision;
  long j;
  char TableName[]  ="GALAXY_CAT"; 

  // Allocate auxiliary integer vector:
  integer = vector<int>(0, Nentries-1);

  // Get number of columns:
  colwrite    = config.reads("CATALOG_COLS");
  Ncols       = CountWords(colwrite);
  // Reaname 'theta' and 'phi' to 'dec' and 'ra' if change of coords. was requested:
  if (config.readi("ANGULAR_COORD")==2) {
    StrReplace(colwrite, "theta", "dec");
    StrReplace(colwrite, "phi", "ra");
  }
  // Get internal variable names of the columns to be written in the catalog:
  columnNames = matrix<char>(0,Ncols, 0,COLNAMELENGTH);
  ss << colwrite;
  for(i=0; i<Ncols; i++) { ss >> word; strcpy(columnNames[i], word.c_str()); } 
  
  // Set column formats and units:
  columnTypes = matrix<char>(0,Ncols, 0,6);
  columnUnits = matrix<char>(0,Ncols, 0,COLNAMELENGTH);

  // This is for Binary tables:
  for(i=0; i<Ncols; i++) { 
    // theta phi z galtype kappa gamma1 gamma2 ellip1 ellip2 pixel maskbit
    if      (strcmp(columnNames[i],"theta"  )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "Radians");}  // 64-bit floating point
    else if (strcmp(columnNames[i],"phi"    )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "Radians");}  // 64-bit floating point
    else if (strcmp(columnNames[i],"ra"     )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "Degrees");}  // 64-bit floating point
    else if (strcmp(columnNames[i],"dec"    )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "Degrees");}  // 64-bit floating point
    else if (strcmp(columnNames[i],"z"      )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"galtype")==0) {sprintf(columnTypes[i],"%s", "1I"); sprintf(columnUnits[i],"%s", "\0");}       // signed 16-bit integer
    else if (strcmp(columnNames[i],"kappa"  )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"gamma1" )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"gamma2" )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"ellip1" )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"ellip2" )==0) {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "\0");}       // 64-bit floating point
    else if (strcmp(columnNames[i],"pixel"  )==0) {sprintf(columnTypes[i],"%s", "1J"); sprintf(columnUnits[i],"%s", "\0");}       // signed 32-bit integer
    else if (strcmp(columnNames[i],"maskbit")==0) {sprintf(columnTypes[i],"%s", "1I"); sprintf(columnUnits[i],"%s", "\0");}       // signed 16-bit integer
    else                                          {sprintf(columnTypes[i],"%s", "1D"); sprintf(columnUnits[i],"%s", "unknown");   // 64-bit floating point
      warning("WriteCatalog2Fits: unknown catalog column "+word.assign(columnNames[i]));}  
  }

  // Get names to be assigned to each of the columns for external output into the FITS file:
  ss2 << headerline;
  for(i=0; i<Ncols; i++) { ss2 >> word2; strcpy(columnNames[i], word2.c_str()); }

  // Create (or overwrite) FITS file with ASCII table:
  fits_create_file(&fpointer, ("!"+filename).c_str(), &status);
  fits_report_error(stderr, status);
  fits_create_tbl(fpointer, BINARY_TBL, Nentries, Ncols, columnNames, columnTypes, columnUnits, TableName, &status);  //This also prints out the header line in FITS file
  fits_report_error(stderr, status);

  //Once the custom column names are written out to the FITS file, need to revert back to the internal variable names to identify the columns, so that the correct numbers get printed:
  ss3 << colwrite;
  for(i=0; i<Ncols; i++) { ss3 >> word3; strcpy(columnNames[i], word3.c_str()); }

  // Write columns to FITS file ASCII/binary table:
  for(i=0; i<Ncols; i++) {
  //for(i=1; i<=Ncols; i++) {
    // Write double variable:
    if (columnTypes[i][1] == 'D' || columnTypes[i][1] == 'E') {
      fits_write_col(fpointer, FIT_PRECISION, i+1, 1, 1, Nentries, table[i], &status);        //This writes out numerical values in columns
    }
    // Write int variable:
    else if (columnTypes[i][1] == 'I' || columnTypes[i][1] == 'J' || columnTypes[i][1] == 'K') {
      for(j=0; j<Nentries; j++) integer[j]=(int)table[i][j];
      fits_write_col(fpointer, TINT, i+1, 1, 1, Nentries, integer, &status);                  //This writes out numerical values in columns
    }
    else error("WriteCatalog2Fits: "+word.assign(columnNames[i])+" has uknown FITS format.");
    // Verify if everything is OK:
    fits_report_error(stderr, status);
  }

  // Do not need columns info anymore:
  free_matrix(columnNames, 0,Ncols, 0,COLNAMELENGTH);
  free_matrix(columnTypes, 0,Ncols, 0,6);
  free_matrix(columnUnits, 0,Ncols, 0,COLNAMELENGTH);
  // Free auxiliary integer vector:
  free_vector(integer, 0,Nentries-1);

  // Close FITS file and exit:
  fits_close_file(fpointer, &status);
  fits_report_error(stderr, status);
  return status;
}


// Reads a Healpix FITS file containing the map2alm weights into a double array:
int ReadHealpixData(int col, const ParameterList & config, double *weights, int datatype) {
  char message[200];
  std::string filename;
  fitsfile *fpointer;
  int status=0, anynul=0;
  long i, firstrow=1, firstelem=1, nelements, nside;
  double *nulval;
  
  nside    = config.readi("NSIDE");
  filename.assign(HEALPIX_DATA);
  if (filename.at(filename.length()-1)!='/') filename = filename+"/";
  // Find out what kind of data will be loaded:
  switch (datatype) {
  case 1:
    // Load map2alm weights:
    filename = filename+"weight_ring_n"+ZeroPad(nside, 10000)+".fits";
    nelements=2*nside;
    break;
  case 2:
    // Load pixel window functions:
    filename = filename+"pixel_window_n"+ZeroPad(nside, 1000)+".fits";
    nelements=4*nside+1;
    break;
  default:
    error("ReadHealpixData: unknown Healpix data type.");
  }

  // Open file:
  fits_open_table(&fpointer, filename.c_str(), READONLY, &status);
  if (status!=0) {
    sprintf(message, "ReadHealpixData: could not open table in FITS file, ERR=%d", status);
    warning(message);
  }
  
  // Prepare to, read and check for errors:
  nulval = vector<double>(0, nelements-1);
  for(i=0; i<nelements; i++) nulval[i]=666.0;
  fits_read_col(fpointer, TDOUBLE, col, firstrow, firstelem, nelements, nulval, weights, &anynul, &status);
  if (status!=0) {
    sprintf(message, "ReadHealpixData: problem reading column in FITS file table, ERR=%d", status);
    warning(message);
  }
  if(anynul!=0) {
    warning("ReadHealpixData: found NULL values in FITS table");
    printf("They are:\n");
    for (i=0; i<nelements; i++) printf("%g ",nulval[i]);
    printf("\n");
  }
  free_vector(nulval, 0, nelements-1);

  // Close file and exit:
  fits_close_file(fpointer, &status);
  if (status!=0) {
    sprintf(message, "ReadHealpixData: could not close FITS file, ERR=%d", status);
    warning(message);
  }
  return status;
}
