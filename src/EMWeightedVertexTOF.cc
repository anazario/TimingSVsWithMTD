#include "TimingWithSVs/TimingSVsWithMTD/interface/EMWeightedVertexTOF.h"

EMWeightedVertexTOF::EMWeightedVertexTOF(const TransientTrackBuilder* ttBuilder,
					 const std::vector<double> &initialWeights,
					 const int maxIter,
					 const double alpha,
                                         const double sigma,
					 const double threshold)
  : ttBuilder_(ttBuilder),
    initialWeights_(initialWeights),
    maxIter_(maxIter),
    alpha_(alpha),
    sigma_(sigma),
    threshold_(threshold),
    loglikelihood_(std::vector<double>()),
    ignoredIndices_(std::vector<int>()) {}

reco::Vertex EMWeightedVertexTOF::WeighVertex(const reco::Vertex &timedSV, const reco::Vertex &originalSV) {

  loglikelihood_.clear();
  
  reco::TrackCollection mtdTracks;
  for (const auto &track : timedSV.refittedTracks())
    if (track.t0() > -500.) mtdTracks.emplace_back(track);
  
  if (mtdTracks.size() < 2) return timedSV;

  const std::vector<double> sigmas(4, sigma_);
  
  auto responsibilities = initializeResponsibilities(mtdTracks);
  double svTof = 0.0;
  
  // Get initial log likelihood
  loglikelihood_.push_back(calculateNegativeLogLikelihood(mtdTracks, responsibilities, svTof, sigmas));

  for (int i = 0; i < maxIter_; i++) {
    double tempTof = updateVertexTime(mtdTracks, responsibilities);
    //if (fabs(svTof - tempTof) < threshold_) break;
    
    svTof = tempTof;
    updateResponsibilities(mtdTracks, responsibilities, svTof);

    // Recalculate and store the log likelihood
    const double newLogLikelihood(calculateNegativeLogLikelihood(mtdTracks, responsibilities, svTof, sigmas));
    loglikelihood_.push_back(newLogLikelihood);

    // Check for convergence
    if(fabs(loglikelihood_[i+1] - loglikelihood_[i]) < threshold_) break;
  }
  
  //if(int(loglikelihood_.size()) == maxIter_)
  //DisplayFitInfo(mtdTracks, responsibilities, svTof);

  /*
  std::cout << "\nFinal responsibilities: ";
  for(const auto &weight : responsibilities) {
    std::cout << "pion: " << weight[0] << ", ";
    std::cout << "kaon: " << weight[1] << ", ";
    std::cout << "proton: " << weight[2] << ", ";
    std::cout << "electron: " << weight[3] << ", ";
  }
  std::cout << std::endl;
  */
  
  auto updatedTracks = updateTrackTimes(mtdTracks, responsibilities);
  return createWeightedVertex(timedSV, originalSV, updatedTracks, svTof);
}

std::vector<std::vector<double>> EMWeightedVertexTOF::initializeResponsibilities(const reco::TrackCollection &tracks) const {
  return std::vector<std::vector<double>>(tracks.size(), initialWeights_);
}

double EMWeightedVertexTOF::updateVertexTime(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities) const {
  double tempTof = 0.0;
  for (size_t t = 0; t < tracks.size(); ++t) {
    reco::Track track(tracks[t]);
    std::vector<double> originalTofs({track.t0(), track.beta(), track.covt0t0(), track.covBetaBeta()});
    for (size_t k = 0; k < originalTofs.size() && notIgnored(k); ++k) {
      tempTof += originalTofs[k] * responsibilities[t][k];
      //if(!notIgnored(k)) std::cout << "notIgnored() is not working on index: " << k << std::endl;
    }
  }
  return tempTof / double(tracks.size());
}

void EMWeightedVertexTOF::updateResponsibilities(const reco::TrackCollection &tracks, std::vector<std::vector<double>> &responsibilities, double vertexTime) const {
  const std::vector<double> masses({0.13957018, 0.493677, 0.9382720813, 0.000511});
  const std::vector<double> sigmas(4, sigma_);
  
  for (size_t t = 0; t < tracks.size(); ++t) {
    reco::Track track(tracks[t]);
    std::vector<double> originalTofs({track.t0(), track.beta(), track.covt0t0(), track.covBetaBeta()});
    std::vector<double> expArguments(originalTofs.size() - ignoredIndices_.size());
    double max_value = std::numeric_limits<double>::lowest();
    
    for (size_t k = 0; k < originalTofs.size() && notIgnored(k); ++k) {
      double diff = originalTofs[k] - vertexTime;
      double value = -diff * diff / (2 * sigmas[k] * sigmas[k]) - alpha_ * masses[k];
      expArguments[k] = value;
      max_value = std::max(max_value, value);
    }
    
    std::transform(expArguments.begin(), expArguments.end(), expArguments.begin(),
		   [max_value](double arg) { return arg - max_value; });
    
    double denominator = 0.0;
    for (size_t k = 0; k < originalTofs.size() && notIgnored(k); ++k)
      denominator += std::exp(expArguments[k]) / sigmas[k];
    
    for (size_t k = 0; k < originalTofs.size() && notIgnored(k); ++k)
      responsibilities[t][k] = (std::exp(expArguments[k]) / sigmas[k]) / denominator;
  }
  
}

