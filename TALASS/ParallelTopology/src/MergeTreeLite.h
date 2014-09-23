#ifndef MERGE_TREE_LITE
#define MERGE_TREE_LITE

#include "DistributedDefinitions.h"
#include "TopoOutputStream.h"
#include "Patch.h"
#include <vector>
#include <cstdlib>


using namespace std;
struct NodeLite {
public:
  LocalIndexType up;
  LocalIndexType down;
  LocalIndexType next;

};


class MergeTreeLite {

public:

  MergeTreeLite() {}

  virtual void initialize() {}

	virtual void add_node(LocalIndexType id){}

	virtual void mark_critical(LocalIndexType id, bool boundary_max) {}

	virtual void add_arc(LocalIndexType up, LocalIndexType down){}

	virtual void output_to_stream(TopoOutputStream* stream, const FunctionType* field, SubBox* domain) {}

    virtual void write_to_file(FILE* fp, SubBox* domain) {}


};

class BasicMergeTreeLite : public MergeTreeLite {
protected:
	const LocalIndexType mNumVertices;
	LocalIndexType* mSegmentation;
	LocalIndexType* mDown;
	char* mFlags;

public:

	BasicMergeTreeLite(LocalIndexType num_vertices) : mNumVertices(num_vertices) {
		mSegmentation = new LocalIndexType[num_vertices];
		mDown = new LocalIndexType[num_vertices];
		mFlags = new char[num_vertices];
		this->initialize();
	}

	~BasicMergeTreeLite() {
	  delete[] mSegmentation;
	  delete[] mDown;
	  delete[] mFlags;
	}

	void setBoundaryMax(LocalIndexType i, bool tf) {
		if (tf) {
			mFlags[i] = 1; // |= 0x10;
		} else {
			mFlags[i]  = 0; //&= 0xef;
		}
	}
	bool getBoundaryMax(LocalIndexType i) {
		return (bool) mFlags[i]; //& 0xef;
	}



	void initialize() {
		for (LocalIndexType i = 0; i < mNumVertices; i++) {
			mSegmentation[i] = LNULL;
			mDown[i] = -1; //LNULL;
			mFlags[i] = 0;
		}
	}

	void add_node(LocalIndexType id){ 
		//printf("add_node: %d\n", id);
		mSegmentation[id] = id; 
      mDown[id] = -1;
   }

	void add_arc(LocalIndexType up, LocalIndexType down){
	  //printf("add_arc: %d %d\n", up, down);
		mSegmentation[down] = mSegmentation[up];
		mDown[up] = down;
	}

	void mark_critical(LocalIndexType id, bool boundary_max) { 
		mSegmentation[id] = id; 
		this->setBoundaryMax(id, boundary_max);
	
	}

	void output_to_stream(TopoOutputStream* stream, const FunctionType* field, SubBox* domain) {

		//bool* visited = new bool[mNumVertices];
		//memset(mNumVertices, 0, sizeof(bool)*mNumVertices);
		vector<LocalIndexType> finalize;

		// output the nodes
		for (LocalIndexType i = 0; i < mNumVertices; i++) {
		  //printf("%d: %d %d\n", i, mSegmentation[i], mDown[i]);
			// if its extremum or saddle segment will be index,
			// if its a global min, will have segment but no down
			if (mSegmentation[i] == LNULL) continue;
			if (mSegmentation[i] == i || 
				(mSegmentation[i] != LNULL && mDown[i] == LNULL)) {
					Token::VertexToken vt;
					vt.index(domain->globalIndex(i));
					vt.value(field[domain->fieldIndex(i)]);
          if (vt.index() == 66126)
            fprintf(stderr,"break\n");
          if (getBoundaryMax(i)) {
					  vt.multiplicity(1 << (domain->boundaryCount(i) - domain->globalBoundaryCount(i)));
					} else {
					  vt.multiplicity(1);
					}
					vt.outstanding(vt.multiplicity());
					// not true -> NOTE THAT MULTIPLICITY IS CURRENTLY BROKEN
					*stream << vt;
					finalize.push_back(vt.index());
					/*
					if (vt.multiplicity() > 1) {
						PointIndex p = domain->gCoordinates(i);
							printf("v: %d (%d, %d, %d) %d-%d %d %d %d %d %f\n", vt.index(), 
								p[0], p[1], p[2],
								domain->boundaryCount(i), domain->globalBoundaryCount(i), 
								i, vt.multiplicity(), mSegmentation[i], mDown[i], vt.value());
					}
					*/
					if (vt.index() == 66126)
					  fprintf(stderr,
					          "v: %d %d %d      %d %d %f\n", vt.index(),i,vt.multiplicity(), mSegmentation[i], mDown[i], vt.value());
			}
		}

		// output the edges
		for (LocalIndexType i = 0; i < mNumVertices; i++) {
			// if down-> segment is not the same as my segment or down is null, edge
			if (mSegmentation[i] == LNULL) continue;
			if (mSegmentation[i] != LNULL && mDown[i] == LNULL) {
					Token::EdgeToken et;
					et.source(domain->globalIndex(mSegmentation[i]));
					et.destination(domain->globalIndex(i));
					*stream << et;
					//printf("eo: %d -> %d\n", domain->globalIndex(mSegmentation[i]), domain->globalIndex(i));

			} else if (mSegmentation[i] != mSegmentation[mDown[i]]) {
					Token::EdgeToken et;
					et.source(domain->globalIndex(mSegmentation[i]));
					et.destination(domain->globalIndex(mDown[i]));
					*stream << et;
					//printf("e: %d -> %d\n", domain->globalIndex(mSegmentation[i]), domain->globalIndex(mDown[i]));
			}
		}
		for (uint32_t i = 0; i < finalize.size(); i++) {
			Token::FinalToken ft;
			ft.index(finalize[i]);
			*stream << ft;
		}

		Token::EmptyToken et;
		*stream << et;

		stream->flush();

	}

