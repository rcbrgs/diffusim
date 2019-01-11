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
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

int main (int argc, char** argv)
{
  char         buffer[128];
  fstream      remainder_file;
  ifstream     input_file;
  int          expected_file_size;      // in bytes
  int          file_size;               // in bytes
  int          input_file_size;
  int          output_pixel_size;
  int          number_of_b_values;
  int          number_of_directions;
  int          number_of_slices;
  int          number_of_b0_components;
  int          offset_b0;
  int          output_file_size;        // in bytes
  int          input_pixel_size;              // in bytes
  int          remainder_file_size;     // in bytes
  int          output_remainder_file_size;     // in bytes
  int          size_x_dimension;        // in pixels
  int          size_y_dimension;        // in pixels
  ofstream     output_file;
  string       input_file_name;
  string       output_file_name;
  string       remainder_file_name;
  
  // sanity
  if ( argc < 11 )
    {
      cout << "ERROR" << endl;
      cout << "USAGE: " << argv[0] << " input_file_name number_of_b_values number_of_directions(including trace, if any) number_of_slices input_pixel_size(in bytes) number_of_b0_components size_x_dimension(in pixels) size_y_dimension(in pixels) output_file_name remainder_file_name output_pixel_size(in bytes)" << endl;
      exit (1);
    }

  // parsing input
  input_file_name         =        argv[1];
  number_of_b_values      = atoi ( argv[2] );
  number_of_directions    = atoi ( argv[3] );
  number_of_slices        = atoi ( argv[4] );
  input_pixel_size        = atoi ( argv[5] );
  number_of_b0_components = atoi ( argv[6] );
  size_x_dimension        = atoi ( argv[7] );
  size_y_dimension        = atoi ( argv[8] );
  output_file_name        =        argv[9];
  remainder_file_name     =        argv[10];
  output_pixel_size       = atoi ( argv[11] );

  // check that sizes match
  offset_b0 = number_of_b0_components * number_of_slices * input_pixel_size * size_x_dimension * size_y_dimension;
  remainder_file_size = number_of_b_values * number_of_slices * input_pixel_size * size_x_dimension * size_y_dimension * number_of_directions;
  output_remainder_file_size = number_of_b_values * number_of_slices * output_pixel_size * size_x_dimension * size_y_dimension * number_of_directions;
  expected_file_size = offset_b0 + remainder_file_size;

  cout << "Expected b0 size:         " << offset_b0 << endl;
  cout << "Expected remainder size:  " << remainder_file_size << endl;
  cout << "Expected input file size: " << expected_file_size << endl;

  input_file.open (input_file_name.c_str ());
  input_file.seekg (0, ios::end);
  input_file_size = input_file.tellg ();

  cout << "Input file size:          " << input_file_size << " ";
  if ( input_file_size != expected_file_size )
    {
      cout << "FAIL" << endl;
      input_file.close ();
      exit (1);
    }
  else
    cout << "OK" << endl;

  output_file.open (output_file_name.c_str (), fstream::binary | fstream::out | fstream::trunc );

  // copy the first direction (which encodes B0)
  input_file.seekg(0, ios::beg);
  while (input_file.tellg () < offset_b0)
    {
      input_file.read   (buffer, input_pixel_size);
      output_file.write (buffer, output_pixel_size);
    }
  output_file.close ();

  // copy the remainder (everything but the B0)
  remainder_file.open (remainder_file_name.c_str (), fstream::binary | fstream::out | fstream::in | fstream::trunc );
  if ( ! remainder_file )
    {
      cout << "ERROR opening remainder_file." << endl;
      exit (1);
    }

  // since the file is already at offset_b0 position...
  while (input_file.tellg () < input_file_size )
    {
      input_file.read      ( buffer, input_pixel_size );
      remainder_file.write ( buffer, output_pixel_size );
    }
  input_file.close ();

  // verify remainder file size
  remainder_file.seekg ( 0, ios::end );
  file_size = remainder_file.tellg ();
  remainder_file.close ();
  //cout << "Expected remainder file size: " << remainder_file_size << endl;
  cout << "Remainder file size:      " << file_size << " ";
  if ( file_size != output_remainder_file_size )
    {
      cout << "FAIL" << endl;
      exit (1);
    }
  else
    cout << "OK" << endl;

  // verify output file size
  expected_file_size = number_of_b0_components * number_of_slices * output_pixel_size * size_x_dimension * size_y_dimension;
  input_file.open (output_file_name.c_str ());
  input_file.seekg (0, ios::end);
  output_file_size = input_file.tellg ();
  //cout << "Expected output file size: " << expected_file_size << endl;
  cout << "Output file size:         " << output_file_size << " ";
  if ( output_file_size != expected_file_size )
    {
      cout << "FAIL" << endl;
      input_file.close ();
      exit (1);
    }
  else
    cout << "OK" << endl;

  input_file.close ();
  return 0;
}
