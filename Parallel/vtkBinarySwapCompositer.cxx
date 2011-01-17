/*
 * vtkBinarySwapCompositer.cxx
 *
 *  Created on: Jun 25, 2009
 *      Author: ollie
 */
#include "vtkBinarySwapCompositer.h"
#include "vtkObjectFactory.h"
#include "vtkMPI.h"
#include "vtkToolkits.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkMultiProcessController.h"
#include "vtkMPICommunicator.h"
#include "vtkTimerLog.h"

#include <iostream>
#include <mpi.h>

vtkStandardNewMacro(vtkBinarySwapCompositer);

//TODO: Should be parameterized on float or char pixels, and on RGB or RGBA
//TODO: non pow 2 screen res has a roundoff error x/2/2/2=result, result*2*2*2!=x

//==============================================================================

class ZCompositePairC {
public:
  ZCompositePairC(unsigned stride) :
    _stride(stride) 
  {}

  void operator() (char *dstRGB, float *dstZ,
                   const char *srcRGB, const float *srcZ) 
  {
    for (int i = 0; i < _stride; i++) 
      {
      if (*srcZ < *dstZ) 
        {
        *dstZ++   = *srcZ++;
        *dstRGB++ = *srcRGB++;
        *dstRGB++ = *srcRGB++;
        *dstRGB++ = *srcRGB++;
        *dstRGB++ = *srcRGB++;
        } 
      else 
        {
        srcZ++;
        dstZ++;
        srcRGB+=4;
        dstRGB+=4;
        }
      }
  }
  
protected:
  const unsigned _stride;
};

class BSwapCompositerIterative {
public:
  BSwapCompositerIterative(unsigned rank, unsigned npixels, unsigned nprocs,
                           MPI_Comm MPIComm) :
    _rank(rank), _npixels(npixels),
    _nprocs(nprocs), _MPIComm(MPIComm) 
  {}
  
  void operator() (char *localRGBA, float *localZ,
                   char *remoteRGBA, float *remoteZ) 
  {
    unsigned int nIters   = ilog2(_nprocs); // total number of iterations
    //cerr << "NPROCS " << _nprocs << " " << nIters << endl;
    unsigned int distance = 1;    // number of ranks between peers
    
    //cerr << "NPIXELS " << _npixels << endl;
    _npixels /= 2;
    //cerr << "NPIXELS " << _npixels << endl;

    // MPI_Reduce_scatter part: recursive distance doubling and vector halving
    for (int i = 0; i < nIters; i++, distance *= 2, _npixels /= 2) 
      {
      unsigned peer = _rank ^ distance;
      //cerr << "I " << i << " " << _rank << " with " << peer << endl;
      //cerr << "NPIXELS " << _npixels << endl;

      char strbuf[256];               
      if (_rank < peer) 
        {
        // work on first half of the data
        // send second half to peer and receive first half from peer

        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, _npixels*2 );
        vtkTimerLog::MarkStartEvent(strbuf);

        MPI_Status mpi_status;
        MPI_Sendrecv(localRGBA + _npixels*4, _npixels*4, MPI_CHAR, peer, 128 + i,
                     remoteRGBA,           _npixels*4, MPI_CHAR, peer, 128 + i,
                     _MPIComm, &mpi_status);
        MPI_Sendrecv(localZ    + _npixels, _npixels, MPI_FLOAT, peer, 128 - i,
                     remoteZ,              _npixels, MPI_FLOAT, peer, 128 - i,
                     _MPIComm, &mpi_status);

        vtkTimerLog::MarkEndEvent(strbuf);

        vtkTimerLog::MarkStartEvent("BSwapComp composite");       
        
        // composite the first half
        ZCompositePairC compositePair(_npixels);
        compositePair(localRGBA, localZ, remoteRGBA, remoteZ);

        vtkTimerLog::MarkEndEvent("BSwapComp composite");       
        } 
      else 
        {
        // work on the last half of the data
        // send first half to and receive second half from peer,
        // remoteRGBA and remoteZ are temporary buffers so we don't
        // have to index into half of them when receiving
        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, _npixels*2 );
        vtkTimerLog::MarkStartEvent(strbuf);

        MPI_Status mpi_status;
        MPI_Sendrecv(localRGBA,  _npixels*4, MPI_CHAR, peer, 128 + i,
                     remoteRGBA, _npixels*4, MPI_CHAR, peer, 128 + i,
                     _MPIComm, &mpi_status);
        MPI_Sendrecv(localZ,     _npixels, MPI_FLOAT, peer, 128 - i,
                     remoteZ,    _npixels, MPI_FLOAT, peer, 128 - i,
                     _MPIComm, &mpi_status);

        vtkTimerLog::MarkEndEvent(strbuf);
        
        // composite the second half
        localRGBA += _npixels*4;
        localZ   += _npixels;
        
        vtkTimerLog::MarkStartEvent("BSwapComp composite");       

        ZCompositePairC compositePair(_npixels);
        compositePair(localRGBA, localZ, remoteRGBA, remoteZ);

        vtkTimerLog::MarkEndEvent("BSwapComp composite");       
        }
      }
    
    // MPI_Gather part: recursive distance halving and vector doubling
    distance = _nprocs/2;
    _npixels *= 2;
    
    //cerr << "NPIXELS " << _npixels << endl;

    vtkTimerLog::MarkStartEvent("BSwapComp gather");       
    for (int i = 0; i < nIters; i++, distance /= 2, _npixels *= 2) 
      {
      //cerr << "NPIXELS " << _npixels << endl;
   //   if (_rank >= pow2(distance)) 
     //   {
       // // we are done, going out of the loop
//        return;
  //      }
      
      unsigned peer = _rank ^ distance;
      
      if (_rank < peer) 
        {
        MPI_Status mpi_status;
        MPI_Recv(localRGBA + _npixels*4, _npixels*4, MPI_CHAR, peer, 256 + i,
                 _MPIComm, &mpi_status);
        MPI_Recv(localZ    + _npixels, _npixels, MPI_FLOAT, peer, 256 - i,
                 _MPIComm, &mpi_status);
        } 
      else 
        {
        MPI_Send(localRGBA, _npixels*4, MPI_CHAR, peer, 256 + i,
                 _MPIComm);
        MPI_Send(localZ,    _npixels, MPI_FLOAT, peer, 256 - i,
                 _MPIComm);
        }
      }
    vtkTimerLog::MarkEndEvent("BSwapComp gather");       

  }
  
  unsigned _rank;  // my MPI_Comm_rank
  unsigned _npixels; // number of pixels I am working on
  
  // number of processes involved in the composition i.e. MPI_Comm_size
  unsigned int _nprocs;
  
  // VTK's MPI wrapper does not support MPI_Sendrecv, we have to call it
  // directly which requires the communicator we are in. The communicator
  // used by vtkCompositer is not the MPI_COMM_WORLD in order to prevent
  // conflict with other MPI communications.
  MPI_Comm _MPIComm;
  
