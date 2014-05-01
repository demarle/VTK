/*****************************************************************************/
/*                                    XDMF                                   */
/*                       eXtensible Data Model and Format                    */
/*                                                                           */
/*  Id : XdmfTopologyType.hpp                                                */
/*                                                                           */
/*  Author:                                                                  */
/*     Kenneth Leiter                                                        */
/*     kenneth.leiter@arl.army.mil                                           */
/*     US Army Research Laboratory                                           */
/*     Aberdeen Proving Ground, MD                                           */
/*                                                                           */
/*     Copyright @ 2011 US Army Research Laboratory                          */
/*     All Rights Reserved                                                   */
/*     See Copyright.txt for details                                         */
/*                                                                           */
/*     This software is distributed WITHOUT ANY WARRANTY; without            */
/*     even the implied warranty of MERCHANTABILITY or FITNESS               */
/*     FOR A PARTICULAR PURPOSE.  See the above copyright notice             */
/*     for more information.                                                 */
/*                                                                           */
/*****************************************************************************/

#ifndef XDMFTOPOLOGYTYPE_HPP_
#define XDMFTOPOLOGYTYPE_HPP_

// Includes
#include "Xdmf.hpp"
#include "XdmfItemProperty.hpp"

/**
 * @brief Property describing the types of elements stored in an
 * XdmfTopology.
 *
 * XdmfTopologyType is a property used by XdmfTopology to specify the
 * element types stored. A specific XdmfTopologyType can be created by
 * calling one of the static methods in the class,
 * i.e. XdmfTopologyType::Tetrahedron().
 *
 * Xdmf supports the following topology types:
 *   NoTopologyType
 *   Polyvertex - Unconnected Points
 *   Polyline - Line Segments
 *   Polygon - N Edge Polygon
 *   Triangle - 3 Edge Polygon
 *   Quadrilateral - 4 Edge Polygon
 *   Tetrahedron - 4 Triangular Faces
 *   Wedge - 4 Triangular Faces, Quadrilateral Base
 *   Hexahedron - 6 Quadrilateral Faces
 *   Edge_3 - 3 Node Quadratic Line
 *   Triangle_6 - 6 Node Quadratic Triangle
 *   Quadrilateral_8 - 8 Node Quadratic Quadrilateral
 *   Quadrilateral_9 - 9 Node Bi-Quadratic Quadrilateral
 *   Tetrahedron_10 - 10 Node Quadratic Tetrahedron
 *   Pyramid_13 - 13 Node Quadratic Pyramid
 *   Wedge_15 - 15 Node Quadratic Wedge
 *   Wedge_18 - 18 Node Bi-Quadratic Wedge
 *   Hexahedron_20 - 20 Node Quadratic Hexahedron
 *   Hexahedron_24 - 24 Node Bi-Quadratic Hexahedron
 *   Hexahedron_27 - 27 Node Tri-Quadratic Hexahedron
 *   Hexahedron_64 - 64 Node Tri-Cubic Hexahedron
 *   Hexahedron_125 - 125 Node Tri-Quartic Hexahedron
 *   Hexahedron_216 - 216 Node Tri-Quintic Hexahedron
 *   Hexahedron_343 - 343 Node Tri-Hexic Hexahedron
 *   Hexahedron_512 - 512 Node Tri-Septic Hexahedron
 *   Hexahedron_729 - 729 Node Tri-Octic Hexahedron
 *   Hexahedron_1000 - 1000 Node Tri-Nonic Hexahedron
 *   Hexahedron_1331 - 1331 Node Tri-Decic Hexahedron
 *   Hexahedron_Spectral_64 - 64 Node Spectral Tri-Cubic Hexahedron
 *   Hexahedron_Spectral_125 - 125 Node Spectral Tri-Quartic Hexahedron
 *   Hexahedron_Spectral_216 - 216 Node Spectral Tri-Quintic Hexahedron
 *   Hexahedron_Spectral_343 - 343 Node Spectral Tri-Hexic Hexahedron
 *   Hexahedron_Spectral_512 - 512 Node Spectral Tri-Septic Hexahedron
 *   Hexahedron_Spectral_729 - 729 Node Spectral Tri-Octic Hexahedron
 *   Hexahedron_Spectral_1000 - 1000 Node Spectral Tri-Nonic Hexahedron
 *   Hexahedron_Spectral_1331 - 1331 Node Spectral Tri-Decic Hexahedron
 *   Mixed - Mixture of Unstructured Topologies
 */
class XDMF_EXPORT XdmfTopologyType : public XdmfItemProperty {

public:

  virtual ~XdmfTopologyType();

  friend class XdmfTopology;

  enum CellType {
    NoCellType = 0,
    Linear = 1,
    Quadratic = 2,
    Cubic = 3,
    Quartic = 4,
    Quintic = 5,
    Sextic = 6,
    Septic = 7,
    Octic = 8,
    Nonic = 9,
    Decic = 10,
    Arbitrary = 100,
    Structured = 101
  };

