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
#include <math.h>
#include <cstdlib>
#include <fstream>
#include <string>

#include "data_structures.hpp"
#include "pretty.hpp"

pretty prt;

typedef struct
{
  attributes attr;
  signal_t attenuated_signal;
} precomputed;

#define MAX_PRECOMPUTED 1000

double integration            ( double, double, double (*) (double) );
double naive_integration      ( double, double, double (*) (double) );
double f_uppercase            ( double );
double int_f_uppercase        ( double, double );
double f_uppercase_square     ( double );
double int_f_uppercase_square ( double, double );
double f_lowercase            ( void );
double g                      ( double );
double attenuation            ( void );
double b_value                ( void );
void   stejskal_tanner        ( void );

using namespace std;

struct parameters
{
  double sim_time_step;         // seconds
  double gamma;                 // radian second^-1 Tesla^-1
  double diffusion_coefficient; // m^2/second
  double gradient;              // T/m
  double gradient_zero;         // T/m
  double delta_lowercase;       // seconds
  double delta_uppercase;       // seconds
  double tau;                   // seconds
  double tau_prime;             // seconds: time when 1st echo amplitude is maximal
  double t1;                    // seconds: time when 1st gradient pulse occurs
};

parameters params;

int main(int argc, char** argv)
{
  attributes   data;
  double       dot_product_aux;
  bool         flag;
  double       conversion_aux;
  double       gradient_modulus;
  double       principal_direction_modulus;
  double_3d    gradient_direction;
  ifstream     sample_file;
  int          sample_file_size;
  int          index_last_value_stored;
  int          number_values_stored;
  int          sizeof_signal_t;
  ofstream     attenuated_out_file;
  precomputed  precomputed_values[MAX_PRECOMPUTED];
  signal_t     signal;
  signal_t     attenuated_signal;
  string       sample_filename;
  string       signal_output_filename;
  string       attenuated_output_filename;
  
  //prt.current_verbosity_level = verbosity_error;
  prt.current_verbosity_level = verbosity_status;
  //prt.current_verbosity_level = verbosity_debug;

  // preparation
  index_last_value_stored = 0;
  number_values_stored = 0;
  sizeof_signal_t = sizeof ( signal_t );
  prt.f ( verbosity_debug, "sizeof ( signal_t )   == %d.\n", sizeof_signal_t );

  // experimental values:
  params.gamma                 = 42.576;
  //params.gradient              = 0.000193009; // b = 0250
  //params.gradient              = 0.000272956; // b = 0500
  //params.gradient              = 0.000334302; // b = 0750
  params.gradient              = 0.000386019; // b = 1000

  params.gradient_zero         = 3;
  params.delta_lowercase       = 13.9e-3;
  params.delta_uppercase       = 23.8e-3;
  params.t1                    = 5e-3;
  //params.sim_time_step         = 1e-4; // ~ 460 steps
  params.tau                   = params.t1 + params.delta_lowercase + ( params.delta_uppercase - params.delta_lowercase ) / 2;   // guessing from sequence shape
  params.tau_prime             = params.tau * 2;
  //params.tau_prime             = 51.67e-3; // from "echo time" parameter in .REC file
  //params.tau                   = params.tau_prime / 2;
  
  // parse input
  if (argc < 7)
    {
      cout << "ERROR: too few arguments." << endl;
      cout << "USAGE: " << argv[0] << " input_file output_filename_prefix gradient_direction_x gradient_direction_y gradient_direction_z number_of_steps" << endl;
      exit (1);
    }
  sample_filename = argv[1];
  signal_output_filename = argv[2];
  attenuated_output_filename = signal_output_filename + "_att.raw";
  signal_output_filename += "_out.raw";
  gradient_direction.x = atof (argv[3]);
  gradient_direction.y = atof (argv[4]);
  gradient_direction.z = atof (argv[5]);
  params.sim_time_step = static_cast<double> ( params.tau_prime / atof ( argv[6] ) );

  // parameters
  sample_file.open (sample_filename.c_str (), ios::in | ios::binary);
  if ( ! sample_file )
    {
      cout << "ERROR: sample file doesn't exist!" << endl;
      exit (1);
    }

  gradient_modulus = gradient_direction.x * gradient_direction.x +
                     gradient_direction.y * gradient_direction.y +
                     gradient_direction.z * gradient_direction.z;
  gradient_modulus = sqrt (gradient_modulus);
  prt.f ( verbosity_information, "gradient_modulus = %+0.3f\n", gradient_modulus );
  // integrate stejskal-tanner equation
  attenuated_out_file.open (attenuated_output_filename.c_str (), ios::out | ios::binary);
  sample_file.seekg (0, ios::end);
  sample_file_size = sample_file.tellg ();
  prt.f ( verbosity_information, "sample size = %d\n", sample_file_size );
  sample_file.seekg (0, ios::beg);
  while ( sample_file.tellg () < sample_file_size )
    {
      flag = true;
      prt.f ( verbosity_information, "%d of %d: ", static_cast<int> ( sample_file.tellg () ), sample_file_size );
      sample_file.read ((char*) &data, sizeof(attributes));
      signal = data.signal;
      // check if value has already been computed
      for (int i = 0; i < number_values_stored; i++)
	{
	  if ( precomputed_values[i].attr.signal == data.signal &&
	       precomputed_values[i].attr.iso_adc == data.iso_adc &&
	       precomputed_values[i].attr.transverse_ratio == data.transverse_ratio &&
	       precomputed_values[i].attr.principal_direction.x  == data.principal_direction.x  &&
	       precomputed_values[i].attr.principal_direction.y  == data.principal_direction.y  &&
	       precomputed_values[i].attr.principal_direction.z  == data.principal_direction.z )
	    {
	      attenuated_signal = precomputed_values[i].attenuated_signal;
	      attenuated_out_file.write ((char*) &attenuated_signal, sizeof_signal_t );
	      flag = false;
	      break;
	    }
	}
      if ( flag ) // There is no equal entry on the lookup table.
	{
	  if ( gradient_direction.x == 0 &&
	       gradient_direction.y == 0 &&
	       gradient_direction.z == 0 ) // If we are considering the null direction...
	    {
	      params.diffusion_coefficient = 0;
	      attenuated_signal = signal;
	      attenuated_out_file.write ((char*) &attenuated_signal, sizeof_signal_t );
	    }
	  else // For new non-null entries...
	    {
	      // Calculate effective ADC
	      // ADC_eff = E . P + E . T
	      //         = E . P + |E||T|senA, A = acos ( E . P / |E||P| )

	      dot_product_aux = gradient_direction.x * data.principal_direction.x +
		gradient_direction.y * data.principal_direction.y +
		gradient_direction.z * data.principal_direction.z;

	      principal_direction_modulus = sqrt ( data.principal_direction.x * data.principal_direction.x +
						   data.principal_direction.y * data.principal_direction.y +
						   data.principal_direction.z * data.principal_direction.z );

	      params.diffusion_coefficient = dot_product_aux +
		gradient_modulus * ( principal_direction_modulus * data.transverse_ratio ) *
		sin ( acos ( dot_product_aux / ( gradient_modulus * principal_direction_modulus ) ) );
	      
	      if ( params.diffusion_coefficient < 0 )
		params.diffusion_coefficient *= -1;

	      // calculate the attenuation
	      conversion_aux = exp ( attenuation () ) * static_cast<double> ( signal );

	      // Verify the result fits the data type:
	      if ( conversion_aux >   pow ( 2, 8 * sizeof_signal_t - 1 ) || 
		   conversion_aux < - pow ( 2, 8 * sizeof_signal_t - 1 ) )
		{
		  prt.f ( verbosity_error, "ERROR: signal outside bounds of sizeof_signal_t\n" );
		  exit ( 1 );
		}
	      attenuated_signal = static_cast<signal_t> ( conversion_aux );
	      attenuated_out_file.write ((char*) &attenuated_signal, sizeof_signal_t);
	    }
	  precomputed_values[index_last_value_stored].attenuated_signal = attenuated_signal;
	  precomputed_values[index_last_value_stored].attr.signal = data.signal;
	  precomputed_values[index_last_value_stored].attr.transverse_ratio = data.transverse_ratio;
	  precomputed_values[index_last_value_stored].attr.iso_adc = data.iso_adc;
	  precomputed_values[index_last_value_stored].attr.principal_direction.x = data.principal_direction.x;
	  precomputed_values[index_last_value_stored].attr.principal_direction.y = data.principal_direction.y;
	  precomputed_values[index_last_value_stored].attr.principal_direction.z = data.principal_direction.z;
	  if (index_last_value_stored < MAX_PRECOMPUTED - 1)
	    index_last_value_stored++;
	  else
	    index_last_value_stored = 0;
	  if (number_values_stored < MAX_PRECOMPUTED - 1)
	    number_values_stored++;
	}
      prt.f ( verbosity_debug, "%+0.3f => %+0.3f", static_cast<double> ( signal ), static_cast<double> ( attenuated_signal ) );
      if ( flag )
	prt.f ( verbosity_information, " (new)\n" );
      else
	prt.f ( verbosity_information, " (cached)\n" );
    }

  // print stored values
  for (int i = 0; i < number_values_stored; i++)
    {
      prt.f ( verbosity_information, "%d: %+0.3f ", i, static_cast<double> (precomputed_values[i].attr.signal) );
      prt.f ( verbosity_information, "%+0.3f,", precomputed_values[i].attr.principal_direction.x ); 
      prt.f ( verbosity_information, "%+0.3f,", precomputed_values[i].attr.principal_direction.y );
      prt.f ( verbosity_information, "%+0.3f => ", precomputed_values[i].attr.principal_direction.z );
      prt.f ( verbosity_information, "%+0.3f\n", static_cast<double> (precomputed_values[i].attenuated_signal) );
    }
  attenuated_out_file.close ();
  sample_file.close ();
  return 0;
}

