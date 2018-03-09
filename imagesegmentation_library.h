#ifndef YR_ImageSegmentation
#define YR_ImageSegmentation

// std libraries
#include <unordered_map>
#include <list>
#include <vector>
#include <utility>


// MetaInfo translates uniquely to Geiger Hit
struct MetaInfo {int side; int row; int column;};
struct Pixel {int x; int y;}; // x=column, y=row


// genuine image segmentation method
class ImageLabel {

private:

  ImageLabel(){width=0; height=0;}
  int width;
  int height;
  std::vector<bool> image; // store as 1D array
  std::vector<unsigned int> component;
  std::vector<int> splitcolumn;
  std::vector<std::pair<int, int> > splits;
  Pixel pp;
  std::unordered_map<unsigned int, std::list<Pixel> > componentMap;
  std::unordered_map<unsigned int, std::list<Pixel> > partial_left; // for splits
  std::unordered_map<unsigned int, std::list<Pixel> > partial_right;
  
  void _doUnion(unsigned int a, unsigned int b);
  void _unionCoords(unsigned int x, unsigned int y, unsigned int x2, unsigned int y2);

protected:

  void setImage(std::vector<bool>);
  void setWidth(int w){width = w;}
  void label();
  

public:

  ImageLabel(int w, int h){width=w; height=h;}
  ~ImageLabel() {
    component.clear();
    image.clear();
    componentMap.clear();
  }

  void label(std::vector<bool> data);
  unsigned int nlabels() {return componentMap.size();}
  std::unordered_map<unsigned int, std::list<Pixel> > getLabels();

  bool is_splitting(std::vector<bool> data); // checks splitting, creates split points
  std::vector<int> split_at() {return splitcolumn;}
  std::vector<std::pair<int, int> > splitpoints() {return splits;}
  std::unordered_map<unsigned int, std::list<Pixel> > getLeftLabels() {return partial_left;}
  std::unordered_map<unsigned int, std::list<Pixel> > getRightLabels() {return partial_right;}

  std::list<std::vector<bool> > imagecollection(); // image collection of all separate labels
};


// Used for the more complex clustering challenges
class GraphClusterer {

private:

  GraphClusterer(){width=0; height=0;}
  int width;
  int height;
  std::unordered_map<unsigned int, std::list<Pixel> > cls;
  
protected:


public:

  GraphClusterer(int w, int h) {
    width=w; 
    height=h;
  }
  ~GraphClusterer() {
  }

  void cluster(std::vector<bool> data); // input for GraphClusterer
  std::unordered_map<unsigned int, std::list<Pixel> > getClusters();
};


// Used for simple clustering 
class ImageSegmentation {

private:

  ImageSegmentation(){width=0; height=0;}
  int width;
  int height;
  std::unordered_map<unsigned int, std::list<Pixel> > cls;
  std::vector<std::unordered_map<unsigned int, std::list<Pixel> > > clscollection;
  ImageLabel* labels;
  GraphClusterer* gr;
  
protected:

  std::unordered_map<unsigned int, std::list<Pixel> > merge_single_to_multi(std::unordered_map<unsigned int, std::list<Pixel> > ll, std::unordered_map<unsigned int, std::list<Pixel> > rr);

public:

  ImageSegmentation(int w, int h) {
    width=w; 
    height=h;
    labels = new ImageLabel(width, height); // have labeling ready
    gr = new GraphClusterer(width, height); // have complex clustering ready
  }
  ~ImageSegmentation() {
    clscollection.clear();
    cls.clear();
    if (labels)
      delete labels;
    if (gr)
      delete gr;
  }

  void cluster(std::vector<bool> data); // input for ImageLabel
  std::unordered_map<unsigned int, std::list<Pixel> > getClusters(); // ready for image2gg
};


// convert tracker data to images and back
class GG2ImageConverter {

private:

  GG2ImageConverter(){hwidth=0; height=0;}
  int hwidth;
  int height;
  std::vector<bool> leftimage; // store as 1D array
  std::vector<bool> rightimage; // store as 1D array

protected:

public:

  // full-tracker image 113 x 18 = height x width
  GG2ImageConverter(int w, int h){hwidth=(int)w*0.5; height=h;}
  ~GG2ImageConverter() {
    leftimage.clear(); // 113 x 9 image tracker half
    rightimage.clear();  // 113 x 9 image
  }

  // provide input to imagelabel object
  void gg2image(std::vector<MetaInfo> data);
  std::vector<bool> getLeft() {return leftimage;}
  std::vector<bool> getRight() {return rightimage;}

  // clusters must come from imagelabel object
  std::unordered_map<unsigned int, std::vector<MetaInfo> > image2gg(std::vector<MetaInfo> data, std::unordered_map<unsigned int, std::list<Pixel> > labels, int side);

};

#endif
