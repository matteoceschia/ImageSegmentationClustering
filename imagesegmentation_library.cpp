// std libraries
#include <queue>
#include <cmath>
#include <algorithm>
#include <iostream>

// this
#include <imagesegmentation_library.h>

// *****
// ** Imagesegmentation methods
// *****
void ImageSegmentation::cluster(std::vector<bool> data) {
  if (clscollection.size())
    clscollection.clear();

  labels->label(data); // create labels from ImageLabel object

  // get the image collection of separate structures
  std::list<std::vector<bool> > icollection = labels->imagecollection();

  // test splitting with an image input

  for (auto& im : icollection) { // for all images in the collection
    labels->label(im); // splitting overwrites labels
    cls = labels->getLabels(); // book them now for simplest single cluster

    if (labels->is_splitting(im)) {
      // check first for all complex segmentation
      std::cout << "use graph." << std::endl;

      gr->cluster(im); // fill clusters
      cls = gr->getClusters();

    }
    // simple clusters already in cls
    clscollection.push_back(cls); // basket collection
  }
}



std::unordered_map<unsigned int, std::list<Pixel> > ImageSegmentation::getClusters() {
  // re-order the cluster numbering
  std::unordered_map<unsigned int, std::list<Pixel> > countedcls;
  unsigned int counter = 1; // count clusters from 1
  for (auto& x : clscollection) { // maps x in a vector
    for (auto& y : x) {           // loop through map entries y, make new keys
      countedcls[counter] = y.second;
      counter++;
    }
  }
  return countedcls; // ready for image2gg
}





// *****
// ** Graph clustering methods
// *****
void GraphClusterer::cluster(std::vector<bool> data) {
  if (cls.size())
    cls.clear();
  std::vector<int> oneDdata;
  std::vector<std::vector<int> > cluster1D;
  
  for (int col=0; col < width; col++) { // loop all columns
    for (int row=0; row < height; row++)
      if (data[row*width + col]) // image comes as boolean vector
	oneDdata.push_back(row);
    
    cluster1D = oneDcluster(oneDdata);
    store[col] = cluster1D; // store in map of nodes
    oneDdata.clear(); // ready for next column
    cluster1D.clear();
  }
  // check store
//   for (auto& entry : store) {
//     std::cout << "Store key = " << entry.first << std::endl;
//     for (auto& nds : entry.second)
//       for (int nd : nds)
// 	std::cout << "row: " << nd << std::endl;
//   }
  cluster_withgraph(); // uses store and outputs in cls data member
  remove_copies(); // clear out cluster store of identical copies.
}


void GraphClusterer::remove_copies() {
  std::set<unsigned int> removal_keys; // only unique entries
  std::list<Pixel> target;
  for (auto& entry : cls) {
    unsigned int key1 = entry.first; // compare this
    for (unsigned int i=key1+1; i<cls.size();i++) { // to the rest in the container
      target = cls[i];
      if (std::equal(target.begin(), target.end(), entry.second.begin()))
	removal_keys.insert(key1); // remove equal copies of clusters, only unique keys here
    }
  }
  std::unordered_map<unsigned int, std::list<Pixel> > newcls;
  std::set<unsigned int>::iterator findit;
  unsigned int counter = 1;
  for (auto& entry : cls) {
    findit = std::find(removal_keys.begin(), removal_keys.end(), entry.first);
    if (findit == removal_keys.end()) { // found no key in the removal set
      newcls[counter] = entry.second; // book this clusters
      counter++; // new key
    }
  }
  std::cout << "cluster size " << cls.size() << " and after copy removal " << newcls.size() << std::endl;
  cls.clear(); // replace current cluster container
  cls = newcls; // store the cleaned copy
}


std::unordered_map<unsigned int, std::list<Pixel> > GraphClusterer::getClusters() {
  return cls;
}


