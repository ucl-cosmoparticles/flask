//
// transformcl.hpp -- transform angular power spectra and correlation functions
//
// author: Nicolas Tessore <n.tessore@ucl.ac.uk>
// license: MIT
//
// version: 2021.3.18
//

#pragma once

#define POCKETFFT_NO_MULTITHREADING
#include "pocketfft/pocketfft_hdronly.h"


namespace flask
{


template<class T>
void cltoxi(unsigned int n, const T* cl, T* xi)
{
    T a, b;
    unsigned int k, l;

    // numerical constants
    const T ONE(1);
    const T HALF(ONE/2);
    const T FOUR_PI(12.566370614359172953850573533118011536788677597500);

    // convert cl to DCT coefficients

    // first row
    a = 1/FOUR_PI;
    b = a;
    xi[0] = b*cl[0];
    for(l = 2; l < n; l += 2)
    {
        b *= ((l-ONE)*(l-ONE))/(l*l);
        xi[0] += b*(2*l+1)*cl[l];
    }

    // remaining rows
    for(k = 1; k < n; ++k)
    {
        a *= (ONE - HALF/k);
        b = a;
        xi[k] = b*(2*k+1)*cl[k];
        for(l = k+2; l < n; l += 2)
        {
            b *= ((l-k-ONE)*(l+k-ONE))/((l-k)*(l+k));
            xi[k] += b*(2*l+1)*cl[l];
        }
    }

    // transform DCT coefficients to xi
    const pocketfft::shape_t shape = { n };
    const pocketfft::stride_t stride = { sizeof(T) };
    const pocketfft::shape_t axes = { 0 };
    pocketfft::dct(shape, stride, stride, axes, 3, xi, xi, ONE, false);
}


template<class T>
void xitocl(unsigned int n, const T* xi, T* cl)
{
    T a, b;
    unsigned int k, l;

    // numerical constants
    const T ONE(1);
    const T TWO(2);
    const T THREE(3);
    const T TWO_PI(6.2831853071795864769252867665590057683943387987502);

    // transform xi to DCT coefficients
    const pocketfft::shape_t shape = { n };
    const pocketfft::stride_t stride = { sizeof(T) };
    const pocketfft::shape_t axes = { 0 };
    pocketfft::dct(shape, stride, stride, axes, 2, xi, cl, ONE/n, false);

    // convert DCT coefficients to cl

    // first row
    a = TWO_PI;
    b = a;
    cl[0] = b*cl[0];
    for(k = 2; k < n; k += 2)
    {
        b *= (k-THREE)/(k+ONE);
        cl[0] += 2*b*cl[k];
    }

    // remaining rows
    for(l = 1; l < n; ++l)
    {
        a *= (ONE - ONE/(2*l+1));
        b = a;
        cl[l] = b*cl[l];
        for(k = l+2; k < n; k += 2)
        {
            b *= (k*(k+l-TWO)*(k-l-THREE))/((k-TWO)*(k+l+ONE)*(k-l));
            cl[l] += b*cl[k];
        }
    }
}

} // namespace flask
