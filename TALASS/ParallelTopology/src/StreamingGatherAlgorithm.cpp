/*
 * StreamingGatherAlgorithm.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: bremer5
 */

#include <iostream>
#include <algorithm>
#include "StreamingGatherAlgorithm.h"
#include "Token.h"

int StreamingGatherAlgorithm::apply(MergeTree& tree, TopoInputStream* inputs,
                                    TopoOutputStream* downstream,
                                    TopoOutputStream* upstream,
                                    const ControlFlow* gather_flow)
{
  // First we initialize some internal pointers for convinience
  mTree = &tree;
  sterror(mTree == NULL,"No union tree given");

  mDownstream = downstream;
  sterror((mDownstream==NULL)
          && (gather_flow->sinks(mTree->id()).size() > 0),
          "No downstream output given but we expect sinks");

  mUpstream = upstream;

  TopoInputStream::iterator it;

  //std::cout << "OUTSTANDING:: "<< inputs->outstanding();
  //std::cout << "Streaming Gather Called()\n" ;

  for (it=inputs->begin();it!=inputs->end();it++) {

    switch (it.type()) {

      // If one of the trees is done. We just ignore the token since
      // the input stream will naturally take care of the outstanding
      // EMPTY tokens
      case Token::EMPTY:
        //std::cout << "TREE ID :: " << mTree->id() << "  EMPTY \n";
        break;


      case Token::VERTEX: {
        const Token::VertexToken& v = it;
        TreeNode* node;
        
        //fprintf(stderr,"VERTEX %d multiplicity %d oustanding %d\n",v.index(),
        //      v.multiplicity(),v.outstanding());

        node = mTree->findElement(v.index());
        // If this node is not yet part of the tree
        if (node == NULL) {

          // Add it to the tree
          mTree->addNode(v);
        }
        else {
          // Otherwise, join it with the existing node
          node->integrate(v);
        }
        
        //if (v.value() < 0.0) 
        //{
        //  std::cout << "Tree ID: " << mTree->id() 
        //            << " Node: " << node->id() << "\n";
        //}
        break;
      }

      case Token::EDGE: {
        const Token::EdgeToken& e = it;
        //std::cout << "Edge\n";

        //if(mTree->id() == 6) {
        //fprintf(stderr,"EDGE %d %d\n",e[0],e[1]);
        //break;
        //}

        TreeNode* high = mTree->findElement(e[0]);
        TreeNode* low = mTree->findElement(e[1]);

        sterror(high == NULL,"Inconsistent stream, could not \
                              find vertex %d of edge [%d,%d]",e[0],e[0],e[1]);
        sterror(low == NULL,"Inconsistent stream, could not \
                             find vertex %d of edge [%d,%d]",e[1],e[0],e[1]);

        TreeNode* highVertex = addEdge(high, low);

        //! If we found a previously finalized vertex
        // Aaditya: We do not finalize here
        //if (highVertex != NULL)
          //finalizeVertex(highVertex); // We must re-finalize it

        break;
      }

      case Token::FINAL: {
        //if(mTree->id() == 6) {
        //std::cout << "FINAL\n";
        //break;
        //}
        const Token::FinalToken& f = it;

        TreeNode* v = mTree->findElement(f.index());
        
        sterror(v==NULL,"Inconsistent stream could not find \
                         vertex %d to finalize.",f.index());
        
        v->finalize();

        //if (v->outstanding() == 0)
          //finalizeVertex(v);

        break;
      }

      case Token::SEG:
        break;
      default:
        sterror(true,"Found undefined token");
        break;
    }
  }

  // Output intermediate merge trees for debugging
/*  
  char filename[32];
  sprintf(filename, "localtree_up_%d.dot", mTree->id());
  FILE* fp = fopen(filename, "w");
  mTree->writeToFile(fp);
  fclose(fp);
*/

  // Check if this is the final tree to be computed
  if (mTree->id() == (gather_flow->size()-1))
    finalize(true); // We clean-up
  else
    finalize(false);
/*
  char filename1[32];
  sprintf(filename1, "localtree_down_%d.dot", mTree->id());
  FILE* fp1 = fopen(filename1, "w");
  mTree->writeToFileBoundary(fp1);
  fclose(fp1);
*/
  return 1;
}

bool StreamingGatherAlgorithm::isRegular(TreeNode* v) const {
  if (getChild(v) == NULL)
    return false;

  if (getParent(v) == NULL)
    return false;

  if (getParent(v)->next() != getParent(v))
    return false;

  return true;
}

TreeNode* StreamingGatherAlgorithm::addEdge(TreeNode* head,TreeNode* tail)
{
  if (greater(*tail, *head))
    std::swap(head, tail);

  head = findIntegrationVertex(head, tail);

  if (head == tail) { // If this edge already exists
    return NULL; // There is nothing to d
  } else if (getChild(head) == NULL) {// If even the root of the tree 
    // is higher than tail. Attach the tail branch to the head branch
    return attachBranch(head, tail);
  } else {
    // Otherwise, we need to merge two path
    return mergeBranches(head, tail);
  }
}

