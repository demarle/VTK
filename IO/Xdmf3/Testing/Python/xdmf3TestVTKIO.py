import os
import random
import resource
import sys
import vtk

try:
  import argparse
except ImportError:
  import _argparse as argparse

from vtk.test import Testing
from vtk.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

CleanUpGood = True
timer = vtk.vtkTimerLog()

testObjects = [
    "ID1",
    "ID2",
    "UF1",
    "RG1",
#    "SG1",
    "PD1",
#    "PD2",
    "UG1",
    "UG2",
    "UG3",
    "UG4",
    "MB{}",
    "MB{ID1}",
    "MB{UF1}",
    "MB{RG1}",
    "MB{SG1}",
    "MB{PD1}",
    "MB{UG1}",
    "MB{ ID1 UF1 RG1 SG1 PD1 UG1 }"
    "HB[ (UF1)(UF1)(UF1) ]"
    ]

def Usage(point):
  usage=resource.getrusage(resource.RUSAGE_SELF)
  return '''%s: usertime=%s systime=%s mem=%s mb\
         '''%(point,usage[0],usage[1],\
              (usage[2]*resource.getpagesize())/1000000.0 )

def raiseErrorAndExit(message):
  raise Exception, message
  sys.exit(vtk.VTK_ERROR)

def DoFilesExist(xdmfFile, hdf5File, vtkFile, deleteIfSo):
  global CleanUpGood
  xexists = os.path.exists(xdmfFile)
  xlenOK = os.path.getsize(xdmfFile) > 0
  if hdf5File:
    hexists = os.path.exists(hdf5File)
    hlenOK = os.path.getsize(hdf5File) > 0
  if vtkFile:
    vexists = os.path.exists(vtkFile)
    vlenOK = os.path.getsize(vtkFile) > 0

  theyDo = xexists and xlenOK
  if hdf5File:
    theyDo = theyDo and hexists and hlenOK
  if vtkFile:
    theyDo = theyDo and vexists and vlenOK

  if theyDo and deleteIfSo and CleanUpGood:
    os.remove(xdmfFile)
    if hdf5File:
      os.remove(hdf5File)
    if vtkFile:
      os.remove(vtkFile)

  return theyDo

def DoDataObjectsDiffer(dobj1, dobj2):
  class1 = dobj1.GetClassName()
  class2 = dobj2.GetClassName()
  if class1 == "vtkImageData" or\
     class1 == "vtkPolyData":
    if class2 == "vtkUniformGrid" or\
       class2 == "vtkUnstructuredGrid":
      pass
    else:
      message = "Error: dobj1 is a " + class1 + " and dobj2 is a " + class2
      raiseErrorAndExit(message)

  elif class1 != class2:
    message = "Error: Class name test failed: " +\
              class1 + " != " + class2
    raiseErrorAndExit(message)

  if dobj1.GetFieldData().GetNumberOfArrays() !=\
     dobj2.GetFieldData().GetNumberOfArrays():
    raiseErrorAndExit("Number of field arrays test failed")

  if not dobj1.IsA("vtkPolyData") and\
     not dobj2.IsA("vtkMultiBlockDataSet") and\
     dobj1.GetActualMemorySize() != dobj2.GetActualMemorySize():
    message = "Memory size test failed"
    message += " M1 = " + str(dobj1.GetActualMemorySize())
    message += " M2 = " + str(dobj2.GetActualMemorySize())
    raiseErrorAndExit(message)

  ds1 = vtk.vtkDataSet.SafeDownCast(dobj1)
  ds2 = vtk.vtkDataSet.SafeDownCast(dobj2)
  if ds1 and ds2:
    if (ds1.GetNumberOfCells() != ds2.GetNumberOfCells()) or\
       (ds1.GetNumberOfPoints() != ds2.GetNumberOfPoints()):
      message = "Number of Cells/Points test Failed."
      message += " C1 = " + str(ds1.GetNumberOfCells())
      message += " C2 = " + str(ds2.GetNumberOfCells())
      message += " P1 = " + str(ds1.GetNumberOfPoints())
      message += " P2 = " + str(ds2.GetNumberOfPoints())
      raiseErrorAndExit(message)
    bds1 = ds1.GetBounds()
    bds2 = ds2.GetBounds()
    if (bds1[0]!=bds2[0]) or\
       (bds1[1]!=bds2[1]) or\
       (bds1[2]!=bds2[2]) or\
       (bds1[3]!=bds2[3]) or\
       (bds1[4]!=bds2[4]) or\
       (bds1[5]!=bds2[5]):
      raiseErrorAndExit("Bounds test failed")

    if (ds1.GetPointData().GetNumberOfArrays() !=\
       ds2.GetPointData().GetNumberOfArrays()) or\
       (ds1.GetCellData().GetNumberOfArrays() !=\
       ds2.GetCellData().GetNumberOfArrays()):
      message = "Number of data arrays test Failed."
      message += " CD1 = " + str(ds1.GetCellData().GetNumberOfArrays())
      message += " CD2 = " + str(ds2.GetCellData().GetNumberOfArrays())
      message += " PD1 = " + str(ds1.GetPointData().GetNumberOfArrays())
      message += " PD2 = " + str(ds2.GetPointData().GetNumberOfArrays())
      raiseErrorAndExit(message)

  return False

