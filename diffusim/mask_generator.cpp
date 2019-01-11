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

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <mxml.h>

#include "data_structures.hpp"
#include "pretty.hpp"

using namespace std;

pretty prt;

double_3d get_square_vertex         ( double_3d, double_3d );
double_3d get_tangent_versor        ( double_3d, double_3d );
void      generate_random_versor    ( double_3d* );
void      generate_uncertain_versor ( double_3d* versor, const unsigned int uncertainty_percentage );
void      set_value_from_node       ( node_pointer*, const string, double*       );
void      set_value_from_node       ( node_pointer*, const string, node_pointer* );
void      set_value_from_node       ( node_pointer*, const string, short*        );
void      set_value_from_node       ( node_pointer*, const string, signal_t*     );
void      set_value_from_node       ( node_pointer*, const string, string*       );
void      set_value_from_node       ( node_pointer*, const string, unsigned int* );

int main (int argc, char** argv )
{
  attributes                 data;
  cylinder_with_aniso_adc*   buffer_cylinder_aniso;
  cylinder_with_iso_adc*     buffer_cylinder_iso;
  cylinder_with_tangent_adc* buffer_cylinder_tan;
  double                     distance_to_center;
  double                     temp;
  double_3d                  center;
  double_3d                  point;
  fstream                    buffer_file;
  fstream                    mask_file;
  ifstream                   phantom_file;
  int                        offset;
  int                        offset_phantom;
  node_pointer*              layer_node;
  node_pointer*              object_node;
  node_pointer*              tree;
  object*                    object_buffer;
  ofstream                   out_file;
  rectangle*                 rectangle_buffer;
  sample                     xml_sample;
  signal_t                      phantom_signal;
  string                     buffer;
  string                     filename;
  string                     geometry;
  string                     raw_file_name;
  string                     xml_file_name;
  stringstream               name_counter;
  stringstream               buffer_sstream;
  unsigned int               current_layer;
  unsigned int               current_object;
  unsigned int               direction_uncertainty_percentage;
  unsigned int               max_x;
  unsigned int               max_y;
  unsigned int               max_z;
  unsigned int               object_x;
  unsigned int               object_y;
  unsigned int               object_z;
  FILE*                      fp;

  if ( argc < 3 )
    {
      cout << "USAGE: " << argv[0] << " raw_file_name xml_file_name direction_uncertainty_percentage" << endl;
      exit (1);
    }

  prt.current_verbosity_level = verbosity_status;
  //prt.current_verbosity_level = verbosity_information;
  //prt.current_verbosity_level = verbosity_debug;
  
  raw_file_name                    = argv[1];
  xml_file_name                    = argv[2];
  direction_uncertainty_percentage = atoi ( argv[3] );

  prt.f ( verbosity_information, "direction_uncertainty_percentage = %d\n", direction_uncertainty_percentage );

  max_x = 0;
  max_y = 0;
  max_z = 0;

  layer_node  = new node_pointer;
  object_node = new node_pointer;
  tree        = new node_pointer;

  fp = fopen(xml_file_name.c_str (), "r");
  tree->pointer = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
  fclose(fp);

  set_value_from_node ( tree, "number_of_layers", &( xml_sample.number_of_layers ) );
  xml_sample.layers = new canvas[xml_sample.number_of_layers];
  cout << "Specification defines " << xml_sample.number_of_layers << " layers." << endl;

  // for each layer...
  for (unsigned int l = 0; l < xml_sample.number_of_layers; l++)
    {
      buffer_sstream.str ("");
      buffer_sstream << l;
      layer_node->pointer = mxmlFindElement ( tree->pointer, tree->pointer, "layer", "number", buffer_sstream.str ().c_str () , MXML_DESCEND );
      current_layer = l;
      set_value_from_node ( layer_node, "number_of_objects", &( xml_sample.layers[current_layer].number_of_objects ) );
      xml_sample.layers[current_layer].objects = new object[ xml_sample.layers[current_layer].number_of_objects ];
      prt.f ( verbosity_information, "Parsing layer %d:\n", current_layer );
      prt.indentation++;
      // for each object...
      for ( unsigned int o = 0; o < xml_sample.layers[l].number_of_objects; o++)
	{
	  // Parse it.
	  buffer_sstream.str ("");
	  buffer_sstream << o;
	  object_node->pointer = mxmlFindElement ( layer_node->pointer, layer_node->pointer, "object", "number", buffer_sstream.str ().c_str () , MXML_DESCEND );
	  object_buffer = new object;
	  current_object = o;
	  set_value_from_node ( object_node, "geometry_type", &( geometry ) );
	  prt.f ( verbosity_information, "Parsing object %d:\n", current_object );
	  prt.indentation++;
	  prt.f ( verbosity_information, "Geometry: %s\n", geometry.c_str () );
	  if ( geometry == "cylinder_with_aniso_adc" )
	    {
	      buffer_cylinder_aniso = new cylinder_with_aniso_adc;
	      set_value_from_node ( object_node, "iso_adc",                &( buffer_cylinder_aniso->voxel.iso_adc ) );
	      set_value_from_node ( object_node, "center_x",               &( buffer_cylinder_aniso->center.x ) );
	      set_value_from_node ( object_node, "center_y",               &( buffer_cylinder_aniso->center.y ) );
	      set_value_from_node ( object_node, "center_z",               &( buffer_cylinder_aniso->center.z ) );
	      set_value_from_node ( object_node, "radius",                 &( buffer_cylinder_aniso->radius ) );
	      set_value_from_node ( object_node, "principal_direction_x",  &( buffer_cylinder_aniso->voxel.principal_direction.x ) );
	      set_value_from_node ( object_node, "principal_direction_y",  &( buffer_cylinder_aniso->voxel.principal_direction.y ) );
	      set_value_from_node ( object_node, "principal_direction_z",  &( buffer_cylinder_aniso->voxel.principal_direction.z ) );
	      set_value_from_node ( object_node, "threshold_low",          &( buffer_cylinder_aniso->signal_threshold_low  ) );
	      set_value_from_node ( object_node, "threshold_high",         &( buffer_cylinder_aniso->signal_threshold_high ) );
	      set_value_from_node ( object_node, "transverse_ratio",       &( buffer_cylinder_aniso->voxel.transverse_ratio ) );

	      object_buffer->type           = cylinder_with_aniso_adc_type;
	      object_buffer->object_pointer = static_cast<void*> ( buffer_cylinder_aniso );
	      xml_sample.layers[current_layer].objects[current_object] = *object_buffer;

	      object_x = buffer_cylinder_aniso->center.x + buffer_cylinder_aniso->radius;
	      object_y = buffer_cylinder_aniso->center.y + buffer_cylinder_aniso->radius;
	      object_z = 0;
	    }
	  if ( geometry == "cylinder_with_iso_adc" )
	    {
	      buffer_cylinder_iso = new cylinder_with_iso_adc;
	      set_value_from_node ( object_node, "iso_adc",                &( buffer_cylinder_iso->voxel.iso_adc ) );
	      set_value_from_node ( object_node, "center_x",               &( buffer_cylinder_iso->center.x ) );
	      set_value_from_node ( object_node, "center_y",               &( buffer_cylinder_iso->center.y ) );
	      set_value_from_node ( object_node, "center_z",               &( buffer_cylinder_iso->center.z ) );
	      set_value_from_node ( object_node, "radius",                 &( buffer_cylinder_iso->radius ) );
	      set_value_from_node ( object_node, "transverse_ratio",       &( buffer_cylinder_iso->voxel.transverse_ratio ) );

	      object_buffer->type           = cylinder_with_iso_adc_type;
	      object_buffer->object_pointer = static_cast<void*> ( buffer_cylinder_iso );
	      xml_sample.layers[current_layer].objects[current_object] = *object_buffer;

	      object_x = buffer_cylinder_iso->center.x + buffer_cylinder_iso->radius;
	      object_y = buffer_cylinder_iso->center.y + buffer_cylinder_iso->radius;
	      object_z = 0;
	    }
	  if ( geometry == "cylinder_with_tangent_adc" )
	    {
	      buffer_cylinder_tan = new cylinder_with_tangent_adc;
	      set_value_from_node ( object_node, "iso_adc",                &( buffer_cylinder_tan->voxel.iso_adc ) );
	      set_value_from_node ( object_node, "center_x",               &( buffer_cylinder_tan->center.x ) );
	      set_value_from_node ( object_node, "center_y",               &( buffer_cylinder_tan->center.y ) );
	      set_value_from_node ( object_node, "center_z",               &( buffer_cylinder_tan->center.z ) );
	      set_value_from_node ( object_node, "radius",                 &( buffer_cylinder_tan->radius ) );
	      set_value_from_node ( object_node, "transverse_ratio",       &( buffer_cylinder_tan->voxel.transverse_ratio ) );

	      object_buffer->type           = cylinder_with_tangent_adc_type;
	      object_buffer->object_pointer = static_cast<void*> (buffer_cylinder_tan);
	      xml_sample.layers[current_layer].objects[current_object] = *object_buffer;

	      object_x = buffer_cylinder_tan->center.x + buffer_cylinder_tan->radius;
	      object_y = buffer_cylinder_tan->center.y + buffer_cylinder_tan->radius;
	      object_z = 0;
	    }
	  if ( geometry == "rectangle" )
	    {
	      rectangle_buffer               = new rectangle;
	      set_value_from_node ( object_node, "iso_adc",                &( rectangle_buffer->voxel.iso_adc ) );
	      set_value_from_node ( object_node, "size_x",                 &( rectangle_buffer->size.x ) );
	      set_value_from_node ( object_node, "size_y",                 &( rectangle_buffer->size.y ) );
	      set_value_from_node ( object_node, "size_z",                 &( rectangle_buffer->size.z ) );
	      set_value_from_node ( object_node, "origin_x",               &( rectangle_buffer->origin.x ) );
	      set_value_from_node ( object_node, "origin_y",               &( rectangle_buffer->origin.y ) );
	      set_value_from_node ( object_node, "origin_z",               &( rectangle_buffer->origin.z ) );
	      set_value_from_node ( object_node, "diffusion_type",         &( buffer ) );
	      
	      if ( buffer == "isotropic" )
		{
		  rectangle_buffer->diffusion = isotropic;
		}
	      else
		{
		  rectangle_buffer->diffusion = single_direction;
		  set_value_from_node ( object_node, "principal_direction_x",  &( rectangle_buffer->voxel.principal_direction.x ) );
		  set_value_from_node ( object_node, "principal_direction_y",  &( rectangle_buffer->voxel.principal_direction.y ) );
		  set_value_from_node ( object_node, "principal_direction_z",  &( rectangle_buffer->voxel.principal_direction.z ) );
		}
	      set_value_from_node ( object_node, "transverse_ratio",        &( rectangle_buffer->voxel.transverse_ratio ) );
	      
	      object_buffer->type           = rectangle_type;
	      object_buffer->object_pointer = static_cast<void*> (rectangle_buffer);
	      xml_sample.layers[current_layer].objects[current_object] = *object_buffer;

	      object_x = rectangle_buffer->size.x;
	      object_y = rectangle_buffer->size.y;
	      object_z = rectangle_buffer->size.z;
	    }

	  prt.f ( verbosity_information, "object_x = %d\n", object_x );
	  prt.f ( verbosity_information, "object_y = %d\n", object_y );
	  prt.f ( verbosity_information, "object_z = %d\n", object_z );

	  if ( object_x > max_x )
	    max_x = object_x;
	  if ( object_y > max_y )
	    max_y = object_y;
	  if ( object_z > max_z )
	    max_z = object_z;
	  prt.indentation--;
	}
      prt.indentation--;
    }

  prt.f ( verbosity_information, "max_x = %d\n", max_x );
  prt.f ( verbosity_information, "max_y = %d\n", max_y );
  prt.f ( verbosity_information, "max_z = %d\n", max_z );

  // zero the buffer
  prt.f ( verbosity_information, "Creating zero-filled buffer file...\n" );
  buffer_file.open ("buffer.raw", ios::out | ios::binary | ios::in | ios::trunc );
  if ( ! buffer_file )
    {
      cout << "ERROR: cannot open buffer_file." << endl;
      exit (1);
    }

  for (unsigned int x = 0; x < max_x; x++)
    for (unsigned int y = 0; y < max_y; y++)
      for (unsigned int z = 0; z < max_z; z++)
	{
	  data.iso_adc                = 0;
	  data.principal_direction.x  = 0;
	  data.principal_direction.y  = 0;
	  data.principal_direction.z  = 0;
	  data.signal                 = 0;
	  data.transverse_ratio       = 0;
	  buffer_file.write ( (char*) &data, sizeof ( attributes ) );	  
	}

  phantom_file.open ( raw_file_name.c_str (), ios::in | ios::binary );

  // generate sample in buffer
  prt.f ( verbosity_status, "Generating mask in the buffer...\n" );
  prt.f ( verbosity_information, "Mask has %d layers.\n", xml_sample.number_of_layers );
  for (unsigned int l = 0; l < xml_sample.number_of_layers; l++)
    {
      prt.f ( verbosity_status, "Generating layer %d ... of %d\n", l + 1, xml_sample.number_of_layers );
      prt.f ( verbosity_information, "Layer has %d objects.\n", xml_sample.layers[l].number_of_objects );
      for (unsigned int o = 0; o < xml_sample.layers[l].number_of_objects; o++)
	{
	  prt.f ( verbosity_status, "Generating object %d ...\n", o );
	  object_buffer = &(xml_sample.layers[l].objects[o]);
	  // rectangular prism object:
	  if (object_buffer->type == rectangle_type)
	    {
	      prt.f ( verbosity_information, "Object %d is a rectangle.\n", o );
	      rectangle_buffer = static_cast<rectangle*> (object_buffer->object_pointer);
	      unsigned int x = rectangle_buffer->origin.x;
	      x++;
	      for (unsigned int x = rectangle_buffer->origin.x; x < (rectangle_buffer->origin.x + rectangle_buffer->size.x); x++)
		{
		  for (unsigned int y = rectangle_buffer->origin.y; y < (rectangle_buffer->origin.y + rectangle_buffer->size.y); y++)
		    for (unsigned int z = rectangle_buffer->origin.z; z < (rectangle_buffer->origin.z + rectangle_buffer->size.z); z++)
		      {
			data.iso_adc = rectangle_buffer->voxel.iso_adc;
			if ( rectangle_buffer->diffusion == isotropic )
			  {
			    generate_random_versor ( &( data.principal_direction ) );
			  }
			if ( rectangle_buffer->diffusion == single_direction )
			  {
			    data.principal_direction.x = rectangle_buffer->voxel.principal_direction.x;
			    data.principal_direction.y = rectangle_buffer->voxel.principal_direction.y;
			    data.principal_direction.z = rectangle_buffer->voxel.principal_direction.z;
			  }
			data.transverse_ratio = rectangle_buffer->voxel.transverse_ratio;
			// read signal from file
			offset = z * max_x * max_y;
			offset += y * max_y;
			offset += x;
			offset_phantom = offset * sizeof( signal_t );
			offset *= sizeof ( attributes );
			phantom_file.seekg (offset_phantom, ios::beg);
			phantom_file.read ((char*) &phantom_signal, sizeof ( signal_t ));
			data.signal = phantom_signal;
			// save data on file
			buffer_file.seekp (offset, ios::beg);
			buffer_file.write ((char*) &data, sizeof (attributes));
		      }
		}
	    }
	  // generate cylinder with anisotrpic diffusion, within a threshold
	  if ( object_buffer->type == cylinder_with_aniso_adc_type )
	    {
	      prt.f ( verbosity_information, "Object %d is a anisotropic cylinder.\n", o );
	      buffer_cylinder_aniso = static_cast<cylinder_with_aniso_adc*> (object_buffer->object_pointer);
	      for (unsigned int z = 0; z < max_z; z++)
		for (unsigned int y = 0; y < max_y; y++)
		  {
		    for (unsigned int x = 0; x < max_x; x++)
		    {
		      temp = static_cast<double> (buffer_cylinder_aniso->center.x);
		      temp = temp - x;
		      distance_to_center = temp * temp;
		      temp = static_cast<double> (buffer_cylinder_aniso->center.y);
		      temp = temp - y;
		      distance_to_center += temp * temp;
		      distance_to_center = sqrt ( distance_to_center );
		      if ( distance_to_center <= static_cast<double> ( buffer_cylinder_aniso->radius ) )
			{
			  offset = z * max_x * max_y;
			  offset += y * max_x;
			  offset += x;
			  offset_phantom = offset * sizeof( signal_t );
			  offset *= sizeof ( attributes );
			  phantom_file.seekg ( offset_phantom, ios::beg );
			  phantom_file.read ( ( char* ) &phantom_signal, sizeof ( signal_t ));
			  if ( static_cast<double> ( phantom_signal ) >= buffer_cylinder_aniso->signal_threshold_low && 
			       static_cast<double> ( phantom_signal ) <= buffer_cylinder_aniso->signal_threshold_high )
			    {
			      data.signal = phantom_signal;
			      // get the direction from the xml
			      data.principal_direction.x = buffer_cylinder_aniso->voxel.principal_direction.x;
			      data.principal_direction.y = buffer_cylinder_aniso->voxel.principal_direction.y;
			      data.principal_direction.z = buffer_cylinder_aniso->voxel.principal_direction.z;
			      // "fudge" the direction, considering some arbitrary value for the uncertainty
			      generate_uncertain_versor ( &( data.principal_direction ), direction_uncertainty_percentage );
			      data.iso_adc = buffer_cylinder_aniso->voxel.iso_adc;
			      data.transverse_ratio = buffer_cylinder_aniso->voxel.transverse_ratio;
			      // save it in the buffer
			      buffer_file.seekp (offset, ios::beg);
			      buffer_file.write ((char*) &data, sizeof (attributes));
			    }
			  else
			    {
			      //prt.f ( verbosity_debug, "-" );
			    }
			}
		      else
			{
			  //prt.f ( verbosity_debug, " " );
			}
		    }
		    //prt.f ( verbosity_debug, "\n" );
		  }
	    }
	  if (object_buffer->type == cylinder_with_tangent_adc_type)
	    {
	      prt.f ( verbosity_information, "Object %d is a tangentially anisotropic cylinder.\n", o );
	      buffer_cylinder_tan = static_cast<cylinder_with_tangent_adc*> (object_buffer->object_pointer);
	      for (unsigned int z = 0; z < max_z; z++)
		for (unsigned int y = 0; y < max_y; y++)
		  {
		    for (unsigned int x = 0; x < max_x; x++)
		    {
		      temp = static_cast<double> (buffer_cylinder_tan->center.x);
		      temp = temp - x;
		      distance_to_center = temp * temp;
		      temp = static_cast<double> (buffer_cylinder_tan->center.y);
		      temp = temp - y;
		      distance_to_center += temp * temp;
		      distance_to_center = sqrt (distance_to_center);
		      if (distance_to_center <= static_cast<double> (buffer_cylinder_tan->radius))
			{
			  point.x = x;
			  point.y = y;
			  point.z = z;
			  center.x = buffer_cylinder_tan->center.x;
			  center.y = buffer_cylinder_tan->center.y;
			  center.z = z;
			  data.principal_direction = get_tangent_versor ( center, point );
			  data.iso_adc = buffer_cylinder_tan->voxel.iso_adc;
			  data.transverse_ratio = buffer_cylinder_tan->voxel.transverse_ratio;
			  // read signal
			  offset = z * max_x * max_y;
			  offset += y * max_x;
			  offset += x;
			  offset_phantom = offset * sizeof( signal_t );
			  offset *= sizeof (attributes);
			  phantom_file.seekg (offset_phantom, ios::beg);
			  phantom_file.read ((char*) &phantom_signal, sizeof ( signal_t ));
			  data.signal = phantom_signal;
			  // write data
			  buffer_file.seekp (offset, ios::beg);
			  buffer_file.write ((char*) &data, sizeof (attributes));
			}
		    }
		  }
	    }
	  if ( object_buffer->type == cylinder_with_iso_adc_type )
	    {
	      prt.f ( verbosity_information, "Object %d is a cylinder with isotropic diffusion.\n", o );
	      buffer_cylinder_iso = static_cast<cylinder_with_iso_adc*> (object_buffer->object_pointer);
	      for (unsigned int z = 0; z < max_z; z++)
		for (unsigned int y = 0; y < max_y; y++)
		  {
		    for (unsigned int x = 0; x < max_x; x++)
		    {
		      temp = static_cast<double> (buffer_cylinder_iso->center.x);
		      temp = temp - x;
		      distance_to_center = temp * temp;
		      temp = static_cast<double> (buffer_cylinder_iso->center.y);
		      temp = temp - y;
		      distance_to_center += temp * temp;
		      distance_to_center = sqrt (distance_to_center);
		      if (distance_to_center <= static_cast<double> (buffer_cylinder_iso->radius))
			{
			  generate_random_versor ( &( data.principal_direction ) );
			  data.iso_adc = buffer_cylinder_iso->voxel.iso_adc;
			  data.transverse_ratio = buffer_cylinder_iso->voxel.transverse_ratio;
			  // read signal
			  offset = z * max_x * max_y;
			  offset += y * max_x;
			  offset += x;
			  offset_phantom = offset * sizeof( signal_t );
			  offset *= sizeof ( attributes );
			  phantom_file.seekg (offset_phantom, ios::beg);
			  phantom_file.read ((char*) &phantom_signal, sizeof ( signal_t ));
			  data.signal = phantom_signal;
			  // write data
			  buffer_file.seekp (offset, ios::beg);
			  buffer_file.write ((char*) &data, sizeof (attributes));
			}
		    }
		  }
	    }
	}
    }
  phantom_file.close ();

  // extract masks file
  prt.f ( verbosity_status, "Saving masks files...\n" );
  mask_file.open ("mask_x.raw", ios::out | ios::binary | ios::trunc );
  buffer_file.seekg ( 0, ios::beg );
  for ( unsigned int z = 0; z < max_z; z++ )
    {
      for ( unsigned int y = 0; y < max_y; y++)
	{
	  for ( unsigned int x = 0; x < max_x; x++)
	    {
	      offset =  z * max_y * max_x;
	      offset += y * max_x;
	      offset += x;
	      offset *= sizeof(attributes);
	      buffer_file.read ((char*) &data, sizeof(attributes));
	      mask_file.write ((char*) &data.principal_direction.x, sizeof( double ));
	    }
	}
    }
  mask_file.close ();
  buffer_file.seekg (0, ios::beg);
  mask_file.open ("mask_y.raw", ios::out | ios::binary);
  for ( unsigned int z = 0; z < max_z; z++ )
    {
      for ( unsigned int y = 0; y < max_y; y++)
	{
	  for ( unsigned int x = 0; x < max_x; x++)
	    {
	      offset =  z * max_y * max_x;
	      offset += y * max_x;
	      offset += x;
	      offset *= sizeof(attributes);
	      buffer_file.read ((char*) &data, sizeof(attributes));
	      mask_file.write ((char*) &data.principal_direction.y, sizeof( double ));
	    }
	}
    }
  mask_file.close ();
  buffer_file.seekg ( 0, ios::beg );
  mask_file.open ( "mask_z.raw", ios::out | ios::binary );
  for ( unsigned int z = 0; z < max_z; z++ )
    {
      for ( unsigned int y = 0; y < max_y; y++ )
	{
	  for ( unsigned int x = 0; x < max_x; x++ )
	    {
	      offset =  z * max_y * max_x;
	      offset += y * max_x;
	      offset += x;
	      offset *= sizeof(attributes);
	      buffer_file.read ((char*) &data, sizeof(attributes));
	      mask_file.write ((char*) &data.principal_direction.z, sizeof( double ));
	    }
	}
    }
  mask_file.close ();

  // split buffer in Z files
  prt.f ( verbosity_status, "Creating the Z files...\n" );
  for ( unsigned int z = 0; z < max_z; z++)
    {
      filename = "sample_adc_z";
      name_counter.seekp(0);
      name_counter.width (3);
      name_counter.fill ('0');
      name_counter << z;
      filename += name_counter.str ();
      filename += ".bin";
      out_file.open (filename.c_str (), ios::out | ios::binary);
      offset = z * max_x * max_y * sizeof (attributes);
      buffer_file.seekg (offset, ios::beg);
      for ( unsigned int x = 0; x < max_x; x++)
	{
	  for ( unsigned int y = 0; y < max_y; y++) 
	    {
	      buffer_file.read ((char*) &data, sizeof (attributes));
	      out_file.write ((char*) &data, sizeof (attributes));
	    }
	}
      out_file.close ();
      }
  buffer_file.close ();

  return 0;
}

