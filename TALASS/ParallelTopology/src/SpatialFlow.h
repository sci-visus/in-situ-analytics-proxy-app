/*
 * SpatialFlow.h
 *
 *  Created on: Feb 27, 2014
 *      Author: landge1
 */

#ifndef SPATIALFLOW_H_
#define SPATIALFLOW_H_

#include <vector>
#include <map> 
#include "ControlFlow.h"

/*! A partial specialization of a ControlFlow based on hierarchical
 *  decomposition of the domain. The flow created takes into account the spatial
 *  neighbors and creates a tree which merges blocks along the largest boundary
 *  between them.
 *
 *  The above is achieved by first representing the spatial decomposition in the
 *  form of a binary tree where each node encapsulates the sub-domains
 *  represented by its left and right child. The root represents the entire
 *  domain. Since we are creating a merge heirarchy for the local trees of each
 *  sub-block, the space is defined in terms of the process decomposition of the volume
 *  i.e. the space dimensions are numProcessX*numProcessY*numProcessZ. 
 *
 *  We construct the above binary tree and then create a merge heirarchy with
 *  the help of this tree based on the merge factor provided by the user
*/

//! Structure representing the spatial co-ordinates of a sub-domain block
struct BlockIdx_t {
  uint32_t x;
  uint32_t y;
  uint32_t z;
};
typedef struct BlockIdx_t BlockIdx;

//! Structure for the dimensions of a sub-domain
struct Dimension_t {
  uint32_t x;
  uint32_t y;
  uint32_t z;
};
typedef struct Dimension_t Dimension;

/*! Data structure to hold the binary decomposition of the domain. We basically
 *  create a binary tree that splits the domain into two halves along the longest
 *  dimension. The following structure is a node in such a tree. It contains the
 *  BlockIdx of the bottom left corner of the sub-domain it encapsulates as well
 *  as the dimensions of the domain. It also contains the pointer to the parent
 *  and left and right child
*/
typedef struct SpatialTreeNode_t SpatialTreeNode;

struct SpatialTreeNode_t  {

  BlockIdx minBlock;
  Dimension dim;
  uint32_t nodeID;
  bool flag;

  SpatialTreeNode *parent;
  SpatialTreeNode *right;
  SpatialTreeNode *left;
} ;


class SpatialFlow : public ControlFlow
{
public:

  //! Default constructor
  SpatialFlow(uint32_t xDim, uint32_t yDim, uint32_t zDim, uint32_t factor,
              bool invert);

  //! Destructor
  ~SpatialFlow() {}

  //! Return the number of elements in the first level
  uint32_t baseCount() const {return mBaseCount;}
  
  //! Return the number of levels in the tree
  uint32_t numLevels() const {return mNumLevels;}

  //! Return the overal number of element
  uint32_t size() const {return mSizeSpatialTree;} 

  uint32_t factor() const {return mFactor;};

  //! Return the children of the tree
  std::vector<uint32_t> sinks(uint32_t id) const;

  //! Return the parent
  std::vector<uint32_t> sources(uint32_t id) const;

  //! Return the children of the tree
  //std::vector<uint32_t> sinks(uint32_t id) ;

  //! Return the parent
  //std::vector<uint32_t> sources(uint32_t id) ;

  
  virtual uint32_t level(uint32_t id) const {return 1;}

  uint32_t getBlockId(uint32_t nodeId);

  void printSinksSources();
protected:

  //! The modulo factor used
  uint32_t mFactor;

  uint32_t mBaseCount;

  uint32_t mSizeSpatialTree;

  uint32_t mNumLevels;

  //! The flag to determing reverse spatial flow
  bool mInvert;

  //! Dimension of the process decomposition
  Dimension mDim;

  //! The number of elements per level
  std::vector<uint32_t> mLevelCount;

  //! The aggregate count of elements of previous levels
  std::vector<uint32_t> mGroups;

  //! Create a spatial flow tree based on the process decomposition of the
  //  domain.
  //  Input Params: 
  //    start_idx, end_idx :These are the start number and end number on this
  //                        level
  //    xDim, yDim, zDim:  :The process decomposition of the domain i.e. number of
  //                        processes in x, y and z direction
  //    factor:            :the merge factor
  //    propagation_dir    :The direction in which to perform the merge in
  //                        this level
  //void createSpatialFlowTreeLevel(uint32_t start_idx, uint32_t end_idx, 
  //                                uint32_t xDim, uint32_t yDim, uint32_t zDim,
  //                                uint32_t factor, uint32_t propagation_dir);

  //! Pointer to root of spatial tree
  SpatialTreeNode* mRoot;

  std::map<uint32_t, SpatialTreeNode*> mNodeMap;

  std::map<uint32_t, std::vector<uint32_t>* > mSinksMap;
  std::map<uint32_t, std::vector<uint32_t>* > mSourcesMap;

  //! method ot create spatial decomposition of the process blocks
  SpatialTreeNode* createSpatialDecomposition(SpatialTreeNode *parent_node,
                                              BlockIdx blockId, uint32_t xDim,
                                              uint32_t yDim, uint32_t zDim);

  void createMergeHierarchy();
  void traverseTree(SpatialTreeNode* node);

  //! Function to return the internal level
  //uint32_t treeLevel(uint32_t id) const;
};


#endif /* SPATIALFLOW_H_ */
