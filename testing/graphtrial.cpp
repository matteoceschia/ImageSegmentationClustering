#include "catch.hpp"
#include <imagesegmentation_library.h>
#include <vector>
#include <unordered_map>

int pattern1()
{
//   pattern                     =  {{0,0,0,0,1},
// 			             {0,0,1,1,1},
// 			             {1,1,1,0,0},
// 			             {1,1,1,0,0},
// 			             {1,0,0,1,0},
// 			             {0,0,0,1,1}};
  //side, row, column, z = metainfo
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls[14][4]       = {{1,0,4,  0.0},
			          {1,1,2, -2.0},
			          {1,1,3, -1.0},
			          {1,1,4, -0.5},
			          {1,2,0, -4.0},
			          {1,2,1, -3.0},
			          {1,2,2, -2.0},
			          {1,3,0, -5.0},
			          {1,3,1, -6.0},
			          {1,3,2, -7.0},
			          {1,4,0, -5.0},
			          {1,4,3, -8.0},
			          {1,5,3, -9.0},
			          {1,5,4,-10.0}};

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

  int w = 5;
  int h = 6;
  GraphClusterer3D gcl(w,h);

  gcl.cluster(clscollection);
  std::unordered_map<unsigned int, std::vector<MetaInfo> > cls = gcl.getClusters();
  return extract(cls); // should be 11
}


int pattern2()
{
//   pattern                     =  {{0,0,0,0,0},
// 			             {0,0,0,0,0},
// 			             {1,1,1,1,1},
// 			             {1,1,1,1,1},
// 			             {0,0,0,0,1},
// 			             {0,0,0,0,0}};
  //side, row, column, z = metainfo
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls2[11][4]      = {{1,1,0,  0.0},
			          {1,1,1,  2.0},
			          {1,1,2,  6.0},
			          {1,1,3, 10.5},
			          {1,1,4, 14.0},
			          {1,2,0, -0.5},
			          {1,2,1,-110.0},
			          {1,2,2,-235.0},
			          {1,2,3,-350.0},
			          {1,2,4,-476.0},
			          {1,3,4,-488.0}};

  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  for (int i=0;i<11;i++) {
    mi.side   = inputcls2[i][0];
    mi.row    = inputcls2[i][1];
    mi.column = inputcls2[i][2];
    mi.z      = inputcls2[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster

  int w = 5;
  int h = 6;
  GraphClusterer3D gcl(w,h);

  gcl.cluster(clscollection);
  std::unordered_map<unsigned int, std::vector<MetaInfo> > cls = gcl.getClusters();
  return extract(cls); // should be 9
}


int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls) {
  // access map
  int nentries=0;
  std::vector<MetaInfo> milist;
  for (auto& entry : cls) {
    if (entry.first == 3) { // cluster number
      milist = entry.second;
      nentries = (int)milist.size();
    }
  }
  return nentries;
}



TEST_CASE( "Cluster A", "[falaise][graphtest][nclustersA]" ) {
  REQUIRE( pattern1() == 11 );
}

TEST_CASE( "Cluster B", "[falaise][graphtest][nclustersB]" ) {
  REQUIRE( pattern2() == 9 );
}
