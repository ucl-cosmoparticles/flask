#ifndef FLASK_NO_HEALPIX_CXX_INCLUDE_DIR
#include <healpix_cxx/healpix_map_fitsio.h> // For FITS files.
#else
#include <healpix_map_fitsio.h>
#endif

#include <iomanip>              // For setprecision.
#include "GeneralOutput.hpp"    // I don't know why, maybe to avoid mismatches.
#include "Utilities.hpp"        // For warnings, errros and dynamic allocation.
#include "flask_aux.hpp"        // For n2fz function.
#include "FieldsDatabase.hpp"

// Based on Henrique's changes from March 8 2020:
#if USEMAP2TGA
#include <levels_facilities.h>  // To use Healpix v<3.50 map2tga_module function.
#endif

/***********************/
/*** Matrices output ***/
/***********************/

// Print a GSL matrix to file
void GeneralOutput(const gsl_matrix *Cov, std::string filename, bool inform) {
  std::ofstream outfile; 

  outfile.open(filename.c_str());
  if (!outfile.is_open()) warning("GeneralOutput: cannot open file "+filename);
  else { 
    PrintGSLMatrix(Cov, &outfile); 
    outfile.close();
    if(inform==1) std::cout << ">> GSL matrix written to "+filename<<std::endl;
  }  
}


// Print all GSL matrices in a vector to files:
void GeneralOutput(gsl_matrix **CovByl, const ParameterList & config, std::string keyword, bool inform) {
  std::string filename;
  std::ofstream outfile; 
  int lmin, lmax, lminout, lmaxout, l;

  if (config.reads(keyword)!="0") {
    lmin    = config.readi("LRANGE", 0);
    lmax    = config.readi("LRANGE", 1);
    lminout = config.readi("LRANGE_OUT", 0);
    lmaxout = config.readi("LRANGE_OUT", 1);
    if (lmaxout > lmax) {
      warning("GeneralOutput: LRANGE_OUT max is beyond LRANGE max. Will use the latter instead.");
      lmaxout = lmax;
    }
    if (lminout < lmin) {
      warning("GeneralOutput: LRANGE_OUT min is beyond LRANGE min. Will use the latter instead.");
      lminout = lmin;
    }
    if (lminout>lmaxout) error("GeneralOutput: LRANGE set in the wrong order.");

    for (l=lminout; l<=lmaxout; l++) {
      filename=config.reads(keyword)+"l"+ZeroPad(l,lmaxout)+".dat";
      outfile.open(filename.c_str());
      if (!outfile.is_open()) warning("GeneralOutput: cannot open file "+filename);
      else { 
	PrintGSLMatrix(CovByl[l], &outfile); 
	outfile.close();
	if(inform==1) std::cout << ">> "+keyword+" ["<<l<<"] written to "+filename<<std::endl;
      }
    }  
  }
}


/*******************/
/*** Cl's output ***/
/*******************/


// Prints all Cl's to a TEXT file:
void GeneralOutput(double **recovCl, bool *yesCl, const FZdatabase & fieldlist, 
		   const ParameterList & config, std::string keyword, bool inform) {
  std::string filename;
  std::ofstream outfile; 
  int k, l, m, i, j, fi, zi, fj, zj, lminout, lmaxout, NCls, Nfields;
  
  filename  = config.reads(keyword);
  // If requested, write Cl's to the file:
  if (filename!="0") {
    outfile.open(filename.c_str());
    if (!outfile.is_open()) warning("GeneralOutput: cannot open "+filename+" file.");
    lminout = config.readi("LRANGE_OUT", 0);
    lmaxout = config.readi("LRANGE_OUT", 1);
    if (lmaxout > config.readi("LRANGE", 1)) { 
      lmaxout = config.readi("LRANGE", 1); 
      warning("GeneralOutput: LRANGE_OUT beyond LRANGE upper bound, will use latter instead.");
    }
    if (lminout < config.readi("LRANGE", 0)) { 
      lminout = config.readi("LRANGE", 0); 
      warning("GeneralOutput: LRANGE_OUT beyond LRANGE lower bound, will use the latter instead.");
    }
    Nfields = fieldlist.Nfields();
    NCls    = (Nfields*(Nfields+1))/2;

    // Write header to file:
    outfile << "# l ";
    for (k=0; k<NCls; k++) if (yesCl[k]==1) {
      l = (int)((sqrt(8.0*(NCls-1-k)+1.0)-1.0)/2.0);
      m = NCls-1-k-(l*(l+1))/2;
      i = Nfields-1-l;
      j = Nfields-1-m;
      fieldlist.Index2Name(i, &fi, &zi);
      fieldlist.Index2Name(j, &fj, &zj);
      outfile << "Cl-f"<<fi<<"z"<<zi<<"f"<<fj<<"z"<<zj<<" ";
    }
    outfile << std::endl;
    
    // Write Cls to file:
    for (l=lminout; l<=lmaxout; l++) {
      outfile<<l<<" "; for (k=0; k<NCls; k++) if (yesCl[k]==1) outfile << recovCl[k][l] << " ";
      outfile << std::endl;      
    }
    
    outfile.close();
    if(inform==1) std::cout << ">> "+keyword+" written to "+filename<<std::endl; 
  }
}


