

#ifdef WIN32
#pragma warning (push, 1) 
#pragma warning (disable:4503)
#pragma warning (disable:4514) 
#pragma warning (disable:4702)
#pragma warning (disable:4710) 
#pragma warning (disable:4786)
#endif 


#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>


#ifdef WIN32 
#pragma warning(pop) 
#endif 


#include "DICOMAppHelper.h"
#include "DICOMCallback.h"

#include <stdlib.h>
#include <math.h>

//#define DEBUG_DICOM_APP_HELPER

struct lt_pair_int_string
{
  bool operator()(const std::pair<int, std::string> s1, 
                  const std::pair<int, std::string> s2) const
  {
    return s1.first < s2.first;
  }
};

DICOMAppHelper::DICOMAppHelper() : SeriesUIDMap(), SliceNumberMap(), TagMap()
{
  this->FileName = NULL;
  this->DICOMDataFile = NULL;
  this->BitsAllocated = 8;
  this->ByteSwapData = false;
  this->PixelSpacing[0] = this->PixelSpacing[1] = 1.0;
  this->Dimensions[0] = this->Dimensions[1] = 0;
  this->PhotometricInterpretation = NULL;
  this->TransferSyntaxUID = NULL;
  this->RescaleOffset = 0.0;
  this->RescaleSlope = 1.0;
  this->ImageData = NULL;
  this->ImageDataLengthInBytes = 0;

  this->SeriesUIDCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->SliceNumberCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->TransferSyntaxCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->BitsAllocatedCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->PixelSpacingCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->HeightCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->WidthCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->PixelRepresentationCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->PhotometricInterpretationCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->RescaleOffsetCB = new DICOMMemberCallback<DICOMAppHelper>;
  this->RescaleSlopeCB = new DICOMMemberCallback<DICOMAppHelper>;

  this->PixelDataCB = new DICOMMemberCallback<DICOMAppHelper>;

}

DICOMAppHelper::~DICOMAppHelper()
{
  this->ClearSeriesUIDMap();
  this->ClearSliceNumberMap();

  this->HeaderFile.close();
  if (this->FileName)
    {
    delete [] this->FileName;
    }

  //
  // Fix warning here.
  //
  if (this->ImageData)
    {
    delete [] (static_cast<char*> (this->ImageData));
    }
  if (this->TransferSyntaxUID)
    {
    delete this->TransferSyntaxUID;
    }
  if (this->PhotometricInterpretation)
    {
    delete this->PhotometricInterpretation;
    }

  delete this->SeriesUIDCB;
  delete this->SliceNumberCB;
  delete this->TransferSyntaxCB;
  delete this->BitsAllocatedCB;
  delete this->PixelSpacingCB;
  delete this->HeightCB;
  delete this->WidthCB;
  delete this->PixelRepresentationCB;
  delete this->PhotometricInterpretationCB;
  delete this->RescaleOffsetCB;
  delete this->RescaleSlopeCB;
  delete this->PixelDataCB;
}

