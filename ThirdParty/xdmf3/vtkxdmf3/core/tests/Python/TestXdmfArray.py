from XdmfCore import *

if __name__ == "__main__":

    values1 = [0, 1, 2, 3]
    array1 = XdmfArray.New()
    array1.insertAsInt8(0, values1)
    print array1.getValuesString()
    assert array1.getArrayType() == XdmfArrayType.Int8()

    array2 = XdmfArray.New()
    array2.insertAsInt16(0, values1)
    print array2.getValuesString()
    assert array2.getArrayType() == XdmfArrayType.Int16()

    array3 = XdmfArray.New()
    array3.insertAsInt32(0, values1[2:4])
    print array3.getValuesString()
    assert array3.getArrayType() == XdmfArrayType.Int32()

    # Stride
    array4 = XdmfArray.New()
    array4.insertAsInt64(0, values1[0:4:2])
    print array4.getValuesString()
    assert array4.getArrayType() == XdmfArrayType.Int64()

    # Reverse
    array5 = XdmfArray.New()
    array5.insertAsFloat32(0, values1[::-1])
    print array5.getValuesString()
    assert array5.getArrayType() == XdmfArrayType.Float32()

    array6 = XdmfArray.New()
    array6.insertAsFloat64(0, values1)
    print array6.getValuesString()
    assert array6.getArrayType() == XdmfArrayType.Float64()

    array7 = XdmfArray.New()
    array7.insertAsUInt8(0, values1)
    print array7.getValuesString()
    assert array7.getArrayType() == XdmfArrayType.UInt8()

    array8 = XdmfArray.New()
    array8.insertAsUInt16(0, values1)
    print array8.getValuesString()
    assert array8.getArrayType() == XdmfArrayType.UInt16()

    array9 = XdmfArray.New()
    array9.insertAsUInt32(0, values1)
    print array9.getValuesString()
    assert array9.getArrayType() == XdmfArrayType.UInt32()

    stringArray = XdmfArray.New()
    stringArray.insertValueAsString(0, "foo")
    strings = ["bar", "car", "cellar"]
    assert stringArray.getArrayType() == XdmfArrayType.String()
    stringArray.insertAsString(0, strings)
    assert stringArray.getSize() == 3
    assert stringArray.getValueAsString(0) == "bar"
    stringArray.pushBackAsString("dog")
    assert stringArray.getSize() == 4
    assert stringArray.getValueAsString(3) == "dog"
    stringArray.resizeAsString(5, "")
    assert stringArray.getSize() == 5
    assert stringArray.getValueAsString(4) == ""
    print stringArray.getValuesString()
