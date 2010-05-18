/*=========================================================================

  Program:   Visualization Toolkit
  Module:    QVTKInteractor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*=========================================================================

  Copyright 2004 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/

/*========================================================================
 For general information about using VTK and Qt, see:
 http://www.trolltech.com/products/3rdparty/vtksupport.html
=========================================================================*/

// .NAME QVTKInteractor - Handle Qt events.
// .SECTION Description
// QVTKInteractor handles relaying Qt events to VTK.

#ifndef Q_VTK_INTERACTOR_H
#define Q_VTK_INTERACTOR_H

#include <vtkRenderWindowInteractor.h>
#include "QVTKWin32Header.h"
#include "vtkstd/map"
class QSignalMapper;
class QTimer;
#include <QtCore/QObject>

#include "vtkTDxConfigure.h" // defines VTK_USE_TDX
#if defined(VTK_USE_TDX) && defined(Q_WS_WIN)
class vtkTDxWinDevice;
#endif
#if defined(VTK_USE_TDX) && defined(Q_WS_MAC)
class vtkTDxMacDevice;
#endif


class QVTKInteractorInternal;

// .NAME QVTKInteractor - An interactor for the QVTKWidget.
// .SECTION Description
// QVTKInteractor is an interactor for a QVTKWiget.

class QVTK_EXPORT QVTKInteractor : public vtkRenderWindowInteractor
{
public:
  static QVTKInteractor* New();
  vtkTypeMacro(QVTKInteractor,vtkRenderWindowInteractor);

  // Description:
  // Overloaded terminiate app, which does nothing in Qt.
  // Use qApp->exit() instead.
  virtual void TerminateApp();

  // Description:
  // Overloaded start method does nothing.
  // Use qApp->exec() instead.
  virtual void Start();
  virtual void Initialize();

  // Description:
  // Start listening events on 3DConnexion device.
  virtual void StartListening();

  // Description:
  // Stop listening events on 3DConnexion device.
  virtual void StopListening();

  // timer event slot
  virtual void TimerEvent(int timerId);

protected:
  // constructor
  QVTKInteractor();
  // destructor
  ~QVTKInteractor();

  // create a Qt Timer
  virtual int InternalCreateTimer(int timerId, int timerType, unsigned long duration);
  // destroy a Qt Timer
  virtual int InternalDestroyTimer(int platformTimerId);
#if defined(VTK_USE_TDX) && defined(Q_WS_WIN)
  vtkTDxWinDevice *Device;
#endif
#if defined(VTK_USE_TDX) && defined(Q_WS_MAC)
  vtkTDxMacDevice *Device;
#endif

private:

  QVTKInteractorInternal* Internal;

  // unimplemented copy
  QVTKInteractor(const QVTKInteractor&);
  // unimplemented operator=
  void operator=(const QVTKInteractor&);

};

// .NAME QVTKInteractorAdapter - A QEvent translator.
// .SECTION Description
// QVTKInteractorAdapter translates QEvents and send them to a
// vtkRenderWindowInteractor.
class QVTKInteractorAdapter : public QObject
{
  Q_OBJECT
public:
  // Description:
  // Constructor: takes QObject parent
  QVTKInteractorAdapter(QObject* parent);

  // Description:
  // Destructor
  ~QVTKInteractorAdapter();

  // Description:
  // Process a QEvent and send it to the interactor
  // returns whether the event was recognized and processed
  bool ProcessEvent(QEvent* e, vtkRenderWindowInteractor* iren);
};


// internal class, do not use
class QVTKInteractorInternal : public QObject
{
  Q_OBJECT
public:
  QVTKInteractorInternal(QVTKInteractor* p);
  ~QVTKInteractorInternal();
public Q_SLOTS:
  void TimerEvent(int id);
public:
  QSignalMapper* SignalMapper;
  typedef vtkstd::map<int, QTimer*> TimerMap;
  TimerMap Timers;
  QVTKInteractor* Parent;
};

#endif
