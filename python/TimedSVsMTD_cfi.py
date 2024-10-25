import FWCore.ParameterSet.Config as cms

timedSVsMTD = cms.EDProducer('TimedSVsProducerMTD',
                             electronSrc = cms.InputTag("gedGsfElectrons"),
                             tracksSrc = cms.InputTag("generalTracks"),
                             svSrc =  cms.InputTag("inclusiveSecondaryVertices"),
                             mtdRecHitsSrc = cms.InputTag("mtdTrackingRecHits"),
                             Propagator = cms.string('PropagatorWithMaterialForMTD'),
                             estimatorMaxChi2 = cms.double(500),
                             estimatorMaxNSigma = cms.double(10),
                             vertices = cms.InputTag("offlinePrimaryVertices"),
)
