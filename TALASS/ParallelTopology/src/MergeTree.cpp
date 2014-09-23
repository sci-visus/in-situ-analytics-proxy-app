/*
 * MergeTree.cpp
 *
 *  Created on: Feb 28, 2012
 *      Author: bremer5
 */

#include "MergeTree.h"
#include <iostream>
#include <algorithm>

using namespace FlexArray;


TreeNode::TreeNode(GlobalIndexType index, FunctionType f, 
                   MultiplicityType multiplicity)
    : MappedElement<GlobalIndexType,TreeNodeIndex>(index), 
      mValue(f),
      mMultiplicity(multiplicity),
      mFlag(false)
{
  mUp = NULL;
  mNext = NULL;
  mDown = NULL;
  mOutstanding = mMultiplicity;
}

TreeNode::TreeNode(GlobalIndexType index, FunctionType f, 
                   MultiplicityType multiplicity,
                   char boundaryLabel)
    : MappedElement<GlobalIndexType,TreeNodeIndex>(index), 
      mValue(f),
      mMultiplicity(multiplicity),
      mFlag(false),
      mBoundaryLabel(boundaryLabel)
{
  mUp = NULL;
  mNext = NULL;
  mDown = NULL;
  mOutstanding = mMultiplicity;
}


TreeNode::TreeNode(const Token::VertexToken& t)
    : MappedElement<GlobalIndexType,TreeNodeIndex>(t.index()),
      mValue(t.value()),
      mMultiplicity(t.multiplicity()),
      mOutstanding(t.outstanding()),
      mFlag(false)
{
  mUp = NULL;
  mNext = NULL;
  mDown = NULL;
}


TreeNode::TreeNode(const TreeNode& node)
    : MappedElement<GlobalIndexType,TreeNodeIndex>(node),
      mValue(node.mValue),
      mMultiplicity(node.mMultiplicity),
      mOutstanding(node.mOutstanding), 
      mFlag(node.mFlag)
{
  mUp = node.mUp;
  mNext = node.mNext;
  mDown = node.mDown;
}

TreeNode& TreeNode::operator=(const TreeNode& node)
{
  MappedElement<GlobalIndexType,TreeNodeIndex>::operator=(node);

  mUp = node.mUp;
  mNext = node.mNext;
  mDown = node.mDown;

  mValue = node.mValue;
  mMultiplicity = node.mMultiplicity;
  mOutstanding = node.mOutstanding;
  mFlag = node.mFlag;

  return *this;
}

void TreeNode::integrate(const Token::VertexToken& v)
{
  sterror(mMultiplicity!=v.multiplicity(),
          "Inconsistent stream multiplicity for vertex %d \
          found old %d  new %d",v.index(),mMultiplicity,v.multiplicity());

  // If this is an extra copy of a boundary min or a boundary saddle
  if (mMultiplicity == 1) {

    // And the number of outstanding vertices by one
    mOutstanding++;
    //mOutstanding--;
  }
  else {

    // The number of yet outstanding copies is
    // Total - [#copies1 + #copies2]
    // Total - [Total - #oustanding1 +  Total - #oustanding2]
    // #outstanding1 + #outstanding2 - Total
    mOutstanding = mOutstanding + v.outstanding()- mMultiplicity;
    //MultiplicityType copiesSeenByThis, copiesSeenByV, TotalRemaining;
    //copiesSeenByThis = 2*mMultiplicity - mOutstanding;
    //copiesSeenByV = 2*mMultiplicity - v.outstanding();
    //TotalRemaining = 2*mMultiplicity - (copiesSeenByThis + copiesSeenByV);
    //mOutstanding = TotalRemaining;
    //mOutstanding = mOutstanding + v.outstanding()- mMultiplicity;
    //mOutstanding--;
  }
}



TreeNode::operator const Token::VertexToken () const
{
  // Note the "+1" for outstanding. By making the node into another
  // token stream we "split" it again into a vertex and a finalization
  // token. This means we must increase the number of expected tokens
  // by one.

  return Token::VertexToken(id(),mValue,mMultiplicity,mOutstanding+1);
}

void TreeNode::down(TreeNode* newChild)
{
  //fprintf(stderr,"Adding edge %d -> %d \n",this->id(),newChild->id());
  if (mDown != NULL)
    mDown->removeUp(this);
 
  if (newChild != NULL)
    newChild->addUp(this);

  mDown = newChild;
}


void TreeNode::removeUp(TreeNode* v)
{
  sterror(mUp==NULL,"Up pointer to remove not found. No up-pointer exists.");

  TreeNode* it = v->mNext;

  // If v is the only up pointer of this
  if (it == v) {
    sterror(v!=mUp,"Tree pointers inconsistent.");
    mUp = NULL; // Remove it
    return;
  }

  // Now traverse the circular list until you are right before v
  while(it->mNext != v)
    it = it->mNext;

  // Shortcut the list
  it->mNext = it->mNext->mNext;

  // Relink v to itself
  v->mNext = v;

  // If we pointed to v as the head of the list
  if (mUp == v)
    mUp = it;
}