void DICOMAppHelper::RegisterCallbacks(DICOMParser* parser)
{
  if (!parser)
    {
    std::cerr << "Null parser!" << std::endl;
    }

  this->Parser = parser;

  SeriesUIDCB->SetCallbackFunction(this, &DICOMAppHelper::SeriesUIDCallback);
  parser->AddDICOMTagCallback(0x0020, 0x000e, DICOMParser::VR_UI, SeriesUIDCB);

  SliceNumberCB->SetCallbackFunction(this, &DICOMAppHelper::SliceNumberCallback);
  parser->AddDICOMTagCallback(0x0020, 0x0013, DICOMParser::VR_IS, SliceNumberCB);

  TransferSyntaxCB->SetCallbackFunction(this, &DICOMAppHelper::TransferSyntaxCallback);
  parser->AddDICOMTagCallback(0x0002, 0x0010, DICOMParser::VR_UI, TransferSyntaxCB);

  BitsAllocatedCB->SetCallbackFunction(this, &DICOMAppHelper::BitsAllocatedCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0100, DICOMParser::VR_US, BitsAllocatedCB);

  PixelSpacingCB->SetCallbackFunction(this, &DICOMAppHelper::PixelSpacingCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0030, DICOMParser::VR_FL, PixelSpacingCB);
  parser->AddDICOMTagCallback(0x0018, 0x0050, DICOMParser::VR_FL, PixelSpacingCB);

  WidthCB->SetCallbackFunction(this, &DICOMAppHelper::WidthCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0011, DICOMParser::VR_US, WidthCB);

  HeightCB->SetCallbackFunction(this, &DICOMAppHelper::HeightCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0010, DICOMParser::VR_US, HeightCB);

  PixelRepresentationCB->SetCallbackFunction(this, &DICOMAppHelper::PixelRepresentationCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0103, DICOMParser::VR_US, PixelRepresentationCB);

  PhotometricInterpretationCB->SetCallbackFunction(this, &DICOMAppHelper::PhotometricInterpretationCallback);
  parser->AddDICOMTagCallback(0x0028, 0x0004, DICOMParser::VR_CS, PhotometricInterpretationCB);

  RescaleOffsetCB->SetCallbackFunction(this, &DICOMAppHelper::RescaleOffsetCallback);
  parser->AddDICOMTagCallback(0x0028, 0x1052, DICOMParser::VR_CS, RescaleOffsetCB);

  RescaleSlopeCB->SetCallbackFunction(this, &DICOMAppHelper::RescaleSlopeCallback);
  parser->AddDICOMTagCallback(0x0028, 0x1053, DICOMParser::VR_FL, RescaleSlopeCB);

  DICOMTagInfo dicom_tags[] = {
    {0x0002, 0x0002, DICOMParser::VR_UI, "Media storage SOP class uid"},
    {0x0002, 0x0003, DICOMParser::VR_UI, "Media storage SOP inst uid"},
    {0x0002, 0x0010, DICOMParser::VR_UI, "Transfer syntax uid"},
    {0x0002, 0x0012, DICOMParser::VR_UI, "Implementation class uid"},
    {0x0008, 0x0018, DICOMParser::VR_UI, "Image UID"},
    {0x0008, 0x0020, DICOMParser::VR_DA, "Series date"},
    {0x0008, 0x0030, DICOMParser::VR_TM, "Series time"},
    {0x0008, 0x0060, DICOMParser::VR_SH, "Modality"},
    {0x0008, 0x0070, DICOMParser::VR_SH, "Manufacturer"},
    {0x0008, 0x1060, DICOMParser::VR_SH, "Physician"},
    {0x0018, 0x0050, DICOMParser::VR_FL, "slice thickness"},
    {0x0018, 0x0060, DICOMParser::VR_FL, "kV"},
    {0x0018, 0x0088, DICOMParser::VR_FL, "slice spacing"},
    {0x0018, 0x1100, DICOMParser::VR_SH, "Recon diameter"},
    {0x0018, 0x1151, DICOMParser::VR_FL, "mA"},
    {0x0018, 0x1210, DICOMParser::VR_SH, "Recon kernel"},
    {0x0020, 0x000d, DICOMParser::VR_UI, "Study UID"},
    {0x0020, 0x000e, DICOMParser::VR_UI, "Series UID"},
    {0x0020, 0x0013, DICOMParser::VR_IS, "Image number"},
    {0x0020, 0x0032, DICOMParser::VR_SH, "Patient position"},
    {0x0020, 0x0037, DICOMParser::VR_SH, "Patient position cosines"},
    {0x0020, 0x1041, DICOMParser::VR_CS, "Slice location"},
    {0x0028, 0x0010, DICOMParser::VR_FL, "Num rows"},
    {0x0028, 0x0011, DICOMParser::VR_FL, "Num cols"},
    {0x0028, 0x0030, DICOMParser::VR_FL, "pixel spacing"},
    {0x0028, 0x0100, DICOMParser::VR_US, "Bits allocated"},
    {0x0028, 0x0120, DICOMParser::VR_UL, "pixel padding"},
    {0x0028, 0x1052, DICOMParser::VR_FL, "pixel offset"}
  };

  int num_tags = sizeof(dicom_tags)/sizeof(DICOMTagInfo);

#ifdef DEBUG_DICOM_APP_HELPER
  DICOMMemberCallback<DICOMAppHelper>** callbackArray = new DICOMMemberCallback<DICOMAppHelper>*[num_tags];
#endif

  for (int j = 0; j < num_tags; j++)
    {
    //
    // Setup internal map.
    //
    DICOMTagInfo tagStruct = dicom_tags[j];
    doublebyte group = tagStruct.group;
    doublebyte element = tagStruct.element;

    std::pair<doublebyte, doublebyte> gePair(group, element);
    std::pair<std::pair<doublebyte, doublebyte>, DICOMTagInfo> mapPair(gePair, tagStruct);
    this->TagMap.insert(mapPair);

#ifdef DEBUG_DICOM_APP_HELPER
    //
    // Make callback
    //
    callbackArray[j] = new DICOMMemberCallback<DICOMAppHelper>;
    callbackArray[j]->SetCallbackFunction(this, &DICOMAppHelper::ArrayCallback);
    //
    // Set callback on parser.
    //
    parser->AddDICOMTagCallback(group, element,datatype, callbackArray[j]);
#endif

    }

}

