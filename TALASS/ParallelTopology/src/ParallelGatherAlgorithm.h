/*
 * ParallelGatherAlgorithm.h
 *
 *  Created on: Mar 11, 2014
 *      Author: landge1
 */

#ifndef PARALLELGATHERALGORITHM_H_
#define PARALLELGATHERALGORITHM_H_

#include "GatherAlgorithm.h"
#include <iostream>
#include <map>

class ParallelGatherAlgorithm : public GatherAlgorithm
{
public:

  //! Default constructor
  ParallelGatherAlgorithm(bool invert = false) : GatherAlgorithm(invert) {}

  //! Destructor
  virtual ~ParallelGatherAlgorithm() {}

  //! Apply the algorithm
  virtual int apply(MergeTree& tree, SortedInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow);

  virtual int apply(MergeTree& tree, TopoInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow) {
    std::cout << "Cannot process TopoInputStreams.. \n";
    return -1;
  }

protected:

  //! The reference to the union tree
  MergeTree* mTree;

  //! The output stream
  TopoOutputStream* mDownstream;

  //! The vector of upstream trees for scattering
  TopoOutputStream* mUpstream;

  struct vertex_val_pair {
    GlobalIndexType idx;
    FunctionType val;
    TreeNode *node;
  };

  typedef struct vertex_val_pair VertexValPair;

  std::vector<VertexValPair> mSortedVertexValArray;

  struct custom_less{
    bool operator()(VertexValPair A, VertexValPair B)
    {
      return (A.val == B.val) ? A.idx > B.idx : A.val > B.val ;
    }
  }customLess;

  /********************************************************************
   ********************* Computation Interface *************************
   *******************************************************************/

  //! Return the child of v
  virtual TreeNode* getChild(TreeNode* v) const {return v->down();}

  //! Return the parent of v
  virtual TreeNode* getParent(TreeNode* v) const {return v->up();}

  //! Determine whether v is a regular node
  virtual bool isRegular(TreeNode* v) const;

  //! Set the child of v to down
  virtual void setChild(TreeNode* v, TreeNode* down) const
  {
    sterror(v==down,"Illegal child pointer, no loops allowed.");
    v->down(down);
  }

  //! Indicate whether v has been finalized
  /*! Determine whether the given tree node will be part of more edges.
   *  Note that each finalize call reduces the multiplicity and all input
   *  nodes are expected to be boundary nodes. As a result a node is
   *  finalizes if and only if its multiplicity has been reduce to 0.
   * @param v: The index of the node in question
   * @return true, if there are more edges coming; false otherwise
   */
  virtual bool isFinalized(TreeNode* v) const {return (v->outstanding() == 0);}


  /****************************************************************
   **************   Internal Parallel API ************************
   ****************************************************************/

  //! Add an edge
  virtual TreeNode* addEdge(TreeNode* high, TreeNode* low);

  //! Finalize the remaining vertices
  virtual int finalize(bool finalTree);

  /********************************************************************
   ********************* Subrountines          *************************
   *******************************************************************/

  //! Determine wether *u > *v
  bool greater(TreeNode u, TreeNode v) const {return Algorithm::greater(u, v);}

  //! Finds the set representative in the Union Find structure
  TreeNode* Find(TreeNode* node);
};

#endif /* PARALLELGATHERALGORITHM_H_ */
