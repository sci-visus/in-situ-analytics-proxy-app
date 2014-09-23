#ifndef BOXSORTER
#define BOXSORTER

#include "Patch.h"
#include "Neighborhood.h"
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

template<typename T>
class Comparator {
public:
	virtual bool before(T a, T b)=0;
};



class BoxSorter : public Comparator<LocalIndexType> {

protected:

	const FunctionType* mField;
	SubBox* mDomain;
	const bool mUseGreater;

public:

	BoxSorter(const FunctionType* field, SubBox* domain, bool useGreater) :
	  mField(field), mDomain(domain), mUseGreater(useGreater) {

	  }

	  virtual void begin() {
	  }

	  virtual LocalIndexType count() { return mDomain->localSize(); }
	  virtual LocalIndexType value() {
		  return 0;
	  }

	  virtual bool valid() {
		  return false;
	  }

	  virtual void advance() {
	  }

      bool before(LocalIndexType a, LocalIndexType b) {
          return this->operator()(a,b);
      } 

	  bool operator()(LocalIndexType a, LocalIndexType b) {
		  a = mDomain->fieldIndex(a);
		  b = mDomain->fieldIndex(b);
		  if (mField[a] < mField[b])
			  return true ^ mUseGreater;
		  if (mField[b] < mField[a])
			  return false ^ mUseGreater;
		  return (a < b) ^ mUseGreater;
		  // return mUseGreater ^ mLT(a, b);
	  }

};

class PreSortedBoxSorter: public BoxSorter {

protected:
	vector<LocalIndexType> mSortedIndices;
	LocalIndexType mNumVertices;

	LocalIndexType mPos;

public:

	PreSortedBoxSorter(const FunctionType* field, SubBox* domain, bool useGreater) :
	  BoxSorter(field, domain, useGreater) {
		  mNumVertices = domain->localSize();
		  mSortedIndices.resize(mNumVertices);
		  for (uint32_t i = 0; i < mNumVertices; i++) {
			  mSortedIndices[i] = i; // push back each index
		  }
		  // this's comparator now utilizes the usegreater. 
		  std::sort(mSortedIndices.begin(), mSortedIndices.end(), *this);
	  }

	  virtual LocalIndexType count() { return mNumVertices;}
	  virtual void begin() {
		  this->mPos = 0;
	  }

	  virtual LocalIndexType value() {
		  return mSortedIndices[mPos];
	  }

	  virtual bool valid() {
		  return mPos < mNumVertices;
	  }

	  virtual void advance() {
		  mPos++;
	  }

};


class PreFilteredPreSortedBoxSorter: public BoxSorter {

protected:
	FunctionType mLow;
	FunctionType mHigh;
	vector<LocalIndexType> mSortedIndices;
	LocalIndexType mNumVertices;

	LocalIndexType mPos;

public:

	PreFilteredPreSortedBoxSorter(const FunctionType* field, SubBox* domain, 
		bool useGreater, FunctionType low, FunctionType high) :
	BoxSorter(field, domain, useGreater) {
		mLow = low;
		mHigh = high;
		mNumVertices = domain->localSize();
		mSortedIndices.clear();
		for (uint32_t i = 0; i < mNumVertices; i++) {
			FunctionType val = this->mField[this->mDomain->fieldIndex(i)];	  
			if(mLow < val && val < mHigh) mSortedIndices.push_back(i); // push back each index
		}
		std::sort(mSortedIndices.begin(), mSortedIndices.end(), *this);


	}

	virtual LocalIndexType count() { return mSortedIndices.size();}

	virtual void begin() {
		this->mPos = 0;
	}

	virtual LocalIndexType value() {
		return mSortedIndices[mPos];
	}

	virtual bool valid() {
		return mPos < mSortedIndices.size();
	}

	virtual void advance() {
		mPos++;
	}

};

class PreSortedPriorityQueueSorter: public BoxSorter {

protected:
  struct HP { 
    FunctionType f; 
    LocalIndexType lid;

  };
  struct HPCOMPARE {	
    bool operator() (const HP &a, const HP &b) {
    if (a.f < b.f)
      return true ;
    if (b.f < a.f)
      return false ;
    return (a.lid < b.lid) ;
   }
  };

