import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Modifier_run2_nanoAOD_LowPU_cff import run2_nanoAOD_LowPU
from PhysicsTools.NanoAOD.common_cff import *

# Bad/clone muon filters - tagging mode to keep the event
from RecoMET.METFilters.badGlobalMuonTaggersMiniAOD_cff import badGlobalMuonTaggerMAOD, cloneGlobalMuonTaggerMAOD
badGlobalMuonTagger = badGlobalMuonTaggerMAOD.clone(
    taggingMode = True
)

cloneGlobalMuonTagger = cloneGlobalMuonTaggerMAOD.clone(
    taggingMode = True
)

from RecoMET.METFilters.BadPFMuonFilter_cfi import BadPFMuonFilter
BadPFMuonTagger = BadPFMuonFilter.clone(
    PFCandidates = cms.InputTag("packedPFCandidates"),
    muons = cms.InputTag("slimmedMuons"),
    vtx = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taggingMode = True,
)

# Bad charge hadron
from RecoMET.METFilters.BadChargedCandidateFilter_cfi import BadChargedCandidateFilter
BadChargedCandidateTagger = BadChargedCandidateFilter.clone(
    PFCandidates = cms.InputTag("packedPFCandidates"),
    muons = cms.InputTag("slimmedMuons"),
    vtx = cms.InputTag("offlineSlimmedPrimaryVertices"),
    taggingMode = True,
)

extraFlagsTable = cms.EDProducer("GlobalVariablesTableProducer",
    variables = cms.PSet(
        Flag_BadGlobalMuon = ExtVar(cms.InputTag("badGlobalMuonTagger:notBadEvent"), bool, doc = "Bad muon flag"),
        Flag_CloneGlobalMuon = ExtVar(cms.InputTag("cloneGlobalMuonTagger:notBadEvent"), bool, doc = "Clone muon flag"),
        Flag_BadPFMuonFilter = ExtVar(cms.InputTag("BadPFMuonTagger"), bool, doc = "Bad PF muon flag"),
        Flag_BadChargedCandidateFilter = ExtVar(cms.InputTag("BadChargedCandidateTagger"), bool, doc = "Bad charged hadron flag"),
    )
)

extraFlagsProducers = cms.Sequence(badGlobalMuonTagger + cloneGlobalMuonTagger + BadPFMuonTagger + BadChargedCandidateTagger)

from RecoMET.METFilters.ecalBadCalibFilter_cfi import *
run2_nanoAOD_LowPU.toModify(ecalBadCalibFilter, EcalRecHitSource = 'reducedEgamma:reducedEERecHits')

ecalBadCalibFilterNanoTagger = ecalBadCalibFilter.clone(
    taggingMode = cms.bool(True),
)

extraFlagsProducers102x = cms.Sequence(ecalBadCalibFilterNanoTagger)
