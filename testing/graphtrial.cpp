#include "catch.hpp"
#include <imagesegmentation_library.h>
#include <vector>
#include <list>
#include <unordered_map>
#include <utility>
// #include <iostream>

std::vector<bool> image() {
  // hard-coded image data provider
  // 6 rows, 5 columns
  int w = 5;
  int h = 6;
  unsigned int input[6][5] =  {{1,0,0,0,1},
			       {1,1,0,1,1},
			       {0,1,1,0,1},
			       {0,1,1,0,0},
			       {1,0,0,1,0},
			       {0,0,0,1,1}};

  std::vector<bool> image;
  for (int i=0;i<h;i++) 
    for (int j=0;j<w;j++) // row-wise store
      image.push_back((bool)input[i][j]);
  return image;
}


std::vector<bool> image2() {
  // hard-coded image data provider
  // 6 rows, 5 columns
  int w = 5;
  int h = 6;
  unsigned int input2[6][5]=  {{1,0,0,1,0},
			       {1,0,0,1,0},
			       {0,1,1,0,0},
			       {0,1,1,0,0},
			       {1,0,0,1,0},
			       {0,1,0,1,1}};
  std::vector<bool> image;
  for (int i=0;i<h;i++) 
    for (int j=0;j<w;j++) // row-wise store
      image.push_back((bool)input2[i][j]);
  return image;
}


unsigned int check_image(){
  std::vector<bool> im = image();
  return im.size();
}


int check_cls_nA(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = gcl.getClusters();

  int value = (int)cls.size(); 
  return value;
}

int check_cls_nB(){
  std::vector<bool> im = image2();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = gcl.getClusters();

  int value = (int)cls.size(); 
  return value;
}

int check_nodes(){
  std::vector<bool> im = image2();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::vector<std::pair<int, int> > nds = gcl.getNodes();
  int value = (int)nds.size(); 
  return value;
}

unsigned int check_node_content(){
  std::vector<bool> im = image2();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::vector<std::pair<int, int> > nds = gcl.getNodes();

  int value;
  for (auto& nd : nds) {
    if (nd.first == 4)
      value = nd.second;
  }
  return value;
}


unsigned int check_cluster_contentA(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = gcl.getClusters();

  std::list<Pixel> pl;
  int value;
  for (auto& entry : cls) {
    if (entry.first == 1)
      pl = entry.second;
  }
  value = (int)pl.size();
  return value;
}


unsigned int check_cluster_contentB(){
  std::vector<bool> im = image2();
  int w = 5;
  int h = 6;

  GraphClusterer gcl(w,h);
  gcl.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = gcl.getClusters();

  std::list<Pixel> pl;
  int value;
  for (auto& entry : cls) {
    if (entry.first == 10)
      pl = entry.second;
  }
  value = (int)pl.size();
  return value;
}


unsigned int check_iseg_chain(){
  std::vector<bool> im = image2();
  int w = 5;
  int h = 6;

  ImageSegmentation iseg(w,h);
  iseg.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = iseg.getClusters();
  unsigned int value = cls.size();

  return value;
}


TEST_CASE( "Graph Data in", "[falaise][iseglib][grdata_in]" ) {
  REQUIRE( check_image() == 30 );
}

TEST_CASE( "Graph N clustersA", "[falaise][graphcluster][nclustersA]" ) {
  REQUIRE( check_cls_nA() == 6 );
}

TEST_CASE( "Graph N clustersB", "[falaise][graphcluster][nclustersB]" ) {
  REQUIRE( check_cls_nB() == 10 );
}

TEST_CASE( "Graph nodes", "[falaise][graphcluster][nodes]" ) {
  REQUIRE( check_nodes() == 8 );
}

TEST_CASE( "Graph Node content", "[falaise][graphcluster][nodescontent]" ) {
  REQUIRE( check_node_content() == 0 );
}

TEST_CASE( "Graph cluster contentA", "[falaise][graphcluster][clustercontentA]" ) {
  REQUIRE( check_cluster_contentA() == 9 );
}

TEST_CASE( "Graph cluster contentB", "[falaise][graphcluster][clustercontentB]" ) {
  REQUIRE( check_cluster_contentB() == 7 );
}

TEST_CASE( "ISegmentation N clustersB", "[falaise][label][nclustersB]" ) {
  REQUIRE( check_iseg_chain() == 10 );
}