void DICOMAppHelper::SeriesUIDCallback(doublebyte,
                                       doublebyte,
                                       DICOMParser::VRTypes,
                                       unsigned char* val,
                                       quadbyte) 
{
  char* newString = (char*) val;
  std::string newStdString(newString);
  std::map<std::string, std::vector<std::string>, ltstdstr>::iterator iter = this->SeriesUIDMap.find(newStdString);
  if ( iter == this->SeriesUIDMap.end())
    {
    std::vector<std::string> newVector;

    std::string filename(this->FileName);

    newVector.push_back(filename);
    this->SeriesUIDMap.insert(std::pair<std::string, std::vector<std::string> > (newStdString, newVector));
    }
  else
    {
    std::string filename(this->FileName);
    (*iter).second.push_back(filename);
    }
} 

void DICOMAppHelper::OutputSeries()
{
  std::cout << std::endl << std::endl;
        
  for (std::map<std::string, std::vector<std::string>, ltstdstr >::iterator iter = this->SeriesUIDMap.begin();
       iter != this->SeriesUIDMap.end();
       iter++)
    {
    std::cout << "SERIES: " << (*iter).first << std::endl;
    std::vector<std::string>& v_ref = (*iter).second;
             
    for (std::vector<std::string>::iterator v_iter = v_ref.begin();
         v_iter != v_ref.end();
         v_iter++)
      {
      std::map<std::string, int, ltstdstr>::iterator sn_iter = SliceNumberMap.find(*v_iter);

      int slice = -1;
      if (sn_iter != SliceNumberMap.end())
        {
        slice = (*sn_iter).second;
        }
      std::cout << "\t" << *v_iter << " [" << slice << "]" <<  std::endl;
      }
                
    }
}

void DICOMAppHelper::SetFileName(const char* filename)
{
  if (this->HeaderFile.is_open())
    {
    this->HeaderFile.close();
    }

  if (this->FileName)
    {
    delete [] this->FileName;
    }

  ::size_t len = strlen(filename);
  this->FileName = new char[len + 1];
  strcpy(this->FileName, filename);

#ifdef DEBUG_DICOM_APP_HELPER
  std::string myfilename(std::string((char*) this->FileName) + ".header.txt");
  this->HeaderFile.open(myfilename.c_str());
#endif

}

