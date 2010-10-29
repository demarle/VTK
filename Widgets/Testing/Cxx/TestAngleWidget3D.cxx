/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestAngleWidget3D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
//
// This example tests the vtkAngleWidget.

#include "vtkSmartPointer.h"
#include "vtkAngleWidget.h"
#include "vtkAngleRepresentation3D.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTestUtilities.h"
#include "vtkCoordinate.h"
#include "vtkMath.h"
#include "vtkHandleWidget.h"
#include "vtkSphereSource.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkAxisActor2D.h"
#include "vtkProperty2D.h"
#include "vtkTesting.h"

char TestAngleWidget3DEventLog[] =
"# StreamVersion 1\n"
"RenderEvent 0 0 0 0 0 0 0\n"
"EnterEvent 2 184 0 0 0 0 0\n"
"MouseMoveEvent 10 194 0 0 0 0 0\n"
"MouseMoveEvent 22 202 0 0 0 0 0\n"
"MouseMoveEvent 34 210 0 0 0 0 0\n"
"MouseMoveEvent 50 216 0 0 0 0 0\n"
"MouseMoveEvent 70 224 0 0 0 0 0\n"
"MouseMoveEvent 92 230 0 0 0 0 0\n"
"MouseMoveEvent 114 234 0 0 0 0 0\n"
"MouseMoveEvent 134 238 0 0 0 0 0\n"
"MouseMoveEvent 158 240 0 0 0 0 0\n"
"MouseMoveEvent 180 240 0 0 0 0 0\n"
"MouseMoveEvent 204 240 0 0 0 0 0\n"
"MouseMoveEvent 226 240 0 0 0 0 0\n"
"MouseMoveEvent 244 240 0 0 0 0 0\n"
"MouseMoveEvent 260 240 0 0 0 0 0\n"
"MouseMoveEvent 274 236 0 0 0 0 0\n"
"MouseMoveEvent 284 236 0 0 0 0 0\n"
"MouseMoveEvent 290 234 0 0 0 0 0\n"
"MouseMoveEvent 291 233 0 0 0 0 0\n"
"MouseMoveEvent 289 233 0 0 0 0 0\n"
"MouseMoveEvent 283 231 0 0 0 0 0\n"
"MouseMoveEvent 271 231 0 0 0 0 0\n"
"MouseMoveEvent 263 229 0 0 0 0 0\n"
"MouseMoveEvent 251 225 0 0 0 0 0\n"
"MouseMoveEvent 239 223 0 0 0 0 0\n"
"MouseMoveEvent 227 221 0 0 0 0 0\n"
"MouseMoveEvent 213 217 0 0 0 0 0\n"
"MouseMoveEvent 197 213 0 0 0 0 0\n"
"MouseMoveEvent 183 213 0 0 0 0 0\n"
"MouseMoveEvent 169 211 0 0 0 0 0\n"
"MouseMoveEvent 153 211 0 0 0 0 0\n"
"MouseMoveEvent 139 211 0 0 0 0 0\n"
"MouseMoveEvent 127 211 0 0 0 0 0\n"
"MouseMoveEvent 115 211 0 0 0 0 0\n"
"MouseMoveEvent 105 211 0 0 0 0 0\n"
"MouseMoveEvent 102 211 0 0 0 0 0\n"
"MouseMoveEvent 99 211 0 0 0 0 0\n"
"MouseMoveEvent 97 211 0 0 0 0 0\n"
"MouseMoveEvent 96 211 0 0 0 0 0\n"
"MouseMoveEvent 95 211 0 0 0 0 0\n"
"MouseMoveEvent 94 211 0 0 0 0 0\n"
"MouseMoveEvent 93 211 0 0 0 0 0\n"
"MouseMoveEvent 92 212 0 0 0 0 0\n"
"MouseMoveEvent 90 212 0 0 0 0 0\n"
"MouseMoveEvent 84 214 0 0 0 0 0\n"
"MouseMoveEvent 83 214 0 0 0 0 0\n"
"MouseMoveEvent 81 214 0 0 0 0 0\n"
"MouseMoveEvent 80 214 0 0 0 0 0\n"
"MouseMoveEvent 79 214 0 0 0 0 0\n"
"MouseMoveEvent 78 214 0 0 0 0 0\n"
"MouseMoveEvent 79 214 0 0 0 0 0\n"
"MouseMoveEvent 81 214 0 0 0 0 0\n"
"MouseMoveEvent 82 214 0 0 0 0 0\n"
"MouseMoveEvent 84 214 0 0 0 0 0\n"
"MouseMoveEvent 86 214 0 0 0 0 0\n"
"MouseMoveEvent 87 214 0 0 0 0 0\n"
"MouseMoveEvent 89 214 0 0 0 0 0\n"
"MouseMoveEvent 91 214 0 0 0 0 0\n"
"MouseMoveEvent 93 214 0 0 0 0 0\n"
"MouseMoveEvent 94 215 0 0 0 0 0\n"
"MouseMoveEvent 95 215 0 0 0 0 0\n"
"MouseMoveEvent 96 215 0 0 0 0 0\n"
"MouseMoveEvent 97 215 0 0 0 0 0\n"
"MouseMoveEvent 98 215 0 0 0 0 0\n"
"MouseMoveEvent 99 215 0 0 0 0 0\n"
"MouseMoveEvent 100 215 0 0 0 0 0\n"
"MouseMoveEvent 102 215 0 0 0 0 0\n"
"MouseMoveEvent 104 214 0 0 0 0 0\n"
"MouseMoveEvent 106 214 0 0 0 0 0\n"
"MouseMoveEvent 114 214 0 0 0 0 0\n"
"MouseMoveEvent 120 212 0 0 0 0 0\n"
"MouseMoveEvent 126 210 0 0 0 0 0\n"
"MouseMoveEvent 132 208 0 0 0 0 0\n"
"MouseMoveEvent 134 208 0 0 0 0 0\n"
"MouseMoveEvent 136 208 0 0 0 0 0\n"
"LeftButtonPressEvent 136 208 0 0 0 0 0\n"
"RenderEvent 136 208 0 0 0 0 0\n"
"LeftButtonReleaseEvent 136 208 0 0 0 0 0\n"
"MouseMoveEvent 135 207 0 0 0 0 0\n"
"RenderEvent 135 207 0 0 0 0 0\n"
"MouseMoveEvent 133 206 0 0 0 0 0\n"
"RenderEvent 133 206 0 0 0 0 0\n"
"MouseMoveEvent 132 205 0 0 0 0 0\n"
"RenderEvent 132 205 0 0 0 0 0\n"
"MouseMoveEvent 126 203 0 0 0 0 0\n"
"RenderEvent 126 203 0 0 0 0 0\n"
"MouseMoveEvent 114 199 0 0 0 0 0\n"
"RenderEvent 114 199 0 0 0 0 0\n"
"MouseMoveEvent 110 195 0 0 0 0 0\n"
"RenderEvent 110 195 0 0 0 0 0\n"
"MouseMoveEvent 94 187 0 0 0 0 0\n"
"RenderEvent 94 187 0 0 0 0 0\n"
"MouseMoveEvent 92 186 0 0 0 0 0\n"
"RenderEvent 92 186 0 0 0 0 0\n"
"MouseMoveEvent 89 184 0 0 0 0 0\n"
"RenderEvent 89 184 0 0 0 0 0\n"
"MouseMoveEvent 84 182 0 0 0 0 0\n"
"RenderEvent 84 182 0 0 0 0 0\n"
"MouseMoveEvent 82 181 0 0 0 0 0\n"
"RenderEvent 82 181 0 0 0 0 0\n"
"MouseMoveEvent 79 177 0 0 0 0 0\n"
"RenderEvent 79 177 0 0 0 0 0\n"
"MouseMoveEvent 79 176 0 0 0 0 0\n"
"RenderEvent 79 176 0 0 0 0 0\n"
"MouseMoveEvent 76 173 0 0 0 0 0\n"
"RenderEvent 76 173 0 0 0 0 0\n"
"MouseMoveEvent 74 167 0 0 0 0 0\n"
"RenderEvent 74 167 0 0 0 0 0\n"
"MouseMoveEvent 68 161 0 0 0 0 0\n"
"RenderEvent 68 161 0 0 0 0 0\n"
"MouseMoveEvent 60 154 0 0 0 0 0\n"
"RenderEvent 60 154 0 0 0 0 0\n"
"MouseMoveEvent 56 148 0 0 0 0 0\n"
"RenderEvent 56 148 0 0 0 0 0\n"
"MouseMoveEvent 49 134 0 0 0 0 0\n"
"RenderEvent 49 134 0 0 0 0 0\n"
"MouseMoveEvent 47 128 0 0 0 0 0\n"
"RenderEvent 47 128 0 0 0 0 0\n"
"MouseMoveEvent 45 122 0 0 0 0 0\n"
"RenderEvent 45 122 0 0 0 0 0\n"
"MouseMoveEvent 44 121 0 0 0 0 0\n"
"RenderEvent 44 121 0 0 0 0 0\n"
"MouseMoveEvent 42 114 0 0 0 0 0\n"
"RenderEvent 42 114 0 0 0 0 0\n"
"MouseMoveEvent 41 112 0 0 0 0 0\n"
"RenderEvent 41 112 0 0 0 0 0\n"
"MouseMoveEvent 39 105 0 0 0 0 0\n"
"RenderEvent 39 105 0 0 0 0 0\n"
"MouseMoveEvent 38 103 0 0 0 0 0\n"
"RenderEvent 38 103 0 0 0 0 0\n"
"MouseMoveEvent 36 97 0 0 0 0 0\n"
"RenderEvent 36 97 0 0 0 0 0\n"
"MouseMoveEvent 34 91 0 0 0 0 0\n"
"RenderEvent 34 91 0 0 0 0 0\n"
"MouseMoveEvent 30 82 0 0 0 0 0\n"
"RenderEvent 30 82 0 0 0 0 0\n"
"MouseMoveEvent 30 80 0 0 0 0 0\n"
"RenderEvent 30 80 0 0 0 0 0\n"
"MouseMoveEvent 29 78 0 0 0 0 0\n"
"RenderEvent 29 78 0 0 0 0 0\n"
"MouseMoveEvent 29 76 0 0 0 0 0\n"
"RenderEvent 29 76 0 0 0 0 0\n"
"MouseMoveEvent 29 74 0 0 0 0 0\n"
"RenderEvent 29 74 0 0 0 0 0\n"
"MouseMoveEvent 28 67 0 0 0 0 0\n"
"RenderEvent 28 67 0 0 0 0 0\n"
"MouseMoveEvent 28 61 0 0 0 0 0\n"
"RenderEvent 28 61 0 0 0 0 0\n"
"MouseMoveEvent 28 60 0 0 0 0 0\n"
"RenderEvent 28 60 0 0 0 0 0\n"
"MouseMoveEvent 28 57 0 0 0 0 0\n"
"RenderEvent 28 57 0 0 0 0 0\n"
"MouseMoveEvent 28 56 0 0 0 0 0\n"
"RenderEvent 28 56 0 0 0 0 0\n"
"MouseMoveEvent 28 55 0 0 0 0 0\n"
"RenderEvent 28 55 0 0 0 0 0\n"
"MouseMoveEvent 28 54 0 0 0 0 0\n"
"RenderEvent 28 54 0 0 0 0 0\n"
"MouseMoveEvent 27 52 0 0 0 0 0\n"
"RenderEvent 27 52 0 0 0 0 0\n"
"MouseMoveEvent 27 51 0 0 0 0 0\n"
"RenderEvent 27 51 0 0 0 0 0\n"
"MouseMoveEvent 26 48 0 0 0 0 0\n"
"RenderEvent 26 48 0 0 0 0 0\n"
"MouseMoveEvent 25 47 0 0 0 0 0\n"
"RenderEvent 25 47 0 0 0 0 0\n"
"MouseMoveEvent 25 46 0 0 0 0 0\n"
"RenderEvent 25 46 0 0 0 0 0\n"
"MouseMoveEvent 25 45 0 0 0 0 0\n"
"RenderEvent 25 45 0 0 0 0 0\n"
"MouseMoveEvent 23 43 0 0 0 0 0\n"
"RenderEvent 23 43 0 0 0 0 0\n"
"MouseMoveEvent 23 42 0 0 0 0 0\n"
"RenderEvent 23 42 0 0 0 0 0\n"
"MouseMoveEvent 23 40 0 0 0 0 0\n"
"RenderEvent 23 40 0 0 0 0 0\n"
"MouseMoveEvent 22 40 0 0 0 0 0\n"
"RenderEvent 22 40 0 0 0 0 0\n"
"MouseMoveEvent 22 39 0 0 0 0 0\n"
"RenderEvent 22 39 0 0 0 0 0\n"
"MouseMoveEvent 22 38 0 0 0 0 0\n"
"RenderEvent 22 38 0 0 0 0 0\n"
"MouseMoveEvent 22 36 0 0 0 0 0\n"
"RenderEvent 22 36 0 0 0 0 0\n"
"MouseMoveEvent 21 36 0 0 0 0 0\n"
"RenderEvent 21 36 0 0 0 0 0\n"
"MouseMoveEvent 21 35 0 0 0 0 0\n"
"RenderEvent 21 35 0 0 0 0 0\n"
"MouseMoveEvent 21 34 0 0 0 0 0\n"
"RenderEvent 21 34 0 0 0 0 0\n"
"MouseMoveEvent 20 34 0 0 0 0 0\n"
"RenderEvent 20 34 0 0 0 0 0\n"
"MouseMoveEvent 20 33 0 0 0 0 0\n"
"RenderEvent 20 33 0 0 0 0 0\n"
"LeftButtonPressEvent 20 33 0 0 0 0 0\n"
"RenderEvent 20 33 0 0 0 0 0\n"
"LeftButtonReleaseEvent 20 33 0 0 0 0 0\n"
"MouseMoveEvent 22 33 0 0 0 0 0\n"
"RenderEvent 22 33 0 0 0 0 0\n"
"MouseMoveEvent 24 33 0 0 0 0 0\n"
"RenderEvent 24 33 0 0 0 0 0\n"
"MouseMoveEvent 26 33 0 0 0 0 0\n"
"RenderEvent 26 33 0 0 0 0 0\n"
"MouseMoveEvent 28 33 0 0 0 0 0\n"
"RenderEvent 28 33 0 0 0 0 0\n"
"MouseMoveEvent 40 31 0 0 0 0 0\n"
"RenderEvent 40 31 0 0 0 0 0\n"
"MouseMoveEvent 45 31 0 0 0 0 0\n"
"RenderEvent 45 31 0 0 0 0 0\n"
"MouseMoveEvent 48 31 0 0 0 0 0\n"
"RenderEvent 48 31 0 0 0 0 0\n"
"MouseMoveEvent 50 31 0 0 0 0 0\n"
"RenderEvent 50 31 0 0 0 0 0\n"
"MouseMoveEvent 52 30 0 0 0 0 0\n"
"RenderEvent 52 30 0 0 0 0 0\n"
"MouseMoveEvent 59 30 0 0 0 0 0\n"
"RenderEvent 59 30 0 0 0 0 0\n"
"MouseMoveEvent 63 30 0 0 0 0 0\n"
"RenderEvent 63 30 0 0 0 0 0\n"
"MouseMoveEvent 65 30 0 0 0 0 0\n"
"RenderEvent 65 30 0 0 0 0 0\n"
"MouseMoveEvent 67 29 0 0 0 0 0\n"
"RenderEvent 67 29 0 0 0 0 0\n"
"MouseMoveEvent 69 29 0 0 0 0 0\n"
"RenderEvent 69 29 0 0 0 0 0\n"
"MouseMoveEvent 76 29 0 0 0 0 0\n"
"RenderEvent 76 29 0 0 0 0 0\n"
"MouseMoveEvent 83 29 0 0 0 0 0\n"
"RenderEvent 83 29 0 0 0 0 0\n"
"MouseMoveEvent 85 29 0 0 0 0 0\n"
"RenderEvent 85 29 0 0 0 0 0\n"
"MouseMoveEvent 92 29 0 0 0 0 0\n"
"RenderEvent 92 29 0 0 0 0 0\n"
"MouseMoveEvent 93 29 0 0 0 0 0\n"
"RenderEvent 93 29 0 0 0 0 0\n"
"MouseMoveEvent 95 29 0 0 0 0 0\n"
"RenderEvent 95 29 0 0 0 0 0\n"
"MouseMoveEvent 99 29 0 0 0 0 0\n"
"RenderEvent 99 29 0 0 0 0 0\n"
"MouseMoveEvent 101 29 0 0 0 0 0\n"
"RenderEvent 101 29 0 0 0 0 0\n"
"MouseMoveEvent 102 29 0 0 0 0 0\n"
"RenderEvent 102 29 0 0 0 0 0\n"
"MouseMoveEvent 104 29 0 0 0 0 0\n"
"RenderEvent 104 29 0 0 0 0 0\n"
"MouseMoveEvent 111 28 0 0 0 0 0\n"
"RenderEvent 111 28 0 0 0 0 0\n"
"MouseMoveEvent 113 28 0 0 0 0 0\n"
"RenderEvent 113 28 0 0 0 0 0\n"
"MouseMoveEvent 116 28 0 0 0 0 0\n"
"RenderEvent 116 28 0 0 0 0 0\n"
"MouseMoveEvent 119 28 0 0 0 0 0\n"
"RenderEvent 119 28 0 0 0 0 0\n"
"MouseMoveEvent 120 28 0 0 0 0 0\n"
"RenderEvent 120 28 0 0 0 0 0\n"
"MouseMoveEvent 123 28 0 0 0 0 0\n"
"RenderEvent 123 28 0 0 0 0 0\n"
"MouseMoveEvent 124 28 0 0 0 0 0\n"
"RenderEvent 124 28 0 0 0 0 0\n"
"MouseMoveEvent 125 28 0 0 0 0 0\n"
"RenderEvent 125 28 0 0 0 0 0\n"
"MouseMoveEvent 127 28 0 0 0 0 0\n"
"RenderEvent 127 28 0 0 0 0 0\n"
"MouseMoveEvent 129 28 0 0 0 0 0\n"
"RenderEvent 129 28 0 0 0 0 0\n"
"MouseMoveEvent 130 27 0 0 0 0 0\n"
"RenderEvent 130 27 0 0 0 0 0\n"
"MouseMoveEvent 131 27 0 0 0 0 0\n"
"RenderEvent 131 27 0 0 0 0 0\n"
"MouseMoveEvent 133 27 0 0 0 0 0\n"
"RenderEvent 133 27 0 0 0 0 0\n"
"MouseMoveEvent 134 27 0 0 0 0 0\n"
"RenderEvent 134 27 0 0 0 0 0\n"
"MouseMoveEvent 138 26 0 0 0 0 0\n"
"RenderEvent 138 26 0 0 0 0 0\n"
"MouseMoveEvent 140 26 0 0 0 0 0\n"
"RenderEvent 140 26 0 0 0 0 0\n"
"MouseMoveEvent 141 26 0 0 0 0 0\n"
"RenderEvent 141 26 0 0 0 0 0\n"
"MouseMoveEvent 142 26 0 0 0 0 0\n"
"RenderEvent 142 26 0 0 0 0 0\n"
"MouseMoveEvent 143 26 0 0 0 0 0\n"
"RenderEvent 143 26 0 0 0 0 0\n"
"MouseMoveEvent 146 26 0 0 0 0 0\n"
"RenderEvent 146 26 0 0 0 0 0\n"
"MouseMoveEvent 147 26 0 0 0 0 0\n"
"RenderEvent 147 26 0 0 0 0 0\n"
"MouseMoveEvent 149 26 0 0 0 0 0\n"
"RenderEvent 149 26 0 0 0 0 0\n"
"MouseMoveEvent 151 26 0 0 0 0 0\n"
"RenderEvent 151 26 0 0 0 0 0\n"
"MouseMoveEvent 152 26 0 0 0 0 0\n"
"RenderEvent 152 26 0 0 0 0 0\n"
"MouseMoveEvent 152 27 0 0 0 0 0\n"
"RenderEvent 152 27 0 0 0 0 0\n"
"MouseMoveEvent 153 27 0 0 0 0 0\n"
"RenderEvent 153 27 0 0 0 0 0\n"
"MouseMoveEvent 154 27 0 0 0 0 0\n"
"RenderEvent 154 27 0 0 0 0 0\n"
"MouseMoveEvent 155 27 0 0 0 0 0\n"
"RenderEvent 155 27 0 0 0 0 0\n"
"MouseMoveEvent 158 28 0 0 0 0 0\n"
"RenderEvent 158 28 0 0 0 0 0\n"
"MouseMoveEvent 159 28 0 0 0 0 0\n"
"RenderEvent 159 28 0 0 0 0 0\n"
"MouseMoveEvent 161 29 0 0 0 0 0\n"
"RenderEvent 161 29 0 0 0 0 0\n"
"LeftButtonPressEvent 161 29 0 0 0 0 0\n"
"RenderEvent 161 29 0 0 0 0 0\n"
"LeftButtonReleaseEvent 161 29 0 0 0 0 0\n"
"MouseMoveEvent 161 30 0 0 0 0 0\n"
"MouseMoveEvent 161 32 0 0 0 0 0\n"
"MouseMoveEvent 162 33 0 0 0 0 0\n"
"MouseMoveEvent 163 33 0 0 0 0 0\n"
"MouseMoveEvent 164 35 0 0 0 0 0\n"
"MouseMoveEvent 164 36 0 0 0 0 0\n"
"MouseMoveEvent 165 38 0 0 0 0 0\n"
"MouseMoveEvent 166 39 0 0 0 0 0\n"
"MouseMoveEvent 167 40 0 0 0 0 0\n"
"MouseMoveEvent 168 42 0 0 0 0 0\n"
"MouseMoveEvent 169 44 0 0 0 0 0\n"
"MouseMoveEvent 169 45 0 0 0 0 0\n"
"MouseMoveEvent 170 47 0 0 0 0 0\n"
"MouseMoveEvent 174 51 0 0 0 0 0\n"
"MouseMoveEvent 175 52 0 0 0 0 0\n"
"MouseMoveEvent 176 53 0 0 0 0 0\n"
"MouseMoveEvent 176 54 0 0 0 0 0\n"
"MouseMoveEvent 177 55 0 0 0 0 0\n"
"MouseMoveEvent 177 56 0 0 0 0 0\n"
"MouseMoveEvent 178 57 0 0 0 0 0\n"
"MouseMoveEvent 179 57 0 0 0 0 0\n"
"MouseMoveEvent 179 58 0 0 0 0 0\n"
"MouseMoveEvent 180 59 0 0 0 0 0\n"
"MouseMoveEvent 180 60 0 0 0 0 0\n"
"MouseMoveEvent 180 61 0 0 0 0 0\n"
"MouseMoveEvent 181 61 0 0 0 0 0\n"
"MouseMoveEvent 181 62 0 0 0 0 0\n"
"MouseMoveEvent 182 63 0 0 0 0 0\n"
"MouseMoveEvent 182 64 0 0 0 0 0\n"
"MouseMoveEvent 182 65 0 0 0 0 0\n"
"MouseMoveEvent 183 65 0 0 0 0 0\n"
"MouseMoveEvent 183 66 0 0 0 0 0\n"
"MouseMoveEvent 184 66 0 0 0 0 0\n"
"KeyPressEvent 184 66 0 0 116 1 t\n"
"CharEvent 184 66 0 0 116 1 t\n"
"MouseMoveEvent 184 67 0 0 0 0 t\n"
"KeyReleaseEvent 184 67 0 0 116 1 t\n"
"MouseMoveEvent 184 68 0 0 0 0 t\n"
"MouseMoveEvent 184 69 0 0 0 0 t\n"
"MouseMoveEvent 184 70 0 0 0 0 t\n"
"MouseMoveEvent 184 72 0 0 0 0 t\n"
"MouseMoveEvent 184 74 0 0 0 0 t\n"
"MouseMoveEvent 184 77 0 0 0 0 t\n"
"MouseMoveEvent 184 80 0 0 0 0 t\n"
"MouseMoveEvent 184 81 0 0 0 0 t\n"
"MouseMoveEvent 183 83 0 0 0 0 t\n"
"MouseMoveEvent 183 85 0 0 0 0 t\n"
"MouseMoveEvent 182 86 0 0 0 0 t\n"
"MouseMoveEvent 182 88 0 0 0 0 t\n"
"MouseMoveEvent 182 90 0 0 0 0 t\n"
"MouseMoveEvent 182 92 0 0 0 0 t\n"
"MouseMoveEvent 182 94 0 0 0 0 t\n"
"MouseMoveEvent 182 96 0 0 0 0 t\n"
"MouseMoveEvent 181 98 0 0 0 0 t\n"
"MouseMoveEvent 181 99 0 0 0 0 t\n"
"MouseMoveEvent 180 100 0 0 0 0 t\n"
"MouseMoveEvent 180 101 0 0 0 0 t\n"
"MouseMoveEvent 180 102 0 0 0 0 t\n"
"MouseMoveEvent 180 104 0 0 0 0 t\n"
"MouseMoveEvent 180 105 0 0 0 0 t\n"
"MouseMoveEvent 180 107 0 0 0 0 t\n"
"MouseMoveEvent 180 109 0 0 0 0 t\n"
"MouseMoveEvent 179 109 0 0 0 0 t\n"
"MouseMoveEvent 179 110 0 0 0 0 t\n"
"MouseMoveEvent 178 111 0 0 0 0 t\n"
"MouseMoveEvent 178 112 0 0 0 0 t\n"
"MouseMoveEvent 178 113 0 0 0 0 t\n"
"MouseMoveEvent 178 114 0 0 0 0 t\n"
"MouseMoveEvent 178 115 0 0 0 0 t\n"
"MouseMoveEvent 178 116 0 0 0 0 t\n"
"MouseMoveEvent 178 117 0 0 0 0 t\n"
"MouseMoveEvent 178 118 0 0 0 0 t\n"
"MouseMoveEvent 178 119 0 0 0 0 t\n"
"MouseMoveEvent 178 120 0 0 0 0 t\n"
"MouseMoveEvent 178 121 0 0 0 0 t\n"
"MouseMoveEvent 178 122 0 0 0 0 t\n"
"MouseMoveEvent 177 123 0 0 0 0 t\n"
"MouseMoveEvent 177 124 0 0 0 0 t\n"
"MouseMoveEvent 177 125 0 0 0 0 t\n"
"MouseMoveEvent 177 126 0 0 0 0 t\n"
"MouseMoveEvent 177 127 0 0 0 0 t\n"
"MouseMoveEvent 177 128 0 0 0 0 t\n"
"MouseMoveEvent 177 129 0 0 0 0 t\n"
"LeftButtonPressEvent 177 129 0 0 0 0 t\n"
"StartInteractionEvent 177 129 0 0 0 0 t\n"
"MouseMoveEvent 176 129 0 0 0 0 t\n"
"RenderEvent 176 129 0 0 0 0 t\n"
"MouseMoveEvent 175 130 0 0 0 0 t\n"
"RenderEvent 175 130 0 0 0 0 t\n"
"MouseMoveEvent 174 131 0 0 0 0 t\n"
"RenderEvent 174 131 0 0 0 0 t\n"
"MouseMoveEvent 173 131 0 0 0 0 t\n"
"RenderEvent 173 131 0 0 0 0 t\n"
"MouseMoveEvent 172 132 0 0 0 0 t\n"
"RenderEvent 172 132 0 0 0 0 t\n"
"MouseMoveEvent 171 132 0 0 0 0 t\n"
"RenderEvent 171 132 0 0 0 0 t\n"
"MouseMoveEvent 170 133 0 0 0 0 t\n"
"RenderEvent 170 133 0 0 0 0 t\n"
"MouseMoveEvent 169 134 0 0 0 0 t\n"
"RenderEvent 169 134 0 0 0 0 t\n"
"MouseMoveEvent 168 135 0 0 0 0 t\n"
"RenderEvent 168 135 0 0 0 0 t\n"
"MouseMoveEvent 167 135 0 0 0 0 t\n"
"RenderEvent 167 135 0 0 0 0 t\n"
"MouseMoveEvent 167 136 0 0 0 0 t\n"
"RenderEvent 167 136 0 0 0 0 t\n"
"MouseMoveEvent 165 136 0 0 0 0 t\n"
"RenderEvent 165 136 0 0 0 0 t\n"
"MouseMoveEvent 165 137 0 0 0 0 t\n"
"RenderEvent 165 137 0 0 0 0 t\n"
"MouseMoveEvent 164 138 0 0 0 0 t\n"
"RenderEvent 164 138 0 0 0 0 t\n"
"MouseMoveEvent 163 139 0 0 0 0 t\n"
"RenderEvent 163 139 0 0 0 0 t\n"
"MouseMoveEvent 162 139 0 0 0 0 t\n"
"RenderEvent 162 139 0 0 0 0 t\n"
"MouseMoveEvent 162 140 0 0 0 0 t\n"
"RenderEvent 162 140 0 0 0 0 t\n"
"MouseMoveEvent 161 140 0 0 0 0 t\n"
"RenderEvent 161 140 0 0 0 0 t\n"
"MouseMoveEvent 160 141 0 0 0 0 t\n"
"RenderEvent 160 141 0 0 0 0 t\n"
"MouseMoveEvent 159 141 0 0 0 0 t\n"
"RenderEvent 159 141 0 0 0 0 t\n"
"MouseMoveEvent 159 142 0 0 0 0 t\n"
"RenderEvent 159 142 0 0 0 0 t\n"
"MouseMoveEvent 158 142 0 0 0 0 t\n"
"RenderEvent 158 142 0 0 0 0 t\n"
"MouseMoveEvent 158 143 0 0 0 0 t\n"
"RenderEvent 158 143 0 0 0 0 t\n"
"MouseMoveEvent 157 144 0 0 0 0 t\n"
"RenderEvent 157 144 0 0 0 0 t\n"
"MouseMoveEvent 157 145 0 0 0 0 t\n"
"RenderEvent 157 145 0 0 0 0 t\n"
"MouseMoveEvent 156 145 0 0 0 0 t\n"
"RenderEvent 156 145 0 0 0 0 t\n"
"MouseMoveEvent 156 146 0 0 0 0 t\n"
"RenderEvent 156 146 0 0 0 0 t\n"
"MouseMoveEvent 155 146 0 0 0 0 t\n"
"RenderEvent 155 146 0 0 0 0 t\n"
"MouseMoveEvent 155 147 0 0 0 0 t\n"
"RenderEvent 155 147 0 0 0 0 t\n"
"MouseMoveEvent 154 147 0 0 0 0 t\n"
"RenderEvent 154 147 0 0 0 0 t\n"
"MouseMoveEvent 154 148 0 0 0 0 t\n"
"RenderEvent 154 148 0 0 0 0 t\n"
"MouseMoveEvent 153 149 0 0 0 0 t\n"
"RenderEvent 153 149 0 0 0 0 t\n"
"MouseMoveEvent 153 150 0 0 0 0 t\n"
"RenderEvent 153 150 0 0 0 0 t\n"
"MouseMoveEvent 152 150 0 0 0 0 t\n"
"RenderEvent 152 150 0 0 0 0 t\n"
"MouseMoveEvent 152 151 0 0 0 0 t\n"
"RenderEvent 152 151 0 0 0 0 t\n"
"MouseMoveEvent 151 152 0 0 0 0 t\n"
"RenderEvent 151 152 0 0 0 0 t\n"
"MouseMoveEvent 150 153 0 0 0 0 t\n"
"RenderEvent 150 153 0 0 0 0 t\n"
"MouseMoveEvent 149 153 0 0 0 0 t\n"
"RenderEvent 149 153 0 0 0 0 t\n"
"MouseMoveEvent 149 154 0 0 0 0 t\n"
"RenderEvent 149 154 0 0 0 0 t\n"
"MouseMoveEvent 148 156 0 0 0 0 t\n"
"RenderEvent 148 156 0 0 0 0 t\n"
"MouseMoveEvent 147 157 0 0 0 0 t\n"
"RenderEvent 147 157 0 0 0 0 t\n"
"MouseMoveEvent 146 158 0 0 0 0 t\n"
"RenderEvent 146 158 0 0 0 0 t\n"
"MouseMoveEvent 146 159 0 0 0 0 t\n"
"RenderEvent 146 159 0 0 0 0 t\n"
"MouseMoveEvent 145 160 0 0 0 0 t\n"
"RenderEvent 145 160 0 0 0 0 t\n"
"MouseMoveEvent 145 161 0 0 0 0 t\n"
"RenderEvent 145 161 0 0 0 0 t\n"
"MouseMoveEvent 145 162 0 0 0 0 t\n"
"RenderEvent 145 162 0 0 0 0 t\n"
"MouseMoveEvent 144 162 0 0 0 0 t\n"
"RenderEvent 144 162 0 0 0 0 t\n"
"MouseMoveEvent 144 163 0 0 0 0 t\n"
"RenderEvent 144 163 0 0 0 0 t\n"
"MouseMoveEvent 144 164 0 0 0 0 t\n"
"RenderEvent 144 164 0 0 0 0 t\n"
"MouseMoveEvent 143 165 0 0 0 0 t\n"
"RenderEvent 143 165 0 0 0 0 t\n"
"MouseMoveEvent 142 165 0 0 0 0 t\n"
"RenderEvent 142 165 0 0 0 0 t\n"
"MouseMoveEvent 141 165 0 0 0 0 t\n"
"RenderEvent 141 165 0 0 0 0 t\n"
"MouseMoveEvent 140 165 0 0 0 0 t\n"
"RenderEvent 140 165 0 0 0 0 t\n"
"MouseMoveEvent 140 166 0 0 0 0 t\n"
"RenderEvent 140 166 0 0 0 0 t\n"
"MouseMoveEvent 139 167 0 0 0 0 t\n"
"RenderEvent 139 167 0 0 0 0 t\n"
"MouseMoveEvent 139 168 0 0 0 0 t\n"
"RenderEvent 139 168 0 0 0 0 t\n"
"MouseMoveEvent 137 168 0 0 0 0 t\n"
"RenderEvent 137 168 0 0 0 0 t\n"
"MouseMoveEvent 136 168 0 0 0 0 t\n"
"RenderEvent 136 168 0 0 0 0 t\n"
"MouseMoveEvent 135 168 0 0 0 0 t\n"
"RenderEvent 135 168 0 0 0 0 t\n"
"MouseMoveEvent 133 168 0 0 0 0 t\n"
"RenderEvent 133 168 0 0 0 0 t\n"
"MouseMoveEvent 131 169 0 0 0 0 t\n"
"RenderEvent 131 169 0 0 0 0 t\n"
"MouseMoveEvent 130 169 0 0 0 0 t\n"
"RenderEvent 130 169 0 0 0 0 t\n"
"MouseMoveEvent 129 169 0 0 0 0 t\n"
"RenderEvent 129 169 0 0 0 0 t\n"
"MouseMoveEvent 128 169 0 0 0 0 t\n"
"RenderEvent 128 169 0 0 0 0 t\n"
"MouseMoveEvent 128 170 0 0 0 0 t\n"
"RenderEvent 128 170 0 0 0 0 t\n"
"MouseMoveEvent 127 170 0 0 0 0 t\n"
"RenderEvent 127 170 0 0 0 0 t\n"
"MouseMoveEvent 126 170 0 0 0 0 t\n"
"RenderEvent 126 170 0 0 0 0 t\n"
"MouseMoveEvent 125 170 0 0 0 0 t\n"
"RenderEvent 125 170 0 0 0 0 t\n"
"MouseMoveEvent 125 171 0 0 0 0 t\n"
"RenderEvent 125 171 0 0 0 0 t\n"
"MouseMoveEvent 124 171 0 0 0 0 t\n"
"RenderEvent 124 171 0 0 0 0 t\n"
"MouseMoveEvent 123 171 0 0 0 0 t\n"
"RenderEvent 123 171 0 0 0 0 t\n"
"MouseMoveEvent 123 172 0 0 0 0 t\n"
"RenderEvent 123 172 0 0 0 0 t\n"
"MouseMoveEvent 122 172 0 0 0 0 t\n"
"RenderEvent 122 172 0 0 0 0 t\n"
"MouseMoveEvent 121 172 0 0 0 0 t\n"
"RenderEvent 121 172 0 0 0 0 t\n"
"MouseMoveEvent 120 172 0 0 0 0 t\n"
"RenderEvent 120 172 0 0 0 0 t\n"
"MouseMoveEvent 119 172 0 0 0 0 t\n"
"RenderEvent 119 172 0 0 0 0 t\n"
"MouseMoveEvent 118 172 0 0 0 0 t\n"
"RenderEvent 118 172 0 0 0 0 t\n"
"MouseMoveEvent 117 172 0 0 0 0 t\n"
"RenderEvent 117 172 0 0 0 0 t\n"
"MouseMoveEvent 116 173 0 0 0 0 t\n"
"RenderEvent 116 173 0 0 0 0 t\n"
"MouseMoveEvent 115 173 0 0 0 0 t\n"
"RenderEvent 115 173 0 0 0 0 t\n"
"MouseMoveEvent 114 173 0 0 0 0 t\n"
"RenderEvent 114 173 0 0 0 0 t\n"
"MouseMoveEvent 114 174 0 0 0 0 t\n"
"RenderEvent 114 174 0 0 0 0 t\n"
"MouseMoveEvent 112 174 0 0 0 0 t\n"
"RenderEvent 112 174 0 0 0 0 t\n"
"MouseMoveEvent 111 174 0 0 0 0 t\n"
"RenderEvent 111 174 0 0 0 0 t\n"
"MouseMoveEvent 110 174 0 0 0 0 t\n"
"RenderEvent 110 174 0 0 0 0 t\n"
"MouseMoveEvent 109 174 0 0 0 0 t\n"
"RenderEvent 109 174 0 0 0 0 t\n"
"MouseMoveEvent 108 174 0 0 0 0 t\n"
"RenderEvent 108 174 0 0 0 0 t\n"
"MouseMoveEvent 107 174 0 0 0 0 t\n"
"RenderEvent 107 174 0 0 0 0 t\n"
"LeftButtonReleaseEvent 107 174 0 0 0 0 t\n"
"EndInteractionEvent 107 174 0 0 0 0 t\n"
"RenderEvent 107 174 0 0 0 0 t\n"
"MouseMoveEvent 108 174 0 0 0 0 t\n"
"MouseMoveEvent 109 174 0 0 0 0 t\n"
"MouseMoveEvent 111 174 0 0 0 0 t\n"
"MouseMoveEvent 113 174 0 0 0 0 t\n"
"MouseMoveEvent 121 174 0 0 0 0 t\n"
"MouseMoveEvent 131 174 0 0 0 0 t\n"
"MouseMoveEvent 139 174 0 0 0 0 t\n"
"MouseMoveEvent 151 174 0 0 0 0 t\n"
"MouseMoveEvent 161 174 0 0 0 0 t\n"
"MouseMoveEvent 171 174 0 0 0 0 t\n"
"MouseMoveEvent 179 174 0 0 0 0 t\n"
"MouseMoveEvent 182 174 0 0 0 0 t\n"
"MouseMoveEvent 185 174 0 0 0 0 t\n"
"MouseMoveEvent 187 174 0 0 0 0 t\n"
"MouseMoveEvent 188 174 0 0 0 0 t\n"
"MouseMoveEvent 189 174 0 0 0 0 t\n"
"MouseMoveEvent 190 173 0 0 0 0 t\n"
"MouseMoveEvent 191 173 0 0 0 0 t\n"
"MouseMoveEvent 192 173 0 0 0 0 t\n"
"KeyPressEvent 192 173 0 0 113 1 q\n"
"CharEvent 192 173 0 0 113 1 q\n"
"ExitEvent 192 173 0 0 113 1 q\n"
;