void GraphClusterer::cluster_withgraph() {
  // first fill edges container, then use graph
  make_edges(); // vector<pair<pixel,pixel>> in edges container
  int entries = (int)nodes.size();
  
  Graph gg(entries);
  // fill graph with index integers corresponding to nodes container
  for (std::pair<Pixel, Pixel>& edge : edges) {
    std::pair<int,int> start_node = std::make_pair(edge.first.first, edge.first.second);
    std::pair<int,int> end_node = std::make_pair(edge.second.first, edge.second.second);
    
    std::vector<std::pair<int,int>>::iterator it1 = std::find(nodes.begin(), nodes.end(), start_node);
    std::vector<std::pair<int,int>>::iterator it2 = std::find(nodes.begin(), nodes.end(), end_node);
    
    // must be found since nodes were made from edges first.
    int start_pos = it1 - nodes.begin();
    int end_pos = it2 - nodes.begin();
    //    std::cout << "Edge index of node start: " << start_pos << " index end:" << end_pos << std::endl;
    gg.addEdge(start_pos, end_pos);
  } // graph filled and operative
  
  // get the start and target nodes for the path search
  std::vector<int> dead_ends = all_deadends(gg);
  std::vector<int> starts = column_nodes(gg, 0); // column 0 for starts
  std::vector<int> targets = column_nodes(gg, width-1); // column width-1 for targets

  // curving paths to consider
  if (targets.empty())
    targets = starts;
  else if (starts.empty())
    starts = targets;

//   for (int idx : dead_ends)
//     std::cout << "dead_end index: " << idx << " ";
//   std::cout << std::endl;
//   for (int idx : starts)
//     std::cout << "starts index: " << idx << " ";
//   std::cout << std::endl;
//   for (int idx : targets)
//     std::cout << "targets index: " << idx << " ";
//   std::cout << std::endl;

  // curving paths to consider again
  starts.insert(starts.end(), targets.begin(), targets.end());
  targets.insert(targets.end(), starts.begin(), starts.end());

  // merge starts and targets with dead ends
  starts.insert(starts.end(), dead_ends.begin(), dead_ends.end());
  targets.insert(targets.end(), dead_ends.begin(), dead_ends.end());

//   for (int idx : starts)
//     std::cout << "All starts index: " << idx << " ";
//   std::cout << std::endl;
//   for (int idx : targets)
//     std::cout << "All targets index: " << idx << " ";
//   std::cout << std::endl;

  // find paths from starts to targets to form clusters  
  std::list<std::vector<std::vector<int> > > tempCluster;
  std::pair<int, int> nodeS;
  std::pair<int, int> nodeT;
  for (int s : starts) {
    nodeS = nodes.at(s);
    for (int t : targets) {
      nodeT = nodes.at(t);
      if (gg.isReachable(s, t)) {
	if (s != t) {
	  gg.bfsPaths(s, t);
	  tempCluster.push_back(gg.paths());
	}
      }
    }
  }
  translate(tempCluster); // fill cls map in suitable format
}
  

void GraphClusterer::translate(std::list<std::vector<std::vector<int> > > temp) {
  // return the path indexes back into nodes
  // and nodes back into a collection of pixels
  // in order to form proper clusters. 
  // Such clusters can then be treated by
  // the image2gg utility.

  unsigned int counter = 1; // cluster numbering from 1
  Pixel pp;
  std::list<Pixel> pixels;
  std::pair<int, int> node;
  std::vector<std::vector<int> > cluster1D;
  std::vector<int> rows;

  for (auto& entry : temp) { // vector<vector<int>>
    for (auto& path : entry ) { // vector<int>
      for (int index : path) {
	node = nodes.at(index); // got the node in a path
	cluster1D = store[node.first]; // unravel the store map, key is column
	rows = cluster1D.at(node.second); // select collection of rows in column
	for (int row : rows) {
	  pp.first = node.first; // column in image
	  pp.second = row;        // row in image
	  pixels.push_back(pp); 
	} // collected all pixels for that node
      } // keep collecting pixels for every node on path
      cls[counter] = pixels; // got all pixels, store as cluster
      pixels.clear(); 
      counter++; // ready for next cluster
    }
  } // count up all clusters in the temp collection as distinct clusters.
}



