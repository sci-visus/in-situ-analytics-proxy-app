/*
* LocalComputeAlgorithm.h
*
*  Created on: Feb 5, 2012
*      Author: bremer5
*/

#ifndef LOCALCOMPUTEALGORITHM_H_
#define LOCALCOMPUTEALGORITHM_H_

#include "PatchAlgorithm.h"
#include "TopoOutputStream.h"
#include "SegmentedMergeTree.h"
#include "Patch.h"
#include "Neighborhood.h"
#include "BoxSorter.h"

#include "MergeTreeLite.h"

//#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

enum LocalAlgorithmType {
  LOCAL_SORTED_UF = 0,
  LOCAL_DENSE_MERGE_TREE_BUILDER = 1,
};


//! The API to compute a local tree
class LocalComputeAlgorithm : public PatchAlgorithm
{
public:

  //! A function value higher than anything reasonable
  static const FunctionType sMaxFunction;

  //! A function value lower than anything reasonable
  static const FunctionType sMinFunction;


  //! The factory function
  static LocalComputeAlgorithm* make(LocalAlgorithmType type, bool invert);

  //! Default constructor
  LocalComputeAlgorithm (bool invert=false) : PatchAlgorithm(invert) {}

  //! Destructor
  virtual ~LocalComputeAlgorithm() {}

  //! Apply this algorithm and store the resulting tree
  virtual int apply(Patch* patch, const int field, SegmentedMergeTree& tree,
                    TopoOutputStream* stream, 
                    FunctionType bottom_range = sMinFunction,
                    FunctionType top_range = sMaxFunction) = 0;

};



//! The API to compute a local tree
class LocalSortedUF : public LocalComputeAlgorithm
{
protected:
  //  LocalIndexType* UFParents;
  //  vector<LocalIndexType> sorted_indices;


  //struct SimpleGreaterSorter {
  //  const FunctionType* mf;
  //  SubBox* domain;
  //  SimpleGreaterSorter(const FunctionType* f, SubBox* dom) : mf(f), 
  //                      domain(dom) {} 
  //  bool operator() (LocalIndexType a, LocalIndexType b) {
  //    //printf("comparing %d %d, with %d\n", a, b, mf);
  //    a = domain->fieldIndex(a);
  //    b = domain->fieldIndex(b);
  //    if (mf[a] > mf[b]) return true;
  //    if (mf[b] > mf[a]) return false;
  //    return a > b;
  //  }
  //};
  //struct SimpleSmallerSorter {
  //  const FunctionType* mf;
  //  SubBox* domain;
  //  SimpleSmallerSorter(const FunctionType* f, SubBox* dom) : mf(f), 
  //                      domain(dom) {} 
  //  bool operator() (LocalIndexType a, LocalIndexType b) {
  //    //printf("comparing %d %d, with %d\n", a, b, mf);
  //    a = domain->fieldIndex(a);
  //    b = domain->fieldIndex(b);
  //    if (mf[a] < mf[b]) return true;
  //    if (mf[b] < mf[a]) return false;
  //    return a < b;
  //  }
  //};

  LocalIndexType Find(LocalIndexType index, LocalIndexType* UFParents)   {
    ////printf("Find:%d\n", index);
    if (UFParents[index] == index){
      //printf("\t=%d\n", index);
      return index;
    }
    if (UFParents[index] == LNULL) {
      UFParents[index] = index;
      //printf("\t=%d\n", index);
      return index;
    }
    LocalIndexType result = Find(UFParents[index], UFParents);

    UFParents[index] = result;
    //printf("\t=%d\n", result);
    return result;
  }

  /*
  LocalIndexType Union(LocalIndexType a, LocalIndexType b, 
                       LocalIndexType* UFParents) const  {
  printf("Union:%d-%d\n",a, b);
  LocalIndexType aroot = Find(a, UFParents);
  LocalIndexType broot = Find(b, UFParents);
  if (aroot == broot) { printf("\t=%d\n", aroot); return aroot;}
  if (this->mInvert) {
  if (this->greater(aroot, broot)) {
  printf("\t=%d\n",broot);
  UFParents[aroot] = broot;
  return broot;
  } else {
  UFParents[broot] = aroot;
  printf("\t=%d\n",aroot);
  return aroot;
  }

  } else {
  if (this->smaller(aroot, broot)) {
  UFParents[aroot] = broot;
  printf("\t=%d\n",broot);
  return broot;
  } else {
  UFParents[broot] = aroot;
  printf("\t=%d\n",aroot);
  return aroot;
  }

  }
  }
  */