// This callback is responsible for setting the angle label.
class vtkAngleCallback : public vtkCommand
{
public:
  static vtkAngleCallback *New() 
    { return new vtkAngleCallback; }
  virtual void Execute(vtkObject*, unsigned long eid, void*)
    {
      if ( eid == vtkCommand::PlacePointEvent )
        {
        std::cout << "point placed\n";
        }
      else if ( eid == vtkCommand::InteractionEvent )
        {
        double point1[3], center[3], point2[3];
        this->Rep->GetPoint1WorldPosition(point1);
        this->Rep->GetCenterWorldPosition(center);
        this->Rep->GetPoint2WorldPosition(point2);
        std::cout << "Angle between " << "("
             << point1[0] << ","
             << point1[1] << ","
             << point1[2] << "), ("
             << center[0] << ","
             << center[1] << ","
             << center[2] << ") and ("
             << point2[0] << ","
             << point2[1] << ","
             << point2[2] << ") is "
             << this->Rep->GetAngle() << " radians." << std::endl;
        }
    }
  vtkAngleRepresentation3D *Rep;
  vtkAngleCallback():Rep(0) {}
};


// The actual test function
int TestAngleWidget3D( int argc, char *argv[] )
{
  vtkSmartPointer<vtkSphereSource> ss 
    = vtkSmartPointer<vtkSphereSource>::New();

  // Create the RenderWindow, Renderer and both Actors
  //
  vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer< vtkRenderer >::New();
  vtkSmartPointer< vtkRenderWindow > renWin = vtkSmartPointer< vtkRenderWindow >::New();
  renWin->AddRenderer(ren1);

  vtkSmartPointer< vtkRenderWindowInteractor > iren 
    = vtkSmartPointer< vtkRenderWindowInteractor >::New();
  iren->SetRenderWindow(renWin);

  // Create a test pipeline
  //
  vtkSmartPointer< vtkPolyDataMapper > mapper 
    = vtkSmartPointer< vtkPolyDataMapper >::New();
  mapper->SetInput(ss->GetOutput());
  vtkSmartPointer< vtkActor > actor = vtkSmartPointer< vtkActor >::New();
  actor->SetMapper(mapper);

  // Create the widget and its representation
  vtkSmartPointer< vtkPointHandleRepresentation3D > handle 
    = vtkSmartPointer< vtkPointHandleRepresentation3D >::New();
  handle->GetProperty()->SetColor(1,0,0);
  vtkSmartPointer< vtkAngleRepresentation3D > rep 
    = vtkSmartPointer< vtkAngleRepresentation3D >::New();
  rep->SetHandleRepresentation(handle);

  vtkSmartPointer< vtkAngleWidget > widget 
    = vtkSmartPointer< vtkAngleWidget >::New();
  widget->SetInteractor(iren);
  widget->CreateDefaultRepresentation();
  widget->SetRepresentation(rep);

  vtkSmartPointer< vtkAngleCallback > mcbk 
    = vtkSmartPointer< vtkAngleCallback >::New();
  mcbk->Rep = rep;
  widget->AddObserver(vtkCommand::PlacePointEvent,mcbk);
  widget->AddObserver(vtkCommand::InteractionEvent,mcbk);

  // Add the actors to the renderer, set the background and size
  //
  ren1->AddActor(actor);
  ren1->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(300, 300);

  // render the image
  //
  iren->Initialize();
  renWin->Render();
  widget->On();
  renWin->Render();

  return vtkTesting::InteractorEventLoop( 
      argc, argv, iren, TestAngleWidget3DEventLog );
}

