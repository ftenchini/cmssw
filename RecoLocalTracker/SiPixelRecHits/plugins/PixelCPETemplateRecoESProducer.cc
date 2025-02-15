#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPETemplateRecoESProducer.h"
#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPETemplateReco.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Framework/interface/ESProducer.h"



#include <string>
#include <memory>

using namespace edm;

PixelCPETemplateRecoESProducer::PixelCPETemplateRecoESProducer(const edm::ParameterSet & p) 
{
  std::string myname = p.getParameter<std::string>("ComponentName");

  //DoLorentz_ = p.getParameter<bool>("DoLorentz"); // True when LA from alignment is used
  DoLorentz_ = p.existsAs<bool>("DoLorentz")?p.getParameter<bool>("DoLorentz"):false;

  pset_ = p;
  fieldlabel_ = p.getParameter<std::string>("MagneticFieldLabel");
  setWhatProduced(this,myname);

  //std::cout<<" from ES Producer Templates "<<myname<<" "<<DoLorentz_<<std::endl;  //dk

}

PixelCPETemplateRecoESProducer::~PixelCPETemplateRecoESProducer() {}

std::unique_ptr<PixelClusterParameterEstimator> 
PixelCPETemplateRecoESProducer::produce(const TkPixelCPERecord & iRecord){ 

  ESHandle<MagneticField> magfield;
  iRecord.getRecord<IdealMagneticFieldRecord>().get(fieldlabel_, magfield );

  edm::ESHandle<TrackerGeometry> pDD;
  iRecord.getRecord<TrackerDigiGeometryRecord>().get( pDD );

  edm::ESHandle<TrackerTopology> hTT;
  iRecord.getRecord<TrackerDigiGeometryRecord>().getRecord<TrackerTopologyRcd>().get(hTT);

  edm::ESHandle<SiPixelLorentzAngle> lorentzAngle;
  const SiPixelLorentzAngle * lorentzAngleProduct = nullptr;
  if(DoLorentz_) { //  LA correction from alignment 
    iRecord.getRecord<SiPixelLorentzAngleRcd>().get("fromAlignment",lorentzAngle);
    lorentzAngleProduct = lorentzAngle.product();
  } else { // Normal, deafult LA actually is NOT needed
    //iRecord.getRecord<SiPixelLorentzAngleRcd>().get(lorentzAngle);
    lorentzAngleProduct=nullptr;  // null is ok becuse LA is not use by templates in this mode
  }

  ESHandle<SiPixelTemplateDBObject> templateDBobject;
  iRecord.getRecord<SiPixelTemplateDBObjectESProducerRcd>().get(templateDBobject);

  return std::make_unique<PixelCPETemplateReco>(pset_,magfield.product(),*pDD.product(),*hTT.product(),lorentzAngleProduct,templateDBobject.product() );
}


