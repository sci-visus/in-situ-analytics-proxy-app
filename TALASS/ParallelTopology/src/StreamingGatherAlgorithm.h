/*
 * StreamingGatherAlgorithm.h
 *
 *  Created on: Feb 21, 2012
 *      Author: bremer5
 */

#ifndef STREAMINGGATHERALGORITHM_H_
#define STREAMINGGATHERALGORITHM_H_

#include "GatherAlgorithm.h"
#include <iostream>

class StreamingGatherAlgorithm : public GatherAlgorithm
{
public:

  //! Default constructor
  StreamingGatherAlgorithm(bool invert = false) : GatherAlgorithm(invert) {}

  //! Destructor
  virtual ~StreamingGatherAlgorithm() {}

  //! Apply the algorithm
  virtual int apply(MergeTree& tree, TopoInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow);

  virtual int apply(MergeTree& tree, SortedInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow) {
    std::cout << "Unable to process SortedInputStreams.. \n";
    return -1;
  }

protected:

  //! The reference to the union tree
  MergeTree* mTree;

  //! The output stream
  TopoOutputStream* mDownstream;

  //! The vector of upstream trees for scattering
  TopoOutputStream* mUpstream;

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
   **************   Internal Streaming API ************************
   ****************************************************************/

  //! Add an edge
  virtual TreeNode* addEdge(TreeNode* high, TreeNode* low);

  //! Finalize a node
  virtual int finalizeVertex(TreeNode* vertex);

  //! Finalize the remaining vertices
  virtual int finalize(bool finalTree);

  /********************************************************************
   ********************* Subrountines          *************************
   *******************************************************************/

  //! Determine wether *u > *v
  bool greater(TreeNode u, TreeNode v) const {return Algorithm::greater(u, v);}

  //! Find the lowest child of high that is >= low
  virtual TreeNode* findIntegrationVertex(TreeNode* high, TreeNode *low);

  //! Attach branch starting at head to the branch ending at tail
  /*! This call assumes that getChild(tail) == NULL and attaches the
   *  branch starting at head to the branch ending at tail. If tail
   *  was finalized the function will return tail to re-evaluate its
   *  type otherwise it return NULL.
   */
  virtual TreeNode* attachBranch(TreeNode* tail, TreeNode* head);

  //! Merge the two branches starting at left and right respectively
  /*! This call merge-sorts the two paths starting at left and right
   *  respectively into a single branch. The initial assumption is
   *  that left > right. If the merging causes a finalized critial
   *  point to potentially change its type a pointer to this vertex is
   *  returned. Otherwise, the function returns NULL.
   */
  virtual TreeNode* mergeBranches(TreeNode* left, TreeNode* right);

  //! Output a vertex
  virtual void outputVertex(const Token::VertexToken& token) const;

  //! Output an edge
  virtual void outputEdge(GlobalIndexType v0, GlobalIndexType v1) const;

  //! Output tree
  virtual void outputTree(TreeNodeIndex root) const;
};


#endif /* STREAMINGGATHERALGORITHM_H_ */
