/*****************************************************************************/
/*                                    XDMF                                   */
/*                       eXtensible Data Model and Format                    */
/*                                                                           */
/*  Id : XdmfCoreItemFactory.hpp                                             */
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

#ifndef XDMFCOREITEMFACTORY_HPP_
#define XDMFCOREITEMFACTORY_HPP_

// Forward Declarations
class XdmfItem;

// Includes
#include <map>
#include <vector>
#include "XdmfCore.hpp"
#include "XdmfSharedPtr.hpp"

/**
 * @brief Factory that constructs XdmfItems using tags and properties.
 *
 * XdmfCoreItemFactory is an abstract base class.
 */
class XDMFCORE_EXPORT XdmfCoreItemFactory {

public:

  virtual ~XdmfCoreItemFactory() = 0;

  /**
   * Create a new XdmfItem.
   *
   * @param itemTag a string containing the tag of the XdmfItem to create.
   * @param itemProperties a map of key/value properties for the the XdmfItem.
   * @param childItems the children of the XdmfItem to create.
   *
   * @return constructed XdmfItem. If no XdmfItem can be constructed,
   * return NULL.
   */
  virtual shared_ptr<XdmfItem>
  createItem(const std::string & itemTag,
             const std::map<std::string, std::string> & itemProperties,
             const std::vector<shared_ptr<XdmfItem> > & childItems) const;

protected:

  XdmfCoreItemFactory();

private:

  XdmfCoreItemFactory(const XdmfCoreItemFactory &);  // Not implemented.
  void operator=(const XdmfCoreItemFactory &);  // Not implemented.

};

#endif /* XDMFCOREITEMFACTORY_HPP_ */