double attenuation (void)
{
  double f_value, coefficient, term1, term2, term3, result;
  f_value = f_lowercase ();
  coefficient = - params.gamma * params.gamma * params.diffusion_coefficient;
  term1 = int_f_uppercase_square (0, params.tau_prime);
  term2 = - 4 * f_value * int_f_uppercase (params.tau, params.tau_prime);
  term3 = 4 * pow (f_value, 2) * (params.tau_prime - params.tau);
  result = coefficient * (term1 + term2 + term3);
  return result;
}

double integration (double lower_bound, double upper_bound, double (*function) (double))
{
  double result;
  result = naive_integration (lower_bound, upper_bound, function);
  return result;
}

double naive_integration (double lower_bound, double upper_bound, double (*function) (double))
{
  double seconds;
  double result = 0;
  for (seconds = lower_bound + params.sim_time_step; seconds <= upper_bound; seconds += params.sim_time_step)
    result += function (seconds) * params.sim_time_step;
  return result;
}

double f_lowercase (void) // for the 180 degrees case
{
  return (params.gradient_zero + params.gradient) * params.delta_lowercase 
    + params.gradient_zero * (params.tau - params.delta_lowercase);
}

double g (double time)
{
  if ((0 <= time && time < params.t1) ||
      (params.t1 + params.delta_lowercase < time && time < (params.t1 + params.delta_uppercase) && (params.t1 + params.delta_uppercase) > params.tau) ||
      (params.t1 + params.delta_lowercase + params.delta_uppercase < time))
    return params.gradient_zero;
  if ((params.t1 <= time && time < (params.t1 + params.delta_lowercase) && (params.t1 + params.delta_lowercase) < params.tau) ||
      (params.t1 + params.delta_uppercase < time && time < (params.t1 + params.delta_lowercase + params.delta_uppercase) && (params.t1 + params.delta_lowercase + params.delta_uppercase) < 2 * params.tau))
    return params.gradient_zero + params.gradient;
  cout << "ERROR: G (" << time << ") undefined." << endl;
  exit (1);
}

double f_uppercase (double upper_bound)
{
  return integration (0, upper_bound, &g);
}

double int_f_uppercase (double lower_bound, double upper_bound)
{
  return integration (lower_bound, upper_bound, &f_uppercase);
}

double f_uppercase_square (double upper_bound)
{
  return pow (f_uppercase (upper_bound), 2);
}

double int_f_uppercase_square (double lower_bound, double upper_bound)
{
  return integration (lower_bound, upper_bound, &f_uppercase_square);
}

double b_value (void)
{
  return 0;
}

