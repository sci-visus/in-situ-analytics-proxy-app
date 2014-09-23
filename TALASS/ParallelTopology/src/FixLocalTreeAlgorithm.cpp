/*
 * FixLocalTreeAlgorithm.cpp
 *
 *  Created on: Feb 22, 2013
 *      Author: landge1
 */

#include <iostream>
#include <algorithm>
#include "FixLocalTreeAlgorithm.h"
#include "Token.h"

  int newParentsCount = 0; 
void FixLocalTreeAlgorithm::constructAMtFromTokens(MergeTree* AMt, 
                                                   TopoInputStream* inputs)
{ 
  MergeTree::iterator itAMt;
  TopoInputStream::iterator it;

  int k = 0;
  for (it=inputs->begin();it!=inputs->end();it++) {
    
    k++;
    switch(it.type()) {
      case Token::EMPTY: {
        //std::cout << "EMPTY \n";
        //break;
        return;
        }

      case Token::VERTEX: {
        const Token::VertexToken& v = it;
        //std::cout << "VERTEX :: " << v.index() << std::endl;
        AMt->addNode(v);
        break;
        }

      case Token::EDGE: {
        const Token::EdgeToken& e = it;
        //std::cout << "Edge\n";
         AMt->addEdge(e[0], e[1]); // (high, low)
        break;
        }

      case Token::FINAL: {
        //std::cout << "FINAL\n";
        const Token::FinalToken& f = it;
        TreeNode* v = AMt->findElement(f.index());
                
        sterror(v==NULL,"Inconsistent stream could not \
                        find vertex %d to finalize.",f.index());
        //std::cout << "Vertex:: " << v->id() 
        //            << " Outstanding:: "  << (int)v->outstanding() 
        //            << " Mult:: " << (int)v->multiplicity() 
        //            << std::endl;
        v->finalize();
        //std::cout << "Vertex:: " << v->id() 
        //            << " Outstanding:: "  << (int)v->outstanding() 
        //            << " Mult:: " << (int)v->multiplicity() 
        //            << std::endl;
        break;
        }

      case Token::SEG: 
        //std::cout << "FINAL\n";
        break;
      default:
        //std::cout << "FINAL\n";
        sterror(true,"Found undefined token");
        break;
     } 
  }
}

bool FixLocalTreeAlgorithm::isNonLocalLeafNode(TreeNode* node) {
  if (node->up() == NULL && node->isNonLocal()) {
    //std::cout << "Non Local Leaf:: " << node->id()
    //          << " Tree:: " << mLTree->id()
    //          << std::endl;
    return true;
  }
  else
    return false;
}

bool FixLocalTreeAlgorithm::isNonLocalMinima(MergeTree *LTree,
                                             TreeNode* node) {
  if (node->down() == NULL && node->isNonLocal()) { 
    return (LTree->minimaVertex.val == node->value()) ? 
              LTree->minimaVertex.idx >= node->id() : 
              LTree->minimaVertex.val > node->value() ; 
  }
  else
    return false;
}

