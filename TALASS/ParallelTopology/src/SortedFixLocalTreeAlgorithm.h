/*
 * SortedFixLocalTreeAlgorithm.h
 *
 *  Created on: March 20, 2014
 *      Author: landge1
 */


#ifndef SORTEDFIXLOCALTREEALGORITHM_H_
#define SORTEDFIXLOCALTREEALGORITHM_H_

#include "ScatterAlgorithm.h"

class SortedFixLocalTreeAlgorithm : public ScatterAlgorithm
{
public:

  //! Default constructor
  SortedFixLocalTreeAlgorithm(bool invert = false) : ScatterAlgorithm(invert) {}

  //! Destructor
  virtual ~SortedFixLocalTreeAlgorithm() {}

  //! Return the child of v
  virtual TreeNode* getChild(TreeNode* v) const {return v->down();}

  //! Return the parent of v
  virtual TreeNode* getParent(TreeNode* v) const {return v->up();}

  //! Determine whether v is a regular node
  //virtual bool isRegular(TreeNode* v) const;

  //! Set the child of v to down
  virtual void setChild(TreeNode* v, TreeNode* down) const
  {
    //std::cout << "Parent : " << v->id() << "  Child: " << down->id() <<
    //std::endl;
    sterror(v==down,"Illegal child pointer, no loops allowed ID :: %d", v->id());
    v->down(down);
  }


  //! Apply the algorithm
  virtual int apply(MergeTree& tree, TopoInputStream* inputs,
                                    TopoOutputStream* outputs) { return 1;}

  virtual int apply(MergeTree& tree, SortedInputStream* inputs,
                                    TopoOutputStream* outputs);

  //! Construct the Augmented Merge Tree from tokens received 
  void constructAMtFromTokens(MergeTree* AMt, SortedInputStream* inputs);

  bool isMaxOrSaddle(TreeNode* node);

  //! Check if two identical nodes in different trees have the same parents
  void checkParents(TreeNode* NodeLT, TreeNode* NodeAMt, MergeTree* AMt);

  bool checkDescendant(TreeNode* newParentFromAMt, TreeNode* oldParentFromLT, 
                     TreeNode* endNode, MergeTree* AMt );

  void UpdateBoundaryParent(TreeNode* nodeLT, MergeTree* AMt);

  //! if we find to merge branches
  TreeNode* addEdge(TreeNode* head,TreeNode* tail);

  TreeNode* findIntegrationVertex(TreeNode* high, TreeNode* low);

  virtual bool isFinalized(TreeNode* v) const {return (v->outstanding() == 0);}

  TreeNode* mergeBranches(TreeNode* left, TreeNode* right);

  //! Finalize a node
  virtual int finalizeVertex(TreeNode* vertex);

  //! Determine whether v is a regular node
  virtual bool isRegular(TreeNode* v) const;
  
  bool isNonLocalLeafNode(TreeNode* node);
  
  bool isNonLocalMinima(MergeTree *LTree, TreeNode* node);
 
private:

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


  // The local tree
  MergeTree* mLTree;
  
  // The augmented merge tree
  MergeTree* mAMtree;
};

#endif /* SORTEDFIXLOCALTREEALGORITHM_H_ */
