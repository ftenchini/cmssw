import FWCore.ParameterSet.Config as cms

siPixelTemplateDBObjectESProducer = cms.ESProducer("SiPixelTemplateDBObjectESProducer",
                                                      MagneticFieldLabel = cms.string(""),
                                                   )
 
