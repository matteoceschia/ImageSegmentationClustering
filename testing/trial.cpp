#include "catch.hpp"
#include <imagesegmentation_library.h>
#include <vector>
#include <list>
#include <unordered_map>

std::vector<bool> image() {
  // hard-coded image data provider
  // 6 rows, 5 columns
  unsigned int input[6][5] =  {{1,0,0,0,1},
			       {1,1,0,1,1},
			       {0,1,0,0,1},
			       {0,1,1,0,0},
			       {1,0,0,1,0},
			       {0,0,0,1,1}};
  std::vector<bool> image;
  for (int i=0;i<h;i++) 
    for (int j=0;j<w;j++) // row-wise store
      image.push_back((bool)input[i][j]);
  return image;
}


unsigned int check_image(){
  std::vector<bool> im = image();
  return im.size();
}


int check_label_n(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageLabel labels(w,h);
  labels.label(im);
  int value = labels.nlabels();
  return value;
}

unsigned int check_label_collection(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageLabel labels(w,h);
  labels.label(im);
  std::list<std::vector<bool> > c = labels.imagecollection();
  return c.size();
}

bool check_label_split(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageLabel labels(w,h);
  bool value = labels.is_splitting(im);
  return value;
}

int check_label_n_split(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageLabel labels(w,h);
  if (labels.is_splitting(im))
    std::vector<int> values = labels.split_at();
  else
    std::vector<int> values; // empty
  unsigned int value = values.size();
  return value;
}

unsigned int check_label_labels(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageLabel labels(w,h);
  labels.label(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cmap = labels.getLabels();
  std::list<Pixel> plist;
  for (std::unordered_map<unsigned int, std::list<Pixel> >::iterator it=cmap.begin(); it!=cmap.end(); ++it) {
    if (it->first == 2)
      plist = it->second;
  }
  unsigned int value = plist.size()
  return value;
}


unsigned int check_iseg_clusters(){
  std::vector<bool> im = image();
  int w = 5;
  int h = 6;
  ImageSegmentation iseg(w,h);
  iseg.cluster(im);
  std::unordered_map<unsigned int, std::list<Pixel> > cls = iseg.getClusters();
  unsigned int value = cls.size()
  return value;
}


TEST_CASE( "Data in", "[falaise][iseglib][data_in]" ) {
  REQUIRE( check_image() == 30 );
}

TEST_CASE( "N labels", "[falaise][label][nlabel]" ) {
  REQUIRE( check_label_n() == 2 );
}

TEST_CASE( "label image collection", "[falaise][label][imcollection]" ) {
  REQUIRE( check_label_collection() == 2 );
}

TEST_CASE( "Is splitting", "[falaise][label][split]" ) {
  REQUIRE( check_label_split() == true );
}

TEST_CASE( "N splits", "[falaise][label][nsplits]" ) {
  REQUIRE( check_label_n_split() == 1 );
}

TEST_CASE( "members in second cluster", "[falaise][label][labels]" ) {
  REQUIRE( check_label_labels() == 10 );
}

TEST_CASE( "N clusters", "[falaise][iseglib][cluster]" ) {
  REQUIRE( check_iseg_clusters() == 3 );
}

