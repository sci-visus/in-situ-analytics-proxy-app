/*
* LocalComputeAlgorithm.h
*
*  Created on: Feb 5, 2012
*      Author: bremer5
*/

#ifndef MERGETREEBUILDER_H
#define MERGETREEBUILDER_H

#include "LocalComputeAlgorithm.h"
#include "UnionFind.h"
#include <iostream>

using namespace std;

class LocalComputeAlgorithm;

//! The API to compute a local tree
class DenseMergeTreeBuilder : public LocalComputeAlgorithm
{
protected:
  BoxSorter* mBS;

  //inline bool isBoundaryMax(char &c) {
  //  return c & 0x0001;
  //}
  //inline void setBoundaryMax(char &c, bool val)  {
  //  c = (c & 0xfffe) | val;
  //}

  //inline bool isSeen(char &c) {
  //  return c & 0x0002;
  //}
  //inline void setSeen(char &c, bool val) {
  //  c = (c & 0xfffd) | (val << 1);
  //}
  //inline bool isCAdded(char& c) {
  //  return c & 0x0004;
  //}
  //inline void setCAdded(char &c, bool val) {
  //  c = (c & 0xfffb) | (val << 2);
  //}

public:



  //! Default constructor
  DenseMergeTreeBuilder(bool invert=false) : LocalComputeAlgorithm(invert)  {
    //printf("Making a new DenseMergeTreeBuilder\n");
  }

  //! Destructor
  virtual ~DenseMergeTreeBuilder() {
  }

