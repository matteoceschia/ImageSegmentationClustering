#include "catch.hpp"
#include <imagesegmentation_library.h>
#include <iostream>
#include <vector>
#include <utility>
#include <unordered_map>


std::vector<int> testcollection()
{
  // line
//   pattern                     =  {{0,0,0,0,1},
// 			             {0,0,1,1,1},
// 			             {1,1,1,0,0},
// 			             {1,1,1,0,0},
// 			             {1,0,0,0,0},
// 			             {0,0,0,0,0}};
  //side, row, column, z = metainfo
  std::vector<int> coll;

  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  SimpleFit sfit;

  double inputcls[11][4]       = {{1,0,4,  0.0},
			          {1,1,2, -2.0},
			          {1,1,3, -1.0},
			          {1,1,4, -0.5},
			          {1,2,0, -4.0},
			          {1,2,1, -3.0},
			          {1,2,2, -2.0},
			          {1,3,0, -5.0},
			          {1,3,1, -4.0},
			          {1,3,2, -4.0},
			          {1,4,0, -5.0}};

  for (int i=0;i<11;i++) {
    mi.side   = inputcls[i][0];
    mi.row    = inputcls[i][1];
    mi.column = inputcls[i][2];
    mi.z      = inputcls[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster

  // helix
//   pattern                     =  {{0,0,0,1,1,0,0,0},
// 			             {0,0,0,0,1,1,0,0},
// 			             {0,0,0,0,0,1,1,0},
// 			             {0,0,0,0,0,1,1,0},
// 			             {0,0,0,0,0,1,1,0},
// 			             {0,0,0,0,0,1,1,0},
// 			             {0,0,0,0,1,1,0,0},
// 			             {0,0,0,1,1,0,0,0},
// 			             {0,0,1,1,0,0,0,0},
// 			             {1,1,1,0,0,0,0,0},
// 			             {1,0,0,0,0,0,0,0}};
  //side, row, column, z = metainfo
  double inputcls2[22][4]      = {{1,0,3,  0.0},
			          {1,0,4,  0.0},
			          {1,1,4,  1.0},
			          {1,1,5,  1.0},
			          {1,2,5,  2.0},
			          {1,2,6,  2.0},
			          {1,3,5,  3.0},
			          {1,3,6,  3.0},
			          {1,4,5,  3.0},
			          {1,4,6,  3.0},
			          {1,5,5,  3.5},
			          {1,5,6,  3.5},
			          {1,6,4,  4.0},
			          {1,6,5,  4.0},
			          {1,7,3,  5.0},
			          {1,7,4,  5.0},
			          {1,8,2,  6.0},
			          {1,8,1,  6.0},
			          {1,9,0,  7.5},
			          {1,9,1,  7.0},
			          {1,9,2,  7.0},
			          {1,10,0, 8.0}};

  store.clear();
  for (int i=0;i<22;i++) {
    mi.side   = inputcls2[i][0];
    mi.row    = inputcls2[i][1];
    mi.column = inputcls2[i][2];
    mi.z      = inputcls2[i][3];
    store.push_back(mi); 
  }
  clscollection[2] = store; // another cluster

  // use SimpleFit
  sfit.init(clscollection);
  sfit.setThreshold(0.9);
  sfit.setOmega(0.1);
  sfit.setBfield(1.0);
  sfit.setErrorXY(0.5);
  sfit.setErrorZ(5.0); // [mm]
  sfit.selectClusters();

  std::vector<std::pair<unsigned int, FitLine> > glf = sfit.getLineFits();
  std::vector<std::pair<unsigned int, FitHelix> > ghf = sfit.getHelixFits();
  coll.push_back((int)glf.size()); // n good lines
  coll.push_back((int)ghf.size()); // n good helices

  std::unordered_map<unsigned int, std::vector<MetaInfo> > filtered = sfit.getClusters();
  coll.push_back((int)filtered.size()); // n good clusters

  return coll;
}

int test1() {
  std::vector<int> coll = testcollection();
  return coll.at(0);
}

int test2() {
  std::vector<int> coll = testcollection();
  return coll.at(1);
}

int test3() {
  std::vector<int> coll = testcollection();
  return coll.at(2);
}

TEST_CASE( "Cluster A", "[falaise][fittest][nclustersA]" ) {
  REQUIRE( test1() == 2 );
}

TEST_CASE( "Cluster B", "[falaise][fittest][nclustersB]" ) {
  REQUIRE( test2() == 1 );
}

TEST_CASE( "Cluster C", "[falaise][fittest][nclustersC]" ) {
  REQUIRE( test3() == 2 );
}

