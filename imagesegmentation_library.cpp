// std libraries
#include <queue>
#include <unordered_set>
#include <set>
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
  std::unordered_map<unsigned int, std::list<Pixel> > partial_left; // for splits
  std::unordered_map<unsigned int, std::list<Pixel> > partial_right; // cluster format

  std::vector<int> spoints;
  std::pair<int,int> test1(2,2); // test for presence
  std::pair<int,int> test2(1,2);
  std::pair<int,int> test3(2,1);

  labels->label(data); // create labels from ImageLabel object

  // get the image collection of separate structures
  std::list<std::vector<bool> > icollection = labels->imagecollection();

  // test splitting with an image input

  bool goToGraph = false;
  for (auto& im : icollection) {
    labels->label(im); // splitting overwrites labels
    cls = labels->getLabels(); // book them now for simplest single cluster

    if (labels->is_splitting(im)) {

      // check first for all complex segmentation
      spoints = labels->split_at();
      if (spoints.size()>1) { // needs different clusterer
	//	std::cout << "split points size " << spoints.size() << std::endl;
	goToGraph = true; //complex segmentation
      }
      bool crossing = false;
      for (auto& sp : labels->splitpoints()) { // std::pair output
	//	std::cout << "split points: (" << std::get<0>(sp) << "," << std::get<1>(sp) << ")" << std::endl;
	if (sp==test1) // needs different clusterer
	  goToGraph = true; // complex segmentation
	if ((crossing && sp==test3) || (crossing && sp==test2)) // crossing tracks
	  goToGraph = true; // complex segmentation
	if (sp==test2 || sp==test3)
	  crossing = true; // half the condition at a first pass
	if (std::get<0>(sp)>2 || std::get<1>(sp)>2) // more than 2 structures found
	  goToGraph = true; // complex segmentation
      }

      std::cout << "done is_splitting with graph flag at " << goToGraph << std::endl;
      if (goToGraph) { // try clustering elsewhere
	gr->cluster(im); // fill clusters
	cls = gr->getClusters();
      }
      else { // try clustering here; simple enough structure
	partial_left = labels->getLeftLabels(); // in cluster map format
	partial_right = labels->getRightLabels();
	int cut = labels->split_at()[0];

	// correct with offset for right label pixels
	std::list<Pixel> coll_rr;
	Pixel newpp;
	for (auto& rr : partial_right) {
	  for (Pixel& pp : rr.second) {
	    newpp.x = pp.x + cut + 1; // offset column
	    newpp.y = pp.y;
	    coll_rr.push_back(newpp);
	  }
	  rr.second = coll_rr;
	  coll_rr.clear();
	}

	std::pair<int, int> signature = labels->splitpoints()[cut];
	// std::cout << "split signature: (" << std::get<0>(signature) << "," << std::get<1>(signature) << ")" << std::endl;
	int ll = std::get<0>(signature);
	int rr = std::get<1>(signature);
	if (ll<rr) // (1,2) case
	  cls = merge_single_to_multi(partial_left, partial_right);
	else       // (2,1) case
	  cls = merge_single_to_multi(partial_right, partial_left);

      }
    }
    // simple clusters already in cls
    clscollection.push_back(cls); // basket collection
    goToGraph = false;
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




std::unordered_map<unsigned int, std::list<Pixel> > ImageSegmentation::merge_single_to_multi(std::unordered_map<unsigned int, std::list<Pixel> > single, std::unordered_map<unsigned int, std::list<Pixel> > multi) {
  std::unordered_map<unsigned int, std::list<Pixel> > newcls;
  std::list<Pixel> collection;
  unsigned int key;
  for (auto& multimap : multi) {
    key = multimap.first;
    collection = multimap.second; // append the singles list to this
    for (auto& singlemap : single ) // should just be one entry
      for (Pixel& pp : singlemap.second) // just the list of Pixels
	collection.push_back(pp); // merging of singles list to multi list
    newcls[key] = collection;
  }
  return newcls;
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
    std::pair<int,int> start_node = std::make_pair(edge.first.x, edge.first.y);
    std::pair<int,int> end_node = std::make_pair(edge.second.x, edge.second.y);
    
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
//   for (int idx : dead_ends)
//     std::cout << "dead_end index: " << idx << " ";
//   std::cout << std::endl;
//   for (int idx : starts)
//     std::cout << "starts index: " << idx << " ";
//   std::cout << std::endl;
//   for (int idx : targets)
//     std::cout << "targets index: " << idx << " ";
//   std::cout << std::endl;

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
      if (gg.isReachable(s, t))
	if (s != t && nodeS.first != nodeT.first) 
	  tempCluster.push_back(gg.bfsPaths(s, t));
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
	  pp.x = node.first; // column in image
	  pp.y = row;        // row in image
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
	  start.x = col - 1;
	  start.y = index;
	  target.x = col;
	  target.y = indexNext;
	  vertices.insert(std::make_pair(start.x, start.y)); // unique nodes only
	  vertices.insert(std::make_pair(target.x, target.y)); // 
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
//     std::cout << "Edge: " << st.x << ", " << st.y << " ;T= "<< ta.x << ", " << ta.y << std::endl;
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
      im[pp.y * width + pp.x] = true;

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
    }
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
	  
	  pp.x = x; // column
	  pp.y = y; // row
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
	if (pp.x == mi.column && pp.y == mi.row && side == mi.side) { // found it
	  hit.side = mi.side;
	  hit.row = mi.row;
	  hit.column = mi.column;
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



std::vector<std::vector<int>> Graph::bfsPaths(int start, int target)
{
  if (!adj.size()) {
    return {};
  }
  // clear data members
  allPaths.clear();
  currentPath.clear();

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
    return allPaths;
  }

  return {};
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

