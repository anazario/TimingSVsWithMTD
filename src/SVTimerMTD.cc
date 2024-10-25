#include "TimingWithSVs/TimingSVsWithMTD/interface/SVTimerMTD.h"

// Constructor
SVTimerMTD::SVTimerMTD(const TransientTrackBuilder* ttBuilder,
                       const Propagator* prop,
                       const MTDDetLayerGeometry* mtdGeometry,
                       const MTDTrackingDetSetVector* mtdRecHits,
                       MeasurementEstimator* estimator) :
  associator_(std::make_unique<TrackMTDAssociator>(ttBuilder, mtdRecHits, prop, mtdGeometry, estimator)),
  ttBuilder_(ttBuilder) {}

// Function to calculate the average time of flight (TOF) for a single secondary vertex
// By default it uses the value in the t0() function, which currently assumes is the Pion mass hypothesis
// Currently using a sentinel value of -999 for both the track TOF, if there is no associated MTD hit,                                                                                                    
// and the sv TOF, if there are no tracks with MTD hits associated with it.  
double SVTimerMTD::CalculateSVAvgTof(const reco::TrackCollection &mtdTracks) const {
  if (mtdTracks.empty()) return -999.;
  std::vector<double> tofs;
  for (const auto &track : mtdTracks)
    tofs.push_back(track.t0());
  double sum = std::accumulate(tofs.begin(), tofs.end(), 0.0);
  return sum / tofs.size();
}

// Core function to get a single timed secondary vertex
reco::Vertex SVTimerMTD::GetTimedSV(const reco::Vertex &sv) const {
  // Get the position of the secondary vertex (SV) in global coordinates
  const GlobalPoint svPosition(sv.x(), sv.y(), sv.z());
  int totalHits(0);  // Counter for total MTD hits found
  reco::TrackCollection timeStampedTracks, mtdTracks;  

  // Loop over each track associated with the secondary vertex
  for (const auto &trackRef : sv.tracks()) {
    // Build a transient track from the track reference
    const reco::TransientTrack ttrack(ttBuilder_->build(*trackRef));

    // Get the outermost state of the trajectory and calculate path length to the track state
    const TrajectoryStateOnSurface tsos = ttrack.outermostMeasurementState();
    const double pathLengthToTsos(TimingHelper::PathLength(ttrack, svPosition, tsos.globalPosition()));

    // Use the TrackMTDAssociator to associate the track with MTD hits
    MTDIdInfo hitInfo = associator_->associate(*trackRef);

    double pathLength;

    // If a valid MTD hit is found, calculate the time and path length, and add a time-stamped track
    if (hitInfo.foundHit()) {
      totalHits++;
      pathLength = pathLengthToTsos + hitInfo.bestHitPathLength();  // Path length including MTD hit
      const double mtdTime(hitInfo.bestTime());  // Get the MTD hit time
      timeStampedTracks.emplace_back(TimingHelper::TimeStampTrack(*trackRef, pathLength, mtdTime));
      mtdTracks.emplace_back(TimingHelper::TimeStampTrack(*trackRef, pathLength, mtdTime));  // Collect MTD-associated tracks
    }
    // If no MTD hit, add a dummy time-stamped track
    else {
      timeStampedTracks.emplace_back(TimingHelper::TimeStampDummy(*trackRef));
    }
  }

  // Calculate the average time of flight (TOF) from MTD tracks
  double avgTof = CalculateSVAvgTof(mtdTracks);

  // Create a new vertex with the time-stamped information
  reco::Vertex timedSV(VertexHelper::TimeStampVertex(sv, avgTof));

  int index = 0;  // Index for matching tracks
  // Loop over the original tracks to add them to the new timed secondary vertex
  for (const auto &trackRef : sv.tracks()) {
    const reco::TransientTrack ttrack(ttBuilder_->build(timeStampedTracks.at(index)));
    timedSV.add(ttrack.trackBaseRef(), ttrack.track(), sv.trackWeight(trackRef));  // Add each time-stamped track to the vertex
    index++;
  }

  return timedSV;
}

// Get timed SVs for a collection
reco::VertexCollection SVTimerMTD::GetTimedSVs(const reco::VertexCollection &svs) const {
  reco::VertexCollection timedSVs;
  for (const auto &sv : svs) {
    timedSVs.emplace_back(GetTimedSV(sv));
  }
  return timedSVs;
}

reco::VertexCollection SVTimerMTD::GetWeightedTimedSVs(const reco::VertexCollection &svs, EMWeightedVertexTOF &emWeighter) const {
  reco::VertexCollection weightedTimedSVs;
  
  auto timedSVs = GetTimedSVs(svs);  // Get the timed vertices
  auto svIt = svs.begin();           // Iterator for the original SVs
  
  for (const auto &timedSV : timedSVs) {
    // Pass the original secondary vertex (svIt) as the second argument
    weightedTimedSVs.emplace_back(emWeighter.WeighVertex(timedSV, *svIt));
    ++svIt;  // Increment the iterator for the next original SV
  }

  return weightedTimedSVs;
}


// Get weighted timed SVs with default EMWeighter
reco::VertexCollection SVTimerMTD::GetWeightedTimedSVs(const reco::VertexCollection &svs) const {
  EMWeightedVertexTOF defaultWeighter(ttBuilder_, {0.7, 0.1, 0.1, 0.1});
  return GetWeightedTimedSVs(svs, defaultWeighter);
}
