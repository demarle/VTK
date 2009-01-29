/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPixelBufferObject.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPixelBufferObject.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderWindow.h"

#include "vtkgl.h"
#include "vtkOpenGL.h"

//#define VTK_PBO_TIMING

#ifdef VTK_PBO_TIMING
#include "vtkTimerLog.h"
#endif

vtkStandardNewMacro(vtkPixelBufferObject);
vtkCxxRevisionMacro(vtkPixelBufferObject, "1.6");
//----------------------------------------------------------------------------
vtkPixelBufferObject::vtkPixelBufferObject()
{
  this->Handle = 0;
  this->Context = 0;
  this->BufferTarget = 0;
}

//----------------------------------------------------------------------------
vtkPixelBufferObject::~vtkPixelBufferObject()
{
  this->SetContext(0);
}

//----------------------------------------------------------------------------
// Description:
// Returns if the context supports the required extensions.
bool vtkPixelBufferObject::IsSupported(vtkRenderWindow* win)
{
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(win);
  if (renWin)
    {
    vtkOpenGLExtensionManager* mgr = renWin->GetExtensionManager();
    
    bool vbo=mgr->ExtensionSupported("GL_VERSION_1_5") ||
      mgr->ExtensionSupported("GL_ARB_vertex_buffer_object");
    
    // pbo extension does not define new functions but uses functions defined
    // by vbo extension.
    bool pbo=mgr->ExtensionSupported("GL_VERSION_2_1") ||
      mgr->ExtensionSupported("GL_ARB_pixel_buffer_object");
      
    return vbo && pbo;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPixelBufferObject::LoadRequiredExtensions(vtkOpenGLExtensionManager* mgr)
{
  bool gl15=mgr->ExtensionSupported("GL_VERSION_1_5")==1;
  bool gl21=mgr->ExtensionSupported("GL_VERSION_2_1")==1;
  
  bool vbo=gl15 || mgr->ExtensionSupported("GL_ARB_vertex_buffer_object");
  
  // pbo extension does not define new functions but uses functions defined
  // by vbo extension.
  bool pbo=gl21 || mgr->ExtensionSupported("GL_ARB_pixel_buffer_object");
  
  bool result=vbo && pbo;
  
  if(result)
    {
    if(gl15)
      {
      mgr->LoadExtension("GL_VERSION_1_5");
      }
    else
      {
      mgr->LoadCorePromotedExtension("GL_ARB_vertex_buffer_object");
      }
    // pbo does not define new functions, nothing to do here.
    }
  return result;
}

//----------------------------------------------------------------------------
void vtkPixelBufferObject::SetContext(vtkRenderWindow* renWin)
{
  if (this->Context == renWin)
    {
    return;
    }

  this->DestroyBuffer(); 

  vtkOpenGLRenderWindow* openGLRenWin = vtkOpenGLRenderWindow::SafeDownCast(renWin);
  this->Context = openGLRenWin;
  if (openGLRenWin)
    {
    if (!this->LoadRequiredExtensions(openGLRenWin->GetExtensionManager()))
      {
      this->Context = 0;
      vtkErrorMacro("Required OpenGL extensions not supported by the context.");
      }
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkRenderWindow* vtkPixelBufferObject::GetContext()
{
  return this->Context;
}

//----------------------------------------------------------------------------
void vtkPixelBufferObject::Bind(BufferType type)
{
  if (!this->Context)
    {
    vtkErrorMacro("No context specified. Cannot Bind.");
    return;
    }

  if (this->Handle)
    {
    int target = this->BufferTarget;
    switch (type)
      {
    case vtkPixelBufferObject::PACKED_BUFFER:
      target =  vtkgl::PIXEL_PACK_BUFFER_ARB;
      break;

    case vtkPixelBufferObject::UNPACKED_BUFFER:
      target = vtkgl::PIXEL_UNPACK_BUFFER_ARB;
      break;
      }

    if (this->BufferTarget && this->BufferTarget != target)
      {
      this->UnBind();
      }
    this->BufferTarget = target;
    vtkgl::BindBuffer(this->BufferTarget, this->Handle);
    vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
    }
  else
    {
    vtkErrorMacro("A pixel buffer can be bound only after it has been created."
      " Are you sure you uploaded some data?");
    }
}

//----------------------------------------------------------------------------
void vtkPixelBufferObject::UnBind()
{
  if (this->Context && this->Handle && this->BufferTarget)
    {
    vtkgl::BindBuffer(this->BufferTarget, 0);
    vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
    this->BufferTarget = 0;
    }
}

//----------------------------------------------------------------------------
void vtkPixelBufferObject::CreateBuffer()
{
  this->Context->MakeCurrent();
  if (!this->Handle)
    {
    GLuint ioBuf;
    vtkgl::GenBuffers(1, &ioBuf);
    vtkGraphicErrorMacro(this->Context,"#__FILE__ #__LINE__");
    this->Handle = ioBuf;
    }
}

//----------------------------------------------------------------------------
void vtkPixelBufferObject::DestroyBuffer()
{
  if (this->Context && this->Handle)
    {
    GLuint ioBuf = static_cast<GLuint>(this->Handle);
    vtkgl::DeleteBuffers(1, &ioBuf);
    }
  this->Handle = 0;
}
/*
template <class T>
static int vtksizeof(T)
{
  return sizeof(T);
}

VTK_TEMPLATE_SPECIALIZE
int vtksizeof(double)
{
  return sizeof(float);
}

static int vtkGetSize(int type)
{
  switch (type)
    {
    vtkTemplateMacro(
      return ::vtksizeof(static_cast<VTK_TT>(0));
    );
    }
  return 0;
}
*/

template< class T >
class vtksizeof
{
public:
  static int GetSize() { return sizeof(T); }
};

template<>
class vtksizeof< double >
{
public:
  static int GetSize() { return sizeof(float); }
};

static int vtkGetSize(int type)
{
  switch (type)
    {
    vtkTemplateMacro(
      return ::vtksizeof<VTK_TT>::GetSize();
    );
    }
  return 0;
}


//----------------------------------------------------------------------------
template <class T>
class vtkUpload3D
{
public:
  static void Upload(   void* pboPtr, T* inData,
                        unsigned int dims[3],
                        int numComponents,
                        vtkIdType continuousIncrements[3],
                        int components,
                        int *componentList)
  {
    //  cout<<"incs[3]="<<continuousIncrements[0]<<" "<<continuousIncrements[1]
    //      <<" "<<continuousIncrements[2]<<endl;

    T* fIoMem = static_cast<T*>(pboPtr);

    int numComp;
    int *permutation=0;
    if(components==0)
      {
        numComp=numComponents;
        permutation=new int[numComponents];
        int i=0;
        while(i<numComp)
          {
            permutation[i]=i;
            ++i;
          }
      }
    else
      {
        numComp=components;
        permutation=componentList;
      }

    vtkIdType tupleSize = 
      static_cast<vtkIdType>(numComponents + continuousIncrements[0]);
    for (unsigned int zz=0; zz < dims[2]; zz++)
      {
      for (unsigned int yy = 0; yy < dims[1]; yy++)
        {
        for (unsigned int xx=0; xx < dims[0]; xx++)
          {
          for (int compNo=0; compNo < numComp; compNo++)
            {
            *fIoMem = inData[permutation[compNo]];
            //cout<<"upload[zz="<<zz<<"][yy="<<yy<<"][xx="<<xx<<"][compNo="<<
            //compNo<<"] from inData to pbo="<<(int)(*fIoMem)<<endl;

            fIoMem++;
            }
          inData += tupleSize+continuousIncrements[0];
          }
        // Reached end of row, go to start of next row.
        inData += continuousIncrements[1] * tupleSize;
        }
      // Reached end of 2D plane.
      inData += continuousIncrements[2] * tupleSize;
      }

    if(components==0)
      {
        delete[] permutation;
      }
  }
};

VTK_TEMPLATE_SPECIALIZE
class vtkUpload3D< double >
{
public:
  static void Upload( void* pboPtr, double* inData,
                 unsigned int dims[3],
                 int numComponents,
                 vtkIdType continuousIncrements[3],
                 int components,
                 int *componentList)
  {
    float* fIoMem = static_cast<float*>(pboPtr);

    int numComp;
    int *permutation=0;
    if(components==0)
      {
        numComp=numComponents;
        permutation=new int[numComponents];
        int i=0;
        while(i<numComp)
          {
            permutation[i]=i;
            ++i;
          }
      }
    else
      {
        numComp=components;
        permutation=componentList;
      }

    vtkIdType tupleSize = 
      static_cast<vtkIdType>(numComponents + continuousIncrements[0]);
    for (unsigned int zz=0; zz < dims[2]; zz++)
      {
      for (unsigned int yy = 0; yy < dims[1]; yy++)
        {
        for (unsigned int xx=0; xx < dims[0]; xx++)
          {
          for (int compNo=0; compNo < numComponents; compNo++)
            {
              *fIoMem = static_cast<float>(inData[permutation[compNo]]);

            //        cout<<"upload specialized double[zz="<<zz<<"][yy="<<yy<<"][xx="<<xx<<"][compNo="<<compNo<<"] from inData="<<(*inData)<<" to pbo="<<(*fIoMem)<<endl;

            fIoMem++;
            }
          
          inData += tupleSize+continuousIncrements[0];
          }
        // Reached end of row, go to start of next row.
        inData += continuousIncrements[1] * tupleSize;
        }
      // Reached end of 2D plane.
      inData += continuousIncrements[2] * tupleSize;
      }
    if(components==0)
      {
        delete[] permutation;
      }
  }
};

//----------------------------------------------------------------------------
bool vtkPixelBufferObject::Upload3D(
  int type, void* data,
  unsigned int dims[3],
  int numComponents,
  vtkIdType continuousIncrements[3],
  int components,
  int *componentList)
{
#ifdef VTK_PBO_TIMING
  vtkTimerLog *timer=vtkTimerLog::New();
  timer->StartTimer();
#endif

  if (!this->Context)
    {
    vtkErrorMacro("No context specified. Cannot upload data.");
    return false;
    }

  this->CreateBuffer();

  this->Bind(vtkPixelBufferObject::PACKED_BUFFER);

  unsigned int size;

  if(components==0)
    {
      size = dims[0]*dims[1]*dims[2]*numComponents;
    }
  else
    {
      size = dims[0]*dims[1]*dims[2]*components;
    }


  vtkgl::BufferData(this->BufferTarget, 
    size*::vtkGetSize(type), NULL, 
    data? vtkgl::STREAM_DRAW: vtkgl::STREAM_READ);
  vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
  this->Type = type;
  if (this->Type == VTK_DOUBLE)
    {
    this->Type = VTK_FLOAT;
    }
  this->Size = size;

  if (data)
    {
    void* ioMem = vtkgl::MapBuffer(this->BufferTarget, vtkgl::WRITE_ONLY);
    vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
    switch (type)
      {
      vtkTemplateMacro(
        ::vtkUpload3D< VTK_TT >::Upload(ioMem, static_cast<VTK_TT*>(data), 
                      dims, numComponents, continuousIncrements,
                      components,componentList);
      );
    default:
      return false;
      }
    vtkgl::UnmapBuffer(this->BufferTarget);
    vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
    }
  
  this->UnBind();
#ifdef VTK_PBO_TIMING
  timer->StopTimer();
  double time=timer->GetElapsedTime();
  timer->Delete();
  cout<<"Upload data to PBO"<<time<<" seconds."<<endl;
#endif
  return true;
}


//----------------------------------------------------------------------------
void vtkPixelBufferObject::ReleaseMemory()
{
  if (this->Context && this->Handle)
    {
    this->Bind(vtkPixelBufferObject::PACKED_BUFFER);
    vtkgl::BufferData(this->BufferTarget, 0, NULL, vtkgl::STREAM_DRAW);
    this->Size = 0;
    }
}

//----------------------------------------------------------------------------
template <class TPBO, class TCPU>
void vtkDownload3D(TPBO *pboPtr,
                   TCPU *cpuPtr,
                   unsigned int dims[3],
                   int numcomps,
                   vtkIdType increments[3])
{
  vtkIdType tupleSize = static_cast<vtkIdType>(numcomps + increments[0]);
  for (unsigned int zz=0; zz < dims[2]; zz++)
    {
    for (unsigned int yy = 0; yy < dims[1]; yy++)
      {
      for (unsigned int xx=0; xx < dims[0]; xx++)
        {
        for (int comp=0; comp < numcomps; comp++)
          {
          *cpuPtr = static_cast<TCPU>(*pboPtr);
          //    cout<<"download[zz="<<zz<<"][yy="<<yy<<"][xx="<<xx<<"][comp="<<comp<<"] from pbo="<<(*pboPtr)<<" to cpu="<<(*cpuPtr)<<endl;
          pboPtr++;
          cpuPtr++;
          }
        cpuPtr += increments[0];
        }
      // Reached end of row, go to start of next row.
      cpuPtr += increments[1]*tupleSize;
      }
    cpuPtr += increments[2]*tupleSize;
    }
}

//----------------------------------------------------------------------------
bool vtkPixelBufferObject::Download3D(
  int type, void* data,
  unsigned int dims[3],
  int numcomps,
  vtkIdType increments[3])
{
#ifdef VTK_PBO_TIMING
  vtkTimerLog *timer=vtkTimerLog::New();
  timer->StartTimer();
#endif
  if (!this->Handle || !this->Context)
    {
    vtkErrorMacro("No GPU data available.");
    return false;
    }

  if (this->Size < dims[0]*dims[1]*dims[2]*numcomps)
    {
    vtkErrorMacro("Size too small.");
    return false;
    }

  this->Bind(vtkPixelBufferObject::PACKED_BUFFER);
  void* ioMem = vtkgl::MapBuffer(this->BufferTarget, vtkgl::READ_ONLY);
  vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
  switch (type)
    {
    vtkTemplateMacro(
      VTK_TT* odata = static_cast<VTK_TT*>(data);
      switch (this->Type)
        {
        vtkTemplateMacro(
          ::vtkDownload3D(static_cast<VTK_TT*>(ioMem), odata, 
            dims, numcomps, increments);
        );
        }
     );
  default:
    return false;
    }
  
  vtkgl::UnmapBuffer(this->BufferTarget);
  vtkGraphicErrorMacro(this->Context,"__FILE__ __LINE__");
  this->UnBind();

#ifdef VTK_PBO_TIMING
  timer->StopTimer();
  double time=timer->GetElapsedTime();
  timer->Delete();
  cout<<"dowmload data from PBO"<<time<<" seconds."<<endl;
#endif

  return true;
}


//----------------------------------------------------------------------------
void vtkPixelBufferObject::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Context: " << this->Context << endl;
  os << indent << "Handle: " << this->Handle << endl;
  os << indent << "Size: " << this->Size << endl;
  os << indent << "VTK Type: " << vtkImageScalarTypeNameMacro(this->Type)
     << endl;
}

