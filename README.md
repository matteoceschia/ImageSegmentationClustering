# ImageSegmentation readme


Yorck Ramachers (Warwick)
Last updated March 9, 2018

The ImageSegmentation module is a reconstruction module. It attempts to
cluster tracker hits and fill the TCD data bank in Falaise in the current
event data model.


## Files:

- ImageSegmentation_library.cpp
- ImageSegmentation_library.h
- ImageSegmentation_clustering_module.cpp
- ImageSegmentation_clustering_module.h
- CMakeLists.txt
- iseg.conf


## Description

Add to an flreconstruct pipeline to cluster tracker hits for reconstruction. To build it, do

``` console
$ ls
CMakeLists.txt                   ImageSegmentation_library.h
README.md                        ImageSegmentation_clustering_module.cpp
ImageSegmentation_library.cpp	 ImageSegmentation_clustering_module.h
iseg.conf
$ mkdir build
$ cd build
$ cmake -DCMAKE_PREFIX_PATH=$(brew --prefix) ..
...
$ make
...
... If you are developing the module, you can test it by calling
$ make test
```

Note: if you get a QT5 error, you may need to specify the QT5 path when you run the cmake line, as given by `brew --prefix qt5-base`. For example, you can run:
``` console
$ cmake -DCMAKE_PREFIX_PATH="$(brew --prefix qt5-base);$(brew --prefix)" ..
``` 

The build will create the `libImageSegmentation.so` shared library. Assuming that you have an `input.brio` file that contains
a `CD` bank from the simulation or a data run, this can be run after editing
the configuration file to point at the library location. Note that the module
requires no parameters.:

``` console
...
[name="flreconstruct.plugins" type="flreconstruct::section"]
plugins : string[1] = "ImageSegmentation"
ImageSegmentation.directory : string = "/my/path/to/build/directory"

# Define the modules in the pipeline:
[name="pipeline" type="imagesegmentation_clustering_module"]
...
$ flreconstruct -i /path/to/input.brio -p iseg.conf -o /tmp/clustered_data.brio
```

## Implementation

This reconstruction module attempts to use the bare required minimum to run in
the Falaise pipeline and use the existing data model. None of the pre-defined
'base' classes are used other than the dpp::base_module which renders this
code into a Falaise module.

Input is the calibrated data bank, default name 'CD'. Output is put into the
default 'TCD' data bank, the tracker cluster data bank. The
TrackerPreClustering is not used since the identical functionality (setting
tracker hits as prompt or delayed, configurably with the delay time threshold)
is already implemented by the digitization hence hits can simply be split at the
start into prompt or delayed as they are read in from the 'CD' data bank. 

This implementation method also avoids having to use the outsourcing of
functionality to a driver class which outsources functionality again to an
algorithm source. This appears to be a relic from the early days of the
'channel' pattern and is not really required anymore. Here, the module 
process() method is employed to actually do work and then call the algorithm
object itself rather than bringing in a redundant middle-man 'driver'
object. 

This module also attempts a first use of the Catch test framework for Falaise
modules. 

## Algorithms: (A) Connected structure search

As the name suggests, this cluster algorithm considers the tracker geiger
cells as pixels of an image, a 2 by 9 by 113 pixel image to be precise. Since
the demonstrator tracker will hardly be ripped apart in the near future, this
arrangement of geiger cells is fixed in the code and not considered to be
configurable. 

The first algorithm targets the simplest structures to cluster in an event,
checks that they are indeed simple and proceeds or hands over the entire event
to a second algorithm to deal with more complicated structures. It does so for
each tracker half separately. The image under consideration is hence a 9
columns times 113 rows pixel image.

The first method for image segmentation clustering uses purely an image
labeling algorithm. Each connected set of pixels (geiger cells) forms a single
structure and is considered a single cluster. This works fine for single
tracks and helices and such like. The advantage is that no assumptions
regarding any other properties (straight, curved, length, etc.) are
required. Pixels are connected if they are neighbours. In our rectangular grid
tracker, each pixel can have eight neighbours apart from the perimeter pixels.

The disadvantage is that no assumptions are made hence N tracks coming
together at some random points will be considered connected and become a
single cluster. That is obviously wrong. The method considers the simplest
possible split of a singly connected structure before handing over to the
second method for anything else. This, together with simple, disconnected
structures, covers the majority of tracker events and is correspondingly
swift. 

For splitting structures, the only consideration is on splits
from one into two tracks, e.g. a V-shape event, for instance due to two
electrons from the same vertex. They can run through the tracker together for
quite some length and split up in two separate tracks at any point. Separate
here means pixels are separated by at least one pixel in the Off state, i.e. a
geiger cell that did not fire.

The method to find the split proceeds as follows: Slice the
half-tracker image in two parts, determine the number of structures in each
separately, then move the cut and repeat. Cutting here take place along the 
9 columns of the tracker. If at any point the number of structures on the left is
different to the number of structures on the right then a split has
occurred. As long as the split is merely from 1 to 2 structures or vice versa
then this image segmentation algorithm creates two clusters and
finishes. Anything more complicated has to be handled by the second cluster
algorithm in order to avoid reaching any more byzantine case studies.

Faulty geiger cells are considered to be set in an On state, i.e. always
present as calibrated geiger hits for the algorithm to work and not create
artificial gaps in cluster structures. The clusters are always overcomplete
which means they can't loose any true cluster hits but can contain more than
strictly needed. The merging for instance combines the two separate track hit
collections with all the singly connected track hits to form two clusters
containing a fraction (sometimes a very large fraction) of hits in common. Now
there are two clusters hence from two presumed particles which contain a set
of common geiger hits of which not all are likely to come from each
particle. However, they could and hence those hits are in the cluster.

The smartness of making physics sense of the collection of geiger hits in a
cluster is assumed to be introduced at a later stage in the reconstruction
chain. This clustering algorithm as the first stage tries to be as dumb as
possible but with the certainty of not loosing any true hits under any
circumstances. 

## Algorithms: (B) Graph clustering for more complex cases
Not implemented yet.
