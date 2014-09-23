/* 
 * SpatialFlow.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: landge1
 *
*/

#include <vector>
#include <iostream>
#include "SpatialFlow.h"

#define PARTITION_X 0
#define PARTITION_Y 1
#define PARTITION_Z 2

SpatialFlow::SpatialFlow(uint32_t x_dim, uint32_t y_dim, uint32_t z_dim, 
                         uint32_t factor, bool invert = false) {
  mFactor = factor;
  mBaseCount = x_dim*y_dim*z_dim;
  mInvert = invert;
  mDim.x = x_dim;
  mDim.y = y_dim;
  mDim.z = z_dim;

  BlockIdx block_id;

  block_id.x = 0;
  block_id.y = 0;
  block_id.z = 0;
  mRoot = createSpatialDecomposition(NULL, block_id, x_dim, y_dim, z_dim);

  mSizeSpatialTree = mRoot->nodeID + 1;

  //traverseTree(mRoot);
  createMergeHierarchy();
  //printSinksSources();
}

void SpatialFlow::traverseTree(SpatialTreeNode* node) {

  if (node == NULL) return;

  std::cout << "Block ID: [" << node->minBlock.x << " "
            << node->minBlock.y << " " << node->minBlock.z 
            << "] Dim: " << node->dim.x << " " << node->dim.y 
            << " " << node->dim.z << " Tree ID: " << node->nodeID ;
  if (node->parent)
  std::cout << " Parent:: " << node->parent->nodeID << "\n";

  traverseTree(node->left);
  traverseTree(node->right);
}

SpatialTreeNode* SpatialFlow::createSpatialDecomposition(
                                                  SpatialTreeNode *parent_node,
                                                  BlockIdx block_id,
                                                  uint32_t x_dim, 
                                                  uint32_t y_dim, 
                                                  uint32_t z_dim) {
  SpatialTreeNode* node = new SpatialTreeNode;
  node->minBlock = block_id;
  node->dim.x = x_dim;
  node->dim.y = y_dim;
  node->dim.z = z_dim;

  node->parent = parent_node;
  node->flag = false;

  uint32_t max_dim = x_dim;
  char flag = PARTITION_X;
  static uint32_t id = mDim.x*mDim.y*mDim.z;

  if (max_dim < y_dim) {
    max_dim = y_dim;
    flag = PARTITION_Y;
  }

  if (max_dim < z_dim) {
    max_dim = z_dim;
    flag = PARTITION_Z;
  }
  
  if (max_dim == 1) {
    node->left = NULL;
    node->right = NULL;
    node->nodeID = node->minBlock.z * mDim.y * mDim.x + 
                   node->minBlock.y * mDim.x + node->minBlock.x;
    mNodeMap[node->nodeID] = node;
    //std::cout << " Node ID:: " << node->nodeID << "\n";
    //          << " Parent ID:: " << node->parent->nodeID << "\n";
    return node;
  }

  switch(flag){

    case PARTITION_X :
    {
      node->left = createSpatialDecomposition(node, block_id, 
                                              x_dim/2, y_dim, z_dim); 
      block_id.x += ((x_dim)/2) ? ((x_dim)/2) : 1;
      node->right = createSpatialDecomposition(node, block_id, x_dim - x_dim/2, 
                                               y_dim, z_dim);
      break;
    }

    case PARTITION_Y :
    {
      node->left = createSpatialDecomposition(node, block_id, x_dim, 
                                              y_dim/2, z_dim); 
      block_id.y += ((y_dim)/2) ? ((y_dim)/2) : 1;
      node->right = createSpatialDecomposition(node, block_id, x_dim, 
                                               y_dim - y_dim/2, z_dim);
      break;
    }

    case PARTITION_Z :
    {
      node->left = createSpatialDecomposition(node, block_id, x_dim, 
                                              y_dim, z_dim/2); 
      block_id.z += ((z_dim)/2) ? ((z_dim)/2) : 1;
      node->right = createSpatialDecomposition(node, block_id, x_dim, 
                                               y_dim, z_dim - z_dim/2);
      break;
    }

    default :
    {
      std::cout << "Error in spatial decomposition!!\n\n";
    }
  }

  node->nodeID = id++;
  mNodeMap[node->nodeID] = node;
  return node;
}