void TreeNode::addUp(TreeNode* v)
{
  sterror(v==NULL,"Cannot add NULL pointer to parent list.");
  //sterror(v->next ==v,"Incoming parent list should be trivial.");

  if (mUp == NULL)
    mUp = v;
  else {
    v->mNext = mUp->mNext;
    mUp->mNext = v;
  }
}

void TreeNode::bypass()
{
  sterror(up() == NULL,"Cannot bypass leaf node.");
  sterror(down() == NULL, "Cannot bypass root node.");

  TreeNode* it = up();

  do {
    it->mDown = mDown;
    it = it->next();
  } while (it != mUp);

  mDown->mUp = mUp;

  // Now we need to take care of our own next list
  if (mNext != this) { // if there is a nontrivial list
    // Find the guys ahead of us
    TreeNode* prev = mNext;

    while (prev->mNext != this)
      prev = prev->mNext;


    prev->mNext = mUp->mNext;
    mUp->mNext = mNext;
  }

  mUp = NULL;
  mDown = NULL;
  mNext = this;
}


void MergeTree::addNode(GlobalIndexType index, FunctionType value, 
                        MultiplicityType boundary_count)
{
  TreeNode node(index,value,boundary_count);

  addNode(node);
}


void MergeTree::addNode(GlobalIndexType index, FunctionType value, 
                        MultiplicityType boundary_count, 
                        char boundaryLabel)
{
  TreeNode node(index,value,boundary_count,boundaryLabel);

  addNode(node);
}

TreeNode* MergeTree::addNodeReturn(GlobalIndexType index, FunctionType value,
                                   MultiplicityType boundary_count)
{
  TreeNode* n;
  TreeNode node(index,value,boundary_count);

  n = addNodeToken(node);

  return n;
}

TreeNode* MergeTree::addNodeToken(const Token::VertexToken& token)
{
  TreeNode* n;

  // At the node to the array and the internal map
  n = insertElement(TreeNode(token));

  if (n != NULL)
    n->mNext = n;

  return n;  
}


void MergeTree::addNode(const Token::VertexToken& token)
{
  TreeNode* n;

  // At the node to the array and the internal map
  n = insertElement(TreeNode(token));

  if (n != NULL)
    n->mNext = n;
}


void MergeTree::addEdge(GlobalIndexType up, GlobalIndexType down)
{
  TreeNode* high = findElement(up);
  TreeNode* low = findElement(down);

  sterror((high == NULL) || (low == NULL),"Vertex of the edge not found");

  high->down(low);
}

void MergeTree::addEdge(TreeNode* high, TreeNode* low)
{
  sterror((high == NULL) || (low == NULL),"Vertex of the edge not found");

  high->down(low);
}

void MergeTree::outputToStream(TopoOutputStream* stream)
{

  MergeTree::iterator it;

  int count  =0 ;
  // For all still active node
  for (it=this->begin();it!=this->end();it++) {
     
    Token::VertexToken token;
    token = Token::VertexToken(*it);
    //token.outstanding(it->multiplicity()*2-1);
    token.outstanding(it->multiplicity());

    (*stream) << token;
    count++;
  }

  //std::cout << "COUNT :: " << count << " TREE ID:: " << this->id()<< std::endl;

  for (it = this->begin(); it != this->end(); it++) {
    if (it->down() != NULL) {
      Token::EdgeToken e(it->id(),it->down()->id());
      (*stream) << e;
    }
  }

  for (it = this->begin(); it != this->end(); it++)
    (*stream) << Token::FinalToken(it->id());


  (*stream) << Token::EmptyToken();

  stream->flush();

  // Output local trees for debugging
/*  
  char filename[32];
  sprintf(filename, "localtree_%d.dot", this->id());
  FILE* fp = fopen(filename, "w");
  this->writeToFile(fp);
  fclose(fp);
*/  
}

