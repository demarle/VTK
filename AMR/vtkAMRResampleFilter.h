/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAMRResampleFilter.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkAMRResampleFilter.h -- Resamples AMR data to a uniform grid.
//
// .SECTION Description
//  This filter is a concrete instance of vtkMultiBlockDataSetAlgorithm and
//  provides functionality for extracting portion of the AMR dataset, specified
//  by a bounding box, in a uniform grid of the desired level of resolution.
//  The resulting uniform grid is stored in a vtkMultiBlockDataSet where the
//  number of blocks correspond to the number of processors utilized for the
//  operation.
//
// .SECTION Caveats
//  Data of the input AMR dataset is assumed to be cell-centered.
//
// .SECTION See Also
//  vtkHierarchicalBoxDataSet, vtkUniformGrid

#ifndef __vtkAMRResampleFilter_h
#define __vtkAMRResampleFilter_h

#include "vtkMultiBlockDataSetAlgorithm.h"
#include <vtkstd/vector> // For STL vector

class vtkInformation;
class vtkInformationVector;
class vtkUniformGrid;
class vtkHierarchicalBoxDataSet;
class vtkMultiBlockDataSet;
class vtkMultiProcessController;
class vtkFieldData;
class vtkCellData;
class vtkPointData;
class vtkIndent;

class VTK_AMR_EXPORT vtkAMRResampleFilter : public vtkMultiBlockDataSetAlgorithm
{
  public:
    static vtkAMRResampleFilter *New();
    vtkTypeMacro(vtkAMRResampleFilter,vtkMultiBlockDataSetAlgorithm);
    void PrintSelf( std::ostream &oss, vtkIndent indent);

    // Description:
    // Set & Get macro for the level of resolution.
    vtkSetMacro(LevelOfResolution,int);
    vtkGetMacro(LevelOfResolution,int);

    // Description:
    // Set & Get macro for the TransferToNodes flag
    vtkSetMacro(TransferToNodes,int);
    vtkGetMacro(TransferToNodes,int);

    // Description:
    // Set & Get macro to allow the filter to operate in both demand-driven
    // and standard modes
    vtkSetMacro(DemandDrivenMode,int);
    vtkGetMacro(DemandDrivenMode,int);

    // Description:
    // Set & Get macro for the number of subdivisions
    vtkSetMacro(NumberOfSubdivisions,int);
    vtkGetMacro(NumberOfSubdivisions,int);

    // Description:
    // Setter for the min/max bounds
    vtkSetVector3Macro(Min,double);
    vtkSetVector3Macro(Max,double);

    // Description:
    // Set & Get macro for the multi-process controller
    vtkSetMacro(Controller, vtkMultiProcessController*);
    vtkGetMacro(Controller, vtkMultiProcessController*);

    // Standard pipeline routines

    // Description:
    // Gets the metadata from upstream module and determines which blocks
    // should be loaded by this instance.
    virtual int RequestInformation(
        vtkInformation *rqst,
        vtkInformationVector **inputVector,
        vtkInformationVector *outputVector );

    virtual int RequestData(
         vtkInformation*,vtkInformationVector**,vtkInformationVector*);
    virtual int FillInputPortInformation(int port, vtkInformation *info);
    virtual int FillOutputPortInformation(int port, vtkInformation *info);

    // Description:
    // Performs upstream requests to the reader
    virtual int RequestUpdateExtent(
        vtkInformation*, vtkInformationVector**, vtkInformationVector* );


  protected:
    vtkAMRResampleFilter();
    virtual ~vtkAMRResampleFilter();

    vtkMultiBlockDataSet *ROI; // Pointer to the region of interest.
    double Min[3];
    double Max[3];
    int NumberOfSubdivisions;
    int TransferToNodes;
    int DemandDrivenMode;
    int LevelOfResolution;
    vtkMultiProcessController *Controller;
// BTX
    vtkstd::vector< int > BlocksToLoad; // Holds the ids of the blocks to load.
// ETX


    // Description:
    // Checks if this filter instance is running on more than one processes
    bool IsParallel();

    // Description:
    // Given the Region ID this function returns whether or not the region
    // belongs to this process or not.
    bool IsRegionMine( const int regionIdx );

    // Description:
    // Given the Region ID, this method computes the corresponding process ID
    // that owns the region based on static block-cyclic distribution.
    int GetRegionProcessId( const int regionIdx );

    // Description:
    // Given a cell index and a grid, this method computes the cell centroid.
    void ComputeCellCentroid(
        vtkUniformGrid *g, const vtkIdType cellIdx, double c[3] );

    // Description:
    // Given the source cell data of an AMR grid, this method initializes the
    // field values, i.e., the number of arrays with the prescribed size. Note,
    // the size must correspond to the number of points if node-centered or the
    // the number of cells if cell-centered.
    void InitializeFields( vtkFieldData *f, vtkIdType size, vtkCellData *src );

    // Description:
    // Copies the data to the target from the given source.
    void CopyData( vtkFieldData *target, vtkIdType targetIdx,
                   vtkCellData *src, vtkIdType srcIdx );

    // Description:
    // Given a query point q and a candidate donor grid, this method checks for
    // the corresponding donor cell containing the point in the given grid.
    bool FoundDonor(double q[3],vtkUniformGrid *donorGrid,int &cellIdx);

    // Description:
    // Transfers the solution from the AMR dataset to the cell-centers of
    // the given uniform grid.
    void TransferToCellCenters(
        vtkUniformGrid *g, vtkHierarchicalBoxDataSet *amrds );

    // Description:
    // Transfer the solution from the AMR dataset to the nodes of the
    // given uniform grid.
    void TransferToGridNodes(
        vtkUniformGrid *g, vtkHierarchicalBoxDataSet *amrds );

    // Description:
    // Transfers the solution
    void TransferSolution(
        vtkUniformGrid *g, vtkHierarchicalBoxDataSet *amrds);

    // Description:
    // Extract the region (as a multiblock) from the given AMR dataset.
    void ExtractRegion(
        vtkHierarchicalBoxDataSet *amrds, vtkMultiBlockDataSet *mbds,
        vtkHierarchicalBoxDataSet *metadata );

    // Description:
    // Checks if the AMR block, described by a uniform grid, is within the
    // bounds of the ROI perscribed by the user.
    bool IsBlockWithinBounds( vtkUniformGrid *grd );

    // Description:
    // Given a user-supplied region of interest and the metadata by a module
    // upstream, this method generates the list of linear AMR block indices
    // that need to be loaded.
    void ComputeAMRBlocksToLoad( vtkHierarchicalBoxDataSet *metadata );

    // Description:
    // This method gets the region of interest as perscribed by the user.
    void GetRegion( double h[3] );

    // Description:
    // Checks if two uniform grids intersect.
    bool GridsIntersect( vtkUniformGrid *g1, vtkUniformGrid *g2 );

    // Description:
    // Returns a reference grid from the amrdataset.
    vtkUniformGrid* GetReferenceGrid( vtkHierarchicalBoxDataSet *amrds );

  private:
    vtkAMRResampleFilter(const vtkAMRResampleFilter&); // Not implemented
    void operator=(const vtkAMRResampleFilter&); // Not implemented
};

#endif /* __vtkAMRResampleFilter_h */