protected:
  unsigned pow2(unsigned p) const 
  {
    return (1 << p);
  }
  
  unsigned ilog2(unsigned p) const 
  {
    int ret = 0;
    for (p >>= 1; p > 0; p >>= 1)
      {
      ret++;
      }
    return ret;
  }
};

// A modified Binary Swap compositer that works with non-2^n processes as
// described by Rolf Rabenseifner. It reduces the number of processes to
// the nearest power of two.
class ReducedBSwapCompositer {
public:
  ReducedBSwapCompositer(unsigned rank, unsigned npixels, unsigned nprocs,
                         MPI_Comm MPIComm) : 
    _rank(rank), _npixels(npixels),
    _nprocs(nprocs), _MPIComm(MPIComm) {}
  
  void operator() (char *localRGBA, float *localZ,
                   char *remoteRGBA, float *remoteZ) 
  {   
    // test if _npixels is a multiple of pow2_floor
    unsigned int pow2_floor = pow2(ilog2(_nprocs));
    //if ( (_npixels % pow2_floor) != 0) 
    //  {
    //  cerr <<
    //    "number of pixels is not a multiple of the number of power of two\n";
    //  return;
    //  }
#if 0    
    // reduce the number of processes to power of 2.
    if (!IsPow2(_nprocs)) 
      {
      char strbuf[256];         
      if (_rank < (_nprocs - pow2_floor)) 
        {
        MPI_Status mpi_status;
        unsigned peer = pow2_floor + _rank;
        unsigned size = _npixels/2;

        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, size*2 );
        vtkTimerLog::MarkStartEvent(strbuf);
        
        // work on first half of the data
        // send second half to peer and receive first half from peer
        MPI_Sendrecv(localRGBA + size*4, size*4, MPI_CHAR, peer, 128,
                     remoteRGBA,       size*4, MPI_CHAR, peer, 128,
                     _MPIComm, &mpi_status);
        MPI_Sendrecv(localZ    + size, size, MPI_FLOAT, peer, 128,
                     remoteZ,          size, MPI_FLOAT, peer, 128,
                     _MPIComm, &mpi_status);

        vtkTimerLog::MarkEndEvent(strbuf);

        vtkTimerLog::MarkStartEvent("BSwapComp composite");       
        // composite the first half
        ZCompositePairC compositePair(size);
        compositePair(localRGBA, localZ, remoteRGBA, remoteZ);
        vtkTimerLog::MarkStartEvent("BSwapComp composite");       
        
        // receive composited second half from peer
        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, size*2 );
        vtkTimerLog::MarkStartEvent(strbuf);

        MPI_Recv(localRGBA + size*4, size*4, MPI_CHAR, peer, 128,
                 _MPIComm, &mpi_status);
        MPI_Recv(localZ    + size, size, MPI_FLOAT, peer, 128,
                 _MPIComm, &mpi_status);
        vtkTimerLog::MarkEndEvent(strbuf);
        } 
      else if (_rank >= pow2_floor) 
        {
        MPI_Status mpi_status;
        unsigned peer = _rank - pow2_floor;
        unsigned size = _npixels/2;
        
        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, size*2 );
        vtkTimerLog::MarkStartEvent(strbuf);

