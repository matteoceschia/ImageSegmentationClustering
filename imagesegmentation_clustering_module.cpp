// Ourselves:
#include <imagesegmentation_clustering_module.h>

// Standard library:
#include <stdexcept>
#include <iostream>
// falaise
#include <falaise/snemo/datamodels/data_model.h>

// Registration instantiation macro :
DPP_MODULE_REGISTRATION_IMPLEMENT(imagesegmentation_clustering_module,
				  "imagesegmentation_clustering_module")


void imagesegmentation_clustering_module::initialize(const datatools::properties  & setup_,
					       datatools::service_manager   & service_manager_,
					       dpp::module_handle_dict_type & /* module_dict_ */)
{
  DT_THROW_IF (this->is_initialized(),
	       std::logic_error,
	       "Module 'ImageSegmentation' is already initialized ! ");
  
  dpp::base_module::_common_initialize(setup_);
 
  // check the label
  _TCD_label_ = snemo::datamodel::data_info::default_tracker_clustering_data_label();

  // Clustering algorithm :
  iseg = new ImageSegmentation(9,113); // tracker planes 113 by 9 by 2 sides
  
  this->_set_initialized(true);

  return;
}

void imagesegmentation_clustering_module::reset()
{
  DT_THROW_IF (! this->is_initialized(),
	       std::logic_error,
	       "Module 'ImageSegmentation' is not initialized !");
  this->_set_initialized(false);
  
  // clean up
  _TCD_label_.clear();
  if (iseg) {
    delete iseg;
  }
  std::cout << "Image segmentation clustering finished." << std::endl;
  return;
}

// Constructor :
imagesegmentation_clustering_module::imagesegmentation_clustering_module(datatools::logger::priority logging_priority_)
  : dpp::base_module(logging_priority_)
{
}

// Destructor :
imagesegmentation_clustering_module::~imagesegmentation_clustering_module()
{
  // MUST reset module at destruction
  if (this->is_initialized()) reset();
}