// We use the spatial tree to create the merge hierarchy. Since the spatial 
// tree is binary in nature, we can construct the merge heirarchy with merge
// factors 2, 4 or 8 by just deleting levels in the spatial tree based on the
// merge factor
void SpatialFlow::createMergeHierarchy() {
  
  SpatialTreeNode *node, *leaf;
  int num_skip_levels = 1; //mFactor/2; // fix with power of 2 using bit ops
  
  if (mFactor == 4) num_skip_levels = 2;
  if (mFactor == 8) num_skip_levels = 3;

  std::map<uint32_t, std::vector<uint32_t>* >::iterator it_sources_map;

  mNumLevels = 1;
  uint32_t num_levels = 1;
  // for every node in spatial tree compute its sinks and sources

  //for (int i=0; i<mSizeSpatialTree-1 ; i++) {
  for (int i=0; i<mBaseCount ; i++) {
    
    leaf = mNodeMap[i];
    node = leaf;

    while (node->parent != NULL && node->flag == false){
      //std::cout << "tree:: " << node->nodeID ; 
      //if (node->parent != NULL)
        //std::cout << " Parent:: " << node->parent->nodeID << "\n";

      // compute sink 
      // Sink would just be the parent depending on merge factor 
      for (int j=0; j < num_skip_levels; j++) {

        if (node->parent == NULL) 
          break;

        node = node->parent;
      }

      std::vector<uint32_t> *sinks = new std::vector<uint32_t>;
      sinks->push_back(node->nodeID);
      mSinksMap[leaf->nodeID] = sinks;

      // compute sources
      // if sources vector already created for the above sink
      it_sources_map = mSourcesMap.find(node->nodeID);
      std::vector<uint32_t> *sources;
  
      if (it_sources_map != mSourcesMap.end()) 
        sources = mSourcesMap[node->nodeID];
      else {
        sources = new std::vector<uint32_t>;
        mSourcesMap[node->nodeID] = sources;
      }
   
      // add the current node of spatial tree as source
      sources->push_back(leaf->nodeID);
      leaf->flag = true;
      leaf = node;
      num_levels++;
    }
    if (num_levels > mNumLevels) 
      mNumLevels = num_levels;

    num_levels = 0;
  }
}

std::vector<uint32_t> SpatialFlow::sinks(uint32_t id) const
{
  if (mInvert) return sources(id);

  if (id == mSizeSpatialTree-1)
    return std::vector<uint32_t>();
  
  std::map<const uint32_t, std::vector<uint32_t>* >::const_iterator it_sinks_map;
  
  it_sinks_map = mSinksMap.find(id);
  
  return *it_sinks_map->second;
}

std::vector<uint32_t> SpatialFlow::sources(uint32_t id) const
{
  if (mInvert) return sinks(id);

  if (id < mBaseCount)
    return std::vector<uint32_t>();
  
  std::map<const uint32_t, std::vector<uint32_t>* >::const_iterator it_sources_map;
  
  it_sources_map = mSourcesMap.find(id);
  
  return *it_sources_map->second;

}

uint32_t SpatialFlow::getBlockId(uint32_t nodeId) {
  
  SpatialTreeNode *node = mNodeMap[nodeId];

  return node->minBlock.z * mDim.y * mDim.x + node->minBlock.y * mDim.x + 
         node->minBlock.x;

}

void SpatialFlow::printSinksSources() {

  std::vector<uint32_t> sinks;
  std::vector<uint32_t> sources;

  std::vector<uint32_t>::iterator it;

  for (int i=0; i<mSizeSpatialTree-1; i++) {
    sinks = this->sinks(i);
    sources = this->sources(i);

    std::cout << "Sink of " << i << " :: ";
    for (int j=0; j<sinks.size(); j++) {
      std::cout << sinks[j] << " ";
    }
    std::cout << "\n";

    std::cout << "Sources of "  << i <<  " :: ";
    for (int j=0; j<sources.size(); j++) {
      std::cout << sources[j] << " ";
    }
    std::cout << "\n";
  }
}



