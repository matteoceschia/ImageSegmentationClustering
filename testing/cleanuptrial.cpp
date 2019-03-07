#include "catch.hpp"
#include <imagesegmentation_library.h>
#include <vector>
#include <unordered_map>

int check_z_separation()
{
  // IMAGE PATTERN
//   unsigned int inputcls[6][5] =  {{0,0,0,0,1},
// 			             {0,0,1,1,1},
// 			             {1,1,1,0,0},
// 			             {1,1,1,0,0},
// 			             {1,0,0,1,0},
// 			             {0,0,0,1,1}};
  //side, row, column, z = metainfo
  // z separate tracks
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls[14][4]       = {{1,0,4, 27.0},
			          {1,1,2, 26.0},
			          {1,1,3, 25.0},
			          {1,1,4, 24.0},
			          {1,2,0, 23.0},
			          {1,2,1, 22.0},
			          {1,2,2, 21.0},
			          {1,3,0,-10.0}, // jump in z
			          {1,3,1,-11.0},
			          {1,3,2,-12.0},
			          {1,4,0,-13.0},
			          {1,4,3,-14.0},
			          {1,5,3,-15.0},
			          {1,5,4,-16.0}};

  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  for (int i=0;i<14;i++) {
    mi.side   = inputcls[i][0];
    mi.row    = inputcls[i][1];
    mi.column = inputcls[i][2];
    mi.z      = inputcls[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster

  ZClusterer clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.setZResolution(3.0);
  clclean.zSplitter(); // method A for clean up
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPartA =  clclean.getClusters();
  return extract(clsPartA); // should be 7 entries
}


int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls) {
  // access map
  int nentries=0;
  std::vector<MetaInfo> milist;
  for (auto& entry : cls) {
    if (entry.first == 2) { // have two clusters
      milist = entry.second;
      nentries = (int)milist.size();
    }
  }
  return nentries;
}



TEST_CASE( "Cluster clean A", "[falaise][clustercleanup][nclustersA]" ) {
  REQUIRE( check_z_separation() == 7 );
}

