/*
# Diffusynth - Synthesize DWI images from T2 images.
# Copyright (C) 2013 Renato Callado Borges
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author can be reached at rborges@if.usp.br
*/

#include <iostream>
#include <cmath>

using namespace std;

double calculate ( double b, double factor, double simplifier );
void print_number ( double number );

int main (int argc, char** argv)
{
  double b;
  double factor;
  double gamma;
  double g_0;
  double delta;
  double simplifier;

  // gamma for protons = 4257 Hz / gauss (Diffusion MRI, p 12, column 2, item 1)
  //                   = 4257 10^4 Hz / T  
  //                   = 4257 10^4 s^-1 T^-1
  // gamma for protons = 42.57 MHz / T (wikipedia proton gyromagnetic ratio)
  //                   = 4257 10^-2 MHz T^-1
  //                   = 4257 10^-2 10^6 s^-1 T^-1
  //                   = 4257 10^4 s^-1 T^-1

  gamma     = 4257e4;    // s^-1 T^-1

  // According to
  // http://www.healthcare.philips.com/main/products/mri/systems/achieva3t/
  // gradient amplitudes up to 80 mT/m
  // g_0 = 80 * 10^-3 T / m
  //     = 8 * 10^-2 * 10^-3 T / mm
  //     = 8 * 10^-5 T mm^-1
  //g_0 = 8e-5;

  cout << "proton gyromagnetic ratio = ";
  print_number ( gamma );
  cout << " s^-1 T^-1" << endl;
  //cout << "gradient amplitude        = ";
  //print_number ( g_0 );
  cout << " T mm^-1" << endl;
  cout << "(Common values: 0.01 < Delta < 0.5 s, 0.001 < delta < Delta)" << endl;
  // The file "method" from our capillary acquisition mentions:
  // LittleDelta = 3
  // BigDelta = 8

  // pulsed pair sequence:
  // b = gamma^2 G^2 delta^2 (Delta - delta/3)

  // Algorithm:  
  // Having b, gamma and the equation, find delta and Delta.
  // Let factor be such that Delta = factor * delta.

  // Inverted equation:
  // delta^2 * ( factor * delta - delta / 3 ) = b / ( gamma^2 * G^2 )
  // delta^2 * ( factor - 1/3 ) * delta = b / ( gamma^2 * G^2 )
  // delta^3 * ( factor - 1/3 ) = b / ( gamma^2 * G^2 )
  // delta^3 = b / ( gamma^2 * G^2 ) / ( factor - 1/3 )
  // delta^3 = b * ( factor - 1/3 ) / ( gamma^2 * G^2 )

  // simplifier = gamma * gamma * g_0 * g_0;

  // delta^3 = b * ( factor - 1/3 ) / simplifier 
  // delta = ( b * ( factor - 1/3 ) / simplifier )^(1/3)

  // for capillaries experiment:
  factor = 8.0 / 3.0;

  for ( b = 400.429797225526 ; b <= 500; b += 100 )
    {
      cout << "====> b = ";
      cout.precision ( 15 );
      cout << b;
      cout << " Delta = ";
      cout.precision ( 2 );
      cout << factor;
      cout << " * delta";
      cout << " <====" << endl;
      factor = 2.66;
      for ( g_0 = 1.24e-4; g_0 < 1.53e-4; g_0 += 1e-6 )
	{
	  simplifier = gamma * gamma * g_0 * g_0;
	  delta = calculate ( b, factor, simplifier );
	  cout.setf ( ios::scientific );
	  cout << "g_0 = " << g_0 << ", delta = ";
	  cout.unsetf ( ios::scientific );
	  print_number ( delta );
	  cout << " s, Delta = ";
	  print_number ( factor * delta );
	  cout << " s" << endl;
	}
    }

  return 0;
}

double calculate ( double b, double factor, double simplifier )
{
  double base;
  base = b * ( factor - 1 / 3.0 ) / simplifier;
  return pow ( base, ( 1 / 3.0 ) );  
}

void print_number ( double number )
{
  cout.setf ( ios::scientific );
  cout.setf ( ios::showpos );
  cout.precision ( 2 );
  cout.width ( 4 );
  cout << number;
  cout.unsetf ( ios::scientific );
  cout.unsetf ( ios::showpos );
}
		       