  /**
   * Supported Xdmf Topology Types
   */
  static shared_ptr<const XdmfTopologyType> NoTopologyType();
  static shared_ptr<const XdmfTopologyType> Polyvertex();
  static shared_ptr<const XdmfTopologyType> 
  Polyline(const unsigned int nodesPerElement);
  static shared_ptr<const XdmfTopologyType>
  Polygon(const unsigned int nodesPerElement);
  static shared_ptr<const XdmfTopologyType> Triangle();
  static shared_ptr<const XdmfTopologyType> Quadrilateral();
  static shared_ptr<const XdmfTopologyType> Tetrahedron();
  static shared_ptr<const XdmfTopologyType> Pyramid();
  static shared_ptr<const XdmfTopologyType> Wedge();
  static shared_ptr<const XdmfTopologyType> Hexahedron();
  static shared_ptr<const XdmfTopologyType> Edge_3();
  static shared_ptr<const XdmfTopologyType> Triangle_6();
  static shared_ptr<const XdmfTopologyType> Quadrilateral_8();
  static shared_ptr<const XdmfTopologyType> Quadrilateral_9();
  static shared_ptr<const XdmfTopologyType> Tetrahedron_10();
  static shared_ptr<const XdmfTopologyType> Pyramid_13();
  static shared_ptr<const XdmfTopologyType> Wedge_15();
  static shared_ptr<const XdmfTopologyType> Wedge_18();
  static shared_ptr<const XdmfTopologyType> Hexahedron_20();
  static shared_ptr<const XdmfTopologyType> Hexahedron_24();
  static shared_ptr<const XdmfTopologyType> Hexahedron_27();
  static shared_ptr<const XdmfTopologyType> Hexahedron_64();
  static shared_ptr<const XdmfTopologyType> Hexahedron_125();
  static shared_ptr<const XdmfTopologyType> Hexahedron_216();
  static shared_ptr<const XdmfTopologyType> Hexahedron_343();
  static shared_ptr<const XdmfTopologyType> Hexahedron_512();
  static shared_ptr<const XdmfTopologyType> Hexahedron_729();
  static shared_ptr<const XdmfTopologyType> Hexahedron_1000();
  static shared_ptr<const XdmfTopologyType> Hexahedron_1331();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_64();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_125();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_216();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_343();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_512();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_729();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_1000();
  static shared_ptr<const XdmfTopologyType> Hexahedron_Spectral_1331();
  static shared_ptr<const XdmfTopologyType> Mixed();

  /**
   * Get a topology type from id.
   *
   * @param id of the topology type.
   *
   * @return topology type corresponding to id - if no topology type is found
   * an NULL pointer is returned.
   */
  static shared_ptr<const XdmfTopologyType> New(const unsigned int id);

  /**
   * Get the cell type associated with this topology type.
   *
   * @return a CellType containing the cell type.
   */
  CellType getCellType() const;

  /**
   * Get the number of edges per element associated with this topology type.
   *
   * @return an unsigned int containing the number of edges per element.
   */
  virtual unsigned int getEdgesPerElement() const;

  /**
   * Get the number of faces per element associated with this topology type.
   *
   * @return an unsigned int containing the number of faces per element.
   */
  virtual unsigned int getFacesPerElement() const;

  /**
   * Get the id of this cell type, necessary in order to create grids
   * containing mixed cells.
   *
   * @return the ID of the topology type.
   */
  virtual unsigned int getID() const;

  /**
   * Get the name of this topology type.
   *
   * @return the name of this topology type.
   */
  virtual std::string getName() const;

  /**
   * Get the number of nodes per element associated with this topology
   * type.
   *
   * @return an unsigned int containing number of nodes per element.
   */
  virtual unsigned int getNodesPerElement() const;

  void
  getProperties(std::map<std::string, std::string> & collectedProperties) const;

protected:

  /**
   * Protected constructor for XdmfTopologyType. The constructor is
   * protected because all topology types supported by Xdmf should be
   * accessed through more specific static methods that construct
   * XdmfTopologyType - i.e. XdmfTopologyType::Tetrahedron()
   */
  XdmfTopologyType(const unsigned int nodesPerElement,
                   const unsigned int facesPerElement,
                   const unsigned int edgesPerElement,
                   const std::string & name,
                   const CellType cellType,
                   const unsigned int id);

private:

  XdmfTopologyType(const XdmfTopologyType &); // Not implemented.
  void operator=(const XdmfTopologyType &); // Not implemented.

  static shared_ptr<const XdmfTopologyType>
  New(const std::map<std::string, std::string> & itemProperties);

  const CellType mCellType;
  const unsigned int mEdgesPerElement;
  const unsigned int mFacesPerElement;
  const unsigned int mID;
  const std::string mName;
  const unsigned int mNodesPerElement;

};

#endif /* XDMFTOPOLOGYTYPE_HPP_ */