double_3d get_square_vertex ( double_3d center, double_3d point )
{
  // this function returns a point q such that the lines connecting
  // center-point and point-q are orthogonal, and q is taken 
  // counter-clockwise considering a circle with center center and
  // radius equal to the distance center-point.
  double angle;
  double distance;
  double Dy;
  double_3d q;  
			      
  q.z = center.z;
  distance = sqrt ( pow ( point.x - center.x, 2 ) + pow ( point.y - center.y, 2 ) );

  // 1st quadrant
  if ( point.x > center.x && point.y >= center.y )
    {
      Dy = abs ( point.y - center.y );
      angle = asin ( Dy / distance );
      if ( angle == asin ( M_PI / 2 ) )
	angle = 0;
      q.x = point.x - distance * sin ( angle );
      q.y = point.y + distance * cos ( angle );
      return q;
    }
  
  // 2nd quadrant
  if ( point.x <= center.x && point.y > center.y )
    {
      Dy = abs ( point.y - center.y );
      angle = asin ( Dy / distance );
      if ( angle == asin ( M_PI / 2 ) )
	angle = 0;
      q.x = point.x - distance * sin ( angle );
      q.y = point.y - distance * cos ( angle );
      return q;
    }
  
  // 3rd quadrant
  if ( point.x < center.x && point.y <= center.y )
    {
      Dy = abs ( point.y - center.y );
      angle = asin ( Dy / distance );
      if ( angle == asin ( M_PI / 2 ) )
	angle = 0;
      q.x = point.x + distance * sin ( angle );
      q.y = point.y - distance * cos ( angle );
      return q;
    }

  // 4th quadrant
  if ( point.x >= center.x && point.y < center.y )
    {
      Dy = abs ( point.y - center.y );
      angle = asin ( Dy / distance );
      if ( angle == asin ( M_PI / 2 ) )
	angle = 0;
      q.x = point.x + distance * sin ( angle );
      q.y = point.y + distance * cos ( angle );
      return q;
    }

  // the very center
  if ( point.x == center.x &&
       point.y == center.y &&
       point.z == center.z )
    return center;
      
  prt.f ( verbosity_error, "ERROR: unknown direction!\n" );
  prt.f ( verbosity_error, "center: %f %f %f\n", center.x, center.y, center.z );
  prt.f ( verbosity_error, "point:  %f %f %f\n", point.x,  point.y,  point.z  );
  exit (1);
}