void GraphClusterer::make_edges() {
  // turn the store container content into container of edges
  int index = 0;
  int indexNext = 0;
  Pixel start; // to hold the column as x and node index as y
  Pixel target; // to hold the column as x and node index as y
  edges.clear();
  nodes.clear();
  vertices.clear();

  //  std::cout << "In make edges:" << std::endl;
  std::vector<std::vector<int>> layer = store[0]; // start with key=column 0

  for (unsigned int col=1; col<store.size(); col++) {
    std::vector<std::vector<int>> entry = store[col];
    for (auto& nodevector : layer) { // vector<int> from vector<vector<int>>
      for (auto& nextnode : entry) {
	if (nodes_connected(nodevector, nextnode)) {
	  start.first = col - 1;
	  start.second = index;
	  target.first = col;
	  target.second = indexNext;
	  vertices.insert(std::make_pair(start.first, start.second)); // unique nodes only
	  vertices.insert(std::make_pair(target.first, target.second)); // 
	  edges.push_back(std::make_pair(start,target));
	}
	indexNext++;
      }
      index++;
      indexNext = 0;
    }
    index = 0;
    indexNext = 0;
    layer = entry; // next layer
  }
  // turn vertices to countable nodes container
  for (auto& nd : vertices)
    nodes.push_back(nd);

  // check
//   std::cout << "Full Edge container: " << std::endl;
//   for (auto& edge : edges) {
//     Pixel st = edge.first;
//     Pixel ta = edge.second;
//     std::cout << "Edge: " << st.first << ", " << st.second << " ;T= "<< ta.first << ", " << ta.second << std::endl;
//   }
//   std::cout << "Full Node container: " << std::endl;
//   int counter = 0;
//   for (auto& nd : nodes) {
//     std::cout << "Node " << counter << ": S= " << nd.first << ", " << nd.second << std::endl;
//     counter++;
//   }
}


bool GraphClusterer::nodes_connected(std::vector<int> va, std::vector<int> vb) {
  // check for overlap in rows of container content

  std::vector<int> output1(va.size()+vb.size()); // max+max output container size
  std::vector<int> output2(va.size()+vb.size()); // there are 3 tests to perform
  std::vector<int> output3(va.size()+vb.size()); // 

  std::sort(va.begin(), va.end()); // complete sort of vector
  std::sort(vb.begin(), vb.end());

  // shifted array
  std::vector<int> p1;
  std::vector<int> m1;
  for (int entry : va) {
    p1.push_back(entry+1);
    m1.push_back(entry-1);
  }

  // now can apply set_intersection on vector
  std::vector<int>::iterator test1 = std::set_intersection(va.begin(), va.end(), vb.begin(), vb.end(), output1.begin());
  output1.resize(test1 - output1.begin());
  if (output1.size()>0) // direct overlap
    return true;

  std::vector<int>::iterator test2 = std::set_intersection(p1.begin(), p1.end(), vb.begin(), vb.end(), output2.begin());
  output2.resize(test2 - output2.begin());
  if (output2.size()>0) // neighbour overlap
    return true;

  std::vector<int>::iterator test3 = std::set_intersection(m1.begin(), m1.end(), vb.begin(), vb.end(), output3.begin());
  output3.resize(test3 - output3.begin());
  if (output3.size()>0) // neighbour overlap
    return true;

  return false;
}

 

std::vector<int> GraphClusterer::all_deadends(Graph gg) {
  std::vector<int> ends;
  std::pair<int, int> node;
  for (int index : gg.nodes()) {
    node = nodes.at(index); // translate back to node from container
    if (gg.singleNode(index)) // loose end?
      if (node.first != 0 && node.first != width-1) // not at the extreme columns
	ends.push_back(index);
  }

  return ends;
}



std::vector<int> GraphClusterer::column_nodes(Graph gg, int col) {
  std::vector<int> found;
  std::pair<int, int> node;
  for (int index : gg.nodes()) {
    node = nodes.at(index); // translate back to node from container
    if (node.first == col)
      found.push_back(index);
  }

  return found;
}