// Processing :
dpp::base_module::process_status imagesegmentation_clustering_module::process(datatools::things & data_record_)
{
  DT_THROW_IF (! this->is_initialized(), std::logic_error,
	       "Module 'ImageSegmentation' is not initialized !");
  
  ///////////////////////////
  // Check calibrated data //
  ///////////////////////////
  
  // Check if some 'calibrated_data' are available in the data model:
  if (!data_record_.has("CD")) {
    std::cerr << "failed to grab CD bank " << std::endl;
    return dpp::base_module::PROCESS_INVALID;
  }

  // grab the 'calibrated_data' entry from the data model :
  const snemo::datamodel::calibrated_data& the_calibrated_data
    = data_record_.get<snemo::datamodel::calibrated_data>("CD");
  const snemo::datamodel::calibrated_data::tracker_hit_collection_type& gg_hits 
    = the_calibrated_data.calibrated_tracker_hits();
  

  ///////////////////////////////////
  // Check tracker clustering data //
  ///////////////////////////////////
  
  bool preserve_former_output = false;
  
  // check if some 'tracker_clustering_data' are available in the data model:
  snemo::datamodel::tracker_clustering_data * ptr_cluster_data = 0;
  if (! data_record_.has(_TCD_label_)) {
    ptr_cluster_data = &(data_record_.add<snemo::datamodel::tracker_clustering_data>(_TCD_label_));
  } else {
    ptr_cluster_data = &(data_record_.grab<snemo::datamodel::tracker_clustering_data>(_TCD_label_));
  }
  snemo::datamodel::tracker_clustering_data & the_clustering_data = *ptr_cluster_data;
  if (the_clustering_data.has_solutions()) 
    if (! preserve_former_output) 
      the_clustering_data.reset();
  

  /********************
   * Process the data *
   ********************/
  
  // Main processing method :
  // Process the clusterizer :
  namespace sdm = snemo::datamodel;

  // make a clustering solution
  sdm::tracker_clustering_solution::handle_type htcs(new sdm::tracker_clustering_solution);
  the_clustering_data.add_solution(htcs, true);
  the_clustering_data.grab_default_solution().set_solution_id(the_clustering_data.get_number_of_solutions() - 1);
  sdm::tracker_clustering_solution & clustering_solution = the_clustering_data.grab_default_solution();


  // Process geiger hits for clustering
  // geiger hits to image
  MetaInfo mi;
  std::vector<MetaInfo> gg_data; // divide in prompt and delayed hits
  std::vector<MetaInfo> gg_data_delayed;
  for (const sdm::calibrated_data::tracker_hit_handle_type& gg_handle : gg_hits) {
    if (! gg_handle) continue;
    const sdm::calibrated_tracker_hit & snemo_gg_hit = gg_handle.get();
    mi.side   = snemo_gg_hit.get_geom_id().get(1); // 0, 1
    mi.row    = snemo_gg_hit.get_geom_id().get(3); // 113
    mi.column = snemo_gg_hit.get_geom_id().get(2); // 9

    if (snemo_gg_hit.is_prompt())
      gg_data.push_back(mi);
    else
      gg_data_delayed.push_back(mi);
  }
//   std::cout << "In process: gg_data size=" << gg_data.size() << std::endl;
//   std::cout << "In process: gg_data_delayed size=" << gg_data_delayed.size() << std::endl;

  GG2ImageConverter g2i(18,113); // full sized tracker, 113 rows at 9 columns for 2 sides
  ClusterCleanup clclean; // final step splitting in z and cleaning collection

  if (gg_data.size()>0) { // work on prompt hits
//     for (MetaInfo& entry : gg_data)
//       std::cout << "Cluster Entry: (" << entry.side << ", " << entry.column << ", " << entry.row << ")" << std::endl;
    g2i.gg2image(gg_data);
    std::vector<bool> ll = g2i.getLeft();
    std::vector<bool> rr = g2i.getRight();

    // using the clusterer, first on left side
    iseg->cluster(ll);
    std::unordered_map<unsigned int, std::list<Pixel> > cls_left = iseg->getClusters();

    if (cls_left.size()>0) {
//       for (auto& vec : cls_left)
// 	for (Pixel& entry : vec.second)
// 	  std::cout << "after getCluster left Entry: (" << entry.x << ", " << entry.y << ")" << std::endl;
      // get clusters for left tracker (side=0)
      std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters_ll = g2i.image2gg(gg_data, cls_left, 0);

      // clean up collection
      clclean.init(clusters_ll);
      //      clclean.setZResolution(10.0); // [mm] z resolution
      clclean.zSplitter(); // method A for clean up
      //      clclean.setPCAAcceptanceThreshold(0.9); // 90% threshold
      clclean.runPCAonImage(); // method B for clean up
      clusters_ll = clclean.getClusters(); // overwrite collection

      // store in clustering solution
      _translate(the_calibrated_data, clustering_solution, clusters_ll);
    }
    // using the clusterer, then right (side=1)
    iseg->cluster(rr);
    std::unordered_map<unsigned int, std::list<Pixel> > cls_right = iseg->getClusters();

    if (cls_right.size()>0) {
//       for (auto& vec : cls_right)
// 	for (Pixel& entry : vec.second)
// 	  std::cout << "after getCluster right Entry: (" << entry.x << ", " << entry.y << ")" << std::endl;
      // get clusters
      std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters_rr = g2i.image2gg(gg_data, cls_right, 1);

      // clean up collection
      clclean.init(clusters_rr);
      //      clclean.setZResolution(10.0); // [mm] z resolution
      clclean.zSplitter(); // method A for clean up
      //      clclean.setPCAAcceptanceThreshold(0.9); // 90% threshold
      clclean.runPCAonImage(); // method B for clean up
      clusters_rr = clclean.getClusters(); // overwrite collection

      // store in clustering solution
      _translate(the_calibrated_data, clustering_solution, clusters_rr);
    }
  }
  if (gg_data_delayed.size()>0) { // work on delayed hits
    g2i.gg2image(gg_data_delayed);
    std::vector<bool> ll_delayed = g2i.getLeft();
    std::vector<bool> rr_delayed = g2i.getRight();

    iseg->cluster(ll_delayed);
    std::unordered_map<unsigned int, std::list<Pixel> > cls_left_delayed = iseg->getClusters();
    if (cls_left_delayed.size()>0) {
      // get clusters
      std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters_ll_delayed = g2i.image2gg(gg_data, cls_left_delayed, 0);

      // clean up collection
      clclean.init(clusters_ll_delayed);
      //      clclean.setZResolution(10.0); // [mm] z resolution
      clclean.zSplitter(); // method A for clean up
      //      clclean.setPCAAcceptanceThreshold(0.9); // 90% threshold
      clclean.runPCAonImage(); // method B for clean up
      clusters_ll_delayed = clclean.getClusters(); // overwrite collection

      // store in clustering solution
      _translate(the_calibrated_data, clustering_solution, clusters_ll_delayed);
    }
    iseg->cluster(rr_delayed);
    std::unordered_map<unsigned int, std::list<Pixel> > cls_right_delayed = iseg->getClusters();
    if (cls_right_delayed.size()>0) {
      // get clusters
      std::unordered_map<unsigned int, std::vector<MetaInfo> > clusters_rr_delayed = g2i.image2gg(gg_data, cls_right_delayed, 1);

      // clean up collection
      clclean.init(clusters_rr_delayed);
      //      clclean.setZResolution(10.0); // [mm] z resolution
      clclean.zSplitter(); // method A for clean up
      //      clclean.setPCAAcceptanceThreshold(0.9); // 90% threshold
      clclean.runPCAonImage(); // method B for clean up
      clusters_rr_delayed = clclean.getClusters(); // overwrite collection

      // store in clustering solution
      _translate(the_calibrated_data, clustering_solution, clusters_rr_delayed);
    }
  }
  return dpp::base_module::PROCESS_SUCCESS;
}


