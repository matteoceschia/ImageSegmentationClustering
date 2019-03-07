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

// ROOT
#include <TGraph2DErrors.h>
#include <Fit/Fitter.h>
#include <Fit/FitResult.h>

#define DUMMY 11111

// MetaInfo translates uniquely to Geiger Hit
struct MetaInfo {int side; int row; int column; double z;};
typedef std::pair<int, int> Pixel;  // x=column, y=row
typedef std::pair<Pixel, double> GGHit;  // x=column, y=row, double z


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
  void bfsPaths(int start, int target); // all paths between s and t
  bool singleNode(int node); // is a single node (one edge only) if true
  std::unordered_set<int> nodes(); // return node container as set
  std::vector<std::vector<int>> paths() {return allPaths;}  
};



// main cluster algorithm in this object
class GraphClusterer3D {

private:
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters;
  std::unordered_map<unsigned int, std::vector<MetaInfo> > finalcls;

  int side;
  int width;
  int height;
  double zres;
  std::set<GGHit> vertices; // unique nodes set
  std::vector<GGHit> nodes; // countable container for indexing
  std::set<std::pair<GGHit, GGHit> > edges; // node connections
  std::unordered_map<int, std::vector<std::vector<int> > > store; // node indices with col key

  // functions
  void make_edges(double maxdiff);
  bool is_neighbour(GGHit start, GGHit target, double maxdiff);
  
protected:
  std::unordered_map<unsigned int, std::vector<MetaInfo> > cluster_withgraph(double maxdiff); // uses store and cls
  void remove_copies();
  std::unordered_map<unsigned int, std::vector<MetaInfo> > translate(std::list<std::vector<std::vector<int> > >& temp);
  std::vector<int> all_deadends(Graph gg);
  std::vector<int> column_nodes(Graph gg, int col);
  std::vector<std::vector<int> > cluster1D(std::vector<int>& idx, double maxdiff);
  void preventLumped(std::set<int>& indices);
  std::vector<int> check_xwall(Graph gg);

public:

  GraphClusterer3D(int w, int h) {
    side = 0;
    width=w; 
    height=h;
    zres = 10.0;
  }
  ~GraphClusterer3D() {
    finalcls.clear();
    clusters.clear();
    nodes.clear();
    vertices.clear();
    edges.clear();
  }
  void setZResolution(double res) {zres = res;} // resolution in z [mm]

  void cluster(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls);
  std::unordered_map<unsigned int, std::vector<MetaInfo> > getClusters() {return finalcls;}
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


//****************************
// preliminary clustering in z
// split up structures in z 
// - cleans up confusion in imaging
//****************************
class ZClusterer {

private:
  double stepwidth; // half resolution in z
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters;
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clustercopy; // for modifications


protected:
  void zSplit(unsigned int clsid, std::vector<MetaInfo>& cls);
  double histogramSplit(std::valarray<double>& zdata, double start, double end);
  void zSplitCluster(unsigned int id, double zlimit);
  double splitFinder(std::vector<int>& hist);

public:

  ZClusterer() {
    stepwidth = 10.0; // minimum z-step half-width
  } // default constructor

  ~ZClusterer() {
    clusters.clear();
  }

  // init and setting functions
  void init(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls); // full info clusters
  void setZResolution(double res) {stepwidth = res;} // resolution in z

  // action functions
  void zSplitter(); // remove projection effects for clusters from image

  // output, cleaned cluster collection
  std::unordered_map<unsigned int, std::vector<MetaInfo> > getClusters() {return clustercopy;}

};



//****************************
// modify cluster map 
// SimpleFit acts as filter
// to select clusters according
// to geometry models, here
// a line in 3D and a helix.
// If these models are not 
// fit cleanly then the cluster
// can be deselected.
// Uses tracker data only.
//****************************
// store the more complex fit result data 
class FitLine {
 private:
  const ROOT::Fit::FitResult* res;
  // store internally
  double c2;
  double p;
  double xyi;
  double xys;
  double xzi;
  double xzs;
  double err0;
  double err1;
  double err2;
  double err3;
  int covst;

 public: 
  FitLine() {res=0;} // default
  FitLine(const ROOT::Fit::FitResult& fr) {
    res = &fr;
    c2  = res->Chi2(); ;
    p   = res->Prob();
    xyi = res->Parameter(0);
    xys = res->Parameter(1);
    xzi = res->Parameter(2);
    xzs = res->Parameter(3);
    err0= res->ParError(0);
    err1= res->ParError(1);
    err2= res->ParError(2);
    err3= res->ParError(3);
    covst= res->CovMatrixStatus();//=0 not calculated, =1 approximated, =2 made pos def , =3 accurate
  }
  ~FitLine() {
  }
  