  //! Apply this algorithm and store the resulting tree
  virtual int apply( Patch* patch, const int field, SegmentedMergeTree& tree,
    TopoOutputStream* stream, FunctionType bottom_range = sMinFunction,
    FunctionType top_range = sMaxFunction) {

      // get the right field
      // and also the right domain, and number of vertices
      const FunctionType* tField = patch->field(field);
      SubBox& tDomain = patch->domain();
      const LocalIndexType num_vertices = tDomain.localSize(); 

      // initialize neighborhood information
      Neighborhood neigh(tDomain);

      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      //// ALTERNATIVES FOR ALGORITHM

      // use segmentation or not
      bool USE_SEGMENTATION = true;
      MergeTree* tMT = &tree;

      if (USE_SEGMENTATION) {
        ((SegmentedMergeTree*) tMT)->initializeSegmentation(num_vertices);
      }


      //if (USE_SEGMENTATION) {
        //tMT = new SegmentedMergeTree(0); // guessing id
      //} else {
      //  tMT = new MergeTree(0);
      //}

      // this is where we select sorting modes
      int SORTING_MODE = 2;
      if (SORTING_MODE == 0) {
        mBS = new PreFilteredPreSortedBoxSorter(tField, &tDomain, !this->mInvert,
                                                sMinFunction, sMaxFunction);
      } else if (SORTING_MODE == 1) {
        mBS = new PreSortedBoxSorter(tField, &tDomain, ! this->mInvert);
      } else if (SORTING_MODE == 2) {
        mBS = new PreSortedPriorityQueueSorter(tField, &tDomain, ! this->mInvert);
        // this is broken, to be fixed
        //mBS = new LiveSortingBoxSorter(tField, &tDomain, ! this->mInvert);
      }

      // select dense vs Sparse Union Find
      bool USE_DENSE_UF = true;
      UnionFind<LocalIndexType, BoxSorter>* tUF;
      if (USE_DENSE_UF) {
        tUF = new CompactUnionFind<LocalIndexType, BoxSorter>(num_vertices, *mBS);
      } else {
        tUF = new SparseUnionFind<LocalIndexType, BoxSorter>(*mBS);
      }

      //// END ALTERNATIVES FOR ALGORITHM
      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      
      // allocate dense structures for storing the segmentation while the tree 
      // is being computed
      LocalIndexType* tSegmentation = new LocalIndexType[num_vertices];
      LocalIndexType* tDown = new LocalIndexType[num_vertices];
      //char* tFlags = new char[num_vertices];
      for (LocalIndexType i = 0; i < num_vertices; i++) {
        tSegmentation[i] = LNULL;
        tDown[i] = LNULL;
        //tFlags[i] = 0;
      }


      char boundaryLabel = 0;

      // Go through elements in sorted order
      for (mBS->begin(); mBS->valid(); mBS->advance()){

        LocalIndexType tCurrent = mBS->value();
        
        FunctionType tVal = tField[tDomain.fieldIndex(tCurrent)];
        // early terminate if we're outside limits
        if (this->mInvert && tVal > top_range) break;
        if (! this->mInvert && tVal < bottom_range) break;

        // make a set in the Union-Find Data structure
        tUF->MakeSet(tCurrent);

        // initially set the segmentation id to itself, and down pointer to null
        tSegmentation[tCurrent] = tCurrent; 
        tDown[tCurrent] = LNULL;

        PointIndex tCurrentPID = tDomain.coordinates(tCurrent);
            
        //================================================
        // BEGIN BOUNDARY MAX TEST
        // if this vertex is boundary, first compute its "boundary" criticality
        int tBoundaryCount = tDomain.boundaryCount(tCurrentPID);
        bool boundaryMax = false;
        if (tBoundaryCount != 0) {
          boundaryMax = true;
          // iterate over neighbors skipping non-sub-boundary
          NeighborIterator it = neigh.begin(tCurrent);
          while (neigh.valid(it)) {
            LocalIndexType tNeighbor = neigh.value(it);
            neigh.advance(it);
        
            // if there is a "seen" sub-boundary neighbor, then this is NOT a boundary max
            if (tUF->Exists(tNeighbor) && tDomain.subBoundary(tCurrentPID, tNeighbor)) {
              boundaryMax = false;
              break;
            }
          }
          // Label the boundary
          boundaryLabel = tDomain.boundaryLabel(tCurrentPID);
          //if (boundaryMax) {
          //  std::cout << " Point:: " << tCurrentPID[0] << ", " << tCurrentPID[1] 
          //            << ", " << tCurrentPID[2] << " : Label: " << (int)boundaryLabel
          //            << " : max : " << (int)boundaryMax 
          //            << " : count : " << tBoundaryCount << std::endl;
          //}
        }
        // END BOUNDARY MAX TEST
        //================================================

        // test whether or not this point is critical by 
        // counting number of "regions" in "lower" link
        // iterate over 26-neighborhood

        vector<LocalIndexType> segs;
        LocalIndexType tNumAllRegions = 0;
        NeighborIterator it = neigh.begin(tCurrent);
        while (neigh.valid(it)) {
          LocalIndexType tNeighbor = neigh.value(it);
          neigh.advance(it);

          // skip over things later in ordering or "not seen"
          if (! tUF->Exists(tNeighbor)) {
            continue;
          }

          // check regions
          LocalIndexType tNeighRegion = tUF->Find(tNeighbor);
          if(tUF->Find(tCurrent) != tNeighRegion) {

            if (tCurrent != tUF->Find(tCurrent)) printf("WRONG ASSUMPTIONS!!!\n"); 
            // the assumption being that the "newest" element becomes the 
            // representative of the union-find set
          
            // so now we set the segment id of the current vertex to the 
            // segment id of its upper region
            segs.push_back(tSegmentation[tNeighRegion]);
            tSegmentation[tCurrent] = tSegmentation[tNeighRegion];
            // and set the "down" pointer of the upper neighbor's representative to 
            tDown[tNeighRegion] = tCurrent;
            tNumAllRegions++;
            
            tUF->Union(tCurrent, tNeighbor);
                        //printf("tUF->Find(%d) = %d\n", tCurrent, tUF->Find(tCurrent));
          }

        }

        // now we set some flags if we can tell that it's a restricted max, 
        // true max, or saddle
        if (tNumAllRegions != 1 || boundaryMax) {
          // if it's a saddle or max it creates its own segmentation, thus:
          tSegmentation[tCurrent] = tCurrent;
          // if it's a boundary max, then mark it
          //setBoundaryMax(tFlags[tCurrent], boundaryMax);

          //now start making tree
          GlobalIndexType tCurrentG = tDomain.globalIndex(tCurrent);
          if (USE_SEGMENTATION) {
          //  printf("got here: %d, %d, %f, %d, %d, %d, %d, %d\n", (int) tCurrent, 
          //         (int) tCurrentG, (float) tField[tDomain.fieldIndex(tCurrent)], 
          //         (int) tNumAllRegions, boundaryMax, tDomain.boundaryCount(tCurrent),
          //         tDomain.globalBoundaryCount(tCurrent), 
          //         1 << (tDomain.boundaryCount(tCurrent)
          //         - tDomain.globalBoundaryCount(tCurrent)) );

            ((SegmentedMergeTree*) tMT)->addNode(tCurrent, tCurrentG,
                                           tField[tDomain.fieldIndex(tCurrent)],
                                           (boundaryMax ?  
                                            1 << (tDomain.boundaryCount(tCurrent) 
                                                 - tDomain.globalBoundaryCount(tCurrent)) : 1),
                                            boundaryLabel);
           //std::cout << "Vertex: " << tCurrentG << " Tree ID:: " <<
           //tMT->id() << std::endl;
          } else {
            tMT->addNode( tCurrentG,
              tField[tDomain.fieldIndex(tCurrent)],
              (boundaryMax ?  1 << (tDomain.boundaryCount(tCurrent) - 
                                    tDomain.globalBoundaryCount(tCurrent)) : 1),
              boundaryLabel);
            //std::cout << "Vertex: " << tCurrentG << " Tree ID:: " <<
            //tMT->id() << std::endl;
          }
          
          for (int i = 0; i < segs.size(); i++) {
            // now add edges. 
            GlobalIndexType tUpperNodeG = tDomain.globalIndex(segs[i]);
            tMT->addEdge(tUpperNodeG,tCurrentG);
            //printf("%d -> %d\n", (int) tUpperNodeG, tCurrentG);
                        //std::cout << "Edge: " << tUpperNodeG << "->" <<
                        //tCurrentG << " Tree ID: " << tMT->id() << std::endl;
            // now any further find on the upper's region will only find the current guy
            // so we can finalize all upper guys
            tMT->finalizeNode(tUpperNodeG);
          }
        }   

        // if we're using segmentations, add it
        if (USE_SEGMENTATION) {
          ((SegmentedMergeTree*) tMT)->addToSegment(tCurrent, tSegmentation[tCurrent]);
        }

      }

      // what we have at this point: 
      // tSegmentation has the segment id for each vertex, LNULL if it was not processed
      // tDown has the downwards vertex in the AUGMENTED merge tree, LNULL if there isn't any
      // tFlags is 1 if the vertex is a boundary max, 0 otherwise
      // tMT has the merge tree MINUS THE MINIMA AND THE EDGES TO THEM. 
      
      // add the local mins and the edges, and finalize
      for (LocalIndexType i = 0; i < num_vertices; i++) {
        GlobalIndexType tCurrentG = tDomain.globalIndex(i);

        if (tSegmentation[i] != LNULL && tSegmentation[i] != tCurrentG && tDown[i] == LNULL) {
          tMT->addNode(tCurrentG, tField[tDomain.fieldIndex(i)], 1, boundaryLabel);
          //std::cout << "Vertex: " << tCurrentG << std::endl;
          GlobalIndexType tUpperNodeG = tDomain.globalIndex(tSegmentation[i]);
          //Aaditya: We do not need the following edge
          if (tUpperNodeG != tCurrentG) {
            tMT->addEdge(tUpperNodeG,tCurrentG);
            //std::cout << "Edge: " << tUpperNodeG << "->" << tCurrentG << std::endl;
          }
          tMT->finalizeNode(tUpperNodeG);
          tMT->finalizeNode(tCurrentG);
        }
      }

      // now output the tree?
      //tMT->outputToStream(stream);
      tMT->outputSortedBoundaryToStream(stream);
      std::cout << "Done computing Local tree... \n";
      // cleanup
      delete[] tSegmentation;
      delete[] tDown;
      delete tUF;

      return 1;

  }
};






#endif /* MERGETREEBUILDER_H */
