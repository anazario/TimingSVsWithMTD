#ifndef TimingWithSVs_TimingSVsWithMTD_SVTimerMTD_h
#define TimingWithSVs_TimingSVsWithMTD_SVTimerMTD_h

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/TrackerRecHit2D/interface/MTDTrackingRecHit.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "TrackingTools/DetLayers/interface/ForwardDetLayer.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"

#include "RecoMTD/DetLayers/interface/MTDDetLayerGeometry.h"
#include "RecoMTD/Records/interface/MTDRecoGeometryRecord.h"

#include "TimingWithSVs/TimingSVsWithMTD/interface/VertexHelper.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingHelper.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TrackHelper.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/MTDHitMatchingInfo.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingUtility.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/EMWeightedVertexTOF.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TrackMTDAssociator.h"

class SVTimerMTD {

public:

  SVTimerMTD(const TransientTrackBuilder* ttBuilder,
             const Propagator* prop,
             const MTDDetLayerGeometry* mtdGeometry,
             const MTDTrackingDetSetVector* mtdRecHits,
             MeasurementEstimator* estimator);

  virtual ~SVTimerMTD() = default;

  // Public functions to get timed secondary vertices
  reco::Vertex GetTimedSV(const reco::Vertex &sv) const;
  reco::VertexCollection GetTimedSVs(const reco::VertexCollection &svs) const;

  // Public functions to get weighted timed secondary vertices
  reco::VertexCollection GetWeightedTimedSVs(const reco::VertexCollection &svs, EMWeightedVertexTOF &emWeighter) const;
  reco::VertexCollection GetWeightedTimedSVs(const reco::VertexCollection &svs) const;

private:

  const std::unique_ptr<TrackMTDAssociator> associator_;
  const TransientTrackBuilder* ttBuilder_;

  double CalculateSVAvgTof(const reco::TrackCollection &mtdTracks) const;

};

#endif
