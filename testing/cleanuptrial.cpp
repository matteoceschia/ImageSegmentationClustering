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
  double inputcls[14][4]       = {{1,0,4, 22.0},
			          {1,1,2, 22.0},
			          {1,1,3, 21.0},
			          {1,1,4, 21.0},
			          {1,2,0, 20.0},
			          {1,2,1, 20.0},
			          {1,2,2, 20.0},
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

  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.zSplitter(); // method A for clean up
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPartA =  clclean.getClusters();
  return extract(clsPartA); // should be 7 entries
}


int check_zsmall_overlapping()
{
  // z overlapping tracks, same image pattern, no big z jumps
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls2[14][4]      = {{1,0,4, 22.0},
			          {1,1,2, 13.0},
			          {1,1,3, 17.0},
			          {1,1,4, 20.0},
			          {1,2,0,  2.0},
			          {1,2,1,  5.0},
			          {1,2,2,  9.0},
			          {1,3,0,  2.0},
			          {1,3,1, -1.0},
			          {1,3,2, -5.0},
			          {1,4,0,  1.0},
			          {1,4,3, -9.0},
			          {1,5,3,-13.0},
			          {1,5,4,-16.0}};
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  for (int i=0;i<14;i++) {
    mi.side   = inputcls2[i][0];
    mi.row    = inputcls2[i][1];
    mi.column = inputcls2[i][2];
    mi.z      = inputcls2[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster
  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.zSplitter(); // method A for clean up
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPartB =  clclean.getClusters();
  return extract(clsPartB); // should be 10
}


int check_zsteep_overlapping()
{
  // large z overlapping tracks, same image pattern
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls3[14][4]      = {{1,0,4, 2200.0},
			          {1,1,2, 1300.0},
			          {1,1,3, 1700.0},
			          {1,1,4, 1200.0},
			          {1,2,0,  200.0},
			          {1,2,1,  500.0},
			          {1,2,2,  900.0},
			          {1,3,0,  200.0},
			          {1,3,1, -100.0},
			          {1,3,2, -500.0},
			          {1,4,0,  200.0},
			          {1,4,3, -900.0},
			          {1,5,3,-1300.0},
			          {1,5,4,-1600.0}};
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  for (int i=0;i<14;i++) {
    mi.side   = inputcls3[i][0];
    mi.row    = inputcls3[i][1];
    mi.column = inputcls3[i][2];
    mi.z      = inputcls3[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster
  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.zSplitter(); // method A for clean up
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPartC =  clclean.getClusters();
  return extract(clsPartC); // should be 8
}


int check_zright_overlapping()
{
  // IMAGE PATTERN 2
//   unsigned int inputcls[6][5] =  {{1,0,0,0,0},
// 			             {1,1,1,0,0},
// 			             {0,0,1,1,1},
// 			             {0,0,1,1,1},
// 			             {0,1,1,1,0},
// 			             {1,1,0,0,0}};
  // large z overlapping tracks, second image pattern
  int extract(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  double inputcls4[15][4]      = {{1,0,0, 2200.0},
			          {1,1,0, 2150.0},
			          {1,1,1, 1700.0},
			          {1,1,2, 1650.0},
			          {1,2,2, 1100.0},
			          {1,2,3,  600.0},
			          {1,2,4,  200.0},
			          {1,3,2, -200.0},
			          {1,3,3,   50.0},
			          {1,3,4,  200.0},
			          {1,4,1, -400.0},
			          {1,4,2, -250.0},
			          {1,4,3,  -50.0},
			          {1,5,0, -600.0},
			          {1,5,1, -500.0}};
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clscollection;
  std::vector<MetaInfo> store;
  MetaInfo mi;
  for (int i=0;i<15;i++) {
    mi.side   = inputcls4[i][0];
    mi.row    = inputcls4[i][1];
    mi.column = inputcls4[i][2];
    mi.z      = inputcls4[i][3];
    store.push_back(mi); 
  }
  clscollection[1] = store; // just one cluster
  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.zSplitter(); // method A for clean up
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPartD =  clclean.getClusters();
  return extract(clsPartD); // should be 9
}


int check_pca1()
{
  double inputcls[14][4]       = {{1,0,4, 22.0},
			          {1,1,2, 22.0},
			          {1,1,3, 21.0},
			          {1,1,4, 21.0},
			          {1,2,0, 20.0},
			          {1,2,1, 20.0},
			          {1,2,2, 20.0},
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
  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.runPCAonImage();
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPCA =  clclean.getClusters();
  return (int) clsPCA.size(); // no cluster passes pca test 90% threshold default
}


int check_pca2()
{
  double inputcls[14][4]       = {{1,0,4, 22.0},
			          {1,1,2, 22.0},
			          {1,1,3, 21.0},
			          {1,1,4, 21.0},
			          {1,2,0, 20.0},
			          {1,2,1, 20.0},
			          {1,2,2, 20.0},
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
  ClusterCleanup clclean;
  clclean.init(clscollection); // data stored and all prepared
  clclean.setPCAAcceptanceThreshold(0.78); // 78% threshold
  clclean.runPCAonImage();
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clsPCA =  clclean.getClusters();
  return (int) clsPCA.size(); // one cluster passes pca test
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

TEST_CASE( "Cluster clean B", "[falaise][clustercleanup][nclustersB]" ) {
  REQUIRE( check_zsmall_overlapping() == 10 );
}

TEST_CASE( "Cluster clean C", "[falaise][clustercleanup][nclustersC]" ) {
  REQUIRE( check_zsteep_overlapping() == 8 );
}

TEST_CASE( "Cluster clean D", "[falaise][clustercleanup][nclustersD]" ) {
  REQUIRE( check_zright_overlapping() == 9 );
}

TEST_CASE( "PCA test 1", "[falaise][clustercleanup][PCAtest]" ) {
  REQUIRE( check_pca1() == 0 );
}

TEST_CASE( "PCA test 2", "[falaise][clustercleanup][PCAtest2]" ) {
  REQUIRE( check_pca2() == 1 );
}