  LocalIndexType Union(LocalIndexType a, LocalIndexType b, 
                       LocalIndexType* UFParents)   {
    //printf("Union:%d-%d\n",a, b);
    LocalIndexType aroot = Find(a, UFParents);
    LocalIndexType broot = Find(b, UFParents);
    if (aroot == broot) { 
      //printf("\t=%d\n", aroot); 
      return aroot;
    }

    if (mBS->before(broot,aroot)) {
      UFParents[broot] = aroot;
      return aroot;
    }
    else {
      UFParents[aroot] = broot;
      return broot;
    }
  }

  //SimpleGreaterSorter mGreaterS;
  //SimpleSmallerSorter mSmallerS;
  BoxSorter* mBS;

public:



  //! Default constructor
  LocalSortedUF(bool invert=false) : LocalComputeAlgorithm(invert)  {
    printf("Making a new LocalSortedUF\n");

  }

  //! Destructor
  virtual ~LocalSortedUF() {
    //delete[] UFParents;
  }




  //! Apply this algorithm and store the resulting tree
  virtual int apply( Patch* patch, const int field, SegmentedMergeTree& tree,
                     TopoOutputStream* stream, 
                     FunctionType bottom_range = sMinFunction,
                     FunctionType top_range = sMaxFunction) {
    

      //printf("HELLO WORLD: YOU HAVE ENTERED THE TWILIGHT ZONE\n");

      // get the right field
      // and also the right domain, and number of vertices
      const FunctionType* tField = patch->field(field);
      SubBox& tDomain = patch->domain();
      const LocalIndexType num_vertices = tDomain.localSize(); 
      // initialize neighborhood information
      Neighborhood neigh(tDomain);

      // this is where we select sorting modes
      // for now just use a pre-sorted box sorter (using stl::sort)
      mBS = new PreFilteredPreSortedBoxSorter(tField, &tDomain,
                                              ! this->mInvert, 
                                              sMinFunction, sMaxFunction);

      //printf("DOING (%d,%d,%d)--(%d,%d,%d)\n",
      //tDomain.lowCorner()[0],tDomain.lowCorner()[1],tDomain.lowCorner()[2],
      //tDomain.highCorner()[0]-1,tDomain.highCorner()[1]-1,
      //tDomain.highCorner()[2]-1);


      // temp storage of full augmented MT
      MergeTreeLite* tAugmentedTree = new BasicMergeTreeLite(num_vertices);

      // allocate space to store the Union-Find data structure
      LocalIndexType* UFParents = new LocalIndexType[num_vertices];
      memset(UFParents, LNULL, sizeof(LocalIndexType) * num_vertices);

      // initialize sorting
      mBS->begin();
      while( mBS->valid() ) {

        LocalIndexType tCurrent = mBS->value();
        FunctionType tVal = tField[tDomain.fieldIndex(tCurrent)];

        //printf("doing: %d-%f\n", tCurrent, tVal);
        // early terminate if we're outside limits
        if (this->mInvert && tVal > top_range) break;
        if (! this->mInvert && tVal < bottom_range) break;

        //printf("g%d\n", tCurrent);
        // make a set in the Union-Find Data structure
        UFParents[tCurrent] = tCurrent;
        // TREE->ADDNODE
        tAugmentedTree->add_node(tCurrent);

        PointIndex tCurrentPID = tDomain.coordinates(tCurrent);
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
          
            // printf("c%d dn%d\n", tCurrent, tNeighbor);

            // if there is a "seen" sub-boundary neighbor,
            // then this is NOT a boundary max
            if (UFParents[tNeighbor] != LNULL && 
                tDomain.subBoundary(tCurrentPID, tNeighbor)) {
              boundaryMax = false;
              break;
            }
          }
        }
           