int FixLocalTreeAlgorithm::apply(MergeTree& tree, TopoInputStream* inputs,
                                 TopoOutputStream* outputs)
{

  MergeTree* AMt = new MergeTree(0);
  mAMtree = AMt;

  // Construct AMt from the Tokens sent upstream
  constructAMtFromTokens(AMt, inputs);

  mLTree = &tree;

  MergeTree::iterator itLT;
  MergeTree::iterator itAMt;

  // Used for tree traversals
  TreeNode* nodeAMt;
  TreeNode* nodeLT;
  TreeNode* newNode=NULL; 

  VertexValPair ver_val;
  static int mTreeNum;
  //int countAMt = 0;
  //for(itAMt=mAMtree->begin(); itAMt != mAMtree->end(); itAMt++) {
  //  countAMt++;
 // }

  // We mark all nodes as not visited
  int countLT = 0;
  for(itLT=mLTree->begin(); itLT != mLTree->end(); itLT++) {
    countLT++;
    itLT->flag(false);
    // We populate the vertex value vector in this loop
    ver_val.idx = itLT->id();
    ver_val.val = itLT->value();
    SortedVertexValArray.push_back(ver_val);
  }

  //if (mLTree->id() == 38) {
  //std::cout << " Number AMt nodes:: " << countAMt 
  //          << " Number LT nodes :: " << countLT << std::endl;
  //}
  // Now we sort the nodes based on function value
  std::sort(SortedVertexValArray.begin(), 
            SortedVertexValArray.end(), customLess);
  
  //if (mLTree->id() == 28) {
  if (mTreeNum == 0) {
    if (!SortedVertexValArray.empty()) {
      //std::cout << "minina :: " << SortedVertexValArray.back().idx
      //          << " Vaal:: " << SortedVertexValArray.back().val << std::endl;
      mLTree->minimaVertex.idx = SortedVertexValArray.back().idx;
      mLTree->minimaVertex.val = SortedVertexValArray.back().val;
    }
  }

  std::vector<VertexValPair>::iterator itArr;

  // Output intermediate merge trees for debugging
  //if (mLTree->id() == 24 || mLTree->id() == 28) {
  /*char filename[32];
  sprintf(filename, "AMtree_%d_%d.dot", mLTree->id(), mTreeNum);
  FILE* fp = fopen(filename, "w");
  AMt->writeToFile(fp);

  sprintf(filename, "LTree_%d_%d.dot", mLTree->id(), mTreeNum);
  fp = fopen(filename, "w");
  mLTree->writeToFile(fp);
*/
//}

  int newNodeCount = 0;  
  newParentsCount = 0;
  int MinNodeCount = 0;
  // Now we loop all the nodes in the LTree
  for (itArr=SortedVertexValArray.begin(); 
       itArr!=SortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    nodeLT = mLTree->findElement(itArr->idx);
    sterror(nodeLT==NULL, "Index %d must be present in LTree", 
            (int)itArr->idx);

    // if node is not visited
    if (nodeLT->flag() == false) {
      // Idea is to start traversal from one of the following nodes
      // if node is not finalized
      // or node is a non-local leaf node
      if (nodeLT->outstanding() > 0 || (isNonLocalLeafNode(nodeLT))) {

        // mark as visited
        nodeLT->flag(true);
        //std::cout << " Vertex ID:: " << nodeLT->id() 
        //          << " Tree:: " << mLTree->id()
        //          << " Out: " << (int)nodeLT->outstanding()
        //          << std::endl;

        // find the node in AMTree
        nodeAMt = AMt->findElement(nodeLT->id());

        // if node in AMt there are two cases:
        // 1. node is outstanding
        // 2. node is on boundary tree
        // else 
        // 1. node is finalized and is on a finalized branch
        if (nodeAMt) {

          // Now traverse both trees and make changes in LTree 
          // by using AMt as reference
          //std::cout << "FOUND MATCH :: Vertex ID:: " << nodeAMt->id() 
          //          <<std::endl;

          // Mark visited
          nodeAMt->flag(true);
          nodeLT->setOutstanding(nodeAMt->outstanding());
            
          // Check if node has become saddle by checking the parents
          checkParents(nodeLT, nodeAMt, AMt);
           
          // Now traverse both trees till we reach local minima
          while (nodeLT->down() != NULL && nodeAMt->down() != NULL) {

            if (nodeAMt->down()->flag() == true && 
                nodeLT->down()->flag() == true &&
                nodeAMt->down()->id() == nodeLT->down()->id()) {
              //std::cout << "\nBreaking at node :: " << nodeLT->id() 
              //          << " Visited next node :: " << nodeLT->down()->id()
              //          << std::endl;
              nodeLT->flag(true);
              nodeAMt->flag(true);
              if (nodeLT->id() == nodeAMt->id()) {
                // Check if node has become saddle by checking the parents
                checkParents(nodeLT, nodeAMt, AMt);
                nodeLT->setOutstanding(nodeAMt->outstanding());
              }

              //std::cout << "Current Node:: "<< nodeLT->id()  
              //          << " Down:: " << nodeLT->down()->id()  
              //          << " TREE ID:: " << mLTree->id() << std::endl;

              break;
            }
             
            //std::cout << "Traversing now...\n";
            //std::cout << "Current Node:: "<< nodeLT->id() 
            //          << " Down:: " << nodeLT->down()->id() 
            //          << " TREE ID:: " << mLTree->id() << std::endl;
          
            // If nodes are identical we check their parents
            if (nodeLT->id() == nodeAMt->id()) {
              // Check if node has become saddle by checking the parents
              checkParents(nodeLT, nodeAMt, AMt);
              nodeLT->setOutstanding(nodeAMt->outstanding());
            }

            if (nodeLT->down()->id() != nodeAMt->down()->id()) {
            
              //std::cout << "###NON MATCHING CHILD!!!\n";
              //std::cout << "Current Node:: "<< nodeLT->id()  
              //          << " New Child:: "<< nodeAMt->down()->id() 
              //          << " Old Child:: "<< nodeLT->down()->id()  
              //          << " TREE ID:: " << mLTree->id() << std::endl;

              // Check if node already exists in LTree. 
              // If it does we have to perform a merge

              newNode = mLTree->findElement(nodeAMt->down()->id());

              if (newNode != NULL) {
                //std::cout << "Node exists in tree...TREE ID:: " << mLTree->id()
                //          << std::endl;
                if(addEdge(nodeLT, newNode)) {
                   //std::cout << "\nAdding edge!! Node :: "
                   //          << nodeLT->id() 
                   //          << " NewNode :: "
                   //          << newNode->id()
                   //          << std::endl;
                }
                newNode->setOutstanding(nodeAMt->down()->outstanding());
                checkParents(newNode, nodeAMt->down(), AMt);
              }
              else {

                // Check if we have reached point where rest 
                // of nodes in AMt are less than Lt
                if (greater(*nodeLT->down(), *nodeAMt->down())) {
                  //std::cout << "Lower vertex encountered at :: " << nodeLT->id()
                  //          << " TREE ID:: " << mLTree->id() << "\n";

                  nodeLT->flag(true);
                  nodeLT = nodeLT->down();
                  continue;
                }

                newNode = mLTree->addNodeReturn(nodeAMt->down()->id(), 
                                               nodeAMt->down()->value(), 
                                               nodeAMt->down()->multiplicity());

                newNode->setOutstanding(nodeAMt->down()->outstanding());
                newNode->nonLocal(true);
                newNodeCount++; 
                //std::cout << "Adding Child: "<< nodeAMt->down()->id() 
                //          << " Parent :: " << nodeLT->id()  
                //          << " Old Child:: " << nodeLT->down()->id() 
                //          << std::endl;
                
                // Add new child
                setChild(newNode, nodeLT->down());

                // Reconnect with Tree
                setChild(nodeLT, newNode);

                //std::cout << "Added Child: "<< nodeLT->down()->id() 
                //          << " Parent :: " << nodeLT->id() 
                //          << " Old Child:: " << nodeLT->down()->down()->id() 
                //          << std::endl;
              }
            }

            nodeLT->flag(true);
            nodeAMt->flag(true);
            nodeAMt = nodeAMt->down();
            nodeLT = nodeLT->down();
          }
            
          // We have reached local minima but need to attach an outstanding
          // non local node at the bottom. This helps in constructing the global 
          // tree later.
          // But somtimes we may reach the minima of a tree in the forest if 
          // thresholding is enabled. If this minima is not the minima of the 
          // forest we need to add nodes at the bottom as trees in the forest
          // can merge.     
          if (nodeLT->down() == NULL && nodeAMt->down() != NULL) {
            if (nodeLT->id() == nodeAMt->id()) {
              // Check if node has become saddle by checking the parents
              checkParents(nodeLT, nodeAMt, AMt);
              nodeLT->setOutstanding(nodeAMt->outstanding());
            }
            if (isNonLocalMinima(mLTree, nodeLT)) {
              // if we are already at non local minima, we just check it is 
              // critical in AMt. If its still critical we do nothing and break
              // the loop. Else we replace the minima with a critical node from
              // AMt just below the local minima and break the loop.    
              if ((nodeAMt->outstanding() == 0) && !(isMaxOrSaddle(nodeAMt))) {
                // We find new critical node in AMt below to add as new minima
                // As current minima has become regular
                nodeAMt = nodeAMt->down();  
                while (nodeAMt != NULL) {
                  if (nodeAMt->outstanding() != 0 || isMaxOrSaddle(nodeAMt) || 
                      nodeAMt->down() == NULL ) {
                    newNode = mLTree->addNodeReturn(nodeAMt->id(),
                                     nodeAMt->value(), nodeAMt->multiplicity());
                    //std::cout << "Swapping Node at Bottom :: " << nodeAMt->id()
                    //          << " To Be Parent:: " << nodeLT->id()   
                    //          << " TREE ID:: " << mLTree->id() << "\n";

                    MinNodeCount++;
                    newNode->setOutstanding(nodeAMt->outstanding());
                    setChild(nodeLT, newNode);

                    nodeLT = newNode; 
                    nodeLT->nonLocal(true);
                    nodeLT->flag(true);
                    checkParents(nodeLT, nodeAMt, AMt);
                    break;
                  }
                  nodeAMt = nodeAMt->down();
                }
              } 
            }
            else {
              nodeAMt = nodeAMt->down();
              while (nodeAMt != NULL ){
                // We have not reached the minima so we add the critical nodes
                // from AMt till we reach minima
                if (nodeAMt->outstanding() != 0 || isMaxOrSaddle(nodeAMt) || 
                    nodeAMt->down() == NULL) {
                  newNode = mLTree->findElement(nodeAMt->id());
               
                  if (newNode != NULL) {
                    //std::cout << "Node exists in tree...TREE ID:: " 
                    //          << mLTree->id()
                    //          << " Node:: " << nodeAMt->id()
                    //          << std::endl;
                    if(addEdge(nodeLT, newNode)) {
                      //std::cout << "\nAdding edge!! Node :: "<< nodeLT->id() <<
                      //std::endl;
                    }
                    nodeLT = newNode;
                  } 
                  else { 
                    newNode = mLTree->addNodeReturn(nodeAMt->id(),
                                     nodeAMt->value(), nodeAMt->multiplicity());
                    //std::cout << "Adding Node at Bottom :: " << nodeAMt->id()
                    //          << " To Be Parent:: " << nodeLT->id()   
                    //          << " TREE ID:: " << mLTree->id() << "\n";
                    MinNodeCount++;

                    setChild(nodeLT, newNode);

                    nodeLT = newNode; 
                    nodeLT->nonLocal(true);
                  }
                  nodeLT->flag(true);
                  nodeLT->setOutstanding(nodeAMt->outstanding());
                  checkParents(nodeLT, nodeAMt, AMt);
                  nodeAMt->flag(true); 

                  if (isNonLocalMinima(mLTree, nodeLT)) 
                      break;
                }
                nodeAMt = nodeAMt->down();
              }
            }
          }
           /* nodeAMt = nodeAMt->down();
            //std::cout << "Local Minima encountered at :: " << nodeLT->id() 
            //          << " TREE ID:: " << mLTree->id() << "\n";
            //std::cout << "Adding Node at Bottom :: " << nodeAMt->id() 
            //          << " TREE ID:: " << mLTree->id() 
            //          << " Outst:: " << (int)nodeAMt->outstanding() << "\n";

            // Fast forward to next important node
            while (nodeAMt->down() != NULL ){
              if (nodeAMt->outstanding() != 0 || isMaxOrSaddle(nodeAMt)) 
                break;
              nodeAMt = nodeAMt->down();
            }

            newNode = mLTree->findElement(nodeAMt->id());
               
            if (newNode != NULL) {
              //std::cout << "Node exists in tree...TREE ID:: " << mLTree->id()
              //          << std::endl;
              if(addEdge(nodeLT, newNode)) {
                //std::cout << "\nAdding edge!! Node :: "<< nodeLT->id() <<
                //std::endl;
              }
            }
            else { */
                /*newNode = mLTree->addNodeReturn(nodeAMt->id(), 
                        nodeAMt->value(), nodeAMt->multiplicity());
                std::cout << "Adding Node at Bottom :: " << nodeAMt->id()
                          << " To Be Parent:: " << nodeLT->id()   
                          << " TREE ID:: " << mLTree->id() << "\n";

                newNode->setOutstanding(nodeAMt->outstanding());
                setChild(nodeLT, newNode);

                newNode->flag(true);
                */
                // we remove all regular finalized nodes from upstream 
                // till we encounter a critical node or an outstanding node
                /*TreeNode* cParentLT = nodeLT->up();

                while(!isMaxOrSaddle(cParentLT) && cParentLT->outstanding()==0)
                {
                  nodeLT = cParentLT;
                  mLTree->addRecord(nodeLT->id(), nodeLT->down()->id());
                  //nodeLT->down()->bypass();
                  mLTree->deleteElement(nodeLT->down());
                  nodeLT->setDown(NULL);
                  cParentLT = cParentLT->up();
                }
                */
        //    }
        //  }

          //std::cout << "\nTRAVERSAL LOOP END!! : TREE ID: " << mLTree->id() 
          //          << std::endl;
        }
      }
    }
  }

  // Finalizing vertices
  for(itLT=mLTree->begin(); itLT != mLTree->end(); ) {

    //TreeNode* node = itLT->down();
    TreeNode* node = itLT;
    
    if (node != NULL) {
      //std::cout << "NODE:: " << node->id()
      //          << " Outstanding:: " << (int)node->outstanding()
      //         << std::endl;
     if (isRegular(node) && node->outstanding()<=0 ) {
       //std::cout << "Removing node.." << node->id() 
       //          << " Tree ID:: " << mLTree->id() << std::endl;
       mLTree->addRecord(getParent(node)->id(), node->id());

       itLT++;
       // Unlink it from the tree
       node->bypass();
    
       newNodeCount--;
       // And remove it from memory
       mLTree->deleteElement(node);
     }
     else 
       itLT++;
    }
    else
      itLT++;
  }

  // The following counters help for identifying the various nodes in the local
  // trees. Used for understanding the nature of trees getting formed and the
  // nodes being added to the local trees.
/*  
    int NonLocalParents = 0;
    int ExternalSaddles = 0;
    int ExternalParents = 0;
    int TrueNonLocalSaddles = 0;
    int LocalNodes = 0;
    
  //if (mLTree->id() == 39) {
  for(itLT=mLTree->begin(); itLT != mLTree->end(); itLT++ ) {

    //TreeNode* node = itLT->down();
    TreeNode* node = itLT;
    bool flag = false;

    if (node != NULL) {
      if (node->isNonLocal()) {
        if (node->up() == NULL) {
          if (node->down()->isNonLocal()) ExternalParents++;
          else NonLocalParents++;
        }
        else {
          TreeNode *parent = node->up();
          do {
            if (!parent->isNonLocal()) {
              TrueNonLocalSaddles++;
              flag = true;
              break;
            }
  //          std::cout << "stuck\n";
            parent = parent->next();
          } while (parent != node->up()); 

          if (!flag) {
            flag = false;
            ExternalSaddles++;
          }
        }
      }
      else 
        LocalNodes++;
    }
  }
*/
  // To print all nodes
/*  std::cout << "Local Tree... \n" ;
  for(itLT=mLTree->begin(); itLT != mLTree->end(); itLT++) {
    std::cout << "Node:: " << itLT->id() 
              << " Oust:: " << (int)itLT->outstanding()
              << " TreeID:: " << mLTree->id() 
              << std::endl;
  }
*/

  // The following help in identifying number of non-local nodes added in the
  // path of local nodes to the roots
/*
  SortedVertexValArray.clear();
  int pathCount = 0;
  int nonLocalPathCount =0;
  int totalNodes = 0;
  // We mark all nodes as not visited
  for(itLT=mLTree->begin(); itLT != mLTree->end(); itLT++) {
    itLT->flag(false);
    totalNodes++;
    if (!itLT->isNonLocal()) {
      // We populate the vertex value vector in this loop
      ver_val.idx = itLT->id();
      ver_val.val = itLT->value();
      SortedVertexValArray.push_back(ver_val);
    }
  }

  // Now we sort the nodes based on function value
  std::sort(SortedVertexValArray.begin(), 
            SortedVertexValArray.end(), customLess);
 
  for (itArr=SortedVertexValArray.begin(); 
       itArr!=SortedVertexValArray.end(); itArr++)
  {
    // find vertex in local tree
    nodeLT = mLTree->findElement(itArr->idx);
    sterror(nodeLT==NULL, "Index %d must be present in LTree", 
            (int)itArr->idx);

    // if node is not visited
    if (nodeLT->flag() == false) {
     
      nodeLT->flag(true);
      pathCount++;

      nodeLT = nodeLT->down();
      do {
        if (nodeLT->flag())
          break;

        if (nodeLT->isNonLocal()) nonLocalPathCount++;
        nodeLT->flag(true);
        nodeLT = nodeLT->down();
        pathCount++;
      } while (nodeLT != NULL);
    }
  } 
 
  //std::cout << "New nodes added :: " << newNodeCount 
  //          << " Parents Added :: " << newParentsCount
  //          << " External Parents:: " << ExternalParents
  //          << " NonLocal Parents :: " << NonLocalParents
  //          << " External Saddles :: " << ExternalSaddles
            //<< " TrueNonLocalSaddles :: " << TrueNonLocalSaddles
  //          << " Minima added :: " << MinNodeCount
  std::cout          << " Local Nodes :: " << SortedVertexValArray.size()//LocalNodes
            << " Path Count :: " << pathCount
            << " Non Local Path Count :: " << nonLocalPathCount
            << " Total nodes :: " << totalNodes
            << " Tree ID:: " << mLTree->id() << "\n"; 
  char filename1[32];
  sprintf(filename1, "interLtree_%d_%d.dot", mLTree->id(), mTreeNum);
  //sprintf(filename1, "interLtree_profile_%d.dot", mLTree->id());
  FILE* fp1 = fopen(filename1, "w");
  mLTree->writeToFile(fp1);

//  }
*/
  //std::cout << "\n\n#### FIX COMPLETE.. TREE ID:: " << mLTree->id() 
  //          << std::endl;
  //}

  mTreeNum++;
  SortedVertexValArray.clear();
  return 1;
}