/********************/
/*** Alm's output ***/
/********************/


// Prints ALL fields alm's to a TEXT file labeled by their FIELD IDs.
void GeneralOutput(Alm<xcomplex <ALM_PRECISION> > *af, const ParameterList & config, std::string keyword, 
		   const FZdatabase & fieldlist, bool inform) {
  std::string filename;
  std::ofstream outfile; 
  int lminout, lmaxout, mmax, l, m, i, Nfields, f, z;

  Nfields = fieldlist.Nfields();
  if (Nfields<=0) error("GeneralOutput: no fields found in alm list.");

  // If requested, write alm's to file:
  if (config.reads(keyword)!="0") {
    filename = config.reads(keyword);
    outfile.open(filename.c_str());
    if (!outfile.is_open()) warning("GeneralOutput: cannot open "+filename+" file.");
    outfile << "# l, m";
    for (i=0; i<Nfields; i++) if (af[i].Lmax()>0) {
      fieldlist.Index2Name(i, &f, &z);
      outfile << ", f" << f << "z" << z << "Re, f" << f << "z" << z << "Im";
    }
    outfile<<std::endl<<std::endl;
    
    // Find out how many multipoles have been set (some alm's can be empty and do not count):
    i = 0;
    while(i<Nfields && af[i].Lmax()<=0) i++;
    l = af[i].Lmax();

    // Check if output bounds are correct:
    lminout = config.readi("LRANGE_OUT", 0);
    lmaxout = config.readi("LRANGE_OUT", 1);
    if (lmaxout > l) { 
      lmaxout = l; 
      warning("GeneralOutput: LRANGE_OUT beyond available data, will use the latter instead.");
    }
    if (lminout < config.readi("LRANGE", 0)) { 
      lminout = config.readi("LRANGE", 0); 
      warning("GeneralOutput: LRANGE_OUT beyond LRANGE lower bound, will use the latter instead.");
    }
    mmax = config.readi("MMAX_OUT");
    if (mmax>lminout) error ("GeneralOutput: current code only allows MMAX_OUT <= LRANGE_OUT lower bound.");

    // Output all alm's:
    if (mmax<0) {
      for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=l; m++) {
	  outfile << l <<" "<< m;
	  for (i=0; i<Nfields; i++) 
	    if(af[i].Lmax()>0) {     // Some alm's might be empty.
#if USEXCOMPLEX
	      outfile <<" "<<std::setprecision(10)<< af[i](l,m).re<<" "<<std::setprecision(10)<< af[i](l,m).im;
#else
	      outfile <<" "<<std::setprecision(10)<< af[i](l,m).real()<<" "<<std::setprecision(10)<< af[i](l,m).imag();
#endif
	    }
	  outfile<<std::endl;
	} 
    }
    // Truncate m in alm output:
    else {
     for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=mmax; m++) {
	  outfile << l <<" "<< m;
	  for (i=0; i<Nfields; i++) 
	    if(af[i].Lmax()>0) {     // Some alm's might be empty.
#if USEXCOMPLEX
	      outfile <<" "<<std::setprecision(10)<< af[i](l,m).re<<" "<<std::setprecision(10)<< af[i](l,m).im;
#else
	      outfile <<" "<<std::setprecision(10)<< af[i](l,m).real()<<" "<<std::setprecision(10)<< af[i](l,m).imag();
#endif

	    }
	  outfile<<std::endl;
	}  
    }
    outfile.close();
    if(inform==1) std::cout << ">> "+keyword+" written to "+filename<<std::endl;
  }
}


