/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkByteSwap.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkByteSwap - perform machine dependent byte swapping
// .SECTION Description
// vtkByteSwap is used by other classes to perform machine dependent byte
// swapping. Byte swapping is often used when reading or writing binary 
// files.

#ifndef __vtkByteSwap_h
#define __vtkByteSwap_h

#include "vtkObject.h"

class VTK_EXPORT vtkByteSwap : public vtkObject
{
public:
  static vtkByteSwap *New();
  vtkTypeMacro(vtkByteSwap,vtkObject);

  // Description:
  // Swap 2 byte word to be LE.
  static void Swap2LE(short *s);


  // Description:
  // Swap four byte word to be LE.
  static void Swap4LE(char *c);
  static void Swap4LE(float *p) { vtkByteSwap::Swap4LE((char *)p);};
  static void Swap4LE(int *i)   { vtkByteSwap::Swap4LE((char *)i);};
  static void Swap4LE(unsigned long *i) { vtkByteSwap::Swap4LE((char *)i);};
  static void Swap4LE(long *i) { vtkByteSwap::Swap4LE((char *)i);};


  // Description:
  // Swap bunch of bytes to be LE. Num is the number of four byte words to swap.
  static void Swap4LERange(char *c,int num);
  static void Swap4LERange(unsigned char *c,int num) 
  { vtkByteSwap::Swap4LERange((char *)c,num);};
  static void Swap4LERange(float *p,int num) 
  { vtkByteSwap::Swap4LERange((char *)p,num);};
  static void Swap4LERange(int *i,int num) 
  { vtkByteSwap::Swap4LERange((char *)i,num);};
  static void Swap4LERange(unsigned long *i,int num) 
  { vtkByteSwap::Swap4LERange((char *)i,num);};


  // Description:
  // Swap four byte word to be BE.
  static void Swap4BE(char *c);
  static void Swap4BE(float *p) { vtkByteSwap::Swap4BE((char *)p);};
  static void Swap4BE(int *i)   { vtkByteSwap::Swap4BE((char *)i);};
  static void Swap4BE(unsigned long *i) { vtkByteSwap::Swap4BE((char *)i);};

  
  // Description:
  // Swap bunch of bytes to be BE. Num is the number of four byte words to swap.
  static void Swap4BERange(char *c,int num);
  static void Swap4BERange(float *p,int num) 
  { vtkByteSwap::Swap4BERange((char *)p,num); };
  static void Swap4BERange(int *i,int num) 
  { vtkByteSwap::Swap4BERange((char *)i,num); };
  static void Swap4BERange(unsigned long *i,int num) 
  { vtkByteSwap::Swap4BERange((char *)i,num); };


  // Description:
  // Swap bunch of bytes to BE. Num is the number of four byte words to swap.
  // The results are written out to prevent having to keep the swapped
  // copy in memory.
  static void SwapWrite4BERange(char *c,int num,FILE *fp);
  static void SwapWrite4BERange(float *p,int num, FILE *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)p,num,fp);};
  static void SwapWrite4BERange(int *i,int num,FILE *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)i,num,fp);};
  static void SwapWrite4BERange(unsigned long *i,int num, FILE *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)i,num,fp);};

  static void SwapWrite4BERange(char *c,int num, ostream *fp);
  static void SwapWrite4BERange(float *p,int num, ostream *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)p,num,fp);};
  static void SwapWrite4BERange(int *i,int num, ostream *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)i,num,fp);};
  static void SwapWrite4BERange(unsigned long *i,int num, ostream *fp) 
  { vtkByteSwap::SwapWrite4BERange((char *)i,num,fp);};


  // Description:
  // Swap 2 byte word to BE.
  static void Swap2BE(short *s);

  // Description:
  // Swap bunch of bytes to BE. Num is the number of two byte words to swap.
  static void Swap2BERange(char *c,int num);
  static void Swap2BERange(short *i,int num) 
  { vtkByteSwap::Swap2BERange((char *)i,num);};

  // Description:
  // Swap bunch of bytes to LE. Num is the number of two byte words to swap.
  static void Swap2LERange(char *c,int num);
  static void Swap2LERange(short *i,int num) 
  { vtkByteSwap::Swap2LERange((char *)i,num);};


  // Description:
  // Swap bunch of bytes to BE. Num is the number of two byte words to swap.
  // The results are written out to prevent having to keep the swapped
  // copy in memory.
  static void SwapWrite2BERange(char *c,int num,FILE *fp);
  static void SwapWrite2BERange(short *i,int num, FILE *fp) 
  {vtkByteSwap::SwapWrite2BERange((char *)i,num,fp);};

  static void SwapWrite2BERange(char *c,int num, ostream *fp);
  static void SwapWrite2BERange(short *i,int num, ostream *fp) 
  {vtkByteSwap::SwapWrite2BERange((char *)i,num,fp);};

  // Description:
  // Swaps the bytes of a buffer.  Uses an arbitrary word size, but
  // assumes the word size is divisible by two.
  static void SwapVoidRange(void *buffer, int numWords, int wordSize);

protected:
  vtkByteSwap() {};
  ~vtkByteSwap() {};
  vtkByteSwap(const vtkByteSwap&) {};
  void operator=(const vtkByteSwap&) {};

};

#endif