std::vector<std::vector<int> > GraphClusterer::oneDcluster(std::vector<int> oneDdata) {
  // input vector of row indices with pixels set to On
  std::vector<std::vector<int> > cluster1D;
  if (oneDdata.size()<1) return cluster1D; // empty case

  std::vector<int> node;
  int row = oneDdata[0]; // starting point
  for (int& entry : oneDdata) {
    if ((entry - row) < 2) { // nearest neighbour
      node.push_back(entry); // first entry is trivially in
      row = entry;
    }
    else { // distance > 1 => next cluster
      cluster1D.push_back(node);
      node.clear();
      node.push_back(entry);
      row  = entry;
    }
  }
  cluster1D.push_back(node); // save the final node collection
  return cluster1D;
}


// *****
// ** Image Label methods
// *****
void ImageLabel::label(std::vector<bool> data) {
  setImage(data);
  label();
}


std::list<std::vector<bool> > ImageLabel::imagecollection() {
  std::list<std::vector<bool> > collection;
  if (!componentMap.size())
    return collection; // return empty if no labeling has taken place before.

  std::vector<bool> im;
  for (auto& x : componentMap) {
    for (int i=0;i<height;i++)
      for (int j=0;j<width;j++)
	im.push_back(false); // zero image, correct size
  
    std::list<Pixel> pixels = x.second; // extract pixels
    for (auto& pp : pixels) 
      im[pp.second * width + pp.first] = true;

    collection.push_back(im);
    im.clear();
  }
  return collection;
}


std::unordered_map<unsigned int, std::list<Pixel> > ImageLabel::getLabels() {
  std::unordered_map<unsigned int, std::list<Pixel> > countedlabels;
  unsigned int counter = 1; // count labels from 1
  for (auto& x: componentMap) { // make new counted keys for labels
    countedlabels[counter] = x.second;
    counter++;
  }
  return countedlabels;
}


void ImageLabel::setImage(std::vector<bool> data) {
  if (image.size())
    image.clear();
  image = data; // deep copy
}



bool ImageLabel::is_splitting(std::vector<bool> data) {
  std::unordered_map<unsigned int, std::list<Pixel> > dummy_left; // for splits
  std::unordered_map<unsigned int, std::list<Pixel> > dummy_right;

  if (splits.size()) {
    splits.clear();
    splitcolumn.clear();
  }
  bool split = false;
  std::vector<bool> left;
  std::vector<bool> right;
  int nleft, nright;
  int copywidth = width;

  // slice image at column
  for (int cut=1; cut<copywidth; cut++) {
    for (int j=0;j<height;j++)
      for (int i=0;i<cut;i++) 
	left.push_back(data[j*copywidth + i]);
    for (int j=0;j<height;j++)
      for (int i=cut;i<copywidth;i++)
      	right.push_back(data[j*copywidth + i]);
    setImage(left);
    setWidth(cut);
    label();
    nleft = (int)nlabels();
    dummy_left = getLabels();
    setImage(right);
    setWidth(copywidth - cut);
    label();
    nright = (int)nlabels();
    dummy_right = getLabels();
    // fill the splitpoints
    splits.push_back(std::make_pair(nleft, nright)); // counts from 0

    if (nleft>0 && nright>0) { // any data at all?
      if(nleft>nright || nleft<nright) { // not equal somewhere = a split
	split = true;
	splitcolumn.push_back(cut-1); // access container from 0 index
	if (splitcolumn.size()<2) {
	  partial_left = dummy_left; // store in case of single split,
	  partial_right = dummy_right; // first split only
	}
      }
      if(nleft>1 && nright>1) { // both sides too much structure
	split = true;
	splitcolumn.push_back(cut-1); // access container from 0 index
      }
    }
    //    std::cout << "split points: (" << nleft << "," << nright << "); split=" << split << std::endl;
    left.clear();
    right.clear();
    dummy_left.clear();
    dummy_right.clear();
  }
  setWidth(copywidth);
  return split;
}



void ImageLabel::label() {
  componentMap.clear();
  component.clear();
  for (int i = 0; i < width*height; i++)
    component.push_back(i);

  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++)
      {
        _unionCoords(x, y, x+1, y);
        _unionCoords(x, y, x, y+1);
        _unionCoords(x, y, x-1, y+1); // left diagonal
        _unionCoords(x, y, x+1, y+1); // right diagonal
      }
  for (int x = 0; x < width; x++)
    {
      for (int y = 0; y < height; y++)
        {
	  if (!image[y*width + x])
	    continue;
	  int c = y*width + x;
	  while (component[c] != c) c = component[c];
	  
	  pp.first = x; // column
	  pp.second = y; // row
	  componentMap[c].push_back(pp);
        }
    }
}


