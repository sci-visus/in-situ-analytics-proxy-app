/*
 * ParallelGatherAlgorithm.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: bremer5
 */

#include <iostream>
#include <algorithm>
#include "ParallelGatherAlgorithm.h"
#include "Token.h"

int ParallelGatherAlgorithm::apply(MergeTree& tree, SortedInputStream* inputs,
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
  SortedInputStream::iterator it;

  it=inputs->begin();
  it++; // we increment iterator to remove first token to get duplicated
  TreeNode *last_seen_node = NULL;
  uint32_t last_seen_count = 0;
  
  while (it.type() != EMPTY) {

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
        
        //fprintf(stderr,"VERTEX %d multiplicity %d oustanding %d tree_id %d\n",
        //        v.index(), v.multiplicity(),v.outstanding(),mTree->id());

        if (last_seen_node && last_seen_node->id() == v.index()) {
          node = last_seen_node;
          last_seen_count++;
        }
        else
          node = mTree->findElement(v.index());
        
        // If this node is not yet part of the tree
        if (node == NULL) {

          // Add it to the tree
          node = mTree->addNodeToken(v);
          node->setRep(node);
          // This finalize is added to offset the FINAL token count
          node->finalize(); 
        }
        else {
          // Otherwise, join it with the existing node
          node->integrate(v);
          // This finalize is added to offset the FINAL token count
          node->finalize();
        }
        last_seen_node = node;
        //  std::cout << "Tree ID: " << mTree->id() 
        //            << " Node: " << node->id() << "\n";
        break;
      }

      case Token::EDGE: {
        const Token::EdgeToken& e = it;

        //if(mTree->id() == 6) {
        //fprintf(stderr,"EDGE %d %d tree ID %d\n",e[0],e[1],mTree->id());
        //break;
        //}

        TreeNode* high = mTree->findElement(e[0]);
        TreeNode* low;
        if (last_seen_node->id() == e[1]) {
          low = last_seen_node;
          last_seen_count++;
        }
        else
          low = mTree->findElement(e[1]);

        sterror(high == NULL,"Inconsistent stream, could not \
                              find vertex %d of edge [%d,%d]",e[0],e[0],e[1]);
        sterror(low == NULL,"Inconsistent stream, could not \
                             find vertex %d of edge [%d,%d]",e[1],e[0],e[1]);

        TreeNode* highVertex = addEdge(high, low);
        break;
      }

      default:
        sterror(true,"Found undefined token");
        break;
    }
    it++;
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
  if (mTree->id() == (gather_flow->size()-1)) {
    finalize(true); // We clean-up
    //std::cout << "Last seen count:: " << last_seen_count << "\n";
  }
  else
    finalize(false);
/*
  char filename1[32];
  sprintf(filename1, "localtree_down_%d.dot", mTree->id());
  FILE* fp1 = fopen(filename1, "w");
  mTree->writeToFile(fp1);
  fclose(fp1);
*/
  return 1;
}

bool ParallelGatherAlgorithm::isRegular(TreeNode* v) const {
  if (getChild(v) == NULL)
    return false;

  if (getParent(v) == NULL)
    return false;

  if (getParent(v)->next() != getParent(v))
    return false;

  return true;
}

TreeNode* ParallelGatherAlgorithm::addEdge(TreeNode* head,TreeNode* tail)
{
  if (greater(*tail, *head))
    std::swap(head, tail);

  TreeNode *set_rep_head;
  TreeNode *set_rep_tail;

  // find head
  set_rep_head = Find(head);
  //std::cout << "Node :: " << head->id() 
  //          << " Set Rep:: " << set_rep_head->id() << "\n";
  // find tail
  set_rep_tail = Find(tail);
  //std::cout << "Node :: " << tail->id() 
  //          << " Set Rep:: " << set_rep_tail->id() << "\n";

  // If head != tail -> union
  if (set_rep_head != set_rep_tail) {
    setChild(set_rep_head, set_rep_tail);
    set_rep_head->setRep(set_rep_tail);
  }
  return NULL;
  // else do nothing -> edge/path already exists
}

TreeNode* ParallelGatherAlgorithm::Find(TreeNode* node) {
  TreeNode *set_rep;
  set_rep = node->setRep();

  while (set_rep->id() != set_rep->setRep()->id()) {
    set_rep = set_rep->setRep();
  }

  node->setRep(set_rep);
  return set_rep;
}

