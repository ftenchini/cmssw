#include "FWCore/PluginManager/interface/PluginManager.h"

#include "SimG4Core/GeometryProducer/interface/GeometryProducer.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "SimG4Core/Geometry/interface/DDDWorld.h"
#include "SimG4Core/Geometry/interface/G4LogicalVolumeToDDLogicalPartMap.h"
#include "SimG4Core/Geometry/interface/SensitiveDetectorCatalog.h"
#include "SimG4Core/MagneticField/interface/CMSFieldManager.h"
#include "SimG4Core/MagneticField/interface/Field.h"
#include "SimG4Core/MagneticField/interface/FieldBuilder.h"
#include "SimG4Core/Notification/interface/SimTrackManager.h"
#include "SimG4Core/Watcher/interface/SimProducer.h"
#include "SimG4Core/Watcher/interface/SimWatcherFactory.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "Geometry/Records/interface/IdealGeometryRecord.h"

#include "DetectorDescription/Core/interface/DDCompactView.h"

#include "G4RunManagerKernel.hh"
#include "G4TransportationManager.hh"

#include <iostream>

static void createWatchers(const edm::ParameterSet &iP,
                           SimActivityRegistry &iReg,
                           std::vector<std::shared_ptr<SimWatcher>> &oWatchers,
                           std::vector<std::shared_ptr<SimProducer>> &oProds) {
  using namespace std;
  using namespace edm;
  std::vector<ParameterSet> watchers;
  try {
    watchers = iP.getParameter<vector<ParameterSet>>("Watchers");
  } catch (edm::Exception const &) {
  }

  for (std::vector<ParameterSet>::iterator itWatcher = watchers.begin(); itWatcher != watchers.end(); ++itWatcher) {
    std::unique_ptr<SimWatcherMakerBase> maker(
        SimWatcherFactory::get()->create(itWatcher->getParameter<std::string>("type")));
    if (maker.get() == nullptr) {
      throw cms::Exception("SimG4CoreGeometryProducer", " createWatchers: Unable to find the requested Watcher");
    }

    std::shared_ptr<SimWatcher> watcherTemp;
    std::shared_ptr<SimProducer> producerTemp;
    maker->make(*itWatcher, iReg, watcherTemp, producerTemp);
    oWatchers.push_back(watcherTemp);
    if (producerTemp)
      oProds.push_back(producerTemp);
  }
}

GeometryProducer::GeometryProducer(edm::ParameterSet const &p)
    : m_kernel(nullptr),
      m_fieldlabel(p.getParameter<std::string>("MagneticFieldLabel")),
      m_pField(p.getParameter<edm::ParameterSet>("MagneticField")),
      m_attach(nullptr),
      m_p(p),
      m_pDD(nullptr),
      m_firstRun(true),
      m_pUseMagneticField(p.getParameter<bool>("UseMagneticField")),
      m_pUseSensitiveDetectors(p.getParameter<bool>("UseSensitiveDetectors")) {
  // Look for an outside SimActivityRegistry
  // this is used by the visualization code
  edm::Service<SimActivityRegistry> otherRegistry;
  if (otherRegistry)
    m_registry.connect(*otherRegistry);
  createWatchers(m_p, m_registry, m_watchers, m_producers);
  produces<int>();
}

GeometryProducer::~GeometryProducer() {
  delete m_attach;
  delete m_kernel;
}

void GeometryProducer::updateMagneticField(edm::EventSetup const &es) {
  if (m_pUseMagneticField) {
    // setup the magnetic field
    edm::ESHandle<MagneticField> pMF;
    es.get<IdealMagneticFieldRecord>().get(m_fieldlabel, pMF);
    const GlobalPoint g(0., 0., 0.);
    edm::LogInfo("GeometryProducer") << "B-field(T) at (0,0,0)(cm): " << pMF->inTesla(g);

    sim::FieldBuilder fieldBuilder(pMF.product(), m_pField);
    CMSFieldManager *fieldManager = new CMSFieldManager();
    G4TransportationManager *tM = G4TransportationManager::GetTransportationManager();
    tM->SetFieldManager(fieldManager);
    fieldBuilder.build(fieldManager, tM->GetPropagatorInField());
    edm::LogInfo("GeometryProducer") << "Magentic field is built";
  }
}

void GeometryProducer::beginLuminosityBlock(edm::LuminosityBlock &, edm::EventSetup const &) {
  // mag field cannot be change in new lumi section - this is commented out
  //     updateMagneticField( es );
}

void GeometryProducer::beginRun(const edm::Run &run, const edm::EventSetup &es) { updateMagneticField(es); }

void GeometryProducer::endRun(const edm::Run &, const edm::EventSetup &) {}

void GeometryProducer::produce(edm::Event &e, const edm::EventSetup &es) {
  if (!m_firstRun)
    return;
  m_firstRun = false;

  edm::LogVerbatim("GeometryProducer") << "Producing G4 Geom";

  m_kernel = G4RunManagerKernel::GetRunManagerKernel();
  if (m_kernel == nullptr)
    m_kernel = new G4RunManagerKernel();
  edm::LogVerbatim("GeometryProducer") << " GeometryProducer initializing ";
  // DDDWorld: get the DDCV from the ES and use it to build the World

  edm::ESTransientHandle<DDCompactView> pDD;
  es.get<IdealGeometryRecord>().get(pDD);
  m_pDD = pDD.product();

  SensitiveDetectorCatalog catalog;
  G4LogicalVolumeToDDLogicalPartMap map;

  const DDDWorld *dddworld = new DDDWorld(&(*pDD), map, catalog, false);
  G4VPhysicalVolume *world = dddworld->GetWorldVolumeForWorker();

  if (nullptr != world)
    edm::LogVerbatim("GeometryProducer") << " World Volume: " << world->GetName();
  m_kernel->DefineWorldVolume(world, true);

  m_registry.dddWorldSignal_(dddworld);

  edm::LogVerbatim("GeometryProducer") << " Magnetic field initialisation";
  updateMagneticField(es);

  if (m_pUseSensitiveDetectors) {
    edm::LogInfo("GeometryProducer") << " instantiating sensitive detectors ";
    // instantiate and attach the sensitive detectors
    m_trackManager = std::unique_ptr<SimTrackManager>(new SimTrackManager);
    if (m_attach == nullptr)
      m_attach = new AttachSD;
    {
      std::pair<std::vector<SensitiveTkDetector *>, std::vector<SensitiveCaloDetector *>> sensDets =
          m_attach->create((*m_pDD), catalog, m_p, m_trackManager.get(), m_registry);

      m_sensTkDets.swap(sensDets.first);
      m_sensCaloDets.swap(sensDets.second);
    }

    edm::LogInfo("GeometryProducer") << " Sensitive Detector building finished; found " << m_sensTkDets.size()
                                     << " Tk type Producers, and " << m_sensCaloDets.size() << " Calo type producers ";
  }

  for (Producers::iterator itProd = m_producers.begin(); itProd != m_producers.end(); ++itProd) {
    (*itProd)->produce(e, es);
  }
}

DEFINE_FWK_MODULE(GeometryProducer);
