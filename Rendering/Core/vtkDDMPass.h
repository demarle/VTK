/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDDMPass.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDDMPass -
// .SECTION Description

#ifndef vtkDDMPass_h
#define vtkDDMPass_h

#include "vtkRenderingCoreModule.h" // For export macro
#include "vtkRenderPass.h"

class VTKRENDERINGCORE_EXPORT vtkDDMPass : public vtkRenderPass
{
public:
  static vtkDDMPass *New();
  vtkTypeMacro(vtkDDMPass,vtkRenderPass);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Perform rendering according to a render state s.
  virtual void Render(const vtkRenderState *s);

 protected:
  // Description:
  // Default constructor.
  vtkDDMPass();

  // Description:
  // Destructor.
  virtual ~vtkDDMPass();

 private:
  vtkDDMPass(const vtkDDMPass&);  // Not implemented.
  void operator=(const vtkDDMPass&);  // Not implemented.
};

#endif