// Prints one single alm table to a TEXT file using a PREFIX and a FIELD ID.
void GeneralOutput(const Alm<xcomplex <ALM_PRECISION> > & a, const ParameterList & config, std::string keyword, int f, int z, bool inform) {
  std::string filename;
  std::ofstream outfile; 
  char message[100];
  int lminout, lmaxout, mmax, l, m;

  // If requested, write alm's to file:
  if (config.reads(keyword)!="0") {
    sprintf(message, "%sf%dz%d.dat", config.reads(keyword).c_str(), f, z);
    filename.assign(message);
    
    outfile.open(message);
    if (!outfile.is_open()) warning("GeneralOutput: cannot open "+filename+" file.");
    outfile << "# l, m, Re(alm), Im(alm)"<<std::endl<<std::endl;

    // Check if output bounds are correct:
    lminout = config.readi("LRANGE_OUT", 0);
    lmaxout = config.readi("LRANGE_OUT", 1);
    if (lmaxout > a.Lmax()) { 
      lmaxout = a.Lmax(); 
      warning("GeneralOutput: LRANGE_OUT beyond available data, will use the latter instead.");
    }
    if (lminout < config.readi("LRANGE", 0)) { 
      lminout = config.readi("LRANGE", 0); 
      warning("GeneralOutput: LRANGE_OUT beyond LRANGE lower bound, will use the latter instead.");
    }
    mmax = config.readi("MMAX_OUT");
    if (mmax>lminout) error ("GeneralOutput: current code only allows MMAX_OUT <= LRANGE_OUT lower bound.");

    // Output all alm's:
    if (mmax<0) {
      for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=l; m++) {
	  outfile << l <<" "<< m;
#if USEXCOMPLEX
	  outfile <<" "<<std::setprecision(10)<< a(l,m).re<<" "<<std::setprecision(10)<< a(l,m).im;
#else
	  outfile <<" "<<std::setprecision(10)<< a(l,m).real()<<" "<<std::setprecision(10)<< a(l,m).imag();
#endif
	  outfile<<std::endl;
	} 
    }
    // Truncate m in alm output:
    else {
     for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=mmax; m++) {
	  outfile << l <<" "<< m;
#if USEXCOMPLEX
	  outfile <<" "<<std::setprecision(10)<< a(l,m).re<<" "<<std::setprecision(10)<< a(l,m).im;
#else
	  outfile <<" "<<std::setprecision(10)<< a(l,m).real()<<" "<<std::setprecision(10)<< a(l,m).imag();
#endif
	  outfile<<std::endl;
	}  
    }
    outfile.close();
    if(inform==1) std::cout << ">> "+keyword+" written to "+filename<<std::endl;
  }
}



