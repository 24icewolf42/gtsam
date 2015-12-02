/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @brief wraps BetweenFactor for several values to python
 * @author Andrew Melim 
 * @author Ellon Paiva Mendes (LAAS-CNRS)
 **/

#include <boost/python.hpp>
#include "gtsam/slam/BetweenFactor.h"
#include "gtsam/geometry/Point2.h"
#include "gtsam/geometry/Rot2.h"
#include "gtsam/geometry/Pose2.h"
#include "gtsam/geometry/Point3.h"
#include "gtsam/geometry/Rot3.h"
#include "gtsam/geometry/Pose3.h"

using namespace boost::python;
using namespace gtsam;

using namespace std;

// template<class VALUE>
// void exportBetweenFactor(const std::string& name){
//   class_<VALUE>(name, init<>())
//   .def(init<Key, Key, VALUE, SharedNoiseModel>())
//   ;
// }

#define BETWEENFACTOR(VALUE) \
  class_< BetweenFactor<VALUE>, bases<NonlinearFactor>, boost::shared_ptr< BetweenFactor<VALUE> > >("BetweenFactor"#VALUE) \
  .def(init<Key,Key,VALUE,noiseModel::Base::shared_ptr>()) \
  .def("measured", &BetweenFactor<VALUE>::measured, return_internal_reference<>()) \
;

void exportBetweenFactors()
{
  BETWEENFACTOR(Point2)
  BETWEENFACTOR(Rot2)
  BETWEENFACTOR(Pose2)
  BETWEENFACTOR(Point3)
  BETWEENFACTOR(Rot3)
  BETWEENFACTOR(Pose3)
}
