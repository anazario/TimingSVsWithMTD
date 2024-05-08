import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
from PhysicsTools.NanoAOD.common_cff import *
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

options.register('processName','Tree',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');
options.parseArguments()

process = cms.Process(options.processName,eras.Run2_2018)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load("Configuration.Geometry.GeometryExtended2026D98_cff")
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("Geometry.CommonTopologies.globalTrackingGeometry_cfi")
process.load("RecoTracker.Configuration.RecoTracker_cff")
process.load("RecoTracker.TrackProducer.TrackRefitters_cff")
process.load("RecoMTD.TransientTrackingRecHit.MTDTransientTrackingRecHitBuilder_cfi")

#verify which one to use
process.GlobalTag.globaltag = '133X_mcRun4_realistic_v1'
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:RelValTTbar_14TeV_2026D98noPU_GEN-SIM-RECO.root',
    ))

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

outfilename = 'testOutputFile.root'
options.register('outputFileName',outfilename,VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

process.TFileService = cms.Service("TFileService", fileName = cms.string(options.outputFileName))

process.timedVertices = cms.EDProducer("TimedSVsProducer",
                                       vertexLabel = cms.string("timedVertices"),
                                       electronSrc = cms.InputTag("gedGsfElectrons"),#"lowPtGsfElectrons"),
                                       tracksSrc = cms.InputTag("generalTracks"),
                                       svSrc =  cms.InputTag("inclusiveSecondaryVertices"),
                                       TrackTransformer = cms.PSet(
                                           DoPredictionsOnly = cms.bool(False),
                                           Fitter = cms.string('KFFitterForRefitInsideOut'),
                                           Smoother = cms.string('KFSmootherForRefitInsideOut'),
                                           Propagator = cms.string('PropagatorWithMaterialForMTD'),
                                           RefitDirection = cms.string('alongMomentum'),     
                                           RefitRPCHits = cms.bool(True),
                                           TrackerRecHitBuilder = cms.string('WithTrackAngle'),
                                           MuonRecHitBuilder = cms.string('MuonRecHitBuilder'),
                                           MTDRecHitBuilder = cms.string('MTDRecHitBuilder')
                                       ),
)

process.p = cms.Path(process.timedVertices)

process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string(outfilename),
                               outputCommands = cms.untracked.vstring('drop *',
                                                                      "keep *_timedVertices_*_*", )
)


process.e = cms.EndPath(process.out)

