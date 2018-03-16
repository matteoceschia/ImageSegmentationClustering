// std libraries
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
	gr->cluster(data); // fill clusters
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
}



std::unordered_map<unsigned int, std::list<Pixel> > GraphClusterer::getClusters() {
  return cls;
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
