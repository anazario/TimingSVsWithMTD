// -*- C++ -*-
//
// Package:    TimingWithSVs/SVsTimingProducer
// Class:      SVsTimingProducer
//
/**\class SVsTimingProducer SVsTimingProducer.cc TimingWithSVs/SVsTimingProducer/plugins/SVsTimingProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andres Abreu
//         Created:  Sat, 21 Oct 2023 20:06:20 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "RecoMTD/DetLayers/interface/MTDDetLayerGeometry.h"
#include "RecoMTD/Records/interface/MTDRecoGeometryRecord.h"

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/TrackerRecHit2D/interface/MTDTrackingRecHit.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/GeomPropagators/interface/HelixExtrapolatorToLine2Order.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "TrackingTools/DetLayers/interface/ForwardDetLayer.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"

#include "RecoVertex/AdaptiveVertexFinder/interface/AdaptiveVertexReconstructor.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

#include "TimingWithSVs/TimingSVsWithMTD/interface/VertexHelper.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingHelper.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/MTDHitMatchingInfo.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/TimingUtility.h" 
#include "TimingWithSVs/TimingSVsWithMTD/interface/SVTimerMTD.h"

//
// class declaration
//

float computeTof(float mass_inv2);

double FindTrackDT(const reco::TransientTrack &ttrack,
		   const std::vector<const DetLayer*> &layers,
		   const Propagator* prop,
		   const MTDTrackingDetSetVector mtdRecHits,
		   MeasurementEstimator* estimator);

class TimedSVsProducerMTD : public edm::stream::EDProducer<> {
public:
  explicit TimedSVsProducerMTD(const edm::ParameterSet&);
  ~TimedSVsProducerMTD() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  
  void produce(edm::Event&, const edm::EventSetup&) override;

  // ----------member data ---------------------------
  typedef std::vector<std::pair<reco::Track, reco::Track>> TimedTrackMap;
  
  std::string vertexLabel_;

  edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  edm::EDGetTokenT<reco::VertexCollection> svToken_;
  edm::EDGetTokenT<MTDTrackingDetSetVector> mtdRecHitsToken_;
  
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> transientTrackBuilder_;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> magneticFieldToken_;

  const std::string propagator_;
  edm::ESGetToken<Propagator, TrackingComponentsRecord> propToken_;

  edm::ESGetToken<MTDDetLayerGeometry, MTDRecoGeometryRecord> dlgeoToken_;
  
  std::unique_ptr<MeasurementEstimator> theEstimator_;

  const float estMaxChi2_;
  const float estMaxNSigma_;

};

TimedSVsProducerMTD::TimedSVsProducerMTD(const edm::ParameterSet& iConfig) 
  : tracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracksSrc"))),
    svToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("svSrc"))),
    mtdRecHitsToken_(consumes<MTDTrackingDetSetVector>(iConfig.getParameter<edm::InputTag>("mtdRecHitsSrc"))),
    transientTrackBuilder_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))),
    magneticFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
    propagator_(iConfig.getParameter<std::string>("Propagator")),
    dlgeoToken_(esConsumes<MTDDetLayerGeometry, MTDRecoGeometryRecord>()),
    estMaxChi2_(iConfig.getParameter<double>("estimatorMaxChi2")),
    estMaxNSigma_(iConfig.getParameter<double>("estimatorMaxNSigma"))
{

  //std::cout << "initializing constructor" << std::endl; 
  propToken_ = esConsumes<Propagator, TrackingComponentsRecord>(edm::ESInputTag("", propagator_));
  theEstimator_ = std::make_unique<Chi2MeasurementEstimator>(estMaxChi2_, estMaxNSigma_);

  produces<reco::VertexCollection>("timedVertices").setBranchAlias("timedVertices");
  produces<reco::VertexCollection>("emWeightedTimedVertices").setBranchAlias("emWeightedTimedVertices");
}

TimedSVsProducerMTD::~TimedSVsProducerMTD() {};

//
// member functions
//
// ------------ method called to produce the data  ------------
void TimedSVsProducerMTD::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

  std::unique_ptr<reco::VertexCollection> timedVertices = std::make_unique<reco::VertexCollection>();
  std::unique_ptr<reco::VertexCollection> emWeightedTimedVertices = std::make_unique<reco::VertexCollection>();
  
  //std::unique_ptr<reco::VertexCollection> timedVerticesLinearDist = std::make_unique<reco::VertexCollection>();

  const edm::Handle<reco::TrackCollection> tracksHandle = iEvent.getHandle(tracksToken_);
  const edm::Handle<reco::VertexCollection> svHandle = iEvent.getHandle(svToken_);
  const TransientTrackBuilder* ttBuilder = &iSetup.getData(transientTrackBuilder_);
  const edm::Handle<MTDTrackingDetSetVector> mtdRecHits = iEvent.getHandle(mtdRecHitsToken_);
  //primaryVertex_ = iEvent.get(pvToken_).at(0);
  
  auto magfield = iSetup.getTransientHandle(magneticFieldToken_);
  auto propH = iSetup.getTransientHandle(propToken_);
  const Propagator* prop = propH.product();
  auto geo = iSetup.getTransientHandle(dlgeoToken_);

  SVTimerMTD svTimer(ttBuilder, prop, geo.product(), mtdRecHits.product(), theEstimator_.get());
  reco::VertexCollection timedSVs(svTimer.GetTimedSVs(*svHandle));
  reco::VertexCollection timedSVsWithWeights(svTimer.GetWeightedTimedSVs(*svHandle));

  timedVertices->insert(timedVertices->end(), timedSVs.begin(), timedSVs.end());
  emWeightedTimedVertices->insert(emWeightedTimedVertices->end(), timedSVsWithWeights.begin(), timedSVsWithWeights.end());
  
  iEvent.put(std::move(timedVertices), "timedVertices");
  iEvent.put(std::move(emWeightedTimedVertices), "emWeightedTimedVertices");
}// Producer end

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void TimedSVsProducerMTD::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(TimedSVsProducerMTD);
