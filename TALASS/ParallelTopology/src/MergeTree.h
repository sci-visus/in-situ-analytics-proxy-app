#ifndef MERGE_TREE
#define MERGE_TREE

#include <cstdlib>
#include <iostream>
#include <queue>
#include "DistributedDefinitions.h"
#include "MappedElement.h"
#include "MappedArray.h"
#include "Token.h"
#include "TopoOutputStream.h"


typedef uint32_t TreeNodeIndex;
const TreeNodeIndex TNULL = (TreeNodeIndex)-1;

class TreeNode : public FlexArray::MappedElement<GlobalIndexType,TreeNodeIndex> {
public:

  friend class FlexArray::MappedArray<TreeNode,GlobalIndexType,TreeNodeIndex>;
  friend class FlexArray::BlockedArray<TreeNode,LocalIndexType>;
  friend class FlexArray::Array<TreeNode,LocalIndexType>;
  friend class MergeTree;

  //! Default constructor
  TreeNode(LocalIndexType index, FunctionType f, 
           MultiplicityType multiplicity, char boundary_label);

  TreeNode(LocalIndexType index, FunctionType f, 
           MultiplicityType multiplicity );

  //! Construct a node from a token
  explicit TreeNode(const Token::VertexToken& token);

  //! Copy constructor
  TreeNode(const TreeNode& node);

  //! Private constructor
  TreeNode() {}

   //! Destructor
  ~TreeNode() {}

  //! Assignment operator
  TreeNode& operator=(const TreeNode& node);

  //! Convert the node into a vertex token
  /*! Convert the node into a vertex token. Note that this function
   *  assumes that for each VertexToken there will be a FinalToken
   *  and thus passes on a copies values of mCopies-1.
   */
  operator const Token::VertexToken() const;

  //! Return the function value
  FunctionType value() const {return mValue;}

  //! Return the number of outstanding finalizations
  MultiplicityType outstanding() const {return mOutstanding;}

  MultiplicityType multiplicity() const {return mMultiplicity;}

  //! Increase the finalize count
  void finalize() {mOutstanding--;}

  void setOutstanding(MultiplicityType outstanding) {
    mOutstanding = outstanding;
  }

  char boundaryLabel() const {return mBoundaryLabel;}

  //! Integrate the information of the given token to this node
  void integrate(const Token::VertexToken& v);

  //! A generic flag used for traversals
  bool flag() const {return mFlag;}

  void flag(bool f) {mFlag = f;}

  bool isNonLocal() const {return mNonLocal;}

  void nonLocal(bool f) {mNonLocal = f;}

  bool isSaddle() const {return mIsSaddle;}

  void isSaddle(bool f) {mIsSaddle = f;}

  //! Return the down pointer
  TreeNode* down() {return mDown;}

  //! Return the down pointer
  const TreeNode* down() const {return mDown;}

  //! Return the up pointer
  TreeNode* up() {return mUp;}

  //! Return the up pointer
  const TreeNode* up() const {return mUp;}

  //! Return the next pointer
  TreeNode* next() {return mNext;}

  void setNext() {mNext=this;}
  void setUp(TreeNode* v) {mUp=v; }
  void setDown(TreeNode* v) {mDown=v; }

  //! Return the next pointer
  const TreeNode* next() const {return mNext;}

  //! Set the down pointer.
  /*! Set the down pointer. Note that this is by design the *only* way to
   *  to connect nodes. This call will automatically disconnect and reconnect
   *  the appropriate up and down pointers. Furthermore, it will enforce the
   *  structure of the merge tree, ie if this node already has a child it
   *  will be disconnected as no node can have two children
   * @param newChild the new down pointer
   */
  void down(TreeNode* newChild);

  //! Bypass this vertex
  void bypass();

  //! Return the set representative for the UF structure
  TreeNode* setRep() { return mSetRep;}

  //! Set the set representative
  void setRep(TreeNode* node) { mSetRep = node; }

private:

  //! Pointer to the parent node
  TreeNode* mUp;

  //! Pointer to one of my siblings.
  TreeNode* mNext;

  //! Pointer to the child node
  TreeNode* mDown;

  //! The function value
  FunctionType mValue;

  //! The multiplicity Count
  MultiplicityType mMultiplicity;

  //! The number of finalizations we are still expecting
  MultiplicityType mOutstanding;

  char mBoundaryLabel;

  //! The set representative
  TreeNode *mSetRep;

  //! A generic flag for traversals
  bool mFlag;

  bool mIsSaddle ;
  bool mNonLocal;

  //! Internal call to remove an up pointer from the list
  void removeUp(TreeNode* v);

  //! Internal call to add an up pointer to the list
  void addUp(TreeNode* v);
};