int ParallelGatherAlgorithm::finalize(bool finalTree) {
  
  // First send augmented merge tree upstream as it also contains valence two
  // nodes. So, we go over the tree once and send those first. We remove these
  // nodes while sending them downstream
 
  //std::cout << "TREE ID :: " <<mTree->id() << "  Sending Upstream!!\n";
  
  // Push all nodes in the priority queue
  MergeTree::iterator it;
  for (it=mTree->begin();it!=mTree->end();it++) {
    mTree->NodePrQueue.push(it);
  }

  // Now send all nodes along with edges in sorted order using the priority
  // queue
  while (!mTree->NodePrQueue.empty()) {
    TreeNode *node = mTree->NodePrQueue.top();

    // we first output the node as a vertex token
    Token::VertexToken token(*node);
    token.outstanding(node->outstanding()+1);
    (*mUpstream) << token;

    // Now we check if the parents are part of the boundary
    // If parents are part of boundary, we send the edges between the
    // node and  the parents as edge tokens
    TreeNode *parent = node->up();
    if (parent != NULL) {
      do {
        Token::EdgeToken e(parent->id(),node->id());
        (*mUpstream) << e;
        parent = parent->next();
      } while (parent != node->up());
    }
    //std::cout << "node :: " << NodePrQueue.top()->id() << "\n";
    //node->flag(false);
    //(*mUpstream) << Token::FinalToken(node->id());
    mTree->NodePrQueue.pop();
  }

  (*mUpstream) << Token::EmptyToken();
   mUpstream->flush();

  if (!mTree->NodePrQueue.empty())
    std::cout << "Q NOT EMPTY!!\n\n";

  // If this is the last tree, we do not send it downstream
  if(finalTree) {
    return 1;
  }
  
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

  VertexValPair ver_val;
  mSortedVertexValArray.clear();

  for (it=mTree->begin();it!=mTree->end();it++) {
    
    TreeNode* node = it;

    ver_val.idx = it->id();
    ver_val.val = it->value();
    ver_val.node = it;
    mSortedVertexValArray.push_back(ver_val);

    if ((int8_t)node->outstanding() > 0) {
      
      // traverse till root and mark all the nodes
      // if encounter a marked node we exit the loop
      do {
        if (node->flag() == false) {

          // mark as visited
          node->flag(true);

          mTree->NodePrQueue.push(node);
        }
        else 
          break;

        node = node->down();        
      } while(node != NULL);
    }
  }

  std::sort(mSortedVertexValArray.begin(), 
            mSortedVertexValArray.end(), customLess);
  
  std::vector<VertexValPair>::iterator itArr;

  // Now mark all the parents of the marked nodes  
  for (itArr=mSortedVertexValArray.begin(); 
       itArr!=mSortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    TreeNode* node;// = itArr;
    node = itArr->node;

    // if marked then mark all parents
    if (node->flag()) {
      TreeNode* parent = node->up();

      if (parent != NULL) {
        do {
          
          // If unmarked, mark the parent and send downstream
          if (parent->flag() == false) {
            parent->flag(true);
            mTree->NodePrQueue.push(parent);
          }
          parent = parent->next();
        } while(parent != node->up());
      }
    }
  }

  // We now send every vertex along with the edges to the output stream
  while (!mTree->NodePrQueue.empty()) {
    TreeNode *node = mTree->NodePrQueue.top();

    // we first output the node as a vertex token
    Token::VertexToken token(*node);
    token.outstanding(node->outstanding()+1);
    (*mDownstream) << token;

    // Now we check if the parents are part of the boundary
    // If parents are part of boundary, we send the edges between the
    // node and  the parents as edge tokens
    TreeNode *parent = node->up();
    if (parent != NULL) {
      do {
        // if parent is visited, its part of the boundary
        if (parent->flag()) {
          Token::EdgeToken e(parent->id(),node->id());
          (*mDownstream) << e;
        }
        parent = parent->next();
      } while (parent != node->up());
    }
    //std::cout << "node :: " << NodePrQueue.top()->id() << "\n";
    //node->flag(false);
    mTree->NodePrQueue.pop();
  }

  mSortedVertexValArray.clear();
  //std::cout << "\nSent Downstream : Tree ID : " << mTree->id() << std::endl;

  (*mDownstream) << Token::EmptyToken(); 
  mDownstream->flush();
  return 1;
}

