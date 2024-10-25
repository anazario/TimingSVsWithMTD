#ifndef TimingWithSVs_TimingSVsWithMTD_EMWeightedVertexTOF_h
#define TimingWithSVs_TimingSVsWithMTD_EMWeightedVertexTOF_h

#include <iomanip>

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

class EMWeightedVertexTOF {
 public:

  EMWeightedVertexTOF(const TransientTrackBuilder* ttBuilder,
		      const std::vector<double> &initialWeights = std::vector<double>(4, 0.25),
		      const int maxIter = 30,
		      const double alpha = 0.,
		      const double sigma = 0.05,
		      const double threshold = 1e-5);
  
  // Public method to weigh the vertex TOF 
  reco::Vertex WeighVertex(const reco::Vertex &timedSV, const reco::Vertex &originalSV);
  std::vector<double> LikelihoodSummary() const {return loglikelihood_;}
  void SetHypothesesToIgnore(const std::vector<int> &indices) {ignoredIndices_ = indices;}
  void DisplayFitInfo(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities, double finalTof) const;
  
 private:
  std::vector<std::vector<double>> initializeResponsibilities(const reco::TrackCollection &tracks) const;
  double updateVertexTime(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities) const;
  void updateResponsibilities(const reco::TrackCollection &tracks, std::vector<std::vector<double>> &responsibilities, double vertexTime) const;
  reco::TrackCollection updateTrackTimes(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities) const;
  reco::Vertex createWeightedVertex(const reco::Vertex &timedSV, const reco::Vertex &originalSV, const reco::TrackCollection &updatedTracks, double vertexTime) const;
  double calculateNegativeLogLikelihood(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities, 
					double vertexTime, const std::vector<double> &sigmas) const;
  bool notIgnored(size_t i) const; 
  // Member variables for alpha, sigma, and ttBuilder
  const TransientTrackBuilder* ttBuilder_;
  const std::vector<double> initialWeights_;
  
  const int maxIter_;
  const double alpha_;
  const double sigma_;
  const double threshold_;
  std::vector<double> loglikelihood_;

  std::vector<int> ignoredIndices_;
};

#endif