void ImageLabel::_doUnion(unsigned int a, unsigned int b) {
  // get the root component of a and b, and set the one's parent to the other
  while (component[a] != a)
    a = component[a];
  while (component[b] != b)
    b = component[b];
  component[b] = a;
}


void ImageLabel::_unionCoords(unsigned int x, unsigned int y, unsigned int x2, unsigned int y2) {
  if (y2 < height && x2 < width && image[y*width + x] && image[y2*width + x2])
    _doUnion(y*width + x, y2*width + x2);
}



// *****
// ** Utility Geiger to Image methods
// *****
// half-tracker image 113 x 9 = height x width = rows x columns
void GG2ImageConverter::gg2image(std::vector<MetaInfo> data) {
  if (leftimage.size())
    leftimage.clear();
  if (rightimage.size())
    rightimage.clear();

  for (int i=0;i<height;i++)
    for (int j=0;j<hwidth;j++) {
      leftimage.push_back(false); // zero image, half tracker
      rightimage.push_back(false); // zero image, half tracker
    }
  for (auto& mi : data) {
    int row = mi.row;
    int col = mi.column;
    if (mi.side < 1) // left tracker
      leftimage[row * hwidth + col] = true;
    else
      rightimage[row * hwidth + col] = true;
  }
//   for (int row=0;row<height;row++) {
//     for (int col=0;col<hwidth;col++)
//       std::cout << leftimage.at(row*9+col) << " ";
//     std::cout << std::endl;
//   }

}


std::unordered_map<unsigned int, std::vector<MetaInfo> > GG2ImageConverter::image2gg(std::vector<MetaInfo> data, std::unordered_map<unsigned int, std::list<Pixel> > labels, int side) {

  std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters;

  // temporary storage
  std::vector<MetaInfo> hits;
  MetaInfo hit;

  // cluster of pixels map loop
  for (std::unordered_map<unsigned int, std::list<Pixel> >::iterator it=labels.begin(); it!=labels.end(); ++it) {
    unsigned int key = it->first;
    std::list<Pixel> pixels = it->second;
    for (auto& pp : pixels) { // for every pixel
      // find the geiger hit in data, multiple times if needed
      for (auto& mi : data) {
	if (pp.first == mi.column && pp.second == mi.row && side == mi.side) { // found it
	  hit.side = mi.side;
	  hit.row = mi.row;
	  hit.column = mi.column;
	  hit.z   = mi.z;
	  hits.push_back(hit);
	}
      }
    }
    clusters[key] = hits;
    hits.clear();
  }
  return clusters;
}


// *****
// ** Utility Graph methods
// *****
Graph::Graph(int nv)
{
  V = nv; // v vertices
  MAXINT = 32768; // some big integer distance
  adj.resize(V); // expect V elements
}


void Graph::addEdge(int v, int w)
{
  // for an undirect graph, vertices of an edge are each accessible from another
  adj[v].insert(w); // Add w to v’s list.
  adj[w].insert(v); // and vice versa

}


std::unordered_set<int> Graph::nodes()
{
  std::unordered_set<int> store;
  for (auto& entry : adj) // entry is unordered_set
    for (int value : entry) // integer in set
      store.insert(value); // only unique integers in set
  return store;
}


bool Graph::singleNode(int node)
{
  std::unordered_set<int> partners = adj[node];
  return partners.size()<2;
}


