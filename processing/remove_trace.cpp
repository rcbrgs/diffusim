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
  fstream      traces_file;
  fstream      remainder_file;
  ifstream     input_file;
  int          file_size;               // in bytes
  int          input_file_size;
  int          length;
  int          number_of_b_values;
  int          number_of_directions;
  int          number_of_slices;
  int          number_of_traces;
  int          traces_file_size;        // in bytes
  int          pixel_size;              // in bytes
  int          remainder_file_size;     // in bytes
  int          size_x_dimension;        // in pixels
  int          size_y_dimension;        // in pixels
  string       input_file_name;
  string       traces_file_name;
  string       remainder_file_name;
  
  // sanity
  if ( argc < 11 )
    {
      cout << "ERROR" << endl;
      cout << "USAGE: " << argv[0] << " input_file_name number_of_b_values number_of_directions(including trace) number_of_traces number_of_slices pixel_size(in bytes) size_x_dimension(in pixels) size_y_dimension(in pixels) traces_file_name remainder_file_name" << endl;
      exit (1);
    }

  // parsing input
  input_file_name         =        argv[1];
  number_of_b_values      = atoi ( argv[2] );
  number_of_directions    = atoi ( argv[3] );
  number_of_traces        = atoi ( argv[4] );
  number_of_slices        = atoi ( argv[5] );
  pixel_size              = atoi ( argv[6] );
  size_x_dimension        = atoi ( argv[7] );
  size_y_dimension        = atoi ( argv[8] );
  traces_file_name        =        argv[9];
  remainder_file_name     =        argv[10];

  // compute file sizes
  input_file_size     = number_of_b_values * number_of_directions * number_of_slices * pixel_size * size_x_dimension * size_y_dimension;
  traces_file_size    = number_of_b_values * number_of_traces     * number_of_slices * pixel_size * size_x_dimension * size_y_dimension;
  remainder_file_size = input_file_size - traces_file_size;

  // check input size
  input_file.open ( input_file_name.c_str() );
  input_file.seekg (0, ios::end);
  file_size = input_file.tellg ();

  cout << "Expected input file size: " << input_file_size << endl;
  cout << "Input file size:          " << file_size << " ";
  if ( input_file_size != file_size )
    {
      cout << "FAIL" << endl;
      input_file.close ();
      exit (1);
    }
  else
    cout << "OK" << endl;

  // copy the last directions (which encodes the traces)
  traces_file.open (traces_file_name.c_str (), fstream::binary | fstream::trunc | fstream::in | fstream::out );
  input_file.seekg(remainder_file_size, ios::beg);
  length = 2;
  while (input_file.tellg () < input_file_size)
    {
      input_file.read   (buffer, length);
      traces_file.write (buffer, length);
    }

  // copy the remainder (everything but the traces)
  remainder_file.open (remainder_file_name.c_str (), fstream::binary | fstream::out | fstream::in | fstream::trunc );
  if ( ! remainder_file )
    {
      cout << "ERROR opening remainder_file." << endl;
      exit (1);
    }
  input_file.seekg(0, ios::beg);
  length = 2;
  while (input_file.tellg () < remainder_file_size )
    {
      input_file.read      ( buffer, length );
      remainder_file.write ( buffer, length );
    }
  input_file.close ();

  // verify traces file size
  traces_file.seekg (0, ios::end);
  file_size = traces_file.tellg ();
  traces_file.close ();
  cout << "Expected traces file size: " << traces_file_size << endl;
  cout << "Traces file size:          " << file_size << " ";
  if ( traces_file_size != file_size )
    {
      cout << "FAIL" << endl;
      exit (1);
    }
  else
    cout << "OK" << endl;

  // verify remainder file size
  remainder_file.seekg ( 0, ios::end );
  file_size = remainder_file.tellg ();
  remainder_file.close ();
  cout << "Expected remainder file size: " << remainder_file_size << endl;
  cout << "Remainder file size:          " << file_size << " ";
  if ( file_size != remainder_file_size )
    {
      cout << "FAIL" << endl;
      exit (1);
    }
  else
    cout << "OK" << endl;

  return 0;
}