double_3d get_tangent_versor ( double_3d center, double_3d point )
{
  // this function returns the versor that begins in point and is tangent
  // to a circle with center center and radius equal to the distance center-point.
  double    magnitude;
  double_3d auxiliary;
  double_3d versor;
  
  auxiliary = get_square_vertex ( center, point );

  versor.x  = auxiliary.x - point.x;
  versor.y  = auxiliary.y - point.y;
  versor.z  = auxiliary.z - point.z;

  magnitude = sqrt ( versor.x * versor.x + 
		     versor.y * versor.y +
		     versor.z * versor.z );
  versor.x /= magnitude;
  versor.y /= magnitude;
  versor.z /= magnitude;

  return versor;
}

void generate_random_versor ( double_3d* versor )
{
  double    phi;
  double    theta;

  phi   = static_cast<double> ( rand () % 361 );
  theta = static_cast<double> ( rand () % 181 );

  versor->x = sin ( theta ) * cos ( phi );
  versor->y = sin ( theta ) * sin ( phi );
  versor->z = cos ( theta );
}

void generate_uncertain_versor ( double_3d* versor, const unsigned int uncertainty_percentage )
{
  double    magnitude;
  double_3d random_versor;

  generate_random_versor ( &random_versor );
  versor->x = versor->x * ( 100 - uncertainty_percentage ) + random_versor.x * uncertainty_percentage;
  versor->x /= 100;
  versor->y = versor->y * ( 100 - uncertainty_percentage ) + random_versor.y * uncertainty_percentage;
  versor->y /= 100;
  versor->z = versor->z * ( 100 - uncertainty_percentage ) + random_versor.z * uncertainty_percentage;
  versor->z /= 100;
  magnitude = sqrt ( versor->x * versor->x + 
		     versor->y * versor->y +
		     versor->z * versor->z );
  versor->x /= magnitude;
  versor->y /= magnitude;
  versor->z /= magnitude;
}

