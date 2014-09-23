

#ifndef UNIONFIND_H
#define UNIONFIND_H

#include "DistributedDefinitions.h"
#include <map>

template<typename IndexType, class IndexTypeComparator>
class UnionFind {
public:
	virtual IndexType Find(IndexType index) { return 0; }
	virtual IndexType Union(IndexType a, IndexType b) { return 0; }
	virtual void MakeSet(IndexType index) {}
	virtual bool Exists(IndexType index) { return 0; }
};


template<typename IndexType, class IndexTypeComparator>
class CompactUnionFind : public UnionFind<IndexType, IndexTypeComparator> {
protected:
	IndexTypeComparator &mComp;
	IndexType* mParents;
	IndexType mNumElements;
public:

	CompactUnionFind(IndexType numElements, IndexTypeComparator &comp) : 
	  mComp(comp), mNumElements(numElements) {
		mParents = new IndexType[numElements];
		for (IndexType i = 0; i < numElements; i++) mParents[i] = (IndexType) LNULL;
	}

	~CompactUnionFind() {
		delete[] mParents;
	}

	bool Exists(IndexType index) {
		return mParents[index] != LNULL;
	}

	IndexType Find(IndexType index) {
		if (mParents[index] == index){
			return index;	// root of tree
		}
		IndexType result = Find(mParents[index]);
		mParents[index] = result; // path compression
		return result;
	}

	IndexType Union(IndexType a, IndexType b) {
		IndexType aroot = Find(a);
		IndexType broot = Find(b);
		if (aroot == broot) { 
			return aroot;
		}

		if (mComp.before(broot,aroot)) {
			mParents[broot] = aroot;
			return aroot;
		}
		else {
			mParents[aroot] = broot;
			return broot;
		}
	}

	void MakeSet(IndexType index) {
		mParents[index] = index;
	}
};


using namespace std;
template<typename IndexType, class IndexTypeComparator>
class SparseUnionFind : public UnionFind<IndexType, IndexTypeComparator> {
protected:
	IndexTypeComparator &mComp;
	map<IndexType, IndexType> mParents;
public:

	SparseUnionFind(IndexTypeComparator &comp) : mComp(comp) {}
	~SparseUnionFind() {}

	IndexType Find(IndexType index) {
		if (mParents[index] == index){
			return index;	// root of tree
		}
		IndexType result = Find(mParents[index]);
		mParents[index] = result; // path compression
		return result;
	}

	bool Exists(IndexType index) {
		return mParents.count(index);
	}

	IndexType Union(IndexType a, IndexType b) {
		IndexType aroot = Find(a);
		IndexType broot = Find(b);
		if (aroot == broot) { 
			return aroot;
		}

		if (mComp.before(broot,aroot)) {
			mParents[broot] = aroot;
			return aroot;
		}
		else {
			mParents[aroot] = broot;
			return broot;
		}
	}
	void MakeSet(IndexType index) {
		mParents[index] = index;
	}
};




#endif /* UNIONFIND_H */