void MergeTree::outputBoundaryToStream(TopoOutputStream* stream)
{
  MergeTree::iterator it;

  /*char filename[32];
  sprintf(filename, "localtree_%d.dot", this->id());
  FILE* fp = fopen(filename, "w");
  this->writeToFile(fp);
  fclose(fp);
*/
  // Remove node if valence two
  for (it=this->begin();it!=this->end();) {

    //TreeNode* node = it->down();
    TreeNode* node = it;
    if (node != NULL) {
     if (isRegular(node) && node->multiplicity() == 1) {
       //std::cout << "Removing node.. Tree ID:: " << mTree->id() << std::endl;
       this->addRecord(node->up()->id(), node->id());

       it++;
       // Unlink it from the tree
       node->bypass();
    
       // And remove it from memory
       this->deleteElement(node);
 
     }
     else 
       it++;
    }
    else
      it++;
  }

  VertexValPair ver_val;

  // Mark all boundary nodes
  for (it=this->begin();it!=this->end();it++) {
    
    TreeNode* node = it;

    ver_val.idx = it->id();
    ver_val.val = it->value();
    SortedVertexValArray.push_back(ver_val);

    if (node->multiplicity() == 1)
      node->setOutstanding(0);
    
    if ((int8_t)node->multiplicity() > 1) {
      
      // traverse till root and mark all the nodes
      // if encounter a marked node we exit the loop
      do {
        if (node->flag() == false) {

          // mark as visited
          node->flag(true);

          // Send vertex downstream
          Token::VertexToken token(*node);
          //token.outstanding(node->multiplicity()*2-1);
          token.outstanding(node->multiplicity());
          (*stream) << token;
          //std::cout << "NODE:: " << node->id() 
          //      << " TREE :: " << this->id()
          //      << " Mult:: " << (int)node->multiplicity()
          //      << " out:: " << (int)node->outstanding()
          //      << std::endl;
        }
        //else 
         // break;

        node = node->down();        
      } while(node != NULL);
    }
  }

  std::sort(SortedVertexValArray.begin(), 
            SortedVertexValArray.end(), customLess);
  
  std::vector<VertexValPair>::iterator itArr;

  // Now mark all the parents of the marked nodes  
  // We need to send atleast one parent of a boundary max (if it exists)
  // This maintains correct merging behavior
  //for (it=this->begin();it!=this->end();it++) {
  for (itArr=SortedVertexValArray.begin(); 
       itArr!=SortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    TreeNode* node;
    node = this->findElement(itArr->idx);
  
    //TreeNode* node = it;

    // if marked then mark all parents
    if (node->flag()) { //&& (int8_t)node->multiplicity() > 1) {
      TreeNode* parent = node->up();

      if (parent != NULL) {
        do {
          
          // If unmarked, mark the parent and send downstream
          if (parent->flag() == false) {
            parent->flag(true);

            // Send parent downstream
            Token::VertexToken token(*parent);
            token.outstanding(parent->multiplicity());
            (*stream) << token;
            //std::cout << "pNODE:: " << parent->id() 
            //    << " TREE :: " << this->id()
            //    << " Mult:: " << (int)parent->multiplicity()
            //    << " out:: " << (int)node->outstanding()
            //    << std::endl;

          }
          parent = parent->next();
        } while(parent != node->up());
      }
    }
  }



  // Send the edges between visited nodes
  for (it = this->begin(); it != this->end(); it++) {
    if (it->down() != NULL && it->flag()) { 
      Token::EdgeToken e(it->id(),it->down()->id());
      (*stream) << e;
    }
  }

  // Send final tokens
  for (it = this->begin(); it != this->end(); it++) {
    if (it->flag())
      (*stream) << Token::FinalToken(it->id());
  }

  (*stream) << Token::EmptyToken();

  SortedVertexValArray.clear();

  stream->flush();

  // Output local trees for debugging
 
  /*if (this->id() == 38 || this->id() == 39 || this->id() == 57 || this->id() == 58) {
  char filename[32];
  sprintf(filename, "localtree_%d_1.dot", this->id());
  FILE* fp = fopen(filename, "w");
  this->writeToFile(fp);
  fclose(fp);
  }*/
}