        // test whether or not this point is critical by 
        // counting number of "regions" in "lower" link
        // iterate over 26-neighborhood
        
        LocalIndexType tNumAllRegions = 0;
        NeighborIterator it = neigh.begin(tCurrent);
        while (neigh.valid(it)) {
          LocalIndexType tNeighbor = neigh.value(it);
          neigh.advance(it);
          
          // skip over things later in ordering or "not seen"
          if (UFParents[tNeighbor] == LNULL) {
            continue;
          }

          // check regions
            if(Find(tCurrent, UFParents) != Find(tNeighbor, UFParents)) {
      
              // TREE->ADDARC
              if (tCurrent != Find(tCurrent, UFParents))
                printf("WRONG ASSUMPTIONS!!!\n");

              tAugmentedTree->add_arc(Find(tNeighbor, UFParents),
                                      Find(tCurrent, UFParents));
              tNumAllRegions++;
              Union(tCurrent, tNeighbor, UFParents);
            }

        }
        if (tNumAllRegions != 1 || boundaryMax) {
          // its a restricted critical point
          // TREE->MARKEXTREMUM
          tAugmentedTree->mark_critical(tCurrent, boundaryMax);
        }

        mBS->advance();
      }

      // cleanup
      
      // TREE->OUTPUT TO STREAM
      tAugmentedTree->output_to_stream(stream, tField, &tDomain);
            
            // **Writing local trees for Debugging purposes**
            //char fileName[32] = {0};
            //MergeTree* mergetree = &tree;
            //sprintf(fileName, "local_tree_%d.dot", mergetree->id());
            //FILE* fp = fopen(fileName, "w");
      //tAugmentedTree->write_to_file(fp, &tDomain);

      delete[] UFParents;
      return 1;



      // if we're out of range, simply send empty token
      ///FunctionType firstVal = tField[tDomain.fieldIndex(sorted_indices[0])];
      ///if ( ((! this->mInvert) && firstVal < bottom_range) ||
      ////  ((this->mInvert) && firstVal > top_range)) {
      ////    *stream << Token::gEmptyToken;

      //////    delete[] UFParents;
      //////    return 1;
      //////}



      ////// now vertices are sorted

      //////--debug
      ////for (uint32_t i = 0; i < num_vertices; i++) {
      ////  printf("%f -> %d\n", tField[sorted_indices[i]], sorted_indices[i]);
      //////}

      //////printf("gothere about to start iterating \n" ); //--debug

      ////// vector of maxs and saddles
      //////vector<LocalIndexType> crits;
      ////// array of segment ID's

      ////vector<GlobalIndexType> tFinalize;
      ////LocalIndexType* tSegmentation = new LocalIndexType[num_vertices];
      ////////unsigned char* tBoundaryCounts = new unsigned char[num_vertices];
      ////////// quickly fill in boundary counts
      ////////for (uint32_t i = 0; i < num_vertices; i++) {
      //////////  unsigned char tBoundaryCount = 1 << (tDomain.boundaryCount(i) - 
      ////////  //  tDomain.globalBoundaryCount(i));
      ////////  unsigned char tBoundaryCount = tDomain.boundaryCount(i);
      ////////  //printf("BC[%d] = %d\n", i, tBoundaryCount);
      ////////  tBoundaryCounts[i] = tBoundaryCount;
      ////////}



      ////// in sorted order, build full augmented merge tree on local piece
      ////for (uint32_t i = 0; i < sorted_indices.size(); i++) {

      ////  LocalIndexType current = sorted_indices[i];
      ////  PointIndex currentp = tDomain.coordinates(current);

      ////  // makeset in UF
      ////  UFParents[current] = current;

      ////  // boundary information for my current vertex
      ////  int tBoundaryCount = tDomain.boundaryCount(current);
      ////  //printf("boundarycount = %d\n", tBoundaryCount); //--debug

      ////  // add the vertex to augmented tree
      ////  FunctionType fval = tField[tDomain.fieldIndex(current)];

      ////  // EARLY TERMINATION IN THE CASE OF OUT OF FUNCITON RANGE
      ////  // see if the NEXT guy is past the filter, then we make THIS guy the minimum
      ////  bool tIsLastNode = false;
      ////  if (i != sorted_indices.size() - 1) {
      ////    FunctionType fNextval = tField[tDomain.fieldIndex(sorted_indices[i+1])];
      ////    if ( ((! this->mInvert) && fNextval < bottom_range) ||
      ////      ((this->mInvert) && fNextval > top_range)) {
      ////      tIsLastNode = true;
      ////    }
      ////  } else {
      ////    tIsLastNode = true;
      ////  }


      ////  //tAugmentedTree.addNode(gid, fval, tBoundaryCount);
      ////  //printf("added Node: %d %f %d\n", current, fval, tBoundaryCount);

      ////  int tNumRegions = 0;
      ////  //printf("about to iteratre neighbors\n"); //--debug

      ////  // now iterate over all neighbors, addind edges to augmented merge tree
      ////  NeighborIterator it = neigh.begin(current);
      ////  //printf("it: %d %d %d\n", it.position, it.baseID, it.boundary); //--debug
      ////  //printf("C[%d]:",current);
      ////  vector<LocalIndexType> tNeighbors;
      ////  vector<LocalIndexType> tDelayUnion;
      ////  vector<LocalIndexType> tDelayUnionSeg;
      ////  vector<LocalIndexType> tNeighborsSeg;
      ////  GlobalIndexType gid = tDomain.globalIndex(current);



      ////  while (neigh.valid(it)) {

      ////    // neighbor is the index of the current neighbor
      ////    LocalIndexType neighbor = neigh.value(it);
      ////    //printf("-%d", neighbor);
      ////    //printf("n=%d\n", neighbor); //--debug
      ////    neigh.advance(it);


      ////    // SKIP OVER ANYTHING SMALLER - or "not seen"
      ////    if (UFParents[neighbor] == LNULL) {
      ////      // then it has not been seen
      ////      continue;
      ////    }
      ////    //int tNegBoundaryCount = tBoundaryCounts[neighbor];
      ////    
      ////    if (Find(current, UFParents) != Find(neighbor, UFParents) &&
      ////      tDomain.subBoundary(currentp, neighbor)) {
      ////        //printf("!");
      ////        //tOthers.push_back(Find(neighbor, UFParents));
      ////        //GlobalIndexType ngid = tDomain.globalIndex(neighbor);
      ////        //tAugmentedTree.addEdge(ngid, gid);
      ////        //printf("added Edge: L:%d->%d G:%d->%d\n", neighbor, current, ngid, gid);
      ////        //Union(current, neighbor, UFParents);
      ////        tNeighbors.push_back(neighbor);
      ////        tNeighborsSeg.push_back(tSegmentation[Find(neighbor, UFParents)]);
      ////        tNumRegions++;
      ////        
      ////        Union(current, neighbor, UFParents);
      ////        //if (it.boundary <= (tDomain.boundaryCount(neighbor) - tDomain.globalBoundaryCount(neighbor))) {
      ////        //  tNumRegions++; 
      ////        //}
      ////      //} else {
      ////        //tNeighbors.push_back(neighbor);
      ////        
      ////        //tSegmentation[current] = tSegmentation[Find(neighbor, UFParents)];
      ////        //Union(current, neighbor, UFParents);

      ////      
      ////    } else {
      ////      tDelayUnion.push_back(neighbor);
      ////      tDelayUnionSeg.push_back(tSegmentation[Find(neighbor, UFParents)]);
      ////    }
      ////  }


      ////  if (tNumRegions == 0) printf("%d NUMREGIONS = 0\n", tDomain.globalIndex(current));
      ////  //printf("\n");
      ////  for (uint32_t j = 0; j < tDelayUnion.size(); j++) {
      ////    LocalIndexType neighbor = tDelayUnion[j];
      ////    if (Find(current, UFParents) != Find(neighbor, UFParents)) {
      ////      tNeighbors.push_back(neighbor);
      ////      tNeighborsSeg.push_back(tDelayUnionSeg[j]);
      ////    }
      ////    tSegmentation[current] = tSegmentation[Find(neighbor, UFParents)];
      ////    Union(current, neighbor, UFParents);
      ////  }

      ////  // add to list of critical points if the numbre of regions is not 1;
      ////  // or it is the local minimum
      ////  if (tNumRegions != 1 || tNeighbors.size() > 1) {
      ////    // then this is an outputtable critical point
      ////    tSegmentation[current] = current;
      ////    
      ////    PointIndex p = tDomain.coordinates(current);
      ////    printf("NODE: id=%d, val=%f, bc=%d, a=%d, b=%d (%d,%d,%d)\n", gid, fval,
      ////      tBoundaryCount,tNumRegions,(int)tNeighbors.size(),
      ////      p[0] + tDomain.lowCorner()[0],
      ////      p[1] + tDomain.lowCorner()[1],
      ////      p[2] + tDomain.lowCorner()[2]
      ////      );
      ////    Token::VertexToken vt;
      ////    vt.index(gid);
      ////    vt.value(fval);
      ////    // new boundary test for timo.
      ////    if (tNeighbors.size() > 1) 
      ////      vt.multiplicity(1);
      ////    else 
      ////      vt.multiplicity(1 << (tBoundaryCount - tDomain.globalBoundaryCount(current)));
      ////    *stream << vt;
      ////    tFinalize.push_back(gid);


      ////    for (uint32_t j = 0; j < tNeighbors.size(); j++) {
      ////      LocalIndexType tNSegID = tNeighborsSeg[j];
      ////      GlobalIndexType ngid = tDomain.globalIndex(tNSegID);
      ////      printf("-EDGE-: L:%d->%d G:%d->%d\n", current,tNSegID,gid, ngid);
      ////      Token::EdgeToken et;
      ////      et.source(ngid); et.destination(gid);
      ////      *stream << et;

      ////    }
      ////  } else {
      ////    
      ////    tSegmentation[current] =tNeighborsSeg[0];
      ////      
      ////    // if it is the local min and has not been detected earlier, output
      ////    if (tIsLastNode) {
      ////      printf("NODEmin: id=%d, val=%f, bc=%d\n", gid, fval, tBoundaryCount);
      ////      Token::VertexToken vt;
      ////      vt.index(gid);
      ////      vt.value(fval);

      ////      // A local minimum does not need a boundary count since it is either
      ////      // a) the global minimum in which case it gets preserved; or b) a
      ////      // strictly local min in which case it is irrelevant
      ////      vt.multiplicity(1);
      ////      *stream << vt;
      ////      tFinalize.push_back(gid);
      ////    
      ////      LocalIndexType tNSegID = tNeighborsSeg[0];
      ////      GlobalIndexType ngid = tDomain.globalIndex(tNSegID);
      ////      printf("-EDGEmin-: L:%d->%d G:%d->%d\n", current,tNSegID,gid, ngid);  
      ////      Token::EdgeToken et;
      ////      et.source(ngid); et.destination(gid);
      ////      *stream << et;

      ////      //force the thing to exit after this
      ////      i = sorted_indices.size();

      ////    }
      ////  }
      ////  // output segmentation id
      ////  GlobalIndexType ngid = tDomain.globalIndex(tSegmentation[current]);
      ////  //printf("\t-->SEG[%d] = %d\n", gid, ngid);
      ////  Token::SegToken st;
      ////  st.id(gid); st.seg(ngid); st.f(fval);
      ////  *stream << st;

      ////  //if (tNumRegions == 0) {
      ////  //  //max, add to tree
      ////  //  UFParents[i] = i;
      ////  //  tree.addNode(i, tField[i], tBoundaryCount);

      ////  //} else if (tNumRegions == 1) {
      ////  //  //regular, only add to segment
      ////  //  tree.addToSegment(Find(i,UFParents), i);

      ////  //} else {
      ////  //  // saddle, so add to tree
      ////  //  tree.addNode(i, tField[i], tBoundaryCount);
      ////  //  for (uint32_t j = 0; j < tOthers.size(); j++) {
      ////  //    tree.addEdge(tOthers[j], i);
      ////  //  }
      ////  //}


      ////}


      ////for (uint32_t i =0; i < tFinalize.size(); i++) {
      ////  Token::FinalToken ft;
      ////  ft.index(tFinalize[i]);
      ////  *stream << ft;
      ////}


      ////delete[] UFParents;
      ////delete[] tSegmentation;
      //////delete[] tBoundaryCounts;

      ////return 1;
  }
};