// A BFS based function to check whether d is reachable from s.
bool Graph::isReachable(int s, int d)
{
  
  // Base case
  if (s == d)
    
    return true;
  
  
  
  // Mark all the vertices as not visited
  bool *visited = new bool[V];
  
  for (int i = 0; i < V; i++)
    visited[i] = false;
  
  
  
  // Create a queue for BFS
  std::list<int> queue;
  
  // Mark the current node as visited and enqueue it
  visited[s] = true;
  
  queue.push_back(s);
  
  while (!queue.empty())
    
    {
      
      // Dequeue a vertex from queue and print it
      s = queue.front();
      
      queue.pop_front();
      
      
      // Get all adjacent vertices of the dequeued vertex s
      
      // If a adjacent has not been visited, then mark it visited
      
      // and enqueue it
      for (auto& entry : adj[s])
	{
	  
	  // If this adjacent node is the destination node, then return true
	  if (entry == d)
	    return true;
	  
	  
	  
	  // Else, continue to do BFS
	  if (!visited[entry])
	    {
	      
	      visited[entry] = true;
	      
	      queue.push_back(entry);
	      
	    }
	  
	}
      
    }
  
  return false;
}



void Graph::bfsPaths(int start, int target)
{
  if (!adj.size()) {
    return;
  }
  // clear data members
  allPaths.clear();
  currentPath.clear();
  if (start == target) return; // nothing to do

  std::queue<int> queue;
  std::vector<int> distance(adj.size(), MAXINT);

  // store child-to-parent information
  std::vector<std::unordered_set<int>> prev(adj.size());

  // init
  distance[start] = 0;
  queue.push(start);
  bool ans = false;
  
  while (queue.size()) {
    auto current = queue.front();
    queue.pop();
    
    for (auto child: adj[current]) {
      if (distance[child] == MAXINT) {
	// child node not visited yet
	queue.push(child);
	distance[child] = distance[current] + 1;
	prev[child].insert(current);

      } else if (distance[child] == distance[current] + 1) {
	// multiple child nodes with save distance
	prev[child].insert(current);
      }
      
      if (child == target) {
	ans = true;
      }
    }
  }
  
  if (ans) {
    dfs(prev, target);
    return;
  }

  return;
}



void Graph::dfs(std::vector<std::unordered_set<int>>& prev,
		int node)
{
  currentPath.push_back(node);
  
  // path ends here
  if (prev[node].size() == 0) {
    allPaths.push_back(std::vector<int>(currentPath.rbegin(), currentPath.rend()));
  }
  
  for (auto parent: prev[node]) {
    dfs(prev, parent);
  }

  // backtracking
  currentPath.pop_back();
}



// *****
// ** ClusterCleanup methods
// *****
void ClusterCleanup::init(std::unordered_map<unsigned int, std::vector<MetaInfo> > cls) {
  if (clusters.size())
    clusters.clear();
  clusters = cls; // copy to work with
  accepted.clear();
}


void ClusterCleanup::zSplitter() {
  unsigned int clsid; // starts counting at 1

  for (auto& entry : clusters) { // gives uint, vector<MetaInfo>
    clsid = entry.first;
    zSplit(clsid, entry.second);
  }
}


void ClusterCleanup::zSweeper() {
  // access existing clusters internally
  for (auto& entry : clusters) 
    histogramSweeper(entry.first);
}


void ClusterCleanup::zSplit(unsigned int clsid, std::vector<MetaInfo>& cls) {
  // find discontinuity in z
  std::vector<double> zarray; // re-initialising valarray
  std::valarray<double> allz(cls.size()); // for finding extremes
  int i=0;
  for (auto& mi : cls) {
    allz[i] = mi.z;
    i++;
  }
  if (i<7) return; // not enough data points to fill rough histogram
  double start = allz.min();
  double end   = allz.max();
  //  std::cout << "zSplit, start z " << start << " end " << end << std::endl;
  
  // Case 1: check all z values on global gap  
  double zlimit = histogramSplit(allz, start, end);

  if (zlimit != DUMMY) {
    std::cout << "case 1: zlimit = " << zlimit << std::endl;
    zSplitCluster(clsid, zlimit);
    return;
  }
}