void DICOMAppHelper::ArrayCallback(doublebyte group,
                                   doublebyte element,
                                   DICOMParser::VRTypes datatype,
                                   unsigned char* val,
                                   quadbyte len) 
{
  char* desc = "No description";
  
  TagMapType::iterator iter = this->TagMap.find(std::pair<doublebyte, doublebyte> (group, element));
  if (iter != this->TagMap.end())
    {
    desc = (*iter).second.description;
    }

  int t2 = int((0x0000FF00 & datatype) >> 8);
  int t1 = int((0x000000FF & datatype));

  char ct2 = static_cast<char>(t2);
  char ct1 = static_cast<char>(t1);

    
  HeaderFile << "(0x";

  HeaderFile.width(4);
  char prev = HeaderFile.fill('0');

  HeaderFile << std::hex << group;
  HeaderFile << ",0x";

  HeaderFile.width(4);
  HeaderFile.fill('0');
    
  HeaderFile << std::hex << element;
  HeaderFile << ") ";

  HeaderFile.fill(prev);
  HeaderFile << std::dec;
  HeaderFile << " " << ct1 << ct2 << " ";
  HeaderFile << "[" << len << " bytes] ";
  
  HeaderFile << desc << " : ";
  
  unsigned int uival = 0;
  float fval = 0;
  double dval = 0;
  int ival = 0;

  if (val)
    {
    switch (datatype)
      {
      case DICOMParser::VR_AE:
      case DICOMParser::VR_AS:
      case DICOMParser::VR_CS:
      case DICOMParser::VR_UI:
      case DICOMParser::VR_DA:
      case DICOMParser::VR_DS:
      case DICOMParser::VR_DT:
      case DICOMParser::VR_LO:
      case DICOMParser::VR_LT:
      case DICOMParser::VR_OB: // ordered bytes
      case DICOMParser::VR_OW: // ordered words
      case DICOMParser::VR_PN:
      case DICOMParser::VR_ST:
      case DICOMParser::VR_TM:
      case DICOMParser::VR_UN:
      case DICOMParser::VR_UT:
      case DICOMParser::VR_SQ: // sequence
      case DICOMParser::VR_SH: // strings
      case DICOMParser::VR_IS:
        HeaderFile << val;
        break;
      case DICOMParser::VR_FL: // float
        fval = static_cast<float> (atof((char*) val));
        HeaderFile << fval;
        break;
      case DICOMParser::VR_FD: // float double
        fval = static_cast<float> (atof((char*) val));
        HeaderFile << dval;
        break;
      case DICOMParser::VR_UL: // unsigned long
      case DICOMParser::VR_SL: // signed long
      case DICOMParser::VR_AT:
        HeaderFile << uival;
        break;
      //case DICOMParser::VR_IS:
      //  ival = DICOMFile::ReturnAsSignedLong(val, this->DICOMDataFile->GetByteSwap()); 
      //  HeaderFile << ival;
      //  break;
      case DICOMParser::VR_SS:
        ival = DICOMFile::ReturnAsSignedShort(val, this->DICOMDataFile->GetByteSwap()); 
        HeaderFile << ival;
        break;
      case DICOMParser::VR_US: // unsigned short
        uival = DICOMFile::ReturnAsUnsignedShort(val, this->DICOMDataFile->GetByteSwap()); 
        HeaderFile << uival;
        break;
      default:
        HeaderFile << val << std::endl;
        break;
      }
    }
  else
    {
    HeaderFile << "NULL";
    }
  
  HeaderFile << std::dec << std::endl;
  HeaderFile.fill(prev);

  delete [] val;
}
    
void DICOMAppHelper::SliceNumberCallback(doublebyte,
                                         doublebyte,
                                         DICOMParser::VRTypes,
                                         unsigned char* val,
                                         quadbyte) 
{
  char* newString = (char*) val;
  this->SliceNumber = atoi(newString);

#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Slice number: " << this->SliceNumber << std::endl;
#endif

  std::string filename(this->FileName);
  this->SliceNumberMap.insert(std::pair<std::string, int> (filename, this->SliceNumber));
}