//! The API to compute a local tree
class GenericFlaggedLocalCompute : public LocalComputeAlgorithm
{
protected:


  LocalIndexType Find(LocalIndexType index, LocalIndexType* UFParents)   {
    ////printf("Find:%d\n", index);
    if (UFParents[index] == index){
      //printf("\t=%d\n", index);
      return index;
    }
    if (UFParents[index] == LNULL) {
      UFParents[index] = index;
      //printf("\t=%d\n", index);
      return index;
    }
    LocalIndexType result = Find(UFParents[index], UFParents);

    UFParents[index] = result;
    //printf("\t=%d\n", result);
    return result;
  }



  LocalIndexType Union(LocalIndexType a, LocalIndexType b,
                       LocalIndexType* UFParents)   {
    //printf("Union:%d-%d\n",a, b);
    LocalIndexType aroot = Find(a, UFParents);
    LocalIndexType broot = Find(b, UFParents);
    if (aroot == broot) { 
      //printf("\t=%d\n", aroot); 
      return aroot;
    }

    if (mBS->before(broot,aroot)) {
      UFParents[broot] = aroot;
      return aroot;
    }
    else {
      UFParents[aroot] = broot;
      return broot;
    }
  }

  //SimpleGreaterSorter mGreaterS;
  //SimpleSmallerSorter mSmallerS;
  BoxSorter* mBS;

public:



  //! Default constructor
  GenericFlaggedLocalCompute(bool invert=false) : LocalComputeAlgorithm(invert)  {
    printf("Making a new GenericFlaggedLocalCompute\n");

  }

  //! Destructor
  virtual ~GenericFlaggedLocalCompute() {
    //delete[] UFParents;
  }




  //! Apply this algorithm and store the resulting tree
  virtual int apply( Patch* patch, const int field, SegmentedMergeTree& tree,
                     TopoOutputStream* stream, 
                     FunctionType bottom_range = sMinFunction,
                     FunctionType top_range = sMaxFunction) {
    

      //printf("HELLO WORLD: YOU HAVE ENTERED THE TWILIGHT ZONE\n");

      // get the right field
      // and also the right domain, and number of vertices
      const FunctionType* tField = patch->field(field);
      SubBox& tDomain = patch->domain();
      const LocalIndexType num_vertices = tDomain.localSize(); 
      // initialize neighborhood information
      Neighborhood neigh(tDomain);

      // this is where we select sorting modes
      // for now just use a pre-sorted box sorter (using stl::sort)
      mBS = new PreFilteredPreSortedBoxSorter(tField, &tDomain,
                                              ! this->mInvert,
                                              sMinFunction, sMaxFunction);

      //printf("DOING (%d,%d,%d)--(%d,%d,%d)\n",
      //tDomain.lowCorner()[0],tDomain.lowCorner()[1],tDomain.lowCorner()[2],
      //tDomain.highCorner()[0]-1,tDomain.highCorner()[1]-1,
      //tDomain.highCorner()[2]-1);


      // temp storage of full augmented MT
      MergeTreeLite* tAugmentedTree = new BasicMergeTreeLite(num_vertices);

      // allocate space to store the Union-Find data structure
      LocalIndexType* UFParents = new LocalIndexType[num_vertices];
      memset(UFParents, LNULL, sizeof(LocalIndexType) * num_vertices);

      // initialize sorting
      mBS->begin();
      while( mBS->valid() ) {

        LocalIndexType tCurrent = mBS->value();
        FunctionType tVal = tField[tDomain.fieldIndex(tCurrent)];

        //printf("doing: %d-%f\n", tCurrent, tVal);
        // early terminate if we're outside limits
        if (this->mInvert && tVal > top_range) break;
        if (! this->mInvert && tVal < bottom_range) break;

        //printf("g%d\n", tCurrent);
        // make a set in the Union-Find Data structure
        UFParents[tCurrent] = tCurrent;
        // TREE->ADDNODE
        tAugmentedTree->add_node(tCurrent);

        PointIndex tCurrentPID = tDomain.coordinates(tCurrent);
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
          
            // printf("c%d dn%d\n", tCurrent, tNeighbor);

            // if there is a "seen" sub-boundary neighbor,
            // then this is NOT a boundary max
            if (UFParents[tNeighbor] != LNULL &&
                tDomain.subBoundary(tCurrentPID, tNeighbor)) {
              boundaryMax = false;
              break;
            }
          }
        }
           