void set_value_from_node ( node_pointer* node, const string element_name, double* variable )
{
  string      result;
  set_value_from_node ( node, element_name, &result );
  *( variable ) = atof ( result.c_str () );
}

void set_value_from_node ( node_pointer* node, const string element_name, node_pointer* pointer )
{
  mxml_node_t* result;

  prt.f ( verbosity_debug, "Find \"%s\" in %p, and put in %p.\n", element_name.c_str (), pointer->pointer );
  result = mxmlFindElement ( node->pointer, node->pointer, element_name.c_str (), NULL, NULL, MXML_DESCEND );
  pointer->pointer = result;
  if ( pointer->pointer == NULL )
    {
      prt.f ( verbosity_error, "ERROR: mxmlFindElement for %s returned NULL node.\n", element_name.c_str () );
      exit (1);
    }
  prt.f ( verbosity_information, "mxmlFindElement \"%s\": ok\n", element_name.c_str () );
}

void set_value_from_node ( node_pointer* node, const string element_name, short* variable )
{
  string      result;
  set_value_from_node ( node, element_name, &result );
  *( variable ) = atoi ( result.c_str () );
}

void set_value_from_node ( node_pointer* node, const string element_name, signal_t* variable )
{
  string      result;
  set_value_from_node ( node, element_name, &result );
  *( variable ) = static_cast<signal_t> ( atof ( result.c_str () ) ); 
}

void set_value_from_node ( node_pointer* node, const string element_name, string* variable )
{
  string        result;
  node_pointer* temp = new node_pointer;

  set_value_from_node ( node, element_name, temp );
  result = temp->pointer->child->value.text.string;
  prt.f ( verbosity_information, "%s = %s\n", element_name.c_str (), result.c_str () );
  *( variable ) = result;
}

void set_value_from_node ( node_pointer* node_p, const string element_name, unsigned int* variable )
{
  string      result;
  set_value_from_node ( node_p, element_name, &result );
  *( variable ) = atoi ( result.c_str () );
}
