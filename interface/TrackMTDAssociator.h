#ifndef TimingWithSVs_TimingSVsWithMTD_TrackMTDAssociator_h
#define TimingWithSVs_TimingSVsWithMTD_TrackMTDAssociator_h

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "TrackingTools/DetLayers/interface/ForwardDetLayer.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"

#include "RecoMTD/DetLayers/interface/MTDDetLayerGeometry.h"
#include "RecoMTD/Records/interface/MTDRecoGeometryRecord.h"

#include "TimingWithSVs/TimingSVsWithMTD/interface/MTDHitMatchingInfo.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingUtility.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/MTDIdInfo.h"

class TrackMTDAssociator {

public:

  TrackMTDAssociator() = default;
  
  TrackMTDAssociator(const TransientTrackBuilder* ttBuilder,
		     const MTDTrackingDetSetVector* mtdRecHits,
		     const Propagator* prop,
		     const MTDDetLayerGeometry* mtdGeometry,
		     MeasurementEstimator* estimator);

  MTDIdInfo associate(const reco::Track &track) const;

  virtual ~TrackMTDAssociator() = default;
  
private:

  const TransientTrackBuilder* ttBuilder_;
  const MTDTrackingDetSetVector* mtdRecHits_;
  const Propagator* prop_;
  const MTDDetLayerGeometry* mtdGeometry_;
  MeasurementEstimator* estimator_;

  std::set<MTDHitMatchingInfo> tryETLLayers(const reco::TransientTrack &ttrack) const;
  std::set<MTDHitMatchingInfo> tryBTLLayers(const reco::TransientTrack &ttrack) const;
  std::set<MTDHitMatchingInfo> FindMTDHits(const reco::TransientTrack &ttrack, const DetLayer* layer) const;
  
};

#endif