        // test whether or not this point is critical by 
        // counting number of "regions" in "lower" link
        // iterate over 26-neighborhood
        
        LocalIndexType tNumAllRegions = 0;
        NeighborIterator it = neigh.begin(tCurrent);
        while (neigh.valid(it)) {
          LocalIndexType tNeighbor = neigh.value(it);
          neigh.advance(it);
          
          // skip over things later in ordering or "not seen"
          if (UFParents[tNeighbor] == LNULL) {
            continue;
          }

          // check regions
            if(Find(tCurrent, UFParents) != Find(tNeighbor, UFParents)) {
      
              // TREE->ADDARC
              if (tCurrent != Find(tCurrent, UFParents))
                printf("WRONG ASSUMPTIONS!!!\n");
              tAugmentedTree->add_arc(Find(tNeighbor, UFParents),
                                      Find(tCurrent, UFParents));
              tNumAllRegions++;
              Union(tCurrent, tNeighbor, UFParents);
            }
        }
        if (tNumAllRegions != 1 || boundaryMax) {
          // its a restricted critical point
          // TREE->MARKEXTREMUM
          tAugmentedTree->mark_critical(tCurrent, boundaryMax);
        }

        mBS->advance();
      }

      // cleanup
      
      // TREE->OUTPUT TO STREAM
      tAugmentedTree->output_to_stream(stream, tField, &tDomain);

            // **Writing local trees for Debugging purposes**
            //char fileName[32] = {0};
            //MergeTree* mergetree = &tree;
            //sprintf(fileName, "local_tree_%d.dot", mergetree->id());
            //FILE* fp = fopen(fileName, "w");
      //tAugmentedTree->write_to_file(fp, &tDomain);

      delete[] UFParents;
      return 1;
  }
};




