#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingHelper.h"

double TimingHelper::PathLength(const reco::TransientTrack &track, const GlobalPoint &vertexPosition, const GlobalPoint &positionAtECAL) {

  //Calculate path length with helical extrapolation
  SteppingHelixPropagator propagator(track.field());

  //Calculate path length using the stepping helix propagator
  double pathLength = propagator.propagateWithPath(track.initialFreeState(), positionAtECAL).second;

  return pathLength;
}

double TimingHelper::LinearDistance(const GlobalPoint &vertexPosition, const GlobalPoint &positionAtECAL) {
  const double deltaX(positionAtECAL.x() - vertexPosition.x());
  const	double deltaY(positionAtECAL.y() - vertexPosition.y());
  const	double deltaZ(positionAtECAL.z() - vertexPosition.z());

  return sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
}

double TimingHelper::Beta(const reco::Track &track, const double mass) {
  return track.p() / sqrt(track.p()*track.p() + mass*mass);
}

double TimingHelper::Time(const reco::Track &track, const double mass, const double pathLength, const double mtdTime) {
  return mtdTime - pathLength/( Beta(track, mass) * 30);
}

reco::Track TimingHelper::TimeStampTrack(const reco::Track &track, const double pathLength, const double mtdTime) { 

  // Track parameters for creating new track instance with path length info
  const double chi2 = track.chi2();
  const int charge = track.charge();
  const double ndof = track.ndof();
  const math::XYZPoint trackRefPoint = track.referencePoint();
  const math::XYZVector trackMomentum = track.momentum();

  // Declare different mass hypotheses (pion, kaon, proton and electron) and calculate their tofs
  const double mass_pi(0.13957018); const double tof_pi(Time(track, mass_pi, pathLength, mtdTime));
  const	double mass_k(0.493677); const double tof_k(Time(track, mass_k, pathLength, mtdTime));
  const double mass_p(0.9382720813); const double tof_p(Time(track, mass_p, pathLength, mtdTime));
  const	double mass_e(0.000511); const double tof_e(Time(track, mass_e, pathLength, mtdTime));
  
  // Create track and save the tofs of the four different mass hypotheses
  const reco::Track updatedTrack(chi2, ndof, trackRefPoint, trackMomentum, charge, track.covariance(),
  				 track.algo(), reco::TrackBase::undefQuality,
                                 tof_pi, tof_k, tof_p, tof_e);

  return updatedTrack;
}

reco::Track TimingHelper::TimeStampDummy(const reco::Track &track) {

  const double chi2 = track.chi2();
  const int charge = track.charge();
  const double ndof = track.ndof();
  const math::XYZPoint trackRefPoint = track.referencePoint();
  const math::XYZVector trackMomentum = track.momentum();
  
  const reco::Track dummyTrack(chi2, ndof, trackRefPoint, trackMomentum, charge, track.covariance(),
			       track.algo(), reco::TrackBase::undefQuality,
			       -999., -999., -999., -999.);

  return dummyTrack;
}

reco::Track TimingHelper::UpdateTimeWithWeights(const reco::Track &track, const std::vector<double> &weights) {

  if (weights.size() != 4) throw std::invalid_argument("Weights vector must have exactly 4 elements");
  
  const double chi2 = track.chi2();
  const int charge = track.charge();
  const double ndof = track.ndof();
  const math::XYZPoint trackRefPoint = track.referencePoint();
  const math::XYZVector trackMomentum = track.momentum();

  const reco::Track updatedTrack(chi2, ndof, trackRefPoint, trackMomentum, charge, track.covariance(),
				 track.algo(), reco::TrackBase::undefQuality,
				 track.t0()*weights[0], track.beta()*weights[1],
				 track.covt0t0()*weights[2], track.covBetaBeta()*weights[3]);

  return updatedTrack;
}
