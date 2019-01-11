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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

int main (int argc, char** argv)
{
  char         buffer[256];
  ifstream     b0_file;
  ifstream     remainder_file;
  int          b0_file_size;
  int          file_size;
  int          length;
  int          output_file_size;
  int          remainder_file_size;
  fstream      output_file;
  string       b0_file_name;
  string       output_file_name;
  string       remainder_file_name;
  stringstream string_buffer;
  
  if (argc < 4)
    {
      cout << "ERROR: too few arguments." << endl;
      cout << "USAGE: " << argv[0] << " b0_file_name remainder_file_name output_file_name" << endl;
      exit (1);
    }

  b0_file_name        = argv[1];
  remainder_file_name = argv[2];
  output_file_name    = argv[3];

  // sanity
  b0_file.open ( b0_file_name.c_str () );
  b0_file.seekg ( 0, ios::end );
  b0_file_size = b0_file.tellg ();
  remainder_file.open ( remainder_file_name.c_str () );
  remainder_file.seekg ( 0, ios::end );
  remainder_file_size = remainder_file.tellg ();
  output_file_size = b0_file_size + remainder_file_size;

  // copy b0 to beginning of output file
  output_file.open ( output_file_name.c_str (), fstream::binary | fstream::trunc | fstream::in | fstream::out );
  output_file.seekp ( 0, ios::beg );
  b0_file.seekg ( 0, ios::beg );
  while ( b0_file.tellg () < b0_file_size )
    {
      length = 2;
      b0_file.read (buffer, length);
      output_file.write (buffer, length);
    }
  
  // append remainder to output file
  remainder_file.seekg ( 0, ios::beg );
  while ( remainder_file.tellg () < remainder_file_size )
    {
      length = 2;
      remainder_file.read (buffer, length);
      output_file.write (buffer, length);
    }

  // cleanup
  b0_file.close ();
  remainder_file.close ();

  // sanity
  output_file.seekg ( 0, ios::end );
  file_size = output_file.tellg ();
  output_file.close ();
  cout << "Expected output file size: " << output_file_size << endl;
  cout << "Output file size:          " << file_size << " ";
  if ( file_size != output_file_size )
    cout << "FAIL" << endl;
  else
    cout << "OK" << endl;

  return 0;
}
