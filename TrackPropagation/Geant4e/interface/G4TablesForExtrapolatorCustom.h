//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4TablesForExtrapolatorCustom.hh 74302 2013-10-02 19:33:07Z vnivanch $
//
//---------------------------------------------------------------------------
//
// ClassName:    G4TablesForExtrapolatorCustom
//  
// Description:  This class keep dedx, range, inverse range tables 
//               for extrapolator
//
// Author:       24.10.14 V.Ivanchenko 
//
// Modification: 
//
//----------------------------------------------------------------------------
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#ifndef TrackPropagation_G4TablesForExtrapolatorCustom_h
#define TrackPropagation_G4TablesForExtrapolatorCustom_h 1

#include "globals.hh"
#include "G4PhysicsTable.hh"
#include "G4DataVector.hh"
#include <vector>

class G4ParticleDefinition;
class G4ProductionCuts;
class G4MaterialCutsCouple;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

enum ExtTableType
{
  fDedxElectron = 0,
  fDedxPositron,
  fDedxProton,
  fDedxMuon,
  fRangeElectron,
  fRangePositron,
  fRangeProton,
  fRangeMuon,
  fInvRangeElectron,
  fInvRangePositron,
  fInvRangeProton,
  fInvRangeMuon,
  fMscElectron
};

class G4TablesForExtrapolatorCustom 
{
public:

  explicit G4TablesForExtrapolatorCustom(G4int verb, G4int bins, G4double e1, G4double e2, G4bool iononly = false);

  ~G4TablesForExtrapolatorCustom();

  const G4PhysicsTable* GetPhysicsTable(ExtTableType type) const; 

private:

  void Initialisation();

  G4PhysicsTable* PrepareTable();

  void ComputeElectronDEDX(const G4ParticleDefinition* part, 
			   G4PhysicsTable* table); 

  void ComputeMuonDEDX(const G4ParticleDefinition* part, 
		       G4PhysicsTable* table); 

  void ComputeProtonDEDX(const G4ParticleDefinition* part, 
			 G4PhysicsTable* table); 

  void ComputeTrasportXS(const G4ParticleDefinition* part, 
			 G4PhysicsTable* table);

  // hide assignment operator
  G4TablesForExtrapolatorCustom & operator=(const G4TablesForExtrapolatorCustom &right) = delete;
  G4TablesForExtrapolatorCustom(const G4TablesForExtrapolatorCustom&) = delete;

  const G4ParticleDefinition* currentParticle;
  const G4ParticleDefinition* electron;
  const G4ParticleDefinition* positron;
  const G4ParticleDefinition* muonPlus;
  const G4ParticleDefinition* muonMinus;
  const G4ParticleDefinition* proton;

  G4DataVector             cuts;

  G4ProductionCuts*        pcuts;
  std::vector<const G4MaterialCutsCouple*> couples;

  G4PhysicsTable*          dedxElectron;
  G4PhysicsTable*          dedxPositron;
  G4PhysicsTable*          dedxMuon;
  G4PhysicsTable*          dedxProton;
  G4PhysicsTable*          rangeElectron;
  G4PhysicsTable*          rangePositron;
  G4PhysicsTable*          rangeMuon;
  G4PhysicsTable*          rangeProton;
  G4PhysicsTable*          invRangeElectron;
  G4PhysicsTable*          invRangePositron;
  G4PhysicsTable*          invRangeMuon;
  G4PhysicsTable*          invRangeProton;
  G4PhysicsTable*          mscElectron;

  G4int       verbose;
  G4int       nbins;
  G4int       nmat;

  G4double    emin;
  G4double    emax;
  G4double    mass;
  G4double    charge2;

  G4bool      splineFlag;

  G4bool      ionOnly;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif

