/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDEMReader.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkImageData.h"
#include "vtkDEMReader.h"

#define VTK_SW	0
#define VTK_NW	1
#define VTK_NE	2
#define VTK_SE	3
#define VTK_METERS_PER_FEET .305
#define VTK_METERS_PER_ARC_SECOND 23.111

void ConvertDNotationToENotation (char *line);

vtkDEMReader::vtkDEMReader()
{
  this->FileName = NULL;
  this->MapLabel[0] = '\0';
  this->NumberOfColumns = 0;
  this->NumberOfRows = 0;
  this->ProfileSeekOffset = 0;
}

vtkDEMReader::~vtkDEMReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    }
}

//----------------------------------------------------------------------------
void vtkDEMReader::UpdateInformation()
{
  float spacing[3], origin[3];
  int extent[6];

  // read the header of the file to determine dimensions, origin and spacing
  this->ReadTypeARecord ();

  // compute the extent based on the header information
  this->ComputeExtentOriginAndSpacing (extent, origin, spacing);

  // fill in the pertinent stuff from the header
  this->GetOutput()->SetOrigin(origin);
  this->GetOutput()->SetSpacing(spacing);

  this->GetOutput()->SetNumberOfScalarComponents(1);
  this->GetOutput()->SetScalarType(VTK_FLOAT);

  // whole dem must be read
  this->GetOutput()->SetWholeExtent(extent);
}


void vtkDEMReader::ModifyOutputUpdateExtent()
{
  this->GetOutput()->SetUpdateExtent(this->GetOutput()->GetWholeExtent());
}



void vtkDEMReader::Execute(vtkImageData *data)
{
  if (data->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro("Execute: This source only outputs ints");
    return;
    }
  
//
// Read header
//
  if (this->ReadTypeARecord () == 0)
    {
    //
    // Read Profiles
    //
    this->ReadProfiles (data);
    }
}

int vtkDEMReader::ReadTypeARecord ()
{
  char record[1025];
  float elevationConversion;
  FILE *fp;

  if (this->GetMTime () < this->ReadHeaderTime)
    {
    return 0;
    }

  if ((fp = fopen(this->FileName, "r")) == NULL)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return -1;
    }

  vtkDebugMacro (<< "reading DEM header: type A record");

  //
  // read the record. it is always 1024 characters long
  //

  fscanf(fp, "%1024c", record);
  record[1024] = '\0';

  //
  // convert any D+ or D- to E+ or E-. c++ and c i/o cannot read D+/-
  //
  ConvertDNotationToENotation (record);

  char *current = record;

  this->MapLabel[144] = '\0';
  sscanf(current, "%144c", this->MapLabel);
  current += 144;

  sscanf(current, "%6d%6d%6d%6d",
		 &this->DEMLevel,
		 &this->ElevationPattern,
		 &this->GroundSystem,
		 &this->GroundZone);
  current += 24;
  sscanf(current, "%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g%24g",
   &this->ProjectionParameters[0],
   &this->ProjectionParameters[1],
   &this->ProjectionParameters[2],
   &this->ProjectionParameters[3],
   &this->ProjectionParameters[4],
   &this->ProjectionParameters[5],
   &this->ProjectionParameters[6],
   &this->ProjectionParameters[7],
   &this->ProjectionParameters[8],
   &this->ProjectionParameters[9],
   &this->ProjectionParameters[10],
   &this->ProjectionParameters[11],
   &this->ProjectionParameters[12],
   &this->ProjectionParameters[13],
   &this->ProjectionParameters[14]);
  current += 360;
  sscanf(current, "%6d%6d%6d",
   &this->PlaneUnitOfMeasure,
   &this->ElevationUnitOfMeasure,
   &this->PolygonSize);
  current += 18;
  sscanf(current, "%24g%24g%24g%24g%24g%24g%24g%24g",
   &this->GroundCoords[0][0], &this->GroundCoords[0][1],
   &this->GroundCoords[1][0], &this->GroundCoords[1][1],
   &this->GroundCoords[2][0], &this->GroundCoords[2][1],
   &this->GroundCoords[3][0], &this->GroundCoords[3][1]);
  current += 192;
  sscanf(current, "%24g%24g",
   &this->ElevationBounds[0], &this->ElevationBounds[1]);
  elevationConversion = 1.0;
  if (this->ElevationUnitOfMeasure == 1) // feet
    {
    elevationConversion = VTK_METERS_PER_FEET;
    }
  else if (this->ElevationUnitOfMeasure == 3) // arc-seconds
    {
    elevationConversion = VTK_METERS_PER_ARC_SECOND;
    }
  this->ElevationBounds[0] *= elevationConversion;
  this->ElevationBounds[1] *= elevationConversion;
  current += 48;
  sscanf(current, "%24g",
   &this->LocalRotation);
  current += 24;
  sscanf(current, "%6d",
   &this->AccuracyCode);
  current += 6;
  sscanf(current, "%12g%12g%12g",
   &this->SpatialResolution[0],
   &this->SpatialResolution[1],
   &this->SpatialResolution[2]);
  current += 36;
  sscanf(current, "%6d%6d",
   &this->ProfileDimension[0],
   &this->ProfileDimension[1]);

  this->ProfileSeekOffset = ftell (fp);

  this->ReadHeaderTime.Modified();
  fclose (fp);

  return 0;
}

void vtkDEMReader::ComputeExtentOriginAndSpacing (int extent[6],
						  float origin[3],
						  float spacing[3])
{
  float eastMost, westMost, northMost, southMost;
  float planeConversion;

  //
  // compute number of samples
  //
  eastMost = this->GroundCoords[VTK_NE][0];
  if (eastMost <  this->GroundCoords[VTK_SE][0])
    {
    eastMost = this->GroundCoords[VTK_SE][0];
    }
  westMost = this->GroundCoords[VTK_NW][0];
  if (westMost >  this->GroundCoords[VTK_SW][0])
    {
    westMost = this->GroundCoords[VTK_SW][0];
    }
  northMost = this->GroundCoords[VTK_NE][1];
  if (northMost <  this->GroundCoords[VTK_NW][1])
    {
    northMost = this->GroundCoords[VTK_NW][1];
    }
  southMost = this->GroundCoords[VTK_SW][1];
  if (southMost >  this->GroundCoords[VTK_SE][1])
    {
    southMost = this->GroundCoords[VTK_SE][1];
    }

  //
  // compute the number of rows and columns
  //
  this->NumberOfColumns = (int) ((eastMost - westMost) / this->SpatialResolution[0] + 1.0);
  this->NumberOfRows = (int) ((northMost - southMost) / this->SpatialResolution[1] + 1.0);

  //
  // convert to extent
  //
  extent[0] = 0; extent[1] = this->NumberOfColumns - 1;
  extent[2] = 0; extent[3] = this->NumberOfRows - 1;
  extent[4] = 0; extent[5] = 0;;

  //
  // compute the spacing in meters
  //
  planeConversion = 1.0;
  if (this->PlaneUnitOfMeasure == 1) // feet
    {
    planeConversion = VTK_METERS_PER_FEET;
    }
  else if (this->PlaneUnitOfMeasure == 3) // arc-seconds
    {
    planeConversion = VTK_METERS_PER_ARC_SECOND;
    }

  //
  // get the origin from the ground coordinates
  //
  origin[0] = this->GroundCoords[VTK_SW][0];
  origin[1] = this->GroundCoords[VTK_SW][1];
  origin[2] = this->ElevationBounds[0];
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;

  spacing[0] = this->SpatialResolution[0] * planeConversion;
  spacing[1] = this->SpatialResolution[1] * planeConversion;
  spacing[2] = 0;
}

int vtkDEMReader::ReadProfiles (vtkImageData *data)
{
  char record[145];
  float *outPtr, *ptr;
  float elevationExtrema[2];
  float localElevation;
  float planCoords[2];
  float units = this->SpatialResolution[2];
  float elevationConversion;
  float lowPoint;
  int column, row;
  int columnCount;
  int elevation;
  int lastRow;
  int numberOfColumns;
  int profileId[2], profileSize[2];
  int rowId, columnId;
  int updateInterval;
  int status = 0;
  FILE *fp;

  this->UpdateInformation ();

  if ((fp = fopen(this->FileName, "r")) == NULL)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return -1;
    }

  vtkDebugMacro (<< "reading profiles");

  // elevation will always be stored in meters
  elevationConversion = 1.0;
  if (this->ElevationUnitOfMeasure == 1) // feet
    {
    elevationConversion = VTK_METERS_PER_FEET;
    }
  else if (this->ElevationUnitOfMeasure == 3) // arc-seconds
    {
    elevationConversion = VTK_METERS_PER_ARC_SECOND;
    }

  units *= elevationConversion;

  // seek to start of profiles
  fseek (fp, this->ProfileSeekOffset, SEEK_SET);

  record[120] = '\0';

  // initialize output to the lowest elevation
  lowPoint = this->ElevationBounds[0];
  ptr = outPtr = (float *) data->GetScalarPointer();
  for (int i = 0; i < this->NumberOfColumns * this->NumberOfRows; i++)
    {
    *ptr++ = lowPoint;
    }
  numberOfColumns = this->NumberOfColumns;
  updateInterval = numberOfColumns / 100;
  columnCount = this->ProfileDimension[1];
  for (column = 0; column < columnCount; column++)
    {
    //
    // read four int's
    //
    status = fscanf (fp, "%6d%6d%6d%6d",
		   &profileId[0],	/* 1 */
		   &profileId[1],	/* 1 */
		   &profileSize[0],	/* 2 */
		   &profileSize[1]);	/* 2 */
    if (status == EOF)
      {
      break;
      }

    //
    // read the doubles as strings so we can convert floating point format
    //
    (void) fscanf(fp, "%120c", record);

    //
    // convert any D+ or D- to E+ or E-
    //
    ConvertDNotationToENotation (record);

    sscanf(record, "%6d%6d%6d%6d%24g%24g%24g%24g%24g",
		   &planCoords[0],	/* 3 */
		   &planCoords[1],	/* 3 */
		   &localElevation,	/* 4 */
		   &elevationExtrema[0],	/* 5 */
		   &elevationExtrema[1]);	/* 5 */

    rowId = profileId[0] - 1;
    columnId = profileId[1] - 1;
    lastRow = rowId + profileSize[0] - 1;

    // report progress at the start of each column
    if (column % updateInterval == 0)
      {
      this->UpdateProgress ((float) column / ((float) columnCount - 1));
      if (this->GetAbortExecute())
	{
	break;
	}
      }
    // read a column
    for (row = rowId; row <= lastRow; row++)
      {
      (void) fscanf(fp, "%6d", &elevation);
      *(outPtr + columnId + row * numberOfColumns) = elevation * units;
      }
    }
  fclose (fp);

  return status;
}

// Description: Converts Fortran D notation to C++ e notation
void ConvertDNotationToENotation (char *line)
{
  char *ptr = line;

  // first convert D+ to E+
  while (*ptr && (ptr = strstr (ptr, "D+")))
    {
    *ptr = 'e'; ptr++;
    *ptr = '+'; ptr++;
    }

  // first now D- to E-
  ptr = line;
  while (*ptr && (ptr = strstr (ptr, "D-")))
    {
    *ptr = 'e'; ptr++;
    *ptr = '-'; ptr++;
    }
}    

void vtkDEMReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageSource::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";
  if (this->FileName)
    {
    this->UpdateInformation ();
    os << indent << "MapLabel: " << this->MapLabel << "\n";
    os << indent << "DEMLevel: " << this->DEMLevel << "\n";
    os << indent << "ElevationPattern: " << this->ElevationPattern
       << (this->ElevationPattern == 1 ? " (regular)" : " (random)") << "\n";
    os << indent << "GroundSystem: " << this->GroundSystem;
    if (this->GroundSystem == 0)
      {
      os << " (Geographic)\n";
      }
    else if (this->GroundSystem == 1)
      {
      os << " (UTM)\n";
      }
    else if (this->GroundSystem == 2)
      {
      os << " (State plane)\n";
      }
    else
      {
      os << " (unknown)\n";
      }
    os << indent << "GroundZone: " << this->GroundZone << "\n";
    os << indent << "ProjectionParameters: all zero" << "\n"; // this->ProjectionParameters
    os << indent << "PlaneUnitOfMeasure: " << this->PlaneUnitOfMeasure;
    if (this->PlaneUnitOfMeasure == 0)
      {
      os << indent << " (radians)\n";
      }
    else if (this->PlaneUnitOfMeasure == 1)
      {
      os << indent << " (feet)\n";
      }
    else if (this->PlaneUnitOfMeasure == 2)
      {
      os << indent << " (meters)\n";
      }
    else if (this->PlaneUnitOfMeasure == 3)
      {
      os << indent << " (arc-seconds)\n";
      }
    else
      {
      os << indent << " (unknown)\n";
      }
  
    os << indent << "ElevationUnitOfMeasure: " << this->ElevationUnitOfMeasure;
    if (this->ElevationUnitOfMeasure == 1)
      {
      os << indent << " (feet)\n";
      }
    else if (this->ElevationUnitOfMeasure == 2)
      {
      os << indent << " (meters)\n";
      }
    else
      {
      os << indent << " (unknown)\n";
      }
    os << indent << "PolygonSize: " << this->PolygonSize << "\n";
    os << indent << "GroundCoordinates: \n";
    os << indent << "        " << this->GroundCoords[0][0] << ", " << this->GroundCoords[0][1] << "\n";
    os << indent << "        " << this->GroundCoords[1][0] << ", " << this->GroundCoords[1][1] << "\n";
    os << indent << "        " << this->GroundCoords[2][0] << ", " << this->GroundCoords[2][1] << "\n";
    os << indent << "        " << this->GroundCoords[3][0] << ", " << this->GroundCoords[3][1] << "\n";
  
    os << indent << "ElevationBounds: " << this->ElevationBounds[0] << ", "
                                        << this->ElevationBounds[1]
                                        << " (meters)\n";
    os << indent << "LocalRotation: " << this->LocalRotation << "\n";
    os << indent << "AccuracyCode: " << this->AccuracyCode << "\n";
    os << indent << "SpatialResolution: " << this->SpatialResolution[0] << ", "
                                          << this->SpatialResolution[1];
    if (this->PlaneUnitOfMeasure == 0)
      {
      os << indent << "(radians)";
      }
    else if (this->PlaneUnitOfMeasure == 1)
      {
      os << indent << "(feet)";
      }
    else if (this->PlaneUnitOfMeasure == 2)
      {
      os << indent << "(meters)";
      }
    else if (this->PlaneUnitOfMeasure == 3)
      {
      os << indent << "(arc-seconds)";
      }
    else
      {
      os << indent << " (unknown)\n";
      }
  
    os << indent << this->SpatialResolution[2];
    if (this->ElevationUnitOfMeasure == 1)
      {
      os << indent << "(feet)\n";
      }
    else if (this->ElevationUnitOfMeasure == 2)
      {
      os << indent << "(meters)\n";
      }
    else
      {
      os << indent << "(unknown)\n";
      }
    os << indent << "ProfileDimension: " << this->ProfileDimension[0] << ", "
                                         << this->ProfileDimension[1] << "\n";
  }
}