int StreamingGatherAlgorithm::finalizeVertex(TreeNode* v) {
  // If the node is now regular
  if (isRegular(v)) {

    // Unlink it from the tree
    v->bypass();

    // And remove it from memory
    mTree->deleteElement(v);
    return 1;
  }

  return 0;
}

int StreamingGatherAlgorithm::finalize(bool finalTree) {
  
  MergeTree::iterator it;

  // If this is the last tree, we do not send it downstream
  if(finalTree) {
    // For all still active node
    for (it=mTree->begin();it!=mTree->end();it++) {

      // For all roots
      //if (it->down != NULL)
      Token::VertexToken token(*it);
      token.outstanding(it->outstanding()+1);
      //std::cout << "Iterator::Value:: " << token.value() << std::endl;
      (*mUpstream) << Token::VertexToken(*it);
    }

    for (it = mTree->begin(); it != mTree->end(); it++) {
      if (it->down() != NULL) {
        Token::EdgeToken e(it->id(),it->down()->id());

        //fprintf(stderr,"Edge %d %d\n",e[0],e[1]);
        (*mUpstream) << e;
      }
    }

    for (it = mTree->begin(); it != mTree->end(); it++) {
      (*mUpstream) << Token::FinalToken(it->id());
    }

    (*mUpstream) << Token::EmptyToken();

    mUpstream->flush();
    return 1;
  }

  // First send augmented merge tree upstream as it also contains valence two
  // nodes. So, we go over the tree once and send those first. We remove these
  // nodes while sending them downstream
 
  //std::cout << "TREE ID :: " <<mTree->id() << "  Sending Upstream!!\n";
  for (it=mTree->begin();it!=mTree->end();it++) {
    //if (mTree->id() == 5 || mTree->id() == 6) { 
    Token::VertexToken token(*it);
    token.outstanding(it->outstanding()+1);
    (*mUpstream) << Token::VertexToken(*it);
  }

   // Send the edges between visited nodes
  for (it = mTree->begin(); it != mTree->end(); it++) {
    if (it->down() != NULL) { 
      Token::EdgeToken e(it->id(),it->down()->id());
      (*mUpstream) << e;
    }
  }

  // Send final tokens
  for (it = mTree->begin(); it != mTree->end(); it++) 
    (*mUpstream) << Token::FinalToken(it->id());

  (*mUpstream) << Token::EmptyToken();
   mUpstream->flush();

  // Now send the boundary nodes along with their children downstream
  // We remove valence two nodes in the process


  // Remove node if valence two
  for (it=mTree->begin();it!=mTree->end();) {

    //TreeNode* node = it->down();
    TreeNode* node = it;
    if (node != NULL) {
     if (isRegular(node) && node->outstanding()<=0) {
       //std::cout << "Removing node.. Tree ID:: " << mTree->id() << std::endl;
       mTree->addRecord(getParent(node)->id(), node->id());

       it++;
       // Unlink it from the tree
       node->bypass();
    
       // And remove it from memory
       mTree->deleteElement(node);
     }
     else 
       it++;
    }
    else
      it++;
  }

  // First mark all outstanding nodes and all the nodes from them to the root

  MergeTree::VertexValPair ver_val;
  mTree->SortedVertexValArray.clear();

  for (it=mTree->begin();it!=mTree->end();it++) {
    
    TreeNode* node = it;

    ver_val.idx = it->id();
    ver_val.val = it->value();
    mTree->SortedVertexValArray.push_back(ver_val);

    if ((int8_t)node->outstanding() > 0) {
      
      // traverse till root and mark all the nodes
      // if encounter a marked node we exit the loop
      do {
        if (node->flag() == false) {

          // mark as visited
          node->flag(true);

          // Send vertex downstream
          Token::VertexToken token(*node);
          token.outstanding(node->outstanding()+1);
          (*mDownstream) << Token::VertexToken(*node);
        }
        else 
          break;

        node = node->down();        
      } while(node != NULL);
    }
  }

  std::sort(mTree->SortedVertexValArray.begin(), 
            mTree->SortedVertexValArray.end(), mTree->customLess);
  
  std::vector<MergeTree::VertexValPair>::iterator itArr;

  // Now mark all the parents of the marked nodes  
  //for (it=mTree->begin();it!=mTree->end();it++) {
  for (itArr=mTree->SortedVertexValArray.begin(); 
       itArr!=mTree->SortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    TreeNode* node;
    node = mTree->findElement(itArr->idx);

    //TreeNode* node = it;

    // if marked then mark all parents
    if (node->flag()) {
      TreeNode* parent = node->up();

      if (parent != NULL) {
        do {
          
          // If unmarked, mark the parent and send downstream
          if (parent->flag() == false) {
            parent->flag(true);

            // Send parent downstream
            Token::VertexToken token(*parent);
            token.outstanding(parent->outstanding()+1);
            (*mDownstream) << Token::VertexToken(*parent);

          }
          parent = parent->next();
        } while(parent != node->up());
      }
    }
  }

  // Send the edges between visited nodes
  for (it = mTree->begin(); it != mTree->end(); it++) {
    if (it->down() != NULL) { 
      if (it->flag()) {
        Token::EdgeToken e(it->id(),it->down()->id());
        (*mDownstream) << e;
      }
    }
  }

  // Send final tokens
  for (it = mTree->begin(); it != mTree->end(); it++) {
    if (it->flag())
      (*mDownstream) << Token::FinalToken(it->id());

    it->flag(false);
  }

  mTree->SortedVertexValArray.clear();
  //std::cout << "\nSent Downstream : Tree ID : " << mTree->id() << std::endl;

  (*mDownstream) << Token::EmptyToken(); 
  mDownstream->flush();
  return 1;
}

