/*==========================================================================

  Program: 
  Module:    otherCoordinate.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  ==========================================================================*/

// .NAME 
// .SECTION Description
// this program tests vtkCoordinate

#include "vtkCoordinate.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"

// All tests need:
//   the following include
//   a Selector proc
//   a Comparator proc
//   a Test proc
//   and a main
#include "rtOtherTestBase.h"

void SelectorCommand(vtkOstream& strm) {
  strm << "sed -e s/0x0/0/ | sed -e s/-0/0/ | grep -v 0x | grep -v Modified ";
}

void ComparatorCommand(vtkOstream& strm) {
  strm << "diff";
}

void ToAll (vtkOstream& strm, vtkCoordinate *c1, vtkViewport *ren1, 
            float *from)
{
  float *value;
  int *ivalue;
  const char *whichCoord = c1->GetCoordinateSystemAsString();

  c1->SetValue (from);

  strm << endl << "========" << endl;
  strm << *c1;
  value = c1->GetComputedWorldValue (ren1);
  strm << whichCoord <<"(" << from[0] << ", " << from[1] << ", " << from[2]
       << ") -> World(" << value[0] << ", " << value[1] << ", " << value[2] 
       << ")" << endl;
  ivalue = c1->GetComputedDisplayValue (ren1);
  strm << whichCoord << "(" << from[0] << ", " << from[1] << ", " << from[2]
       << ") -> Display(" << ivalue[0] << ", " << ivalue[1] << ")" << endl;
  ivalue = c1->GetComputedLocalDisplayValue (ren1);
  strm << whichCoord << "(" << from[0] << ", " << from[1] << ", " << from[2]
       << ") -> LocalDisplay(" << ivalue[0] << ", " << ivalue[1] 
       << ")" << endl;
  ivalue = c1->GetComputedViewportValue (ren1);
  strm << whichCoord << "(" << from[0] << ", " << from[1] << ", " << from[2]
       << ") -> Viewport(" << ivalue[0] << ", " << ivalue[1] << ")" << endl;


}
void Test(vtkOstream& strm)
{
  // actual test
  strm << "Testing vtkCoordinate" << endl;
  vtkCoordinate *c1 = vtkCoordinate::New();
  vtkCoordinate *c2 = vtkCoordinate::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  vtkRenderer *ren1 = vtkRenderer::New();
  vtkCamera *camera = vtkCamera::New();
  float from[3];
  
  ren1->SetActiveCamera (camera);
  renWin->AddRenderer (ren1);
  renWin->SetSize (100, 100);
  
  strm << "Origin: (" << ren1->GetOrigin()[0] << ", " << ren1->GetOrigin()[1] << ")" << endl;
  strm << "Center: (" << ren1->GetCenter()[0] << ", " << ren1->GetOrigin()[1] << ")" << endl;

  strm << endl << "********** A NULL Viewport **********" << endl;
  
  c1->SetCoordinateSystemToWorld();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);  

  c1->SetCoordinateSystemToDisplay();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedDisplay();
  from[0] = .5; from[1] = .5; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToViewport();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedViewport();
  from[0] = .5; from[1] = .5; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToView();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  strm << endl << "********** A specified Viewport **********" << endl;
  c1->SetViewport (ren1);
  
  c1->SetCoordinateSystemToWorld();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);  

  c1->SetCoordinateSystemToDisplay();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedDisplay();
  from[0] = .5; from[1] = .5; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToViewport();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedViewport();
  from[0] = .5; from[1] = .5; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToView();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  strm << endl << "********** With a Reference Coordinate **********" << endl;

  c2->SetCoordinateSystemToNormalizedDisplay();
  c2->SetCoordinateSystemToWorld();
  c2->SetValue (0.0, 0.0, 0.0);
  c1->SetReferenceCoordinate (c2);
  
  strm << *c2;
  
  c1->SetCoordinateSystemToWorld();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);  

  c1->SetCoordinateSystemToDisplay();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedDisplay();
  from[0] = .5; from[1] = .5; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToViewport();
  from[0] = 50; from[1] = 50; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToNormalizedViewport();
  from[0] = .5; from[1] = .5; from[2] = 0;
  ToAll (strm, c1, ren1, from);
  
  c1->SetCoordinateSystemToView();
  from[0] = 0.0; from[1] = 0.0; from[2] = 0.0;
  ToAll (strm, c1, ren1, from);
  
  
  c1->Delete ();
  c2->Delete ();
  renWin->Delete ();
  ren1->Delete ();
  camera->Delete ();

  strm << "Testing completed" << endl;
  
}

int main(int argc, char* argv[])
{
  rtOtherTestBase::RunTest(argc, argv, SelectorCommand, ComparatorCommand, Test);
  return 0;  
}