void DICOMAppHelper::TransferSyntaxCallback(doublebyte,
                                            doublebyte,
                                            DICOMParser::VRTypes,
                                            unsigned char* val,
                                            quadbyte) 
{

#ifdef DEBUG_DICOM_APP_HELPER
#ifdef WIN32
  char platformByteOrder = 'L';
#else
  char platformByteOrder = 'B';
#endif
  std::cout << "Platform byte order: " << platformByteOrder << std::endl;
#endif

  static char* TRANSFER_UID_EXPLICIT_BIG_ENDIAN = "1.2.840.10008.1.2.2";


  DICOMMemberCallback<DICOMAppHelper>* cb = new DICOMMemberCallback<DICOMAppHelper>;
  cb->SetCallbackFunction(this, &DICOMAppHelper::ToggleSwapBytesCallback);

  if (strcmp(TRANSFER_UID_EXPLICIT_BIG_ENDIAN, (char*) val) == 0)
    {
    this->ByteSwapData = true;
    this->Parser->AddDICOMTagCallback(0x0800, 0x0000, DICOMParser::VR_UNKNOWN, cb);
#ifdef DEBUG_DICOM_APP_HELPER
    std::cerr <<"Registering callback for swapping bytes." << std::endl;
#endif
    }
  
  if (this->TransferSyntaxUID)
    {
    delete this->TransferSyntaxUID;
    }
  this->TransferSyntaxUID = new std::string((char*) val);

#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Transfer Syntax UID: " << *this->TransferSyntaxUID;
  std::cout << " " << this->TransferSyntaxUIDDescription(this->TransferSyntaxUID->c_str()) << std::endl;
#endif
}

void DICOMAppHelper::BitsAllocatedCallback(doublebyte,
                                           doublebyte,
                                           DICOMParser::VRTypes,
                                           unsigned char* val,
                                           quadbyte) 
{
  this->BitsAllocated = this->DICOMDataFile->ReturnAsUnsignedShort(val, this->DICOMDataFile->GetByteSwap());
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Bits allocated: " << this->BitsAllocated << std::endl;
#endif
}


void DICOMAppHelper::ToggleSwapBytesCallback(doublebyte,
                                             doublebyte,
                                             DICOMParser::VRTypes,
                                             unsigned char* ,
                                             quadbyte len) 
{
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "ToggleSwapBytesCallback" << std::endl;
#endif
  bool bs = this->DICOMDataFile->GetByteSwap();
  this->DICOMDataFile->SetByteSwap(!bs);

#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Set byte swap to: " << this->DICOMDataFile->GetByteSwap() << std::endl;
#endif

  long pos = this->DICOMDataFile->Tell();

  //
  // The +4 is probably a hack, but it's a guess at the length of the previous field.
  //
  this->DICOMDataFile->SkipToPos(pos - len + 4);
}


void DICOMAppHelper::PixelSpacingCallback(doublebyte group,
                                          doublebyte element,
                                          DICOMParser::VRTypes,
                                          unsigned char* val,
                                          quadbyte) 
{
  float fval = DICOMFile::ReturnAsFloat(val, this->DICOMDataFile->GetByteSwap());

  if (group == 0x0028 && element == 0x0030)
    {
    this->PixelSpacing[0] = this->PixelSpacing[1] = fval;
    }
  else if (group == 0x0018 && element == 0x0050)
    {
    this->PixelSpacing[2] = fval;
    }
}

void DICOMAppHelper::WidthCallback(doublebyte,
                                   doublebyte,
                                   DICOMParser::VRTypes,
                                   unsigned char* val,
                                   quadbyte)
{
  unsigned short uival = DICOMFile::ReturnAsUnsignedShort(val, this->DICOMDataFile->GetByteSwap()); 
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Width: " << uival << std::endl;
#endif

  this->Width = uival;
  this->Dimensions[0] = this->Width;
}

void DICOMAppHelper::HeightCallback(doublebyte,
                                    doublebyte,
                                    DICOMParser::VRTypes,
                                    unsigned char* val,
                                    quadbyte) 
{
  unsigned short uival = DICOMFile::ReturnAsUnsignedShort(val, this->DICOMDataFile->GetByteSwap()); 
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Height: " << uival << std::endl;
#endif
  this->Height = uival;
  this->Dimensions[1] = this->Height;
}


void DICOMAppHelper::PixelRepresentationCallback( doublebyte,
                                                  doublebyte,
                                                  DICOMParser::VRTypes,
                                                  unsigned char* val,
                                                  quadbyte)
{
  unsigned short uival = DICOMFile::ReturnAsUnsignedShort(val, this->DICOMDataFile->GetByteSwap());
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Pixel Representation: " << (uival ? "Signed" : "Unsigned") << std::endl;
#endif
  this->PixelRepresentation = uival;
}