def TestXdmfConversion(dataInput, fileName):
  global CleanUpGood, timer
  xdmfFile = fileName + ".xmf"
  hdf5File = fileName + ".h5"
  vtkFile = fileName + ".vtk"

  xwriter = vtk.vtkXdmf3Writer()
  xwriter.SetLightDataLimit(10000)
  xwriter.WriteAllTimeStepsOn()
  xwriter.SetFileName(xdmfFile)
  xwriter.SetInputData(dataInput)
  timer.StartTimer()
  xwriter.Write()
  timer.StopTimer()
  print "vtkXdmf3Writer took", timer.GetElapsedTime(), "seconds to write",\
    xdmfFile

  ds = vtk.vtkDataSet.SafeDownCast(dataInput)
  if ds:
    dsw = vtk.vtkDataSetWriter()
    dsw.SetFileName(vtkFile)
    dsw.SetInputData(ds)
    dsw.Write()

  if not DoFilesExist(xdmfFile, None, None, False):
    message = "Writer did not create " + xdmfFile
    raiseErrorAndExit(message)

  xReader = vtk.vtkXdmf3Reader()
  xReader.SetFileName(xdmfFile)
  timer.StartTimer()
  xReader.Update()
  timer.StopTimer()
  print "vtkXdmf3Reader took", timer.GetElapsedTime(), "seconds to read",\
    xdmfFile

  rOutput = xReader.GetOutputDataObject(0)

  fail = DoDataObjectsDiffer(dataInput, rOutput)

  if fail:
    raiseErrorAndExit("Xdmf conversion test failed")
  else:
    if ds:
      DoFilesExist(xdmfFile, hdf5File, vtkFile, CleanUpGood)
    else:
      DoFilesExist(xdmfFile, hdf5File, None, CleanUpGood)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Test Xdmf3 IO")
  parser.add_argument("-dc", "--dont-clean", dest="dontClean",\
                      help="Do not delete files created during test\
                      after testing complete (Default = False)",\
                      action="store_true")
  args = parser.parse_args()
  if args.dontClean:
    CleanUpGood = False

  fail = False

  # TEST SET 1
  print Usage("Before starting TEST SET 1")
  dog = vtk.vtkDataObjectGenerator()
  for testObject in testObjects:
    i = 0
    fileName = "xdmfIOtest_" + str(i)
    print "Test vtk object", testObject
    dog.SetProgram(testObject)
    dog.Update()
    TestXdmfConversion(dog.GetOutput(), fileName)
    i += 1


  # TEST SET 2
  print Usage("Before starting TEST SET 2")
  print "Test temporal data"
  tsrc = vtk.vtkTimeSourceExample()
  tsrc.GrowingOn()
  tsrc.SetXAmplitude(2.0)

  tFilePrefix = "xdmfIOTest_Temporal"
  tFileName = tFilePrefix + ".xdmf"
  thFileName = tFilePrefix + ".h5"
  xwriter = vtk.vtkXdmf3Writer()
  xwriter.SetLightDataLimit(10000)
  xwriter.WriteAllTimeStepsOn()
  xwriter.SetFileName(tFileName)
  xwriter.SetInputConnection(0, tsrc.GetOutputPort(0))
  timer.StartTimer()
  xwriter.Write()
  timer.StopTimer()
  print "vtkXdmf3Writer took", timer.GetElapsedTime(), "seconds to write",\
    tFileName

  fail = DoFilesExist(tFileName, thFileName, None, CleanUpGood)

  if not fail:
    raiseErrorAndExit("Failed Temporal Test")

  # TEST SET 3
  print Usage("Before starting TEST SET 3")
  print "Test Graph data"
  gsrc = vtk.vtkRandomGraphSource()
  gsrc.Update()

  gFilePrefix = "xdmfIOTest_Graph"
  gFileName = gFilePrefix + ".xdmf"
  ghFileName = gFilePrefix + ".h5"
  xwriter = vtk.vtkXdmf3Writer()
  xwriter.SetLightDataLimit(10000)
  xwriter.SetFileName(gFileName)
  xwriter.SetInputConnection(0, gsrc.GetOutputPort(0))
  timer.StartTimer()
  xwriter.Write()
  timer.StopTimer()
  print "vtkXdmf3Writer took", timer.GetElapsedTime(), "seconds to write",\
    gFileName

  fail = DoFilesExist(gFileName, ghFileName, None, CleanUpGood)

  if not fail:
    raiseErrorAndExit("Failed Graph Test")

  print Usage("End of Testing")