void imagesegmentation_clustering_module::_translate(const snemo::datamodel::calibrated_data& the_calibrated_data,
						     snemo::datamodel::tracker_clustering_solution & clustering_solution, 
						     std::unordered_map<unsigned int, std::vector<MetaInfo> >& clusters) 
{
  namespace sdm = snemo::datamodel;
  MetaInfo mi;
  // translate back to gg hits
  for (auto& entry : clusters) { // loop through map
    // Append a new cluster :
    sdm::tracker_cluster::handle_type tch(new sdm::tracker_cluster);
    clustering_solution.grab_clusters().push_back(tch);
    sdm::tracker_cluster::handle_type & cluster_handle
      = clustering_solution.grab_clusters().back();
    // set cluster id number
    cluster_handle.grab().set_cluster_id(clustering_solution.get_clusters().size() - 1);
    // identify all cluster image pixels as geiger hits
    for (MetaInfo& val : entry.second) { // loop over std::vector
      //      std::cout << "translate: Cluster Key: " << entry.first <<" Entry: (" << val.side << ", " << val.column << ", " << val.row << ")" << std::endl;
      for (const sdm::calibrated_data::tracker_hit_handle_type& gg_handle : the_calibrated_data.calibrated_tracker_hits()) {
	if (! gg_handle) continue;
	const sdm::calibrated_tracker_hit & snemo_gg_hit = gg_handle.get();
	mi.side   = snemo_gg_hit.get_geom_id().get(1);
	mi.row    = snemo_gg_hit.get_geom_id().get(3);
	mi.column = snemo_gg_hit.get_geom_id().get(2);
	if (val.side==mi.side && val.row==mi.row && val.column==mi.column) {
	  cluster_handle.grab().grab_hits().push_back(gg_handle); // found, store in cluster
	}
      }
    }
  }
  return;
}
