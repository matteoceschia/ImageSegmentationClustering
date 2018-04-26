/* Description:
 *
 *   Module for Geiger hits clustering
 *
 * History:
 *
 */

#ifndef FALAISE_IMAGESEGMENTATION_CLUSTERING_MODULE_H
#define FALAISE_IMAGESEGMENTATION_CLUSTERING_MODULE_H 1

// Third party:
#include <string>
// - Bayeux/dpp:
#include <bayeux/dpp/base_module.h>
// - Falaise:
#include <falaise/snemo/datamodels/calibrated_data.h>
#include <falaise/snemo/datamodels/tracker_clustering_data.h>
// This project:
#include <imagesegmentation_library.h>


/// \brief Tracker clustering module using the image segmentation algorithm
class imagesegmentation_clustering_module : public dpp::base_module
{
	
public:

	/// Constructor
	imagesegmentation_clustering_module(datatools::logger::priority = datatools::logger::PRIO_FATAL);
	
	/// Destructor
	virtual ~imagesegmentation_clustering_module();
	
	/// Initialization
	virtual void initialize(const datatools::properties  & setup_,
				datatools::service_manager   & service_manager_,
				dpp::module_handle_dict_type & module_dict_);
	
	/// Reset
	virtual void reset();
	
	/// Data record processing
	virtual process_status process(datatools::things & data_);
	

protected:
	void _translate(const snemo::datamodel::calibrated_data&,
			snemo::datamodel::tracker_clustering_solution &, 
			std::unordered_map<unsigned int, std::vector<MetaInfo> >&, bool delayed);


private:
	int eventCounter;
	std::string _TCD_label_;
	ImageSegmentation* iseg;
	
	// Macro to automate the registration of the module :
	DPP_MODULE_REGISTRATION_INTERFACE(imagesegmentation_clustering_module)
};

#endif