void ClusterCleanup::zSplitCluster(unsigned int id, double zlimit) {
  // global z split cluster
  std::vector<MetaInfo> cls = clusters[id]; // to be split
  std::vector<MetaInfo> newcls;
  std::vector<int> keepElement;
  
  int i = 0; // counter
  for (auto& mi : cls) {
    double z = mi.z;
    // consistent z values remain in both clusters
    if (z < zlimit + stepwidth && z > zlimit - stepwidth) { // absolute 10mm consistency interval
      newcls.push_back(mi);
      keepElement.push_back(i);
    }
    else if (z <= zlimit - stepwidth) { // outside below zlimit, into newcls
      newcls.push_back(mi);
      //      std::cout << "newcls z = " << z << std::endl;
    }
    else                       // z>=zlimit+stepwidth
      keepElement.push_back(i);
    i++;
  }

  std::vector<MetaInfo> modcls; // dummy storage
  for (int which : keepElement) {
    modcls.push_back(cls.at(which));
    //    std::cout << "modcls z = " << cls.at(which).z << std::endl;
  }

  // avoid single pixels to be split off
  if (keepElement.size() == 1 || newcls.size() == 1) {
    return; // do nothing
  }

  // modify cluster collection
  clusters[id] = modcls;
  clusters[clusters.size()+1] = newcls; // keys count from 1
}



double ClusterCleanup::histogramSplit(std::valarray<double>& allz, double start, double end) {
  // discretize z-axis
  int nbins = 6; // coarse histogram resolution in z, avoid gaps

  double step = fabs((end - start)/nbins);
  //  std::cout << "histoSplit, step " << step << std::endl;
  if (step / stepwidth / 2.0 <= 2.0) return DUMMY; // z coordinate error size = flat in z, no split

  std::vector<int> histogram(nbins+1, 0); // fill with zero

  for (int j=0;j<(int)allz.size();j++) { // for all z
    int bucket = (int)floor((allz[j]-start) / step); // which bin
    //    std::cout << "histoSplit, bucket " << bucket << " from " << allz[j] << " - " << start << std::endl;
    histogram[bucket] += 1; // increment
  }

//   for (int bin : histogram)
//     std::cout << "histoSplit, bin " << bin << std::endl;

  std::vector<int>::iterator it;
  it = std::find(histogram.begin(), histogram.end(), 0);
  if (it != histogram.end()) { // found a zero in the histogram, a gap
    int pos = it - histogram.begin(); // index of first zero in histo

    std::reverse(histogram.begin(), histogram.end()); // check for zero from the other end
    it = std::find(histogram.begin(), histogram.end(), 0);
    int pos2 = histogram.size() - (it-histogram.begin()); // index of last zero in histo
    
    if (pos==pos2) return DUMMY; // not just a single zero in the histogram
    double loc = (pos+pos2)/2.0; // average index position
    //    std::cout << "histoSplit, split index " << pos << " and " << pos2 << std::endl;
    return loc * step + start; // return z value of average of empty bins as border
  }
  else
    return DUMMY;
}


void ClusterCleanup::histogramSweeper(unsigned int clsid) {
  std::vector<MetaInfo> cls = clusters[clsid];

  // discretize z-axis
  std::valarray<double> allz(cls.size()); // for finding extremes
  int i=0;
  for (auto& mi : cls) {
    allz[i] = mi.z;
    i++;
  }
  double start = allz.min();
  double end   = allz.max();

  int nbins = 6; // coarse histogram resolution in z, avoid gaps
  if (i<=nbins) return; // not enough data points to fill rough histogram
  std::vector<MetaInfo> newcls;

  double step = fabs((end - start)/nbins);
  if (step / stepwidth / 2.0 <= 2.0) return; // nothing to do

  std::vector<int> histogram(nbins+1, 0); // fill with zero

  for (int j=0;j<(int)allz.size();j++) { // for all z
    int bucket = (int)floor((cls[j].z - start) / step); // which bin
    histogram[bucket] += 1; // increment
  }

  int sum1 = 0;
  int sum2 = 0;
  std::vector<int>::iterator it;
  it = std::find(histogram.begin(), histogram.end(), 0);
  if (it != histogram.end()) { // found a zero in the histogram, a gap
    int pos = it - histogram.begin(); // index of first zero in histo

    for (int jj=0;jj<pos;jj++) sum1+=histogram[jj];

    std::reverse(histogram.begin(), histogram.end()); // check for zero from the other end
    it = std::find(histogram.begin(), histogram.end(), 0);
    int pos2 = histogram.size() - (it-histogram.begin()); // index of last zero in histo

    for (int jj=0;jj<pos2;jj++) sum2+=histogram[jj]; // histo has been reversed, add from 0

    double loc = (pos+pos2)/2.0; // average index position
    double zlimit = loc * step + start;

    //    std::cout << "in sweeper, found split: entries below zlimit = " << sum1 << " entries above = " << sum2 << std::endl;
    // which side of zlimit to keep?
    bool keep_below_zlimit = false;
    if (sum1 < sum2)
      keep_below_zlimit = false;
    else
      keep_below_zlimit = true;
    
    // remove the small number of wrong z entries in clusters from x,y copies
    for (auto& mi : cls) {
      if (keep_below_zlimit && mi.z<zlimit)
	newcls.push_back(mi);
      if (!keep_below_zlimit && mi.z>zlimit)
	newcls.push_back(mi);
    }
    clusters[clsid] = newcls; // overwrite clusters after sweep
  }
  else 
    return; // nothing to do
}


