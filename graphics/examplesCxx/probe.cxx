#include "vtk.h"

main ()
{
  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer(renderer);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

  // read data
  vtkPLOT3DReader *reader = vtkPLOT3DReader::New();
    reader->DebugOn();
    reader->SetXYZFileName("../../../vtkdata/combxyz.bin");
    reader->SetQFileName("../../../vtkdata/combq.bin");
    reader->SetFileFormat(VTK_WHOLE_SINGLE_GRID_NO_IBLANKING);
    reader->Update(); //force a read to occur

  // create outline
  vtkStructuredGridOutlineFilter *outlineF = vtkStructuredGridOutlineFilter::New();
    outlineF->SetInput(reader->GetOutput());
  vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
    outlineMapper->SetInput(outlineF->GetOutput());
  vtkActor *outline = vtkActor::New();
    outline->SetMapper(outlineMapper);
    outline->GetProperty()->SetColor(0,0,0);

  // create cursor
  vtkCursor3D *cursor = vtkCursor3D::New();
    cursor->SetFocalPoint(reader->GetOutput()->GetCenter());
    cursor->SetModelBounds(reader->GetOutput()->GetBounds());
    cursor->AxesOn();
    cursor->OutlineOff();
    cursor->XShadowsOff();
    cursor->YShadowsOff();
    cursor->ZShadowsOff();
  vtkPolyDataMapper *cursorMapper = vtkPolyDataMapper::New();
    cursorMapper->SetInput(cursor->GetOutput());
  vtkActor *cursorActor = vtkActor::New();
    cursorActor->SetMapper(cursorMapper);
    cursorActor->GetProperty()->SetColor(1,0,0);

  // create probe
  vtkProbeFilter *probe = vtkProbeFilter::New();
    probe->SetSource(reader->GetOutput());
    probe->SetInput(cursor->GetFocus());

  // create a cone geometry for glyph
  vtkConeSource *cone = vtkConeSource::New();
    cone->SetResolution(16);
    cone->SetRadius(0.25);

  // create glyph
  vtkGlyph3D *glyph = vtkGlyph3D::New();
    glyph->SetInput(probe->GetOutput());
    glyph->SetSource(cone->GetOutput());
    glyph->SetVectorModeToUseVector();
    glyph->SetScaleModeToScaleByScalar();
    glyph->SetScaleFactor(10);
  vtkPolyDataMapper *glyphMapper = vtkPolyDataMapper::New();
    glyphMapper->SetInput(glyph->GetOutput());
  vtkActor *glyphActor = vtkActor::New();
    glyphActor->SetMapper(glyphMapper);

  renderer->AddActor(outline);
  renderer->AddActor(cursorActor);
  renderer->AddActor(glyphActor);
  renderer->SetBackground(1.0,1.0,1.0);
  renWin->SetSize(752,752);
  renWin->Render();

  iren->Start();

  // Clean up
  renderer->Delete();
  renWin->Delete();
  iren->Delete();
  reader->Delete();
  outlineF->Delete();
  outlineMapper->Delete();
  outline->Delete();
  cursor->Delete();
  cursorMapper->Delete();
  cursorActor->Delete();
  probe->Delete();
  cone->Delete();
  glyph->Delete();
  glyphMapper->Delete();
  glyphActor->Delete();
}