// Prints one single alm table to a TEXT file.
void GeneralOutput(const Alm<xcomplex <ALM_PRECISION> > & a, const ParameterList & config, std::string keyword, bool inform) {
  std::string filename;
  std::ofstream outfile; 
  int lminout, lmaxout, mmax, l, m;

  // If requested, write alm's to file:
  if (config.reads(keyword)!="0") {
    filename = config.reads(keyword);
    outfile.open(filename.c_str());
    if (!outfile.is_open()) warning("GeneralOutput: cannot open "+filename+" file.");
    outfile << "# l, m, Re(alm), Im(alm)"<<std::endl<<std::endl;
    
    // Check if output bounds are correct:
    lminout = config.readi("LRANGE_OUT", 0);
    lmaxout = config.readi("LRANGE_OUT", 1);
    if (lmaxout > a.Lmax()) { 
      lmaxout = a.Lmax(); 
      warning("GeneralOutput: LRANGE_OUT beyond available data, will use the latter instead.");
    }
    if (lminout < config.readi("LRANGE", 0)) { 
      lminout = config.readi("LRANGE", 0); 
      warning("GeneralOutput: LRANGE_OUT beyond LRANGE lower bound, will use the latter instead.");
    }
    mmax = config.readi("MMAX_OUT");
    if (mmax>lminout) error ("GeneralOutput: current code only allows MMAX_OUT <= LRANGE_OUT lower bound.");

    // Output all alm's:
    if (mmax<0) {
      for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=l; m++) {
	  outfile << l <<" "<< m;
#if USEXCOMPLEX
	  outfile <<" "<<std::setprecision(10)<< a(l,m).re<<" "<<std::setprecision(10)<< a(l,m).im;
#else
	  outfile <<" "<<std::setprecision(10)<< a(l,m).real()<<" "<<std::setprecision(10)<< a(l,m).imag();
#endif
	  outfile<<std::endl;
	} 
    }
    // Truncate m in alm output:
    else {
     for(l=lminout; l<=lmaxout; l++)
	for(m=0; m<=mmax; m++) {
	  outfile << l <<" "<< m;
#if USEXCOMPLEX
	  outfile <<" "<<std::setprecision(10)<< a(l,m).re<<" "<<std::setprecision(10)<< a(l,m).im;
#else
	  outfile <<" "<<std::setprecision(10)<< a(l,m).real()<<" "<<std::setprecision(10)<< a(l,m).imag();
#endif
	  outfile<<std::endl;
	}  
    }
    outfile.close();
    if(inform==1) std::cout << ">> "+keyword+" written to "+filename<<std::endl;
  }
}



/********************/
/*** Maps output  ***/
/********************/


// Prints a list of maps to a many FITS files:
void GeneralOutputFITS(Healpix_Map<MAP_PRECISION> *mapf, const ParameterList & config, std::string keyword, 
		       const FZdatabase & fieldlist, bool inform) {
  std::string filename, tempstr;
  char message[100], message2[100], *arg[4];
  char opt1[]="-bar";
  int i, Nfields, f, z;
  
  Nfields=fieldlist.Nfields();

  if (config.reads(keyword)!="0") {
    // Write to FITS:
    tempstr  = config.reads(keyword);
    for (i=0; i<Nfields; i++) {
      fieldlist.Index2Name(i, &f, &z);
      sprintf(message, "%sf%dz%d.fits", tempstr.c_str(), f, z);
      filename.assign(message);
      write_Healpix_map_to_fits("!"+filename,mapf[i],planckType<MAP_PRECISION>()); // Filename prefixed by ! to overwrite.
      if(inform==1) std::cout << ">> "<<keyword<< "["<<i<<"] written to "<<filename<<std::endl;
      // Write to TGA if requested:
      if (config.readi("FITS2TGA")==1 || config.readi("FITS2TGA")==2) {
#if USEMAP2TGA
	sprintf(message2, "%sf%dz%d.tga", tempstr.c_str(), f, z);
	arg[1]=message; arg[2]=message2; arg[3]=opt1;
	map2tga_module(4, (const char **)arg);
	if(inform==1) std::cout << ">> "<<keyword<< "["<<i<<"] written to "<<message2<<std::endl;
	if (config.readi("FITS2TGA")==2) {
	  sprintf(message2, "rm -f %s", message);
	  system(message2);
	  if(inform==1) std::cout << "-- "<<filename<<" file removed."<<std::endl;
	}
#else
    warning("Using FITS format (TGA format only available for Healpix version < 3.60).");
#endif
      }
    }
  }
}