void ClusterCleanup::consolidate() {
  std::unordered_map<unsigned int, std::vector<MetaInfo> > newcls; // dummy storage
  unsigned int counter = 1; // cluster keys start from 1
  for (auto& entry : clusters) { // gives uint, vector<MetaInfo>
    std::vector<unsigned int>::iterator findit = std::find(accepted.begin(), accepted.end(), entry.first);
    if (findit!=accepted.end()) { // found
      newcls[counter] = entry.second;
      counter++;
    }
  }
  clusters.clear();
  clusters = newcls;
}


void ClusterCleanup::checkAcceptance(const VectorXd& ev, unsigned int id) {
  double sum = ev[0] + ev[1];
  //  VectorXd evnormed = ev.normalized();
  VectorXd evnormed(2);
  evnormed << ev[0]/sum, ev[1]/sum; // fill with relative fractions

  for (int i=0;i<evnormed.size();i++) {
    if (evnormed[i] > threshold) // check for one dominant eigenvalue
      accepted.push_back(id);
    else if (evnormed[i] < threshold && evnormed[i] > 0.5)
      std::cout << "Ev normed failed cut: " << evnormed[i] << std::endl;
  }
}



VectorXd ClusterCleanup::pca2d(const Matrix<double, Dynamic, 2>& data, int points) {
  double mean;
  VectorXd meanVector;
  Matrix<double, Dynamic, 2> dummy(data);
  // PCA START
  // Subtract mean
  for (int i = 0; i < data.cols(); i++){
    mean = (data.col(i).sum())/(float)points; //compute mean
    // create a vector with constant value = mean
    meanVector  = VectorXd::Constant(points,mean); 
    dummy.col(i) -= meanVector;
  }
  
  // Covariance matrix of mean zero data
  MatrixXd cov = dummy.transpose() * dummy;
  cov *= 1.0/((float)points-1.0); // normalisation (n-1)^-1
  
  // Eigenvectors and eigenvalues
  SelfAdjointEigenSolver<MatrixXd> eigensolver(cov);

  if (eigensolver.info() != Success) {
    //abort(); carry on
    VectorXd eval(2);
    eval << 0, 1; // fill with dummy extremes to pass cut
    return eval;
  }
  else {
    VectorXd eval = eigensolver.eigenvalues().real();
    return eval;
  }
}


void ClusterCleanup::runPCAonImage() {
  unsigned int clsid;
  int points;
  Matrix<double, Dynamic, 2> Data; // cluster data container, nx2 matrix
  
  int i=0;
  for (auto& entry : clusters) { // gives uint, vector<MetaInfo>
    clsid = entry.first;
    points = (int)entry.second.size();
    if (points<9) { 
      accepted.push_back(clsid);
      continue; // avoid loosing small clusters
    }
    Data.resize(points, NoChange); // now points x 2 matrix
    for (MetaInfo& mi : entry.second) { // fill 2 columns
      Data(i,0) = mi.column;
      Data(i,1) = mi.row;
      i++; // fill rows
    }
    i = 0; // reset row counter
    VectorXd evalues = pca2d(Data, points); // has to be image data: points rows x 2 columns
    checkAcceptance(evalues, clsid);
  }
  consolidate(); // new clusters from accepted
}
