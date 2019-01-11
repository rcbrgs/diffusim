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
#include <sstream>

using namespace std;

int main (int argc, char** argv)
{
  char         buffer[128];
  fstream      output_file[256];
  ifstream     input_file;
  int          b_value_block_size;      // in bytes
  int          direction_block_size;    // in bytes
  int          file_size;               // in bytes
  int          input_file_size;
  int          length;
  int          number_of_b_values;
  int          number_of_directions;
  int          number_of_slices;
  int          offset;
  int          offset_beg;
  int          offset_end;
  int          output_file_size;        // in bytes
  int          pixel_size;              // in bytes
  int          size_x_dimension;        // in pixels
  int          size_y_dimension;        // in pixels
  string       file_name;
  string       input_file_name;
  string       output_file_name_prefix;
  stringstream streambuf;
    
  // sanity
  if ( argc < 9 )
    {
      cout << "ERROR" << endl;
      cout << "USAGE: " << argv[0] << " input_file_name number_of_b_values number_of_directions number_of_slices pixel_size(in bytes) size_x_dimension(in pixels) size_y_dimension(in pixels) output_file_name_prefix" << endl;
      exit (1);
    }

  // parsing input
  input_file_name          =        argv[1];
  number_of_b_values       = atoi ( argv[2] );
  number_of_directions     = atoi ( argv[3] );
  number_of_slices         = atoi ( argv[4] );
  pixel_size               = atoi ( argv[5] );
  size_x_dimension         = atoi ( argv[6] );
  size_y_dimension         = atoi ( argv[7] );
  output_file_name_prefix  =        argv[8];
  if ( number_of_b_values >= 256 )
    {
      cout << "ERROR: maximum of 256 b-values." << endl;
      exit (1);
    }

  // compute file sizes
  b_value_block_size   = number_of_slices * pixel_size * size_x_dimension * size_y_dimension;
  direction_block_size = b_value_block_size   * number_of_b_values;
  output_file_size     = b_value_block_size   * number_of_directions;
  input_file_size      = direction_block_size * number_of_directions;

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

  // sanity
  for (int i=0; i < number_of_b_values; i++)
    {
      streambuf.seekp (0);
      streambuf.width (3);
      streambuf.fill ('0');
      streambuf << i;
      file_name = output_file_name_prefix;
      file_name += streambuf.str ();
      file_name += ".raw";
      output_file[i].open (file_name.c_str (), fstream::binary | fstream::trunc | fstream::in | fstream::out );
    }

  // for each direction...
  for (int i = 0; i < number_of_directions; i++)
    {
      cout << "Direction " << i << ":";
      offset = direction_block_size * i;
      // get each b-value...
      for (int j = 0; j < number_of_b_values; j++)
	{
	  cout << " b-value " << j;
	  offset_beg = offset + b_value_block_size * j;
	  offset_end = offset_beg + b_value_block_size;
	  input_file.seekg(offset_beg, ios::beg);
	  // and save it in the right file.
	  while (input_file.tellg () < offset_end)
	    {
	      length = 2;
	      input_file.read (buffer, length);
	      output_file[j].write (buffer, length);
	    }
	}
      cout << endl;
    }

  input_file.close ();

  // verify output file size
  for ( int i = 0; i < number_of_b_values; i++ )
    {
      output_file[i].seekg ( 0, ios::end );
      file_size = output_file[i].tellg ();
      output_file[i].close ();
      cout << "Expected output " << i << " file size: " << output_file_size << endl;
      cout << "Output " << i << " file size:          " << file_size << " ";
      if ( file_size != output_file_size )
	cout << "FAIL" << endl;
      else
	cout << "OK" << endl;
    }

  return 0;
}
