#ifndef TimingWithSVs_TimingSVsWithMTD_TimingHelper_h
#define TimingWithSVs_TimingSVsWithMTD_TimingHelper_h

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"

class TimingHelper {

 public:

  static double PathLength(const reco::TransientTrack &track, const GlobalPoint &vertexPosition, const GlobalPoint &positionAtECAL);
  static double LinearDistance(const GlobalPoint &vertexPosition, const GlobalPoint &positionAtECAL);
  static double Beta(const reco::Track &track, const double mass);
  static double Time(const reco::Track &track, const double mass, const double pathLength, const double mtdTime = 0);
  static reco::Track TimeStampTrack(const reco::Track &track, const double pathLength, const double mtdTime = 0);
  static reco::Track TimeStampDummy(const reco::Track &track);
  static reco::Track UpdateTimeWithWeights(const reco::Track &track, const std::vector<double> &weights);
};

#endif