reco::TrackCollection EMWeightedVertexTOF::updateTrackTimes(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities) const {
  reco::TrackCollection updatedTracks;
  for (size_t t = 0; t < tracks.size(); ++t)
    updatedTracks.emplace_back(TimingHelper::UpdateTimeWithWeights(tracks[t], responsibilities[t]));
  return updatedTracks;
}

reco::Vertex EMWeightedVertexTOF::createWeightedVertex(const reco::Vertex &timedSV, const reco::Vertex &originalSV, const reco::TrackCollection &updatedTracks, double vertexTime) const {
  reco::Vertex weightedTimedSV(VertexHelper::TimeStampVertex(timedSV, vertexTime));
  
  for (const auto &trackRef : originalSV.tracks()) {
    const reco::Track track(*trackRef);
    if (!TrackHelper::FindTrackInCollection(track, updatedTracks)) {
      const reco::TransientTrack ttrack(ttBuilder_->build(TimingHelper::TimeStampDummy(*trackRef)));
      weightedTimedSV.add(ttrack.trackBaseRef(), ttrack.track(), timedSV.trackWeight(trackRef));
    } else {
      const int index(TrackHelper::FindTrackIndex(track, updatedTracks));
      const reco::TransientTrack ttrack(ttBuilder_->build(updatedTracks[index]));
      weightedTimedSV.add(ttrack.trackBaseRef(), ttrack.track(), timedSV.trackWeight(trackRef));
    }
  }
  
  return weightedTimedSV;
}

double EMWeightedVertexTOF::calculateNegativeLogLikelihood(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities,
							   const double vertexTime, const std::vector<double> &sigmas) const {
  double logLikelihood = 0.0;

  for (size_t t = 0; t < tracks.size(); ++t) {
    reco::Track track(tracks[t]);
    // Extract ToF information (t0, beta, covariance, etc.)
    std::vector<double> originalTofs({track.t0(), track.beta(), track.covt0t0(), track.covBetaBeta()});
    
    // Iterate over each hypothesis (mass, particle type) and calculate the contribution to the log-likelihood
    for (size_t k = 0; k < originalTofs.size() && notIgnored(k); ++k) {
      double diff = originalTofs[k] - vertexTime;
      // Add the negative log-likelihood term for the current hypothesis and track
      logLikelihood -= responsibilities[t][k] * (diff * diff) / (2 * sigmas[k] * sigmas[k]);
    }
  }
  
  return logLikelihood;
}

bool EMWeightedVertexTOF::notIgnored(size_t i) const {
    return std::find(ignoredIndices_.begin(), ignoredIndices_.end(), i) == ignoredIndices_.end();
}

void EMWeightedVertexTOF::DisplayFitInfo(const reco::TrackCollection &tracks, const std::vector<std::vector<double>> &responsibilities, double finalTof) const {
    // Display number of iterations (loglikelihood_ size)
    std::cout << "Number of iterations: " << loglikelihood_.size() << std::endl;

    // Display log-likelihood values
    std::cout << "Log-likelihood progression: " << std::endl;
    for (size_t i = 0; i < loglikelihood_.size(); ++i) {
        std::cout << "  Iteration " << i + 1 << ": " << std::fixed << std::setprecision(6) << loglikelihood_[i] << std::endl;
    }

    // Display final TOF
    std::cout << "\nFinal Vertex TOF: " << std::fixed << std::setprecision(3) << finalTof << std::endl;

    // Display final weights for each track and hypothesis
    std::cout << "\nFinal Weights for each track:" << std::endl;
    for (size_t t = 0; t < tracks.size(); ++t) {
        std::cout << "  Track " << t + 1 << " (t0: " << std::fixed << std::setprecision(3) << tracks[t].t0() << "): ";
        for (size_t k = 0; k < responsibilities[t].size() && notIgnored(k); ++k) {
            std::cout << "Hypothesis " << k + 1 << ": " << std::fixed << std::setprecision(3) << responsibilities[t][k] << "  ";
        }
        std::cout << std::endl;
    }

    // Display any hypotheses that were ignored
    if (!ignoredIndices_.empty()) {
        std::cout << "\nIgnored hypotheses: ";
        for (int idx : ignoredIndices_) {
            std::cout << idx + 1 << " ";  // Display indices starting from 1 for user readability
        }
        std::cout << std::endl;
    } else {
        std::cout << "\nNo hypotheses were ignored." << std::endl;
    }
}
