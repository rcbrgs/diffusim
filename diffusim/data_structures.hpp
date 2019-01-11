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

#ifndef DATA_STRUCTURE
#define DATA_STRUCTURE

typedef struct
{
  unsigned int x;
  unsigned int y;
  unsigned int z;
} uint_3d;

typedef struct
{
  double x;
  double y;
  double z;
} double_3d;

typedef int signal_t;

typedef struct
{
  double    iso_adc;
  double_3d principal_direction;
  signal_t  signal;
  double    transverse_ratio;
} attributes;

#ifdef _mxml_h_
typedef struct { mxml_node_t* pointer; } node_pointer;
#endif

enum geometry_type { rectangle_type = 0,
		     cylinder_with_aniso_adc_type,
                     cylinder_with_iso_adc_type,
		     cylinder_with_tangent_adc_type };

enum diffusion_type { isotropic = 0,
		      single_direction };
		      
class object
{
 public:
  geometry_type type;
  void*         object_pointer;
};

class rectangle
{
 public:
  diffusion_type diffusion;
  uint_3d        origin;
  uint_3d        size;
  attributes     voxel;
};

class cylinder_with_aniso_adc
{
 public:
  double       signal_threshold_low;
  double       signal_threshold_high;
  unsigned int radius;
  uint_3d      center;
  attributes   voxel;
};

class cylinder_with_iso_adc
{
 public:
  unsigned int radius;
  uint_3d      center;
  attributes   voxel;
};

class cylinder_with_tangent_adc
{
 public:
  unsigned int radius;
  uint_3d      center;
  attributes   voxel;
};

typedef struct
{
  unsigned int number_of_objects;
  object* objects;
} canvas;

typedef struct
{
  unsigned int number_of_layers;
  canvas* layers;
} sample;

#endif