class MergeTree : public FlexArray::MappedArray<TreeNode,GlobalIndexType,
                                                TreeNodeIndex> {
public:

  //! Typedef to satisfy the compiler
  typedef FlexArray::BlockedArray<TreeNode,LocalIndexType> BlockedArray;

  //! Constructor
  MergeTree(GraphID id, uint8_t block_bits=BlockedArray::sBlockBits)
    : FlexArray::MappedArray<TreeNode,GlobalIndexType,TreeNodeIndex>(block_bits),
      mId(id) {}

  //! Destructor
  ~MergeTree() {}

  //! Return id
  GraphID id() const {return mId;}
  
  struct vertex_val_pair { 
    GlobalIndexType idx;
    FunctionType val;
  };

  typedef struct vertex_val_pair VertexValPair;

  std::vector<VertexValPair> SortedVertexValArray;

  struct node_compare{
    bool operator()(TreeNode* nodeA, TreeNode* nodeB) {
      return (nodeA->value() == nodeB->value()) ? nodeA->id() < nodeB->id() :
                                              nodeA->value() < nodeB->value();
    }
  }nodeCompare;
  
  std::priority_queue< TreeNode*, std::vector<TreeNode*>, node_compare> NodePrQueue;

  //! Add a node to the tree
  virtual void addNode(GlobalIndexType index, FunctionType value, 
                       MultiplicityType boundary_count);

  virtual void addNode(GlobalIndexType index, FunctionType value, 
                       MultiplicityType boundary_count,
                       char boundary_label);

  virtual TreeNode* addNodeReturn(GlobalIndexType index, FunctionType value,
                                  MultiplicityType boundary_count);

  //! Add a node to the tree
  virtual void addNode(const Token::VertexToken& token);
  virtual TreeNode* addNodeToken(const Token::VertexToken& token);


  virtual void finalizeNode(GlobalIndexType index) {}

  //! Add an edge to the tree (note that the nodes are expected to be ordered
  virtual void addEdge(GlobalIndexType upper, GlobalIndexType lower);

  virtual void addEdge(TreeNode* upper, TreeNode* lower);
  //! Get the treenode at id
  TreeNode* getTreeNode(GlobalIndexType id) { return this->findElement(id);}

  bool isRegular(TreeNode* v) const;

  virtual void outputToStream(TopoOutputStream* stream);
  
  virtual void outputBoundaryToStream(TopoOutputStream* stream);
  
  void outputSortedBoundaryToStream(TopoOutputStream* stream);

  void writeToFileBoundary(FILE* fp);

  void writeToFileBinary(FILE* fp) {
    MergeTree::iterator it;
    
    // We store the end vertices of every edge
    //std::vector<GlobalIndexType> TreeEdges;
    std::vector<VertexValPair> TreeEdges;
    int count=0;

    for(it = this->begin(); it != this->end(); it++){
      if (it->id() == 587416) { 
        std::cout << "Its there!!!"<< it->value() << " \n";
        if (it->down() != NULL) 
          std::cout << "Down : " <<it->down()->id() << "\n"; 
        if (it->up() != NULL) 
          std::cout << "Up : " <<it->up()->id() << "\n"; 
      }
      if (it->down() != NULL){// && it->down()->down()!=NULL) {
        VertexValPair v1, v2;
        v1.idx = it->id();
        v1.val = it->value();
        
        v2.idx = it->down()->id();
        v2.val = it->down()->value();

        TreeEdges.push_back(v1);
        TreeEdges.push_back(v2);

        fwrite(&v1, sizeof(VertexValPair), 1, fp);
        fwrite(&v2, sizeof(VertexValPair), 1, fp);
        //TreeEdges.push_back(it->id());
        //TreeEdges.push_back(it->down()->id());

        if(it->up() != NULL && (it->up()->next() == it->up()))
          std::cout << "Tree ID:: " << this->id() 
                    << " Warning!! : Valence 2 node detected:: " << it->id() 
                    << " outstanding:: " << (int)it->outstanding() << "\n";
      }
      else if (it->up() == NULL && it->down() == NULL) {
        VertexValPair v1;
        v1.idx = it->id();
        v1.val = it->value();
        fwrite(&v1, sizeof(VertexValPair), 1, fp);
        fwrite(&v1, sizeof(VertexValPair), 1, fp);
      }
      //else if (it->up()!=NULL && it->down() == NULL)
      //   count++;
      //else 
      //   std::cout << "Some Other\n";
      }

    //std::cout << "Minima:: " << count << "\n";
    VertexValPair *treeBuffer = &TreeEdges[0];
    //GlobalIndexType *treeBuffer = &TreeEdges[0];

    //fwrite(treeBuffer, sizeof(GlobalIndexType), TreeEdges.size(), fp);
    //fwrite(treeBuffer, sizeof(VertexValPair), TreeEdges.size(), fp);

  
  }

  void writeToFile(FILE* fp){

    MergeTree::iterator it;

    fprintf(fp, "digraph G {\n");
    for(it = this->begin(); it != this->end(); it++){
     
     //if (it->id() == 0)
        //std::cout << "Val:: " << it->value() << " UP: " <<  std::endl;
        //std::cout << "Outst:: " << (int)it->outstanding() <<  std::endl;

      if (it->down() != NULL /*&& it->outstanding()*/)
        fprintf(fp, "%d, %f -> %d, %f\n", it->id(), it->value(), 
                                          it->down()->id(), it->value());
      if (it->up() == NULL && it->down() == NULL)
        fprintf(fp, "Ind:: %d, %f \n", it->id(), it->value()); 
    }

    fprintf(fp, "}");
    //fclose(fp);
  }

  void addRecord(GraphID parent, GraphID child) {
    nodeRecord[child] = parent;
  }

  GraphID getParentFromRecord(GraphID child) {
    GraphID parentID = -1;
    std::map< GraphID, GraphID> :: iterator it;
   
    it = nodeRecord.find(child);
    
    while (it!=nodeRecord.end()) {
      parentID = it->second;
      it = nodeRecord.find(parentID);
    }

    if (parentID == -1)
      std::cout<< "Error :: Parent record missing!! Child:: "<< child <<"\n";
    else
      nodeRecord[child] = parentID;

    return parentID;
  }


  //bool vertexCompare(VertexValPair A, VertexValPair B);

  struct custom_less{
    bool operator()(VertexValPair A, VertexValPair B)
    {   
      return (A.val == B.val) ? A.idx > B.idx : A.val > B.val ;
    }   
  }customLess;

  VertexValPair minimaVertex;  

protected:

  //! The global id of this tree
  const GraphID mId;

  //! Record to maintain history of nodes being removed from tree
  std::map< GraphID, GraphID > nodeRecord;
};


#endif