  double chi2() {return c2;}
  double prob() {return p;}
  
  double xyintercept() {return xyi;}
  double xyslope() {return xys;}
  double xzintercept() {return xzi;}
  double xzslope() {return xzs;}
  double xyierr() {return err0;}
  double xyserr() {return err1;}
  double xzierr() {return err2;}
  double xzserr() {return err3;}
  
  int covstatus() {return covst;}
};


class FitHelix {
 private:
  const ROOT::Fit::FitResult* res;
  double c2;
  double p;
  double xr;
  double yr;
  double zor;
  double om;
  double tanl;
  double bf;
  double err0;
  double err1;
  double err2;
  double err3;
  double err4;
  int covst;

 public:
  FitHelix() {res=0;} // default
  FitHelix(const ROOT::Fit::FitResult& fr) {
    res = &fr;
  // store internally
    c2  = res->Chi2(); ;
    p   = res->Prob();
    xr  = res->Parameter(0);
    yr  = res->Parameter(1);
    zor = res->Parameter(2);
    om  = res->Parameter(3);
    tanl= res->Parameter(4);
    bf  = res->Parameter(5);
    err0 = res->ParError(0);
    err1 = res->ParError(1);
    err2 = res->ParError(2);
    err3 = res->ParError(3);
    err4 = res->ParError(4);
    covst= res->CovMatrixStatus();//=0 not calculated, =1 approximated, =2 made pos def , =3 accurate
  }
  ~FitHelix() {
  }

  double chi2() {return c2;}
  double prob() {return p;}
  
  double xref() {return xr;}
  double yref() {return yr;}
  double z0() {return zor;}
  double omega() {return om;}
  double tanlambda() {return tanl;}
  double bfield() {return bf;}
  double xreferr() {return err0;}
  double yreferr() {return err1;}
  double z0err() {return err2;}
  double omegaerr() {return err3;}
  double tanlerr() {return err4;}
  
  int covstatus() {return covst;}
};


//**********************
// modify cluster map by
// removing clusters not 
// passing fit criteria.
//**********************
class SimpleFit {

private:
  double threshold; // pass for acceptable fit
  double bfield;    // fixed fit parameter
  double invradius; // critical fit parameter start value
  // fixed errors for fitting
  double errxy; // geiger cell error in indices
  double errz; // z error [mm]
  
  std::vector<unsigned int> accept; // cluster id's accepted
  std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters;

  // ROOT data container
  TGraph2DErrors* tgraph;
  ROOT::Fit::Fitter fitter;
  FitLine LineResult;
  FitHelix HelixResult;
  std::vector<std::pair<unsigned int, FitLine> > goodLineFits;
  std::vector<std::pair<unsigned int, FitHelix> > goodHelixFits;


protected:
  bool fitline(unsigned int clsid);
  bool fithelix(unsigned int clsid);


public:

  SimpleFit() {
    threshold = 0.01; // test chi2 probability value
    bfield = 0.0;
    invradius = 0.01;
    errxy = 0.5; // index value error
    errz = 9.0;  // genuine z-error
    tgraph = new TGraph2DErrors(); // construct only once, clear thereafter
  } // default constructor
  
  ~SimpleFit() {
    accept.clear();
    clusters.clear();
    goodLineFits.clear();
    goodHelixFits.clear();
    if (tgraph)
      delete tgraph;
  }

  // init and setting functions
  void init(std::unordered_map<unsigned int, std::vector<MetaInfo> >& cls); // full info clusters
  void setThreshold(double t) {threshold = t;} // probability threshold
  void setBfield(double t) {bfield = t;}
  void setOmega(double t) {invradius = t;}
  void setErrorXY(double t) {errxy = t;}
  void setErrorZ(double t) {errz = t;}

  // action functions
  void selectClusters(); // remove clusters not passing fitting criteria

  // output, cleaned cluster collection
  std::unordered_map<unsigned int, std::vector<MetaInfo> > getClusters();
  std::vector<std::pair<unsigned int, FitLine> > getLineFits() {return goodLineFits;}
  std::vector<std::pair<unsigned int, FitHelix> > getHelixFits() {return goodHelixFits;}

};

#endif
