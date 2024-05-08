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

#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/GeomPropagators/interface/HelixExtrapolatorToLine2Order.h"
#include "TrackingTools/TrackRefitter/interface/TrackTransformer.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"

#include "RecoVertex/AdaptiveVertexFinder/interface/AdaptiveVertexReconstructor.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

#include "TimingWithSVs/TimingSVsWithMTD/interface/VertexHelper.h"

//
// class declaration
//

class TimedSVsProducer : public edm::stream::EDProducer<> {
public:
  explicit TimedSVsProducer(const edm::ParameterSet&);
  ~TimedSVsProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // ----------member data ---------------------------
  std::string vertexLabel_;

  edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  edm::EDGetTokenT<reco::VertexCollection> svToken_;

  std::unique_ptr<TrackTransformer> theTransformer_;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> transientTrackBuilder_;  
};

TimedSVsProducer::TimedSVsProducer(const edm::ParameterSet& iConfig) 
  : tracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracksSrc"))),
    svToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("svSrc"))),
    theTransformer_(std::make_unique<TrackTransformer>(iConfig.getParameterSet("TrackTransformer"), consumesCollector())),
    transientTrackBuilder_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))) {

  vertexLabel_ = iConfig.getParameter<std::string>("vertexLabel");
  produces<reco::VertexCollection>(vertexLabel_).setBranchAlias(vertexLabel_);

}

TimedSVsProducer::~TimedSVsProducer() {}

//
// member functions
//
// ------------ method called to produce the data  ------------
void TimedSVsProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

  std::unique_ptr<reco::VertexCollection> timedVertices = std::make_unique<reco::VertexCollection>();

  const edm::Handle<reco::TrackCollection> tracksHandle = iEvent.getHandle(tracksToken_);
  const edm::Handle<reco::VertexCollection> svHandle = iEvent.getHandle(svToken_);

  const TransientTrackBuilder* ttBuilder = &iSetup.getData(transientTrackBuilder_);
  theTransformer_->setServices(iSetup);

  std::cout << "script is running" << std::endl;

  for(const reco::Vertex &sv : *svHandle) {
    
    reco::TrackCollection svTracks = VertexHelper::GetTracks(sv);
    for(const reco::Track &track : svTracks) {

      const auto& trajs = theTransformer_->transform(track);
      const reco::TransientTrack ttrack(ttBuilder->build(track));

      if(!trajs.empty())
	std::cout << "Trajectory succesful" << std::endl;
    }
  }

  iEvent.put(std::move(timedVertices), "timedVertices");
}// Producer end

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void TimedSVsProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(TimedSVsProducer);