        // work on the second half of the data
        // send first half to and receive second half from peer,
        MPI_Sendrecv(localRGBA,  size*4, MPI_CHAR, peer, 128,
                     remoteRGBA, size*4, MPI_CHAR, peer, 128,
                     _MPIComm, &mpi_status);
        MPI_Sendrecv(localZ,     size, MPI_FLOAT, peer, 128,
                     remoteZ,                size, MPI_FLOAT, peer, 128,
                     _MPIComm, &mpi_status);

        vtkTimerLog::MarkEndEvent(strbuf);
        
        // composite the second half
        localRGBA += size*4;
        localZ   += size;
        
        vtkTimerLog::MarkStartEvent("BSwapComp composite");       
        ZCompositePairC compositePair(size);
        compositePair(localRGBA, localZ, remoteRGBA, remoteZ);
        vtkTimerLog::MarkEndEvent("BSwapComp composite");       

        snprintf(strbuf, 256, "BSwapComp %d exchange with %d uchar %d",
                 _rank, peer, size*2 );
        vtkTimerLog::MarkStartEvent(strbuf);
        
        // send composited second half to peer
        MPI_Send(localRGBA, size*4, MPI_CHAR, peer, 128, _MPIComm);
        MPI_Send(localZ,    size, MPI_FLOAT, peer, 128, _MPIComm);

        vtkTimerLog::MarkEndEvent(strbuf);
        
        // we are done.
        return;
        }      
      }
#endif
    // the rest of 2^N processes
    BSwapCompositerIterative BSwap(_rank, _npixels, pow2(ilog2(_nprocs)), _MPIComm);
    BSwap(localRGBA, localZ, remoteRGBA, remoteZ);
  }
  
protected:
  unsigned pow2(unsigned p) const 
  {
    return (1 << p);
  }
  
  unsigned ilog2(unsigned p) const 
  {
    int ret = 0;
    for (p >>= 1; p > 0; p >>= 1)
      ret++;
    return ret;
  }
  
  unsigned _rank;  // my MPI_Comm_rank
  unsigned _npixels; // number of pixels I am working on
  
  // number of processes involved in the composition i.e. MPI_Comm_size
  unsigned int _nprocs;
  
  // VTK's MPI wrapper does not support MPI_Sendrecv, we have to call it
  // directly which requires the communicator we are in. The communicator
  // used by vtkCompositer is not the MPI_COMM_WORLD in order to prevent
  // conflict with other MPI communications.
  MPI_Comm _MPIComm;
  
private:
  bool IsPow2(unsigned p) 
  {
    return ( (p > 0) && ((p & (p - 1)) == 0) );
  }
};

//==============================================================================

void vtkBinarySwapCompositer::CompositeBuffer(vtkDataArray *pBuf,
           vtkFloatArray *zBuf,
           vtkDataArray *pTmp,
           vtkFloatArray *zTmp)
{
 int myId    = this->Controller->GetLocalProcessId();
 int nprocs  = this->NumberOfProcesses;
 int npixels = zBuf->GetNumberOfTuples();

 MPI_Comm MPIComm;
 if (vtkMPICommunicator *vtkMPIComm =
  vtkMPICommunicator::SafeDownCast(this->Controller->GetCommunicator())) 
   {
   MPIComm = *(vtkMPIComm->GetMPIComm()->GetHandle());
   } 
 else 
   {
   MPIComm = MPI_COMM_WORLD;
   }

 vtkTimerLog::MarkStartEvent("BinarySwapCompositer::CompositeBuffer");
 
 // create BinarySwapCompositer function object and call to its operator()
 char *localRGBA  = reinterpret_cast<char *>(pBuf->GetVoidPointer(0));
 float *localZ     = reinterpret_cast<float *>(zBuf->GetVoidPointer(0));
 char *remoteRGBA = reinterpret_cast<char *>(pTmp->GetVoidPointer(0));
 float *remoteZ    = reinterpret_cast<float *>(zTmp->GetVoidPointer(0));
 
 ReducedBSwapCompositer fObj(myId, npixels, nprocs, MPIComm);
 fObj(localRGBA, localZ, remoteRGBA, remoteZ);

 //cout << "void vtkBinarySwapCompositer::CompositeBuffer" << endl; 
 vtkTimerLog::MarkEndEvent("BinarySwapCompositer::CompositeBuffer");
}
