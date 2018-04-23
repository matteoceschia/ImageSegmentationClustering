#ifndef YR_ImageSegmentation
#define YR_ImageSegmentation

// std libraries
#include <unordered_map>
#include <list>
#include <set>
#include <unordered_set>
#include <vector>
#include <valarray>
#include <utility>

// Eigen3
#include <Eigen/Dense>
using namespace Eigen;

#define DUMMY 11111

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


// Usage as helper class: Create complex Graph nodes or edges
// in a countable container independent of this graph class. Once finished
// with a static container content of those complex vertex objects,
// all it takes is to use the unique element index in that container
// to initate this graph and use it entirely with integers throughout.
// Any result from here can then address complex objects in their 
// container by index, again outside this class. 
class Graph
{
  // undirected, unweighted graph object using integers as Node identifiers
  // for simple usage as a pure helper class.
private:
  int V; // number of vertices
  int MAXINT;
  
  std::vector<std::unordered_set<int>> adj; // adjacency lists
  
  std::vector<std::vector<int>> allPaths; // result path
  std::vector<int> currentPath; // helper path
  
  void dfs(std::vector<std::unordered_set<int>>& prev,
	   int node); // depth-first search helper function
  
public:
  
  Graph(int nv); // Constructor with number of vertices
  
  void addEdge(int v, int w); // function to add an edge to graph
  bool isReachable(int s, int d); // returns true if there is a path from s to d
  std::vector<std::vector<int>> bfsPaths(int start, int target); // all paths between s and t
  bool singleNode(int node); // is a single node (one edge only) if true
  std::unordered_set<int> nodes(); // return node container as set
};



// Used for the more complex clustering challenges
class GraphClusterer {

private:

  GraphClusterer(){width=0; height=0;}
  int width;
  int height;
  std::unordered_map<unsigned int, std::list<Pixel> > cls;
  
  // index store of nodes with column map index where nodes hold pixel indices
  std::unordered_map<int, std::vector<std::vector<int> > > store;

  std::set<std::pair<int, int> > vertices; // unique nodes set
  std::vector<std::pair<int, int> > nodes; // countable container for indexing
  std::vector<std::pair<Pixel, Pixel> > edges; // node connections

  // functions
  void make_edges();
  bool nodes_connected(std::vector<int> va, std::vector<int> vb);

protected:
  void cluster_withgraph(); // uses store and cls
  void translate(std::list<std::vector<std::vector<int> > > temp);
  std::vector<int> all_deadends(Graph gg);
  std::vector<int> column_nodes(Graph gg, int col);
  std::vector<std::vector<int> > oneDcluster(std::vector<int> data);


public:

  GraphClusterer(int w, int h) {
    width=w; 
    height=h;
  }
  ~GraphClusterer() {
    cls.clear();
    store.clear();
    nodes.clear();
    vertices.clear();
    edges.clear();
  }

  void cluster(std::vector<bool> data); // input for GraphClusterer
  std::unordered_map<unsigned int, std::list<Pixel> > getClusters();
  std::vector<std::pair<int, int> > getNodes() {return nodes;}
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


//**********************
// modify cluster map by
// (a) remove projection effects in z - split clusters that only appear as one
// (b) remove clusters that are too non-straight with pca in 2d
//**********************
class ClusterCleanup {

private:
  double threshold; // PCA ratio threshold
  double stepwidth; // half resolution in z
  std::vector<unsigned int> accepted; // accepted cluster id's
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters;


protected:
  VectorXd pca2d(const Matrix<double, Dynamic, 2>& data, int points);
  void checkAcceptance(const VectorXd& ev, unsigned int id);
  void zSplit(unsigned int clsid, std::vector<MetaInfo>& cls);
  double histogramSplit(std::valarray<double>& zdata, double start, double end);
  void zSplitCluster(unsigned int id, double zlimit);
  void consolidate(); // use accepted ids to modify cluster map


public:

  ClusterCleanup() {
    threshold = 0.9; // 90% default pca axis ratio
    stepwidth = 5.0; // minimum z-step half-width
  } // default constructor

  ~ClusterCleanup() {
    accepted.clear();
    clusters.clear();
  }

  // init and setting functions
  void init(std::unordered_map<unsigned int, std::vector<MetaInfo> > cls); // full info clusters
  void setPCAAcceptanceThreshold(double thresh) {threshold = thresh;}
  void setZResolution(double res) {stepwidth = res*0.5;} // half resolution in z

  // action functions
  void zSplitter(); // remove projection effects for clusters from image
  void runPCAonImage(); // 2D PCA on image data

  // output, cleaned cluster collection
  std::unordered_map<unsigned int, std::vector<MetaInfo> > getClusters() {return clusters;}

};

#endif