bool FixLocalTreeAlgorithm::isMaxOrSaddle(TreeNode* node){

  // Is maxima
  if (node->up() == NULL)
    return true;

  // Is saddle
  if (node->up()->next() != node->up())
    return true;

  return false;
}

void FixLocalTreeAlgorithm::checkParents(TreeNode* NodeLT, TreeNode* NodeAMt, 
                                         MergeTree* AMt){

  // if nodeLT is nonLocalLeaf node we update parent only 
  // if this node is no longer critical in AMt
  if (isNonLocalLeafNode(NodeLT) && isMaxOrSaddle(NodeAMt))
    return;
  TreeNode* UpLinkLT = NodeLT->up();
  TreeNode* UpLinkAMt = NodeAMt->up();
  //std::cout << "Checking Parents of Node :: " << NodeLT->id() << std::endl;
  // if NodeLT is maximum
  if (UpLinkLT == NULL && UpLinkAMt == NULL ) {
   // std::cout << "\nNode:: "<< NodeLT->id() << " is Global Maximum!!\n";
    return;
  }

  // if NodeLT is local maximum
  if (UpLinkLT == NULL && UpLinkAMt != NULL ) {
    //std::cout << "\nNode:: "<< NodeLT->id() << " is Local Maximum!!\n";
    
    // Add parent/s to NodeLT
    TreeNode* parentAMt = UpLinkAMt;
    TreeNode* newNode;
    do {
      // We need to find the maxima or saddle node above this parent and 
      // add it to LT. We call this node criticalParent i.e. cParent
      TreeNode* cParentAMt = parentAMt;
      while(!isMaxOrSaddle(cParentAMt) && cParentAMt->outstanding() == 0)
        cParentAMt = cParentAMt->up();
      
      newNode = mLTree->findElement(cParentAMt->id());
      if (newNode == NULL) {
        newNode = mLTree->addNodeReturn(cParentAMt->id(), 
                             cParentAMt->value(), cParentAMt->multiplicity());

        newNode->setOutstanding(cParentAMt->outstanding());
        newNode->nonLocal(true);
        
        newParentsCount++;
        //std::cout << "\n ADDING PARENT Node:: "<< cParentAMt->id() << std::endl;
        setChild(newNode, NodeLT);
      }
      else {
        addEdge(newNode, NodeLT);
        //std::cout << "\nAdding edge!! Node :: "<< NodeLT->id()
        //          << " and Node:: " << newNode->id() << std::endl;
      }
      parentAMt = parentAMt->next();
    } while (parentAMt != UpLinkAMt);

    return;
  }

  // if NodeAMt is local maximum then we do nothing
  if (UpLinkAMt == NULL)
    return;

  // Both nodes are valence two then we do not need to check parents as
  // it would be covered by the other cases
  if (UpLinkLT != UpLinkLT->next() || UpLinkAMt != UpLinkAMt->next()) {  

    // Now check if a new parent has been added in the AMtree. 
    // If new parent in AMtree add it to LTree
    TreeNode* parentAMt = UpLinkAMt;
    TreeNode* newNode;
    //std::cout << "One of the nodes is not valence 2... \n"; 
    bool foundFlag = false;
    do {
      TreeNode* parentLT = UpLinkLT;
      foundFlag = false;
      do {
        if (parentLT->id() == parentAMt->id()) {
          foundFlag = true;
          break;
        }

        parentLT = parentLT->next();
      }while (parentLT != UpLinkLT);

      if (!foundFlag) {

        // We need to find the maxima or saddle node above this parent 
        // and add it to LT. We call this node criticalParent i.e. cParent
        TreeNode* cParentAMt = parentAMt;
        while(!isMaxOrSaddle(cParentAMt) && cParentAMt->outstanding() == 0)
          cParentAMt = cParentAMt->up();

        // Add new parent or mark as saddle
        NodeLT->isSaddle(true);
        newNode = mLTree->findElement(cParentAMt->id());

        if (newNode != NULL) {
          addEdge(newNode, NodeLT);
          //std::cout << "\nAdding edge!! Node :: "<< NodeLT->id()
          //          << " and Node:: " << newNode->id() << std::endl;
        }
        else {

          newNode = mLTree->addNodeReturn(cParentAMt->id(), 
                             cParentAMt->value(), cParentAMt->multiplicity());
          //std::cout << "\nNode :: " << NodeLT->id() << " Add new Parent!!\n";
          //std::cout << "\n ADDING PARENT Node:: "<< parentAMt->id() 
          //          << std::endl;
          newNode->setOutstanding(cParentAMt->outstanding());
          newNode->nonLocal(true);
      
          newParentsCount++;
          setChild(newNode, NodeLT);

          // Check if added parent is descendant of existing parents
          if (checkDescendant(cParentAMt, UpLinkLT, UpLinkAMt->down(), AMt)){
            addEdge(newNode, UpLinkLT);
            //checkParents(newNode, cParentAMt, AMt);
          }
        }
      }
      parentAMt = parentAMt->next();
    }while (parentAMt != UpLinkAMt);
  }
  else {
     //std::cout << "Valence 2 nodes :: Node :: " << NodeLT->id() << std::endl;
     // Check if nodes have different parents. In this case, the node in LT
     // becomes a saddle after we add a new parent to LT. 
     if (UpLinkLT->id() != UpLinkAMt->id()) {
     
       //std::cout << "Different parents for valence 2..."
       //          << " Parent LT:: " << UpLinkLT->id() 
       //          << " Parent AMt:: " << UpLinkAMt->id()
       //          << std::endl;
       TreeNode* cParentAMt = UpLinkAMt;

       while(!isMaxOrSaddle(cParentAMt) && cParentAMt->outstanding() == 0)
         cParentAMt = cParentAMt->up();

       // Add new parent or mark as saddle
       TreeNode* newNode = mLTree->findElement(cParentAMt->id());

       if (newNode != NULL) {
         addEdge(newNode, NodeLT);
         //std::cout << "\nAdding edge!! Node :: "<< NodeLT->id() 
         //          << " and Node:: " << newNode->id() << std::endl;
       }
       else {

         newNode = mLTree->addNodeReturn(cParentAMt->id(), 
                            cParentAMt->value(), cParentAMt->multiplicity());
         //std::cout << "\nNode :: " << NodeLT->id() << " Add new Parent!!\n";
         //std::cout << "\n ADDING PARENT Node:: "<< cParentAMt->id()
         //          << std::endl;
         newNode->setOutstanding(cParentAMt->outstanding());
         newNode->nonLocal(true);

         newParentsCount++;
         setChild(newNode, NodeLT);

         // Check if added parent is descendant of existing parents
         if (checkDescendant(cParentAMt, UpLinkLT, UpLinkAMt->down(), AMt)){
           addEdge(newNode, UpLinkLT);
           //checkParents(newNode, cParentAMt, AMt); 
           // we have to do the above  this else the added 
           // parent is regular and will get removed
         }
       }
     }
    return;
  }
}