////! The API to compute a local tree
//class DenseMergeTreeBuilder : public LocalComputeAlgorithm
//{
//protected:
//
//  BoxSorter* mBS;
//public:
//
//
//
//  //! Default constructor
//  DenseMergeTreeBuilder(bool invert=false) : LocalComputeAlgorithm(invert)  {
//    printf("Making a new DenseMergeTreeBuilder\n");
//  }
//
//  //! Destructor
//  virtual ~DenseMergeTreeBuilder() {
//  }
//
//  //! Apply this algorithm and store the resulting tree
//  virtual int apply( Patch* patch, const int field, SegmentedMergeTree& tree,
//    TopoOutputStream* stream, FunctionType bottom_range = sMinFunction,
//    FunctionType top_range = sMaxFunction) {
//
//      // get the right field
//      // and also the right domain, and number of vertices
//      const FunctionType* tField = patch->field(field);
//      SubBox& tDomain = patch->domain();
//      const LocalIndexType num_vertices = tDomain.localSize(); 
//    
//      // initialize neighborhood information
//      Neighborhood neigh(tDomain);
//
//      // this is where we select sorting modes
//      // for now just use a pre-sorted box sorter (using stl::sort)
//
//      mBS = new PreFilteredPreSortedBoxSorter(tField, &tDomain, ! this->mInvert, sMinFunction, sMaxFunction);
//
//      //printf("DOING (%d,%d,%d)--(%d,%d,%d)\n",
//      //tDomain.lowCorner()[0],tDomain.lowCorner()[1],tDomain.lowCorner()[2],
//      //tDomain.highCorner()[0]-1,tDomain.highCorner()[1]-1,tDomain.highCorner()[2]-1);
//
//
//      // temp storage of full augmented MT
//      MergeTreeLite* tAugmentedTree = new BasicMergeTreeLite(num_vertices);
//
//      // allocate space to store the Union-Find data structure
//      LocalIndexType* UFParents = new LocalIndexType[num_vertices];
//      memset(UFParents, LNULL, sizeof(LocalIndexType) * num_vertices);
//
//      // initialize sorting
//      mBS->begin();
//      while( mBS->valid() ) {
//
//        LocalIndexType tCurrent = mBS->value();
//        FunctionType tVal = tField[tDomain.fieldIndex(tCurrent)];
//
//        //printf("doing: %d-%f\n", tCurrent, tVal);
//        // early terminate if we're outside limits
//        if (this->mInvert && tVal > top_range) break;
//        if (! this->mInvert && tVal < bottom_range) break;
//
//        //printf("g%d\n", tCurrent);
//        // make a set in the Union-Find Data structure
//        UFParents[tCurrent] = tCurrent;
//        // TREE->ADDNODE
//        tAugmentedTree->add_node(tCurrent);
//
//        PointIndex tCurrentPID = tDomain.coordinates(tCurrent);
//        // if this vertex is boundary, first compute its "boundary" criticality
//        
//        int tBoundaryCount = tDomain.boundaryCount(tCurrentPID);
//        bool boundaryMax = false;
//        if (tBoundaryCount != 0) {
//          
//          boundaryMax = true;
//          // iterate over neighbors skipping non-sub-boundary
//          NeighborIterator it = neigh.begin(tCurrent);
//          while (neigh.valid(it)) {
//            LocalIndexType tNeighbor = neigh.value(it);
//            neigh.advance(it);
//          
//                 // printf("c%d dn%d\n", tCurrent, tNeighbor);
//
//            // if there is a "seen" sub-boundary neighbor, then this is NOT a boundary max
//            if (UFParents[tNeighbor] != LNULL && tDomain.subBoundary(tCurrentPID, tNeighbor)) {
//              boundaryMax = false;
//              break;
//        
//            }
//
//
//
//          }
//
//          
//        }
//           
//        // test whether or not this point is critical by 
//        // counting number of "regions" in "lower" link
//        // iterate over 26-neighborhood
//        
//        LocalIndexType tNumAllRegions = 0;
//        NeighborIterator it = neigh.begin(tCurrent);
//        while (neigh.valid(it)) {
//          LocalIndexType tNeighbor = neigh.value(it);
//          neigh.advance(it);
//          
//          // skip over things later in ordering or "not seen"
//          if (UFParents[tNeighbor] == LNULL) {
//            continue;
//          }
//
//          // check regions
//            if(Find(tCurrent, UFParents) != Find(tNeighbor, UFParents)) {
//      
//              // TREE->ADDARC
//              if (tCurrent != Find(tCurrent, UFParents)) printf("WRONG ASSUMPTIONS!!!\n");
//              tAugmentedTree->add_arc(Find(tNeighbor, UFParents), Find(tCurrent, UFParents));
//              tNumAllRegions++;
//              Union(tCurrent, tNeighbor, UFParents);
//            }
//
//        }
//        if (tNumAllRegions != 1 || boundaryMax) {
//          // its a restricted critical point
//          // TREE->MARKEXTREMUM
//          tAugmentedTree->mark_critical(tCurrent, boundaryMax);
//        }
//
//        mBS->advance();
//      }
//
//      // cleanup
//      
//      // TREE->OUTPUT TO STREAM
//      tAugmentedTree->output_to_stream(stream, tField, &tDomain);
//
//      delete[] UFParents;
//      return 1;
//
//  }
//};
#include "MergeTreeBuilder.h"






#endif /* LOCALCOMPUTEALGORITHM_H_ */
