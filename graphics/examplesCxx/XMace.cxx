
// include OS specific include file to mix in X code
#include "vtkXRenderWindow.h"
#include "vtkXRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkSphereSource.h"
#include "vtkConeSource.h"
#include "vtkGlyph3D.h"
#include "vtkPolyDataMapper.h"
#include <Xm/ArrowB.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

void quit_cb(Widget,XtPointer,XtPointer);

main (int argc, char *argv[])
{
  // X window stuff
  XtAppContext app;
  Widget toplevel, form, toplevel2, vtk;
  Widget button[4];
  int depth;
  Visual *vis;
  Display *display;
  Colormap col;
  
  // VTK stuff
  vtkXRenderWindow *renWin;
  vtkRenderer *ren1;
  vtkActor *sphereActor1, *spikeActor1;
  vtkSphereSource *sphere;
  vtkConeSource *cone;
  vtkGlyph3D *glyph;
  vtkPolyDataMapper *sphereMapper, *spikeMapper;
  vtkXRenderWindowInteractor *iren;
  
  renWin = (vtkXRenderWindow *)vtkRenderWindow::New();
  ren1 = vtkRenderer::New();
  renWin->AddRenderer(ren1);
  
  sphere = vtkSphereSource::New();
  sphereMapper = vtkPolyDataMapper::New();
  sphereMapper->SetInput(sphere->GetOutput());
  sphereActor1 = vtkActor::New();
  sphereActor1->SetMapper(sphereMapper);

  cone = vtkConeSource::New();

  glyph = vtkGlyph3D::New();
  glyph->SetInput(sphere->GetOutput());
  glyph->SetSource(cone->GetOutput());
  glyph->SetVectorModeToUseNormal();
  glyph->SetScaleModeToScaleByVector();
  glyph->SetScaleFactor(0.25);

  spikeMapper = vtkPolyDataMapper::New();
  spikeMapper->SetInput(glyph->GetOutput());

  spikeActor1 = vtkActor::New();
  spikeActor1->SetMapper(spikeMapper);

  ren1->AddActor(sphereActor1);
  ren1->AddActor(spikeActor1);
  ren1->SetBackground(0.4,0.1,0.2);

  // do the xwindow ui stuff
  XtSetLanguageProc(NULL,NULL,NULL);
  toplevel = XtVaAppInitialize(&app,"Prog6",NULL,0,&argc,argv,NULL, NULL);
  
  // get the display connection and give it to the renderer
  display = XtDisplay(toplevel);
  renWin->SetDisplayId(display);
  depth = renWin->GetDesiredDepth();
  vis = renWin->GetDesiredVisual();
  col = renWin->GetDesiredColormap();
  
  toplevel2 = XtVaCreateWidget("top2",topLevelShellWidgetClass, toplevel,
			       XmNdepth, depth,
			       XmNvisual, vis,
			       XmNcolormap, col,
			       NULL);
  form     = XtVaCreateWidget("form",xmFormWidgetClass, toplevel2, NULL);
  vtk      = XtVaCreateManagedWidget("vtk",xmPrimitiveWidgetClass, form, 
				     XmNwidth, 300,
				     XmNheight, 300,
				     NULL);

  button[0] = XtVaCreateManagedWidget("arrow1",xmArrowButtonWidgetClass, form,
				      XmNarrowDirection, XmARROW_LEFT,
				      XmNwidth, 50,
				      XmNheight, 50,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, vtk,
				      XmNleftAttachment, XmATTACH_POSITION,
				      XmNleftPosition, 0,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 25,
				      NULL);

  XtAddCallback(button[0],XmNactivateCallback,quit_cb,NULL);

  XtManageChild(form);
  XtRealizeWidget(toplevel2);
  XtMapWidget(toplevel2);
  
  // we typecast to an X specific interactor
  // Since we have specifically decided to make this 
  // an X windows program
  iren = (vtkXRenderWindowInteractor *)vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);
  iren->SetWidget(vtk);
  iren->Initialize(app);
  renWin->Render();

  XtAppMainLoop(app);
}

/* quit when the arrow */
void quit_cb(Widget w,XtPointer client_data, XtPointer call_data)
{
  exit(0);
}

