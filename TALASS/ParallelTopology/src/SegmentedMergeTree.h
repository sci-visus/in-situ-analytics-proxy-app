/*
 * SegmentedMergeTree.h
 *
 *  Created on:
 *      Author: 
 */

#ifndef SEGMENTEDMERGETREE_H_
#define SEGMENTEDMERGETREE_H_

#include "MergeTree.h"
#include <vector>

class SegmentedMergeTree : public MergeTree
{

	//std::vector< std::vector<LocalIndexType> > segmentations;
	GlobalIndexType* segmentation;
	LocalIndexType seg_size;
public:

  //! Typedef to satisfy the compiler
	typedef FlexArray::BlockedArray<TreeNode,LocalIndexType> BlockedArray;

  SegmentedMergeTree(GraphID id, uint8_t block_bits=BlockedArray::sBlockBits) : MergeTree(id, block_bits) {
	this->segmentation = NULL;
	this->seg_size = 0;
  }

 virtual ~SegmentedMergeTree() {
	 delete[] this->segmentation;
 }


 void initializeSegmentation(LocalIndexType size) {
	 this->seg_size = size;
	 this->segmentation = new GlobalIndexType[size];
	 for (LocalIndexType i = 0; i < size; i++) this->segmentation[i] = LNULL;
 }

 // simply copy the segmentation id of root_index to that of curr_index
 void addToSegment(LocalIndexType curr_index, LocalIndexType root_index){
	 if (root_index == LNULL || this->segmentation[root_index] == LNULL) { 
		 printf("ERROR: add to segment of unsegmented root index\n");
		 return;
	 }
	 this->segmentation[curr_index] = this->segmentation[root_index];
 }
	//void addToSegment(LocalIndexType node_index, LocalIndexType curr_index) {
	//  LocalIndexType v = findElementIndex(node_index);
	//	segmentations[v].push_back(curr_index);
	//}

	
  virtual void addNode(LocalIndexType lindex, GlobalIndexType gindex, 
                       FunctionType value, MultiplicityType boundary_count,
                       char boundary_label) {
	  MergeTree::addNode(gindex, value, boundary_count, boundary_label);

	  this->segmentation[lindex] = gindex;
	  //printf("got here: %d, %d, %f, %d\n", (int) lindex, (int) gindex, (float) value, (int) boundary_count);
	  // Add a new segmentation (do we need to check that the order matches ?)
	  //segmentations.push_back(std::vector<LocalIndexType>());

	  // add a node to its own segmetnation
	  //segmentations.back().push_back(index);
  }

  uint32_t getSegmentationSize() {
      return (uint32_t)seg_size;
  }

  GlobalIndexType getSegmentationID(LocalIndexType lindex) {
      return segmentation[lindex];
  }

  void setSegmentationID(LocalIndexType lindex, GlobalIndexType segID) {
      segmentation[lindex] = segID;
  }


};


#endif /* SEGMENTEDUNIONTREE_H_ */