    void write_to_file(FILE* fp, SubBox* domain){

        fprintf(fp, "digraph G {\n");
        
        printf( "OUTPUTING LOCAL TREE!!\n");
        for (LocalIndexType i = 0; i < mNumVertices; i++) {

			if (mSegmentation[i] == LNULL) continue;
            if (mSegmentation[i] != LNULL && mDown[i] == LNULL) continue;

            if (mSegmentation[i] != mSegmentation[mDown[i]]) {
				fprintf(fp, "%d -> %d\n", domain->globalIndex(mSegmentation[i]), domain->globalIndex(mDown[i]));
        
            }
        }
        fprintf(fp, "}");
        fclose(fp);
    }
};


//class OutStreamingMergeTreeLite : public MergeTreeLite {
//protected:
//	const LocalIndexType mNumVertices;
//	LocalIndexType* mSegmentation;
//	LocalIndexType* mDown;
//	TopoOutputStream* mStream;
//
//public:
//
//	OutStreamingMergeTreeLite(LocalIndexType num_vertices, TopoOutputStream* stream) : mNumVertices(num_vertices) {
//		mSegmentation = new LocalIndexType[num_vertices];
//		mDown = new LocalIndexType[num_vertices];
//		mStream = stream;
//	}
//
//	void initialize() {
//		for (LocalIndexType i = 0; i < mNumVertices; i++) {
//			mSegmentation[i] = LNULL;
//			mDown[i] = -1; //LNULL;
//		}
//	}
//
//
//	void add_node(LocalIndexType id){ 
//		printf("add_node: %d\n", id);
//		mSegmentation[id] = id; 
//      mDown[id] = -1;
//   }
//
//	void add_arc(LocalIndexType up, LocalIndexType down){
//      printf("add_arc: %d %d\n", up, down);
//		mSegmentation[down] = mSegmentation[up];
//		mDown[up] = down;
//	}
//
//	void mark_critical(LocalIndexType id, bool boundary_max) { 
//		mSegmentation[id] = id; 
//		//this->setBoundaryMax(id, boundary_max);
//	
//	}
//
//	void output_to_stream(TopoOutputStream* stream, const FunctionType* field, SubBox* domain) {
//
//		//bool* visited = new bool[mNumVertices];
//		//memset(mNumVertices, 0, sizeof(bool)*mNumVertices);
//		vector<LocalIndexType> finalize;
//
//		// output the nodes
//		for (LocalIndexType i = 0; i < mNumVertices; i++) {
//         printf("%d: %d %d\n", i, mSegmentation[i], mDown[i]);
//			// if its extremum or saddle segment will be index,
//			// if its a global min, will have segment but no down
//			if (mSegmentation[i] == i || 
//				(mSegmentation[i] != LNULL && mDown[i] == LNULL)) {
//					Token::VertexToken vt;
//					vt.index(domain->globalIndex(i));
//					vt.value(field[domain->fieldIndex(i)]);
//					//if (getBoundaryMax(i)) {
//					//	vt.multiplicity(1 << (domain->boundaryCount(i) - domain->globalBoundaryCount(i)));
//					//} else {
//					//	vt.multiplicity(1);
//					//}
//					// not true -> NOTE THAT MULTIPLICITY IS CURRENTLY BROKEN
//					*stream << vt;
//					finalize.push_back(vt.index());
//               printf("v: %d %d %d %d %f\n", vt.index(),i, mSegmentation[i], mDown[i], vt.value());
//			}
//		}
//
//		// output the edges
//		for (LocalIndexType i = 0; i < mNumVertices; i++) {
//			// if down-> segment is not the same as my segment or down is null, edge
//			if (mSegmentation[i] != LNULL && mDown[i] == LNULL) {
//					Token::EdgeToken et;
//					et.source(domain->globalIndex(mSegmentation[i]));
//					et.destination(domain->globalIndex(i));
//					*stream << et;
//               printf("eo: %d -> %d\n", 
//                     domain->globalIndex(mSegmentation[i]),
//                     domain->globalIndex(i));
//
//			} else if (mSegmentation[i] != mSegmentation[mDown[i]]) {
//					Token::EdgeToken et;
//					et.source(domain->globalIndex(mSegmentation[i]));
//					et.destination(domain->globalIndex(mDown[i]));
//					*stream << et;
//               printf("e: %d -> %d\n", 
//                     domain->globalIndex(mSegmentation[i]),
//                     domain->globalIndex(mDown[i]));
//			}
//		}
//		for (int i = 0; i < finalize.size(); i++) {
//			Token::FinalToken ft;
//			ft.index(finalize[i]);
//			*stream << ft;
//		}
//
//	}
//
//};
//


#endif