// Prints a list of maps to a single TEXT file.
void GeneralOutputTEXT(Healpix_Map<MAP_PRECISION> *mapf, const ParameterList & config, 
		       std::string keyword, const FZdatabase & fieldlist, bool inform) {
  std::string filename;
  std::ofstream outfile;
  int i, j, Nfields, field, z, npixels, *nside, n, coordtype;
  pointing coord;

  if (config.reads(keyword)!="0") {
    Nfields   = fieldlist.Nfields(); 
    coordtype = config.readi("ANGULAR_COORD");
    if (coordtype<0 || coordtype>2) warning("GeneralOutputTEXT: unknown ANGULAR_COORD option, keeping Theta & Phi in radians.");

    // Check which maps are allocated and avoid different-size maps.
    j=0;
    nside = vector<int>(0,Nfields-1);
    for (i=0; i<Nfields; i++) {
      nside[i] = mapf[i].Nside(); // Record all maps Nsides.
      if (nside[i]!=0) {          // Look for allocated maps.
	if (j==0) {               // Initialize j to first non-zero Nside.
	  j=nside[i]; n=i;
	}
	else if (nside[i]!=j) error ("GeneralOutput: maps do not have the same number of pixels.");
      }
    }
    npixels=12*j*j;
    
    // Output to file:
    filename=config.reads(keyword);
    outfile.open(filename.c_str());
    if (!outfile.is_open()) warning("GeneralOutput: cannot open file "+filename);
    else {
      // Set Headers:
      if (coordtype==2) outfile << "# ra, dec";
      else outfile << "# theta, phi";
      for (i=0; i<Nfields; i++) if (nside[i]!=0) { 
	  fieldlist.Index2Name(i, &field, &z);
	  outfile << ", f"<<field<<"z"<<z;
	}
      outfile << std::endl;
      // LOOP over pixels:
      for (j=0; j<npixels; j++) {
	// Output coordinates:
	coord = mapf[n].pix2ang(j);
	if (coordtype==1) {
	  coord.theta = rad2deg(coord.theta); 
	  coord.phi   = rad2deg(coord.phi);
	  outfile << coord.theta <<" "<< coord.phi;
	}
	else if (coordtype==2) {
	  coord.theta = theta2dec(coord.theta); 
	  coord.phi   = phi2ra(coord.phi);
	  outfile << coord.phi <<" "<< coord.theta;
	}
	else outfile << coord.theta <<" "<< coord.phi;
	// Output fields:
	for (i=0; i<Nfields; i++) if (nside[i]!=0) outfile <<" "<< mapf[i][j];
	outfile << std::endl;
      }
    }
    outfile.close();
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    free_vector(nside, 0,Nfields-1);
  } 
}


// Prints array of Healpix maps to either a single TEXT (if fits=0 or unespecified) or many FITS (if fits=1) files.
void GeneralOutput(Healpix_Map<MAP_PRECISION> *mapf, const ParameterList & config, 
		   std::string keyword, const FZdatabase & fieldlist, bool fits, bool inform) {
  if (fits==1) GeneralOutputFITS(mapf, config, keyword, fieldlist, inform);
  else GeneralOutputTEXT(mapf, config, keyword, fieldlist, inform);
}