  priority_queue<HP, vector<HP>, HPCOMPARE> mSortedIndices;
  LocalIndexType mNumVertices;
  LocalIndexType mPos;
public:
  PreSortedPriorityQueueSorter(const FunctionType* field, SubBox* domain, bool useGreater) :
  BoxSorter(field, domain, useGreater) {
    mNumVertices = domain->localSize();
    //printf("start sort\n");
    for (uint32_t i = 0; i < mNumVertices; i++) {
      HP asdf; asdf.lid = i; asdf.f = mField[domain->fieldIndex(i)];
      mSortedIndices.push(asdf);
    }
    //printf("end sort\n");
  }

 	 

  virtual LocalIndexType count() { return mNumVertices;}
  virtual void begin() {
  }

  virtual LocalIndexType value() {
    return mSortedIndices.top().lid; //[mPos].lid;
  }

  virtual bool valid() {
    return ! mSortedIndices.empty(); //mPos < mNumVertices;
  }

  virtual void advance() {
    mSortedIndices.pop(); //mPos++;
  } 

};
 //mSortedIndices[i].lid = i; // push back each index
 // //mSortedIndices[i].f = mField[domain->fieldIndex(i)]; // push back each index
 //  }


//class LiveSortingBoxSorter: public BoxSorter {
//
//protected:
//	class idfpair {
//	public:
//		LocalIndexType mid; FunctionType mval;
//		idfpair(LocalIndexType id, FunctionType val) :
//		  mid(id), mval(val){}
//		  struct lt {
//			  bool compare(idfpair &a, idfpair &b) {
//			  if (a.mval < b.mval) return true;
//			  if (b.mval < a.mval) return false;
//			  return a.mid < b.mid;
//			 }
//		  };
//		  struct gt {
//			  bool compare(idfpair &a, idfpair &b) {
//				if (a.mval > b.mval) return true;
//				if (b.mval > a.mval) return false;
//				return a.mid > b.mid;		
//			  }
//		  };
//	};
//
//	priority_queue<idfpair, idfpair::gt> mGreaterSortedIndices;
//	priority_queue<idfpair, idfpair::lt> mSmallerSortedIndices;
//  LocalIndexType mNumVertices;
//
//  LocalIndexType mPos;
//  	FunctionType mLow;
//	FunctionType mHigh;
//	Neighborhood mNeigh;
//
//	bool isLocalMin(idfpair current) {
//		
//		for(NeighborIterator it = mNeigh.begin(current.mid); mNeigh.valid(it); mNeigh.advance(it)) {
//			LocalIndexType tNeighbor = mNeigh.value(it);
//			idfpair negp(tNeighbor, this->mField[this->mDomain->fieldIndex(tNeighbor)];
//			if (idfpair::lt::compare(negp, current)) return false;	
//		}
//	}
//
//public:
//
//
//
//
//
//  LiveSortingBoxSorter(const FunctionType* field, SubBox* domain, bool useGreater,
//	  FunctionType low, FunctionType high) : mNeigh(domain), 
//    BoxSorter(field, domain, useGreater) {
//
//		mLow = low;
//		mHigh = high;
//	  for (uint32_t i = 0; i < mNumVertices; i++) {
//		  FunctionType val = this->mField[this->mDomain->fieldIndex(i)];	  
//		  if(mLow < val && val < mHigh) mSortedIndices.push_back(i); // push back each index
//		}
//    if (this->mUseGreater) {
//		for (uint32_t i = 0; i < mNumVertices; i++) {
//		  FunctionType val = this->mField[this->mDomain->fieldIndex(i)];	  
//		  if(mLow < val && val < mHigh) mSortedIndices.push_back(i); // push back each index
//		}
//
//
//
//
//      std::sort(mSortedIndices.begin(), mSortedIndices.end(), this->mGT);
//    } else {
//      std::sort(mSortedIndices.begin(), mSortedIndices.end(), this->mLT);
//    }
//
//  }
//
//	virtual LocalIndexType count() { return mNumVertices;}
//  virtual void begin() {
//	this->mPos = 0;
//  }
//
//  virtual LocalIndexType value() {
//    return mSortedIndices[mPos];
//  }
//
//  virtual bool valid() {
//    return mPos < mNumVertices;
//  }
//
//  virtual void advance() {
//    mPos++;
//  }
//
//};




#endif
