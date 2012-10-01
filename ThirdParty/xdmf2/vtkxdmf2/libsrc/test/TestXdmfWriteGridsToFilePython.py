### Author: Kenneth Leiter
### E-mail: kenneth.leiter@us.army.mil
###
### A python test that writes out a test python file... TestVTKXdmfReader attempts to read in the file generated
### here.

import sys

import Xdmf
from Xdmf import *

if __name__ == '__main__':   
    
    heavyDataSetName = 'testFile.h5'
    outputName = 'testFile.xmf'
    
    root = XdmfRoot()
    dom = XdmfDOM()
    domain = XdmfDomain()
    
    root.SetDOM(dom);
    root.Build();
    root.Insert(domain);
    
    grid = XdmfGrid()
    grid.SetName("INT64 Polyvertex")
    assert(grid.GetName() == "INT64 Polyvertex")
    geom = grid.GetGeometry()
    geom.SetLightDataLimit(0)
    assert(geom.GetLightDataLimit() == 0)
    geom.SetGeometryType(XDMF_GEOMETRY_XYZ)
    assert(geom.GetGeometryType() == Xdmf.XDMF_GEOMETRY_XYZ)
    geom.SetNumberOfPoints(3)
    assert(geom.GetNumberOfPoints() == 3)
    points = geom.GetPoints()
    points.SetHeavyDataSetName(heavyDataSetName + ":/" + grid.GetName() + "/XYZ")
    assert(points.GetHeavyDataSetName() == (heavyDataSetName + ":/" + grid.GetName() + "/XYZ"))
    points.SetNumberType(XDMF_INT64_TYPE)
    assert(points.GetNumberType() == Xdmf.XDMF_INT64_TYPE)
    points.SetNumberOfElements(9)
    assert(points.GetNumberOfElements() == 9)
    points.SetValueFromInt64(0, 9223372036854775807)
    points.SetValueFromInt64(1, 9223372036854775807)
    points.SetValueFromInt64(2, 9223372036854775807)
    points.SetValueFromInt64(3, -9223372036854775808)
    points.SetValueFromInt64(4, -9223372036854775808)
    points.SetValueFromInt64(5, -9223372036854775808)
    points.SetValueFromInt64(6, 0)
    points.SetValueFromInt64(7, 0)
    points.SetValueFromInt64(8, 0)
    assert(points.GetValueAsInt64(0) == 9223372036854775807)
    assert(points.GetValueAsInt64(3) == -9223372036854775808)
    assert(points.GetValueAsInt64(6) == 0)
    top = grid.GetTopology()
    top.SetTopologyType(XDMF_POLYVERTEX)
    assert(top.GetTopologyType() == Xdmf.XDMF_POLYVERTEX)
    top.SetNumberOfElements(3)
    assert(top.GetNumberOfElements() == 3)
    domain.Insert(grid)
    grid.Build()
    
    attr1 = XdmfAttribute()
    attr1.SetName("Scalar UINT8")
    attr1.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_NODE)
    attr1.SetAttributeType(XDMF_ATTRIBUTE_TYPE_SCALAR)
    attr1.SetLightDataLimit(0)
    attr1.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid.GetName() + "/" + attr1.GetName())
    attr1.GetValues().SetNumberType(XDMF_UINT8_TYPE)
    attr1.GetValues().SetNumberOfElements(3)
    attr1.GetValues().SetValueFromInt64(0, 255)
    attr1.GetValues().SetValueFromInt64(1, 0)
    attr1.GetValues().SetValueFromInt64(2, 50)
    
    attr2 = XdmfAttribute()
    attr2.SetName("Vector UINT16")
    attr2.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_NODE)
    attr2.SetAttributeType(XDMF_ATTRIBUTE_TYPE_VECTOR)
    attr2.SetLightDataLimit(0)
    attr2.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid.GetName() + "/" + attr2.GetName())
    attr2.GetValues().SetNumberType(XDMF_UINT16_TYPE)
    attr2.GetValues().SetNumberOfElements(9)
    attr2.GetValues().SetValueFromInt64(0, 65535)
    attr2.GetValues().SetValueFromInt64(1, 65535)
    attr2.GetValues().SetValueFromInt64(2, 65535)
    attr2.GetValues().SetValueFromInt64(3, 0)
    attr2.GetValues().SetValueFromInt64(4, 0)
    attr2.GetValues().SetValueFromInt64(5, 0)
    attr2.GetValues().SetValueFromInt64(6, 100)
    attr2.GetValues().SetValueFromInt64(7, 100)
    attr2.GetValues().SetValueFromInt64(8, 100)
    
    attr3 = XdmfAttribute()
    attr3.SetName("Tensor UINT32")
    attr3.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_NODE)
    attr3.SetAttributeType(XDMF_ATTRIBUTE_TYPE_TENSOR)
    attr3.SetLightDataLimit(0)
    attr3.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid.GetName() + "/" + attr3.GetName())
    attr3.GetValues().SetNumberType(XDMF_UINT32_TYPE)
    attr3.GetValues().SetNumberOfElements(27)
    attr3.GetValues().SetValueFromInt64(0, 4294967295)
    attr3.GetValues().SetValueFromInt64(1, 4294967295)
    attr3.GetValues().SetValueFromInt64(2, 0)
    attr3.GetValues().SetValueFromInt64(3, 50)
    attr3.GetValues().SetValueFromInt64(4, 100)
    attr3.GetValues().SetValueFromInt64(5, 200)
    attr3.GetValues().SetValueFromInt64(6, 500)
    attr3.GetValues().SetValueFromInt64(7, 1000)
    attr3.GetValues().SetValueFromInt64(8, 1000)
    attr3.GetValues().SetValueFromInt64(9, 1000)
    attr3.GetValues().SetValueFromInt64(10, 1000)
    attr3.GetValues().SetValueFromInt64(11, 500)
    attr3.GetValues().SetValueFromInt64(12, 200)
    attr3.GetValues().SetValueFromInt64(13, 100)
    attr3.GetValues().SetValueFromInt64(14, 50)
    attr3.GetValues().SetValueFromInt64(15, 0)
    attr3.GetValues().SetValueFromInt64(16, 4294967295)
    attr3.GetValues().SetValueFromInt64(17, 4294967295)
    attr3.GetValues().SetValueFromInt64(18, 4294967295)
    attr3.GetValues().SetValueFromInt64(19, 4294967295)
    attr3.GetValues().SetValueFromInt64(20, 0)
    attr3.GetValues().SetValueFromInt64(21, 50)
    attr3.GetValues().SetValueFromInt64(22, 100)
    attr3.GetValues().SetValueFromInt64(23, 200)
    attr3.GetValues().SetValueFromInt64(24, 500)
    attr3.GetValues().SetValueFromInt64(25, 1000)
    attr3.GetValues().SetValueFromInt64(26, 1000)
    
    attr4 = XdmfAttribute()
    attr4.SetName("Scalar INT8")
    attr4.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_GRID)
    attr4.SetAttributeType(XDMF_ATTRIBUTE_TYPE_SCALAR)
    attr4.SetLightDataLimit(0)
    attr4.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid.GetName() + "/" + attr4.GetName())
    attr4.GetValues().SetNumberType(XDMF_INT8_TYPE)
    attr4.GetValues().SetNumberOfElements(1)
    attr4.GetValues().SetValueFromInt64(0, 127)
    
    grid.Insert(attr1)
    grid.Insert(attr2)
    grid.Insert(attr3)
    grid.Insert(attr4)
    grid.Build()
    
    assert(grid.GetAttribute(0).GetName() == attr1.GetName())
    assert(grid.GetAttribute(0).GetValues().GetValueAsInt64(2) == 50)
    
    grid2 = XdmfGrid()
    grid2.SetName("FLOAT64 Quadrilateral")
    geom = grid2.GetGeometry()
    geom.SetGeometryType(XDMF_GEOMETRY_XY)
    geom.SetNumberOfPoints(4)
    geom.SetLightDataLimit(0)
    points = geom.GetPoints()
    points.SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/XYZ")
    points.SetNumberType(XDMF_FLOAT64_TYPE)
    points.SetNumberOfElements(8)
    points.SetValueFromFloat64(0, -1000.5)
    points.SetValueFromFloat64(1, 0)
    points.SetValueFromFloat64(2, 0)
    points.SetValueFromFloat64(3, -500.25)
    points.SetValueFromFloat64(4, 1000.5)
    points.SetValueFromFloat64(5, 0)
    points.SetValueFromFloat64(6, 0)
    points.SetValueFromFloat64(7, 2196.99)
    top = grid2.GetTopology()
    top.SetTopologyType(XDMF_QUAD)
    top.SetNumberOfElements(1)
    top.SetLightDataLimit(0)
    top.GetConnectivity().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/Connections")
    top.GetConnectivity().SetNumberType(XDMF_INT32_TYPE)
    top.GetConnectivity().SetNumberOfElements(4)
    top.GetConnectivity().SetValueFromInt64(0, 0)
    top.GetConnectivity().SetValueFromInt64(1, 1)
    top.GetConnectivity().SetValueFromInt64(2, 2)
    top.GetConnectivity().SetValueFromInt64(3, 3)
    domain.Insert(grid2)
    grid2.Build()

    attr5 = XdmfAttribute()
    attr5.SetName("Vector INT16")
    attr5.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_GRID)
    attr5.SetAttributeType(XDMF_ATTRIBUTE_TYPE_VECTOR)
    attr5.SetLightDataLimit(0)
    attr5.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/" + attr5.GetName())
    attr5.GetValues().SetNumberType(XDMF_INT16_TYPE)
    attr5.GetValues().SetNumberOfElements(3)
    attr5.GetValues().SetValueFromInt64(0, 32767)
    attr5.GetValues().SetValueFromInt64(1, -32768)
    attr5.GetValues().SetValueFromInt64(2, 0)
    
    attr6 = XdmfAttribute()
    attr6.SetName("Tensor INT32")
    attr6.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_GRID)
    attr6.SetAttributeType(XDMF_ATTRIBUTE_TYPE_TENSOR)
    attr6.SetLightDataLimit(0)
    attr6.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/" + attr6.GetName())
    attr6.GetValues().SetNumberType(XDMF_INT32_TYPE)
    attr6.GetValues().SetNumberOfElements(9)
    attr6.GetValues().SetValueFromInt64(0, 2147483647)
    attr6.GetValues().SetValueFromInt64(1, 2147483647)
    attr6.GetValues().SetValueFromInt64(2, 2147483647)
    attr6.GetValues().SetValueFromInt64(3, -2147483648)
    attr6.GetValues().SetValueFromInt64(4, -2147483648)
    attr6.GetValues().SetValueFromInt64(5, -2147483648)
    attr6.GetValues().SetValueFromInt64(6, 0)
    attr6.GetValues().SetValueFromInt64(7, 0)
    attr6.GetValues().SetValueFromInt64(8, 0)
    
    attr7 = XdmfAttribute()
    attr7.SetName("Scalar INT64")
    attr7.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_CELL)
    attr7.SetAttributeType(XDMF_ATTRIBUTE_TYPE_SCALAR)
    attr7.SetLightDataLimit(0)
    attr7.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/" + attr7.GetName())
    attr7.GetValues().SetNumberType(XDMF_INT64_TYPE)
    attr7.GetValues().SetNumberOfElements(1)
    attr7.GetValues().SetValueFromInt64(0, 9223372036854775807)
    
    attr8 = XdmfAttribute()
    attr8.SetName("Vector FLOAT32")
    attr8.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_CELL)
    attr8.SetAttributeType(XDMF_ATTRIBUTE_TYPE_VECTOR)
    attr8.SetLightDataLimit(0)
    attr8.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/" + attr8.GetName())
    attr8.GetValues().SetNumberType(XDMF_FLOAT32_TYPE)
    attr8.GetValues().SetNumberOfElements(3)
    attr8.GetValues().SetValueFromFloat64(0, 0)
    attr8.GetValues().SetValueFromFloat64(1, -100.525)
    attr8.GetValues().SetValueFromFloat64(2, 1000.69)
  
    attr9 = XdmfAttribute()
    attr9.SetName("Tensor FLOAT64")
    attr9.SetAttributeCenter(XDMF_ATTRIBUTE_CENTER_CELL)
    attr9.SetAttributeType(XDMF_ATTRIBUTE_TYPE_TENSOR)
    attr9.SetLightDataLimit(0)
    attr9.GetValues().SetHeavyDataSetName(heavyDataSetName + ":/" + grid2.GetName() + "/" + attr9.GetName())
    attr9.GetValues().SetNumberType(XDMF_FLOAT64_TYPE)
    attr9.GetValues().SetNumberOfElements(9)
    attr9.GetValues().SetValueFromFloat64(0, 0)
    attr9.GetValues().SetValueFromFloat64(1, -1000)
    attr9.GetValues().SetValueFromFloat64(2, 1000)
    attr9.GetValues().SetValueFromFloat64(3, .005)
    attr9.GetValues().SetValueFromFloat64(4, -.005)
    attr9.GetValues().SetValueFromFloat64(5, .5)
    attr9.GetValues().SetValueFromFloat64(6, 100.99)
    attr9.GetValues().SetValueFromFloat64(7, 1000.9)
    attr9.GetValues().SetValueFromFloat64(8, -1000.9)

    grid2.Insert(attr5)
    grid2.Insert(attr6)
    grid2.Insert(attr7)
    grid2.Insert(attr8)
    grid2.Insert(attr9)
    grid2.Build()

    dom.Write(outputName)