// Prints two lists of maps to a single TEXT file.
void GeneralOutput(Healpix_Map<MAP_PRECISION> *gamma1, Healpix_Map<MAP_PRECISION> *gamma2, 
		   const ParameterList & config, std::string keyword, const FZdatabase & fieldlist, bool inform) {
  std::string filename;
  std::ofstream outfile;
  int i, j, Nfields, field, z, npixels, *nside, n, coordtype;
  pointing coord;

  if (config.reads(keyword)!="0") {
    Nfields=fieldlist.Nfields(); 
    
    coordtype = config.readi("ANGULAR_COORD");
    if (coordtype<0 || coordtype>2) warning("GeneralOutputTEXT: unknown ANGULAR_COORD option, keeping Theta & Phi in radians.");

    // Check which maps are allocated and avoid different-size maps.
    j=0;
    nside = vector<int>(0,Nfields-1);
    for (i=0; i<Nfields; i++) {
      nside[i]  = gamma1[i].Nside(); // Record all maps Nsides.
      if (nside[i]!=gamma2[i].Nside()) error ("GeneralOutput: expecting pair of maps with same number of pixels.");
      if (nside[i]!=0) {             // Look for allocated maps.
	if (j==0) {                  // Initialize j to first non-zero Nside.
	  j=nside[i]; n=i;
	}
	else if (nside[i]!=j) error ("GeneralOutput: maps do not have the same number of pixels.");
      }
    }
    npixels=12*j*j;
    
    // Output to file:
    filename=config.reads(keyword);
    outfile.open(filename.c_str());
    if (!outfile.is_open()) warning("GeneralOutput: cannot open file "+filename);
    else {
      // Set Headers:
      if (coordtype==2) outfile << "# ra, dec";
      else outfile << "# theta, phi";
      for (i=0; i<Nfields; i++) if (nside[i]!=0) { 
	  fieldlist.Index2Name(i, &field, &z);
	  outfile <<", f"<<field<<"z"<<z<<"[1]"<<", f"<<field<<"z"<<z<<"[2]";
	}
      outfile << std::endl;
      // LOOP over allocated fields:
      for (j=0; j<npixels; j++) {
	// Coordinates output:
	coord = gamma1[n].pix2ang(j);
	if (coordtype==1) {
	  coord.theta = rad2deg(coord.theta); 
	  coord.phi   = rad2deg(coord.phi);
	  outfile << coord.theta <<" "<< coord.phi;
	}
	else if (coordtype==2) {
	  coord.theta = theta2dec(coord.theta); 
	  coord.phi   = phi2ra(coord.phi);
	  outfile << coord.phi <<" "<< coord.theta;
	}
	else outfile << coord.theta <<" "<< coord.phi;
	// Fields output:
	for (i=0; i<Nfields; i++) if (nside[i]!=0) outfile <<" "<< gamma1[i][j]<<" "<< gamma2[i][j];
	outfile << std::endl;
      }
    }
    outfile.close();
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    free_vector(nside, 0,Nfields-1);
  } 
}


// Prints one single map to FITS file based on a PREFIX and a FIELD ID:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & map, const ParameterList & config, std::string keyword, int *fnz, bool inform) {
  std::string filename, tgafile;
  char *arg[4];
  char message1[100], message2[100];
  char opt1[]="-bar";
  if (config.reads(keyword)!="0") {
    sprintf(message1, "%sf%dz%d.fits", config.reads(keyword).c_str(), fnz[0], fnz[1]);
    filename.assign(message1);

    // Write to FITS:
    write_Healpix_map_to_fits("!"+filename, map, planckType<MAP_PRECISION>()); // Filename prefixed by ! to overwrite.
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    // Write to TGA if requested:
    if (config.readi("FITS2TGA")==1 || config.readi("FITS2TGA")==2) {
#if USEMAP2TGA
      tgafile = filename;
      tgafile.replace(tgafile.find(".fits"),5,".tga");
      sprintf(message1, "%s", filename.c_str());
      sprintf(message2, "%s", tgafile.c_str());
      arg[1]=message1; arg[2]=message2; arg[3]=opt1;
      map2tga_module(4, (const char **)arg);
      if(inform==1) std::cout << ">> "<<keyword<<" written to "<<tgafile<<std::endl;
      if (config.readi("FITS2TGA")==2) {
	sprintf(message2, "rm -f %s", message1);
	system(message2);
	if(inform==1) std::cout << "-- "<<filename<<" file removed."<<std::endl;
      }
#else
      warning("Using FITS format (TGA format only available for Healpix version < 3.60).");
#endif
    }
  } 
}


// Prints one single combination of kappa, gamma1 and gamma2 maps to FITS file based on a PREFIX and a FIELD ID:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & kmap, const Healpix_Map<MAP_PRECISION> & g1map, 
		   const Healpix_Map<MAP_PRECISION> & g2map, const ParameterList & config, std::string keyword, int f, int z, bool inform) {
  std::string filename, tgafile;
  char *arg[4];
  char message1[100], message2[100];
  char opt1[]="-bar";
  if (config.reads(keyword)!="0") {
    sprintf(message1, "%sf%dz%d.fits", config.reads(keyword).c_str(), f, z);
    filename.assign(message1);

    // Write to FITS:
    write_Healpix_map_to_fits("!"+filename, kmap, g1map, g2map, planckType<MAP_PRECISION>());
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    // Write to TGA if requested:
    if (config.readi("FITS2TGA")==1 || config.readi("FITS2TGA")==2) {
#if USEMAP2TGA
      tgafile = filename;
      tgafile.replace(tgafile.find(".fits"),5,".tga");
      sprintf(message1, "%s", filename.c_str());
      sprintf(message2, "%s", tgafile.c_str());
      arg[1]=message1; arg[2]=message2; arg[3]=opt1;
      map2tga_module(4, (const char **)arg);
      if(inform==1) std::cout << ">> "<<keyword<<" written to "<<tgafile<<std::endl;
      if (config.readi("FITS2TGA")==2) {
	sprintf(message2, "rm -f %s", message1);
	system(message2);
	if(inform==1) std::cout << "-- "<<filename<<" file removed."<<std::endl;
      }
#else
      warning("Using FITS format (TGA format only available for Healpix version < 3.60).");
#endif
    }
  } 
}


