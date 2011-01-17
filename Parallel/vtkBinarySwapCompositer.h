/*
 * vtkBinarySwapCompositer.h
 *
 *  Created on: Jun 25, 2009
 *      Author: ollie
 */

#ifndef VTKBSWAPCOMPOSITER_H_
#define VTKBSWAPCOMPOSITER_H_

#include "vtkCompositer.h"

// This is only a wrapper for the vtkCompositer interface. The real work is done
// by BinarySwapCompositer and its subclasses.
class VTK_PARALLEL_EXPORT vtkBinarySwapCompositer : public vtkCompositer
{
public:
 static vtkBinarySwapCompositer *New();
 vtkTypeMacro(vtkBinarySwapCompositer, vtkCompositer);

 virtual void CompositeBuffer(vtkDataArray *pBuf, vtkFloatArray *zBuf,
            vtkDataArray *pTmp, vtkFloatArray *zTmp);

protected:
 vtkBinarySwapCompositer()  {};
 ~vtkBinarySwapCompositer() {};

private:
 vtkBinarySwapCompositer(const vtkBinarySwapCompositer&); // not implemented
 void operator=(const vtkBinarySwapCompositer&);  // not implemented

 bool IsPow2(unsigned p) {
  return ( (p > 0) && ((p & (p - 1)) == 0) );
 }
};

#endif /* VTKBSWAPCOMPOSITER_H_ */