void DICOMAppHelper::PhotometricInterpretationCallback( doublebyte,
                                                        doublebyte,
                                                        DICOMParser::VRTypes,
                                                        unsigned char* val,
                                                        quadbyte)
{
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Photometric Interpretation: " << (char*) val << std::endl;
#endif
  if (this->PhotometricInterpretation)
    {
    delete this->PhotometricInterpretation;
    }

  this->PhotometricInterpretation = new std::string((char*) val);
}

void DICOMAppHelper::PixelDataCallback( doublebyte,
                                        doublebyte,
                                        DICOMParser::VRTypes,
                                        unsigned char* data,
                                        quadbyte len)
{
  int numPixels = this->Dimensions[0] * this->Dimensions[1];
  if (len < numPixels)
    {
    numPixels = len;
    }
  if (numPixels < 0)
    {
    numPixels = 0;
    }

#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "numPixels : " << numPixels << std::endl;
#endif

  int ptrIncr = int(this->BitsAllocated/8.0);

  unsigned short* ushortInputData = reinterpret_cast<unsigned short*>(data);
  unsigned char* ucharInputData = data;
  short* shortInputData = reinterpret_cast<short*> (data);

  float* floatOutputData; // = NULL;
  
  bool isFloat = this->RescaledImageDataIsFloat();

  if (isFloat)
    {
#ifdef DEBUG_DICOM_APP_HELPER
    std::cout << "Slope and offset are not integer valued : ";
    std::cout << this->RescaleSlope << ", " << this->RescaleOffset << std::endl;
#endif
    if (this->ImageData)
      {
      delete [] (static_cast<char*> (this->ImageData));
      }
    this->ImageData = new float[numPixels];
    floatOutputData = static_cast<float*> (this->ImageData);

    this->ImageDataType = DICOMParser::VR_FL;
    this->ImageDataLengthInBytes = numPixels * sizeof(float);
    float newFloatPixel;

    if (ptrIncr == 1)
      {
      for (int i = 0; i < numPixels; i++)
        {
        newFloatPixel = float(this->RescaleSlope * ucharInputData[i] + this->RescaleOffset);
        floatOutputData[i] = newFloatPixel;
        }
#ifdef DEBUG_DICOM_APP_HELPER
      std::cout << "Did rescale, offset to float from char." << std::endl;
      std::cout << numPixels << " pixels." << std::endl;
#endif
      }
    else if (ptrIncr == 2)
      {
      for (int i = 0; i < numPixels; i++)
        {
        newFloatPixel = float(this->RescaleSlope * ushortInputData[i] + this->RescaleOffset);
        floatOutputData[i] = newFloatPixel;
        }
#ifdef DEBUG_DICOM_APP_HELPER
      std::cout << "Did rescale, offset to float from short." << std::endl;
      std::cout << numPixels << " pixels." << std::endl;
#endif
      }
    }
  else
    {
#ifdef DEBUG_DICOM_APP_HELPER
    std::cout << "Slope and offset are integer valued : ";
    std::cout << this->RescaleSlope << ", " << this->RescaleOffset << std::endl;
#endif

    if (ptrIncr == 1)
      {
      if (this->ImageData)
        {
        delete [] (static_cast<char*> (this->ImageData));
        }
      this->ImageData = new char[numPixels];
  
      char*  charOutputData =  static_cast<char*>  (this->ImageData);

      this->ImageDataType = DICOMParser::VR_OB;
      this->ImageDataLengthInBytes = numPixels * sizeof(char);
      char newCharPixel;

      for (int i = 0; i < numPixels; i++)
        {
        newCharPixel = char(this->RescaleSlope * ucharInputData[i] + this->RescaleOffset);
        charOutputData[i] = newCharPixel;
        }
#ifdef DEBUG_DICOM_APP_HELPER
      std::cout << "Did rescale, offset to char from char." << std::endl;
      std::cout << numPixels << " pixels." << std::endl;
#endif
      }
    else if (ptrIncr == 2)
      {
      if (this->ImageData)
        {
        delete [] (static_cast<char*> (this->ImageData));
        }
      this->ImageData = new short[numPixels];
      short* shortOutputData = static_cast<short*> (this->ImageData);

      this->ImageDataType = DICOMParser::VR_OW;
      this->ImageDataLengthInBytes = numPixels * sizeof(short);
      short newShortPixel;
      for (int i = 0; i < numPixels; i++)
        {
        newShortPixel = short(this->RescaleSlope * shortInputData[i] + this->RescaleOffset);
        shortOutputData[i] = newShortPixel;
        }
#ifdef DEBUG_DICOM_APP_HELPER
      std::cout << "Did rescale, offset to short from short." << std::endl;
      std::cout << numPixels << " pixels." << std::endl;
#endif
      }
    }
}