TreeNode* StreamingGatherAlgorithm::findIntegrationVertex(TreeNode* high, 
                                                          TreeNode* low)
{
  // Search for the lowest child of high that is still above low
  while ((getChild(high) != NULL) && !greater(*(low), *getChild(high))) {
    high = getChild(high);
  }

  return high;
}

TreeNode* StreamingGatherAlgorithm::attachBranch(TreeNode* tail, 
                                                 TreeNode* head) 
{
  sterror(getChild(tail)!=NULL,"You can only attach to disjoined branches.");

  setChild(tail, head); // tail becomes the new root
  
  if (isFinalized(tail)) // If head was finalized earlier
    return tail;// We must re-evaluate its type
  else
    return NULL;
}

TreeNode* StreamingGatherAlgorithm::mergeBranches(TreeNode* left,
                                                  TreeNode* right) 
{
  sterror(!greater(*left,*right),"Assumption violated.\
                                 Left should be > right.");

  TreeNode* next_right;
  TreeNode* next_left;

  // While the two paths have not merged
  while ((left != right) && (getChild(left) != NULL)) {

    // We need to splice right between left and child(left)

    // Store the next vertex on the right path
    next_right = getChild(right);

    // To make things slightly faster we use a tmp variable
    next_left = getChild(left);
    setChild(left,right);
    setChild(right,next_left);

    // Now we determine whether we may have changed a finalized type

    // If we have reached the end of the right path
    if (next_right == NULL) {
      if (isFinalized(right)) // And it previous root was finalized
        return right; // It just changed to become a regular vertex
      else
        return NULL; // Otherwise we simply stop here
    }

    // If left used to be the root of its tree
    if (next_left == NULL) {
      if (isFinalized(left)) // and it was finalized
        return left; // It just changed to become a regular vertex
      else
        return NULL; // We are done anyway
    }

    // If both paths continue we reset the pointers

    left = right;
    right = next_right;

    // Advance the left path until you find the next integration
    // point. In case a derived class wants to use this version of
    // mergeBranches we make sure we stay within our class hierarchy
    left = StreamingGatherAlgorithm::findIntegrationVertex(left, right);
  }

  // If we get here this means that left == right != NULL
  sterror(left==NULL,"Two paths should not merge at NULL.");

  // If we broke the traversal because there only remains an attaching
  // to do
  if (left != right) {

    setChild(left, right); // tail becomes the new root

    if (isFinalized(left)) // If head was finalized earlier it was a root
      return left;// We must re-evaluate its type (it became regular)
    else
      return NULL;
  }

  // If the saddle at which we merge has been finalized it may have
  // changed type
  if ((left == right) && isFinalized(left))
    return left; // The saddle must be fixed

  return NULL;
}

void StreamingGatherAlgorithm::outputVertex(const Token::VertexToken& token) const
{
  std::vector<TopoOutputStream*>::const_iterator it;

  (*mDownstream) << token;
  //(*mUpstream) << token;
}

void StreamingGatherAlgorithm::outputEdge(GlobalIndexType v0, 
                                          GlobalIndexType v1) const
{
  std::vector<TopoOutputStream*>::const_iterator it;
  Token::EdgeToken token;

  token.source(v0);
  token.destination(v1);

  (*mDownstream) << token;
  //(*mUpstream) << token;
}

void StreamingGatherAlgorithm::outputTree(TreeNodeIndex root) const
{/* THis is defunct ... not clear exactly what it was to do and where
  if ((*mTree)[root].down() == NULL) {
    Token::VertexToken token;

    token.index((*mTree)[root].id());
    token.multiplicity((*mTree)[root].multiplicity());
    token.value((*mTree)[root].value());

    //(*mDownstream) << token;
    //(*mDownstream) << (*mTree)[root];
    //(*mUpstream) << (*mTree)[root];
  }
  else {

    TreeNodeIndex down = ((*mTree)[root].down())->id();
    Token::EdgeToken token;

    token.source(root);
    token.destination(down);

    (*mDownstream) << token;
  }
  */
  /*do {
   outputTree(up);

   if (up == ((*mTree)[root].up)->id())
   //(*mUpstream) << (*mTree)[root]; // UP or down ?

   token.source(up);
   token.destination(root);

   (*mDownstream) << token;
   //(*mUpstream) << token;


   up = ((*mTree)[up].next)->id();
   } while (up != ((*mTree)[root].up)->id());
   }
   */
  //Token::FinalToken final;

  //final.index(root);

  //(*mDownstream) << final;
  //(*mUpstream) << final;
}