bool FixLocalTreeAlgorithm::checkDescendant(TreeNode* newParentFromAMt, 
                                            TreeNode* oldParentFromLT, 
                                            TreeNode* endNode, MergeTree* AMt)
{
  // We check if parents are descendants in AMt
  // We first search for existing parent from LT in AMt
  TreeNode* oldParentInAMt = AMt->findElement(oldParentFromLT->id());

  if (oldParentInAMt != NULL) {
    if (greater(*newParentFromAMt, *oldParentInAMt))
      std::swap(oldParentInAMt, newParentFromAMt);

    while (oldParentInAMt != endNode) {
      if (oldParentInAMt == newParentFromAMt)
        return true;
      oldParentInAMt = oldParentInAMt->down();
    }
  }

  return false;
}

TreeNode* FixLocalTreeAlgorithm::addEdge(TreeNode* head,TreeNode* tail)
{
  if (greater(*tail, *head))
    std::swap(head, tail);

  head = findIntegrationVertex(head, tail);

  if (head == tail) { // If this edge already exists
    //stderr(" Edge already exists\n");   
    return NULL; // There is nothing to d
  } 
  else {
    // Otherwise, we need to merge two path
    return mergeBranches(head, tail);
  }
}

TreeNode* FixLocalTreeAlgorithm::findIntegrationVertex(TreeNode* high, 
                                                       TreeNode* low)
{
  // Search for the lowest child of high that is still above low
  while ((getChild(high) != NULL) && !greater(*(low), *getChild(high))) {
    high = getChild(high);
    if (high->id() == low->id())
      break;
  }

  return high;
}

TreeNode* FixLocalTreeAlgorithm::mergeBranches(TreeNode* left,
    TreeNode* right) {
  if (left->id() == right->id()) return NULL;
  sterror(!greater(*left,*right),"Assumption violated. \
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
    left = FixLocalTreeAlgorithm::findIntegrationVertex(left, right);
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

int FixLocalTreeAlgorithm::finalizeVertex(TreeNode* v) {
  // If the node is now regular
  if (isRegular(v)) {
    // Add it to the nodeRecord for figuring out segmentation
    mLTree->addRecord(getParent(v)->id(), v->id());

    // Unlink it from the tree
    v->bypass();
    
    // And remove it from memory
    mLTree->deleteElement(v);

    return 1;
  }
  return 0;
}

bool FixLocalTreeAlgorithm::isRegular(TreeNode* v) const {
  if (getChild(v) == NULL)
    return false;

  if (getParent(v) == NULL)
    return false;

  if (getParent(v)->next() != getParent(v))// || v->isSaddle() == true)
    return false;

  return true;
}


