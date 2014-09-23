#ifndef NEIGHBORHOOD
#define NEIGHBORHOOD




struct NeighborIterator{
	int position;
	LocalIndexType baseID;
	int boundary;
};

const int neighborList[27][3] = {
	{ 0,0,0 },
	{ -1,-1,-1 },{ 0,-1,-1 },{ 1,-1,-1 },
	{ -1, 0,-1 },{ 0, 0,-1 },{ 1, 0,-1 },
	{ -1, 1,-1 },{ 0, 1,-1 },{ 1, 1,-1 },
	
	{ -1,-1, 0 },{ 0,-1, 0 },{ 1,-1, 0 },
	{ -1, 0, 0 },            { 1, 0, 0 },
	{ -1, 1, 0 },{ 0, 1, 0 },{ 1, 1, 0 },

	{ -1,-1, 1 },{ 0,-1, 1 },{ 1,-1, 1 },
	{ -1, 0, 1 },{ 0, 0, 1 },{ 1, 0, 1 },
	{ -1, 1, 1 },{ 0, 1, 1 },{ 1, 1, 1 }
};

class Neighborhood {




	LocalIndexType mOffsetList[27];

	const Box& mBox;

	bool insideBox(LocalIndexType id, int pos) {
      PointIndex p= mBox.coordinates(id);
	  for (int i = 0; i < 3; i++) p[i] += neighborList[pos][i];
	  return mBox.insideBox(p);
	}

public:

	Neighborhood(const Box& box) : mBox(box) { 
		for (int i = 0; i < 27; i++) {
			mOffsetList[i] = neighborList[i][0] + 
				neighborList[i][1] * mBox.localDimensions()[0] +
            neighborList[i][2] * mBox.localDimensions()[0] *
            mBox.localDimensions()[1];
		}
		//++ NEED LOCAL DIMENSIONS HERE AVAILABLE
	}

	virtual void initialize(){
   
   
   }

	virtual NeighborIterator begin(LocalIndexType index) {
		NeighborIterator it;
		it.position = 0;
		it.baseID = index;
      PointIndex p = mBox.coordinates(index);
     // printf("begin on (%d, %d, %d)\n", p[0], p[1], p[2]);
		it.boundary = mBox.boundaryCount(index);
		
		// now get first valid position
		advance(it);
		return it;
	}
	virtual void advance(NeighborIterator& it) {
		
		if (! valid(it)) return;
		if (it.boundary > 0) {
			it.position++;

        // printf("usin bounday coxe\n");
			while (valid(it) &&
				! insideBox(it.baseID, it.position)) {
					it.position++;
			}
		} else {
			it.position++;
		}
      PointIndex p = mBox.coordinates(it.baseID+ mOffsetList[it.position]);
     // printf("n=(%d,%d,%d)\n", p[0], p[1], p[2]);
	
	}
	virtual bool valid(NeighborIterator& it) { 
		return it.position < 27; // hardcoded neighborhood
	}
	virtual LocalIndexType value(NeighborIterator& it) {
    //  PointIndex p = mBox.coordinates(it.baseID+ mOffsetList[it.position]);
    //  printf("n=(%d,%d,%d)\n", p[0], p[1], p[2]);
		return it.baseID + mOffsetList[it.position];
	}



};


#endif