void DICOMAppHelper::RegisterPixelDataCallback()
{
  //DICOMMemberCallback<DICOMAppHelper>* cb = new DICOMMemberCallback<DICOMAppHelper>;
  //cb->SetCallbackFunction(this, &DICOMAppHelper::PixelDataCallback);
  //this->Parser->AddDICOMTagCallback(0x7FE0, 0x0010, DICOMParser::VR_OW, cb);
  this->PixelDataCB->SetCallbackFunction(this, &DICOMAppHelper::PixelDataCallback);
  this->Parser->AddDICOMTagCallback(0x7FE0, 0x0010, DICOMParser::VR_OW, this->PixelDataCB);
}

void DICOMAppHelper::RescaleOffsetCallback( doublebyte,
                                          doublebyte,
                                          DICOMParser::VRTypes,
                                          unsigned char* val,
                                          quadbyte)
{
  float fval = DICOMFile::ReturnAsFloat(val, this->DICOMDataFile->GetByteSwap());
  this->RescaleOffset = fval;
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Pixel offset: " << this->RescaleOffset << std::endl;
#endif
}

const char* DICOMAppHelper::TransferSyntaxUIDDescription(const char* uid)
{
  static const char* DICOM_IMPLICIT_VR_LITTLE_ENDIAN = "1.2.840.10008.1.2";
  static const char* DICOM_LOSSLESS_JPEG = "1.2.840.10008.1.2.4.70";
  static const char* DICOM_LOSSY_JPEG_8BIT = "1.2.840.10008.1.2.4.50";
  static const char* DICOM_LOSSY_JPEG_16BIT = "1.2.840.10008.1.2.4.51";
  static const char* DICOM_EXPLICIT_VR_LITTLE_ENDIAN = "1.2.840.10008.1.2.1";
  static const char* DICOM_EXPLICIT_VR_BIG_ENDIAN = "1.2.840.10008.1.2.2";
  static const char* DICOM_GE_PRIVATE_IMPLICIT_BIG_ENDIAN = "1.2.840.113619.5.2";

  if (!strcmp(DICOM_IMPLICIT_VR_LITTLE_ENDIAN, uid))
    {
    return "Implicit VR, Little Endian";
    }
  else if (!strcmp(DICOM_LOSSLESS_JPEG, uid))
    {
    return "Lossless JPEG";
    }
  else if (!strcmp(DICOM_LOSSY_JPEG_8BIT, uid))
    {
    return "Lossy JPEG 8 bit";
    }
  else if (!strcmp(DICOM_LOSSY_JPEG_16BIT, uid))
    {
    return "Lossy JPEG 16 bit.";
    }
  else if (!strcmp(DICOM_EXPLICIT_VR_LITTLE_ENDIAN, uid))
    {
    return "Explicit VR, Little Endian.";
    }
  else if (!strcmp(DICOM_EXPLICIT_VR_BIG_ENDIAN, uid))
    {
    return "Explicit VR, Big Endian.";
    }
  else if (!strcmp(DICOM_GE_PRIVATE_IMPLICIT_BIG_ENDIAN, uid))
    {
    return "GE Private, Implicit VR, Big Endian Image Data.";
    }
  else
    {
    return "Unknown.";
    }

}


