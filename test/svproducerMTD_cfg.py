import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
from PhysicsTools.NanoAOD.common_cff import *
from FWCore.ParameterSet.VarParsing import VarParsing
#from RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilder_cfi import *

options = VarParsing('python')

options.register('processName','Tree',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');
options.parseArguments()

process = cms.Process(options.processName,eras.Phase2C17I13M9)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("RecoTracker.Configuration.RecoTracker_cff")
process.load("RecoTracker.TrackProducer.TrackRefitters_cff")
process.load('Configuration.Geometry.GeometryExtended2026D98Reco_cff')
process.load("TrackingTools.TrackAssociator.DetIdAssociatorESProducer_cff")
process.load("RecoMTD.TrackExtender.PropagatorWithMaterialForMTD_cfi")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T25', '')
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

from TimingWithSVs.TimingSVsWithMTD.TimedSVsMTD_cfi import *
process.timedVertices = timedSVsMTD
process.p = cms.Path(process.timedVertices)

process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string(outfilename),
                               outputCommands = cms.untracked.vstring('drop *',
                                                                      "keep *_timedVertices_*_*",
                                                                      "keep *_emWeightedTimedVertices_*_*",)
)

process.e = cms.EndPath(process.out)

