#include "TrackingTools/GsfTracking/interface/GsfMultiStateUpdator.h"

#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "TrackingTools/PatternTools/interface/MeasurementExtractor.h"
#include "TrackingTools/TransientTrackingRecHit/interface/TransientTrackingRecHit.h"
#include "Geometry/Surface/interface/BoundPlane.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/GsfTools/interface/BasicMultiTrajectoryState.h"
#include "TrackingTools/GsfTracking/interface/PosteriorWeightsCalculator.h"
#include "TrackingTools/GsfTools/interface/MultiTrajectoryStateAssembler.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

TrajectoryStateOnSurface GsfMultiStateUpdator::update(const TrajectoryStateOnSurface& tsos,
						      const TransientTrackingRecHit& aRecHit) const {
  
  std::vector<TrajectoryStateOnSurface> predictedComponents = tsos.components();
  if (predictedComponents.empty()) {
    edm::LogError("GsfMultiStateUpdator") << "Trying to update trajectory state with zero components! " ;
    return TrajectoryStateOnSurface();
  }

  std::vector<double> weights = PosteriorWeightsCalculator(predictedComponents).weights(aRecHit);
  if ( weights.empty() )  return TrajectoryStateOnSurface();

  MultiTrajectoryStateAssembler result;

  int i = 0;
  for (std::vector<TrajectoryStateOnSurface>::const_iterator iter = predictedComponents.begin();
       iter != predictedComponents.end(); iter++) {
    TrajectoryStateOnSurface updatedTSOS = KFUpdator().update(*iter, aRecHit);
    result.addState(TrajectoryStateOnSurface(updatedTSOS.localParameters(),
					     updatedTSOS.localError(), updatedTSOS.surface(), 
					     &(tsos.globalParameters().magneticField()),
					     (*iter).surfaceSide(), weights[i]));
    i++;
  }

  return result.combinedState();
}
