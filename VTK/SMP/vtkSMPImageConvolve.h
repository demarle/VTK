#ifndef __vtkSMPImageConvolve_h
#define __vtkSMPImageConvolve_h

#include "vtkImageAlgorithm.h"

class VTK_SMP_EXPORT vtkSMPImageConvolve : public vtkImageAlgorithm
{
public:
  // Description:
  // Construct an instance of vtkImageConvolve filter.
  static vtkSMPImageConvolve *New();
  vtkTypeMacro(vtkSMPImageConvolve,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the kernel size
  vtkGetVector3Macro(KernelSize, int);

  // Description:
  // Set the kernel to be a given 3x3 or 5x5 or 7x7 kernel.
  void SetKernel3x3(const double kernel[9]);
  void SetKernel5x5(const double kernel[25]);
//BTX
  void SetKernel7x7(double kernel[49]);
//ETX

  // Description:
  // Return an array that contains the kernel.
  double* GetKernel3x3();
  void GetKernel3x3(double kernel[9]);
  double* GetKernel5x5();
  void GetKernel5x5(double kernel[25]);
//BTX
  double* GetKernel7x7();
  void GetKernel7x7(double kernel[49]);
//ETX

  // Description:
  // Set the kernel to be a 3x3x3 or 5x5x5 or 7x7x7 kernel.
  void SetKernel3x3x3(const double kernel[27]);
//BTX
  void SetKernel5x5x5(double kernel[125]);
  void SetKernel7x7x7(double kernel[343]);
//ETX

  // Description:
  // Return an array that contains the kernel
  double* GetKernel3x3x3();
  void GetKernel3x3x3(double kernel[27]);
//BTX
  double* GetKernel5x5x5();
  void GetKernel5x5x5(double kernel[125]);
  double* GetKernel7x7x7();
  void GetKernel7x7x7(double kernel[343]);
//ETX

protected:
  vtkSMPImageConvolve();
  ~vtkSMPImageConvolve();

  virtual int RequestData(vtkInformation *request,
                   vtkInformationVector **inputVector,
                   vtkInformationVector *outputVector);

  void GetKernel(double *kernel);
  double* GetKernel();
  void SetKernel(const double* kernel,
                 int sizeX, int sizeY, int sizeZ);


  int KernelSize[3];
  double Kernel[343];
private:
  vtkSMPImageConvolve(const vtkSMPImageConvolve&);  // Not implemented.
  void operator=(const vtkSMPImageConvolve&);  // Not implemented.
};

#endif
