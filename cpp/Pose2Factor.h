/**
 *  @file  Pose2Factor.H
 *  @authors Frank Dellaert, Viorela Ila
 **/

#pragma once

#include "BetweenFactor.h"
#include "Pose2.h"
#include "Pose2Config.h"

namespace gtsam {

	/** This is just a typedef now */
	typedef BetweenFactor<Pose2Config, Pose2Config::Key, Pose2> Pose2Factor;

} /// namespace gtsam