void DICOMAppHelper::RescaleSlopeCallback(doublebyte,
                     doublebyte ,
                     DICOMParser::VRTypes ,
                     unsigned char* val,
                     quadbyte )
{
  float fval = DICOMFile::ReturnAsFloat(val, this->DICOMDataFile->GetByteSwap());
#ifdef DEBUG_DICOM_APP_HELPER
  std::cout << "Rescale slope: " << fval << std::endl;
#endif
  this->RescaleSlope = fval;
}

bool DICOMAppHelper::RescaledImageDataIsFloat()
{
  int s = int(this->RescaleSlope);
  int o = int(this->RescaleOffset);

  float sf = float(s);
  float of = float(o);

  double d1 = fabs(sf - this->RescaleSlope);
  double d2 = fabs(of - this->RescaleOffset);

  if (d1 > 0.0 || d2 > 0.0)
    {
    return true;
    }
  else
    {
    return false;
    }
}

void DICOMAppHelper::GetImageData(void*& data, DICOMParser::VRTypes& dataType, unsigned long& len)
{
  data = this->ImageData;
  dataType = this->ImageDataType;
  len = this->ImageDataLengthInBytes;
}

bool DICOMAppHelper::RescaledImageDataIsSigned()
{
  bool rescaleSigned = (this->RescaleSlope < 0.0);
  bool pixelRepSigned = (this->PixelRepresentation == 1);
  bool offsetSigned = (this->RescaleOffset < 0.0);
 
  return (rescaleSigned || pixelRepSigned || offsetSigned);
}


void DICOMAppHelper::GetSliceNumberFilenamePairs(std::vector<std::pair<int, std::string> >& v)
{
  v.clear();

  std::map<std::string, std::vector<std::string>, ltstdstr >::iterator miter  = this->SeriesUIDMap.begin();

  std::vector<std::string> files = (*miter).second;

  for (std::vector<std::string>::iterator fileIter = files.begin();
       fileIter != files.end();
       fileIter++)
       {
       std::pair<int, std::string> p;
       p.second = std::string(*fileIter);
       int slice_number = -1;
       std::map<std::string, int, ltstdstr>::iterator sn_iter = SliceNumberMap.find(*fileIter);
       if (sn_iter != SliceNumberMap.end())
        {
        slice_number = (*sn_iter).second;
        }
       p.first = slice_number;
       v.push_back(p);
       }
  std::sort(v.begin(), v.end(), lt_pair_int_string());
}

void DICOMAppHelper::ClearSliceNumberMap()
{ 
  this->SliceNumberMap.clear();
}

void DICOMAppHelper::ClearSeriesUIDMap()
{
 this->SeriesUIDMap.clear();
}

void DICOMAppHelper::SortFilenamesBySlice()
{
  this->SortedFilenames.clear();

  std::map<std::string, std::vector<std::string>, ltstdstr >::iterator miter  = this->SeriesUIDMap.begin();

  std::vector<std::string> files = (*miter).second;

  for (std::vector<std::string>::iterator fileIter = files.begin();
       fileIter != files.end();
       fileIter++)
       {
       std::pair<int, std::string> p;
       p.second = std::string(*fileIter);
       int slice_number = -1;
       std::map<std::string, int, ltstdstr>::iterator sn_iter = SliceNumberMap.find(*fileIter);
       if (sn_iter != SliceNumberMap.end())
        {
        slice_number = (*sn_iter).second;
        }
       p.first = slice_number;
       this->SortedFilenames.push_back(p);
       }
  std::sort(this->SortedFilenames.begin(), this->SortedFilenames.end(), lt_pair_int_string());

}

unsigned int DICOMAppHelper::GetNumberOfSortedFilenames()
{
  return this->SortedFilenames.size();
}

std::string DICOMAppHelper::GetFilenameForSlice(unsigned int slice_number)
{
  if (slice_number < this->SortedFilenames.size())
    {
    return (this->SortedFilenames[slice_number]).second;
    }
  return std::string(" ");
}
