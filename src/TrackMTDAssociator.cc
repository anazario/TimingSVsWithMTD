#include "TimingWithSVs/TimingSVsWithMTD/interface/TrackMTDAssociator.h"


TrackMTDAssociator::TrackMTDAssociator(const TransientTrackBuilder* ttBuilder,
				       const MTDTrackingDetSetVector* mtdRecHits,
				       const Propagator* prop,
				       const MTDDetLayerGeometry* mtdGeometry,
				       MeasurementEstimator* estimator)
  : ttBuilder_(ttBuilder),
    mtdRecHits_(mtdRecHits),
    prop_(prop),
    mtdGeometry_(mtdGeometry),
    estimator_(estimator) {}

MTDIdInfo TrackMTDAssociator::associate(const reco::Track &track) const {

  const reco::TransientTrack ttrack(ttBuilder_->build(track));

  std::set<MTDHitMatchingInfo> btlHits = tryBTLLayers(ttrack);
  std::set<MTDHitMatchingInfo> etlHits = tryETLLayers(ttrack);

  return MTDIdInfo(btlHits, etlHits);
}

std::set<MTDHitMatchingInfo> TrackMTDAssociator::tryETLLayers(const reco::TransientTrack &ttrack) const {
				
  std::set<MTDHitMatchingInfo> etlHits;

  TrajectoryStateOnSurface tsos = ttrack.outermostMeasurementState();

  for(const auto &layer : mtdGeometry_->allETLLayers()) {

    const BoundDisk& disk = static_cast<const ForwardDetLayer*>(layer)->specificSurface();

    const float diskZ = disk.position().z();
    if (tsos.globalPosition().z() * diskZ < 0) continue;  // only propagate to the disk that's on the same side
    std::set<MTDHitMatchingInfo> foundHits(FindMTDHits(ttrack, layer));
    etlHits.insert(foundHits.begin(), foundHits.end());
  }

  return etlHits;
}

std::set<MTDHitMatchingInfo> TrackMTDAssociator::tryBTLLayers(const reco::TransientTrack &ttrack) const {

  std::set<MTDHitMatchingInfo> btlHits;

  for(const auto &layer : mtdGeometry_->allBTLLayers()) {
    std::set<MTDHitMatchingInfo> foundHits(FindMTDHits(ttrack, layer));
    btlHits.insert(foundHits.begin(), foundHits.end());
  }
  return btlHits;
}

std::set<MTDHitMatchingInfo> TrackMTDAssociator::FindMTDHits(const reco::TransientTrack &ttrack, const DetLayer* layer) const {

  auto cmp_for_detset = [](const unsigned one, const unsigned two) {
                          return one < two;
                        };

  TrajectoryStateOnSurface tsos = ttrack.outermostMeasurementState();

  std::set<MTDHitMatchingInfo> trackHits;

  std::pair<bool, TrajectoryStateOnSurface> compatibility = layer->compatible(tsos, *prop_, *estimator_);
  if(compatibility.first) {
    const std::vector<DetLayer::DetWithState> compDets = layer->compatibleDets(tsos, *prop_, *estimator_);
    for (const auto& detWithState : compDets) {

      auto range = mtdRecHits_->equal_range(detWithState.first->geographicalId(), cmp_for_detset);
      if (range.first == range.second) continue;

      auto pl = prop_->propagateWithPath(tsos, detWithState.second.surface());
      if(pl.second == 0.) continue;
      for (auto detitr = range.first; detitr != range.second; ++detitr) {
        for (const auto& hit : *detitr) {
          auto est = estimator_->estimate(detWithState.second, hit);
          if (!est.first) continue;

          TrackTofPidInfo tof = computeTrackTofPidInfo(ttrack.track().p2(),
                                                       std::abs(pl.second),
                                                       hit.time(),
                                                       hit.timeError(),
                                                       0.,
                                                       0.,
                                                       false);

	  
          MTDHitMatchingInfo mi;
          mi.hit = &hit;
          mi.time = hit.time();
          mi.estChi2 = est.second;
          mi.timeChi2 = tof.dtchi2_best;
          mi.pathLengthTsosToMTD = pl.second;
	  mi.id = detWithState.first->geographicalId();
	  mi.tsosPosition = tsos.globalPosition();
          trackHits.insert(mi);
        }
      }
    }
  }

  return trackHits;
}
