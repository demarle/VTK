/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkFollower.h
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
// .NAME vtkFollower - a subclass of actor that always faces the camera
// .SECTION Description
// vtkFollower is a subclass of vtkActor that always follows its specified 
// camera. More specifically it will not change its position or scale,
// but it will continually update its orientation so that it is right side
// up and facing the camera. This is typically used for text labels in a
// scene. All of the adjustments that can be made to an actor also will
// take effect with a follower.  So, if you change the orientation of the
// follower by 90 degrees, then it will follow the camera, but be off by 
// 90 degrees.

// .SECTION see also
// vtkActor vtkCamera

#ifndef __vtkFollower_h
#define __vtkFollower_h

#include "vtkActor.h"
#include "vtkCamera.h"

class VTK_EXPORT vtkFollower : public vtkActor
{
 public:
  vtkTypeMacro(vtkFollower,vtkActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Creates a follower with no camera set
  static vtkFollower *New();

  // Description:
  // This causes the actor to be rendered. It in turn will render the actor's
  // property, texture map and then mapper. If a property hasn't been
  // assigned, then the actor will create one automatically. 
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);
  virtual void Render(vtkRenderer *ren);
  
  // Description:
  // Copy the follower's composite 4x4 matrix into the matrix provided.
  virtual void GetMatrix(vtkMatrix4x4 *m);
  virtual void GetMatrix(double m[16])
    {this->GetMatrix(this->Matrix); vtkMatrix4x4::DeepCopy(m,this->Matrix);};
//BTX
  virtual vtkMatrix4x4& GetMatrix() {return *(this->GetMatrixPointer());}
//ETX

  // Description:
  // Set/Get the camera to follow. If this is not set, then the follower
  // won't know who to follow.
  vtkSetObjectMacro(Camera,vtkCamera);
  vtkGetObjectMacro(Camera,vtkCamera);

  // Description:
  // For legacy compatibility. Do not use.
  void GetMatrix(vtkMatrix4x4 &m) {this->GetMatrix(&m);}
  
protected:
  vtkFollower();
  ~vtkFollower();
  vtkFollower(const vtkFollower&) {};
  void operator=(const vtkFollower&) {};

  vtkCamera *Camera; 
  vtkActor  *Device;
private:
  // hide the two parameter Render() method from the user and the compiler.
  virtual void Render(vtkRenderer *, vtkMapper *) {};
};

#endif



