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
  ifstream input_file;
  ofstream output_file;
  stringstream string_buffer;
  string file_name;
  string input_suffix;
  int number_of_blocks;
  int file_size;
  char* buffer;
  int output_size;

  if (argc < 4)
    {
      cout << "ERROR: too few arguments." << endl;
      cout << "USAGE: " << argv[0] << " output_file_name input_files_suffix number_of_files" << endl;
      exit (1);
    }

  file_name = argv[1];
  input_suffix = argv[2];
  number_of_blocks = atoi (argv[3]);

  output_file.open (file_name.c_str ());
  buffer = new char;
  output_size = 0;

  for (int i = 0; i < number_of_blocks; i++)
    {
      //cout << output_size << " ";
      //      file_name = input_suffix;
      file_name = "";
      string_buffer.seekp (0);
      string_buffer.width (3);
      string_buffer.fill ('0');
      string_buffer << i;
      file_name += string_buffer.str ();
      file_name += input_suffix;
      file_name += ".raw";
      //cout << file_name << endl;
      input_file.open (file_name.c_str ());
      input_file.seekg(0,ios::end);
      file_size = input_file.tellg ();
      input_file.seekg(0,ios::beg);
      output_file.seekp(output_size,ios::beg);
      while (input_file.tellg () < file_size)
	{
	  input_file.read (buffer, sizeof (char));
	  output_file.write (buffer, sizeof (char));
	}
      input_file.close ();
      output_size += file_size;
      //cout << " " << output_size << endl;
    }
  
  delete buffer;
  output_file.close ();
  return 0;
}