// Prints one single combination of kappa, gamma1 and gamma2 maps to FITS file.
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & kmap, const Healpix_Map<MAP_PRECISION> & g1map, 
		   const Healpix_Map<MAP_PRECISION> & g2map, const ParameterList & config, std::string keyword, bool inform) {
  std::string filename, tgafile;
  char *arg[4];
  char message1[100], message2[100];
  char opt1[]="-bar";
  if (config.reads(keyword)!="0") {
    // Write to FITS:
    filename=config.reads(keyword); 
    //sprintf(message1, "rm -f %s", filename.c_str());
    //system(message1); // Have to delete previous fits files first.
    write_Healpix_map_to_fits("!"+filename, kmap, g1map, g2map, planckType<MAP_PRECISION>()); // Filename prefixed by ! to overwrite.
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    // Write to TGA if requested:
    if (config.readi("FITS2TGA")==1 || config.readi("FITS2TGA")==2) {
#if USEMAP2TGA
      tgafile = filename;
      tgafile.replace(tgafile.find(".fits"),5,".tga");
      sprintf(message1, "%s", filename.c_str());
      sprintf(message2, "%s", tgafile.c_str());
      arg[1]=message1; arg[2]=message2; arg[3]=opt1;
      map2tga_module(4, (const char **)arg);
      if(inform==1) std::cout << ">> "<<keyword<<" written to "<<tgafile<<std::endl;
      if (config.readi("FITS2TGA")==2) {
	sprintf(message2, "rm -f %s", message1);
	system(message2);
	if(inform==1) std::cout << "-- "<<keyword<<" FITS file removed."<<std::endl;
      }
#else
      warning("Using FITS format (TGA format only available for Healpix version < 3.60).");
#endif
    }
  } 
}


// Prints one single map to FITS and/or TGA file.
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & map, const ParameterList & config, std::string keyword, bool inform) {
  std::string filename, tgafile;
  char *arg[4];
  char message1[100], message2[100];
  char opt1[]="-bar";

  if (config.reads(keyword)!="0") {
    // Write to FITS:
    filename=config.reads(keyword); 
    write_Healpix_map_to_fits("!"+filename, map, planckType<MAP_PRECISION>()); // Filename prefixed by ! to overwrite.
    if(inform==1) std::cout << ">> "<<keyword<<" written to "<<filename<<std::endl;
    // Write to TGA if requested:
    if (config.readi("FITS2TGA")==1 || config.readi("FITS2TGA")==2) {
#if USEMAP2TGA
      tgafile = filename;
      tgafile.replace(tgafile.find(".fits"),5,".tga");
      sprintf(message1, "%s", filename.c_str());
      sprintf(message2, "%s", tgafile.c_str());
      arg[1]=message1; arg[2]=message2; arg[3]=opt1;
      map2tga_module(4, (const char **)arg);
      if(inform==1) std::cout << ">> "<<keyword<<" written to "<<tgafile<<std::endl;
      if (config.readi("FITS2TGA")==2) {
	sprintf(message2, "rm -f %s", message1);
	system(message2);
	if(inform==1) std::cout << "-- "<<keyword<<" FITS file removed."<<std::endl;
      }
#else
      warning("Using FITS format (TGA format only available for Healpix version < 3.60).");
#endif
    }
  } 
}