void MergeTree::outputSortedBoundaryToStream(TopoOutputStream* stream)
{
  MergeTree::iterator it;

  /*char filename[32];
  sprintf(filename, "localtree_%d.dot", this->id());
  FILE* fp = fopen(filename, "w");
  this->writeToFile(fp);
  fclose(fp);
*/
  // Remove node if valence two
  for (it=this->begin();it!=this->end();) {

    //TreeNode* node = it->down();
    TreeNode* node = it;
    if (node != NULL) {
     if (isRegular(node) && node->multiplicity() == 1) {
       //std::cout << "Removing node.. Tree ID:: " << mTree->id() << std::endl;
       this->addRecord(node->up()->id(), node->id());

       it++;
       // Unlink it from the tree
       node->bypass();
    
       // And remove it from memory
       this->deleteElement(node);
 
     }
     else 
       it++;
    }
    else
      it++;
  }

  VertexValPair ver_val;

  // Mark all boundary nodes
  for (it=this->begin();it!=this->end();it++) {
    
    TreeNode* node = it;

    ver_val.idx = it->id();
    ver_val.val = it->value();
    SortedVertexValArray.push_back(ver_val);

    if (node->multiplicity() == 1)
      node->setOutstanding(0);
    
    if ((int8_t)node->multiplicity() > 1) {
      
      // traverse till root and mark all the nodes
      // if encounter a marked node we exit the loop
      do {
        if (node->flag() == false) {

          // mark as visited
          node->flag(true);

          // add node to priority queue
          this->NodePrQueue.push(node);          

          //std::cout << "NODE:: " << node->id() 
          //      << " TREE :: " << this->id()
          //      << " Mult:: " << (int)node->multiplicity()
          //      << " out:: " << (int)node->outstanding()
          //      << std::endl;
        }
        //else 
         // break;

        node = node->down();        
      } while(node != NULL);
    }
  }

  std::sort(SortedVertexValArray.begin(), 
            SortedVertexValArray.end(), customLess);
  
  std::vector<VertexValPair>::iterator itArr;

  // Now mark all the parents of the marked nodes  
  // We need to send atleast one parent of a boundary max (if it exists)
  // This maintains correct merging behavior
  //for (it=this->begin();it!=this->end();it++) {
  for (itArr=SortedVertexValArray.begin(); 
       itArr!=SortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    TreeNode* node;
    node = this->findElement(itArr->idx);
  
    //TreeNode* node = it;

    // if marked then mark all parents
    if (node->flag()) { //&& (int8_t)node->multiplicity() > 1) {
      TreeNode* parent = node->up();

      if (parent != NULL) {
        do {
          
          // If unmarked, mark the parent and send downstream
          if (parent->flag() == false) {
            parent->flag(true);

            // Send parent downstream
            // Add the parent to the prioirity queue
            this->NodePrQueue.push(parent);

            //std::cout << "pNODE:: " << parent->id() 
            //    << " TREE :: " << this->id()
            //    << " Mult:: " << (int)parent->multiplicity()
            //    << " out:: " << (int)node->outstanding()
            //    << std::endl;
          }
          parent = parent->next();
        } while(parent != node->up());
      }
    }
  }
  
  // We now send every vertex along with the edges to the output stream
  //for(int i=0; i<NodePrQueue.size(); i++) {
  while (!NodePrQueue.empty()) {
    TreeNode *node = NodePrQueue.top();

    // we first output the node as a vertex token
    Token::VertexToken token(*node);
    token.outstanding(node->multiplicity());
    (*stream) << token;
    
    //std::cout << "Tree Id:: " << this->id() 
    //          <<" node :: " << NodePrQueue.top()->id() << "\n";
    // Now we check if the parents are part of the boundary
    // If parents are part of boundary, we send the edges between the node and
    // the parents as edge tokens
    TreeNode *parent = node->up();
    if (parent != NULL) {
      do {
        // if parent is visited, its part of the boundary
        if (parent->flag()) {
          Token::EdgeToken e(parent->id(),node->id());
          (*stream) << e;
          //std::cout << "Tree Id:: " << this->id() 
          //          << " Edge :: " << parent->id() << " " 
          //          << node->id() << std::endl;
        }
        parent = parent->next();
      } while (parent != node->up());
    }
    NodePrQueue.pop();
  }
  
  (*stream) << Token::EmptyToken();

  SortedVertexValArray.clear();

  stream->flush();

  // Output local trees for debugging
 
  /*if (this->id() == 38 || this->id() == 39 || this->id() == 57 || this->id() == 58) {
  char filename[32];
  sprintf(filename, "localtree_%d_1.dot", this->id());
  FILE* fp = fopen(filename, "w");
  this->writeToFile(fp);
  fclose(fp);
  }*/
}

void MergeTree::writeToFileBoundary(FILE* fp) {
  MergeTree::iterator it;

  fprintf(fp, "digraph G {\n");
  for(it = this->begin(); it != this->end(); it++){
     TreeNode* node = it;

    if ((int)node->outstanding() > 0) {

      // Traverse from boundary node to root 
      // Add all unvisted nodes and edges to outputstream
      while (node != NULL) {

                
        // Check if visited
        if (node->flag() == false) {

          // Mark visited
          node->flag(true);
           
          if (node->down() != NULL /*&& it->outstanding()*/)
            fprintf(fp, "%d -> %d\n", node->id(), node->down()->id());

        }
        // Advance downwards
        node = node->down();
        
      }
    }
  }
  fprintf(fp, "}");
  //fclose(fp);

}

bool MergeTree::isRegular(TreeNode* v) const {
  if (v->down() == NULL) {
    if (v->id() == 82725090) std::cout << "Its Minima\n";
    return false;
  }

  if (v->up() == NULL) {
    if (v->id() == 82725090) std::cout << "Its Maxima\n";
    return false;
  }

  if (v->up()->next() != v->up()) {
    if (v->id() == 82725090) std::cout << "Its saddle\n";
    return false;
  }

  return true;
}




