#ifndef __GERMANIUM_HH
#define __GERMANIUM_HH

#include <iostream>
#include <vector>
#include <math.h>
#include "TObject.h"
#include "TVector3.h"
#include "TMath.h"
#include "TArrayS.h"

using namespace std;

////////////////// Germanium Hit/////////////////////////////

/////////////////////////////////////////////////////////
class GermaniumHit : public TObject {
public:
  GermaniumHit();
  void Clear();
  void PrintHit();
  void SetTimeStamp(ULong64_t timestamp){ftimestamp = timestamp;}
  void SetTimeShifted(ULong64_t timeshifted){ftimeshifted = timeshifted;}
  void SetChannel(Short_t channel){fchannel = channel;}
  void SetRaw(Short_t energy){fraw = energy;}
  void SetEnergy(Float_t energy){fenergy = energy;}
  void SetTrace(TArrayS *trace);
  ULong64_t GetTimeStamp(){return ftimestamp;}
  ULong64_t GetTimeShifted(){return ftimeshifted;}
  UShort_t GetChannel(){return fchannel;}
  UShort_t GetRaw(){return fraw;}
  Float_t GetEnergy(){return fenergy;}
  vector <Short_t> GetTrace(){return ftrace;}
protected:
  ULong64_t ftimestamp;
  ULong64_t ftimeshifted;
  UShort_t fchannel;
  Short_t fraw;
  Float_t fenergy;
  vector<Short_t> ftrace;
  
  ClassDef(GermaniumHit, 1);
};

////////////////// Germanium ////////////////////////////////

/////////////////////////////////////////////////////////
class Germanium : public TObject {
public:
  Germanium();
  ~Germanium(){Clear();}
  void Clear();
  void AddHit(GermaniumHit* det);

  int GetHitPattern(){return fhitpattern;}
  int GetMult(){return fmult;}
  vector<GermaniumHit*> GetHits(){return fhits;}
  GermaniumHit* GetHit(int n){
    if(n<fmult)
      return fhits[n];
    return NULL;
  }
  void PrintEvent();
protected:
  //! An integer whose n-th bit is 1 iff the detector in cluster n fired.
  int fhitpattern;
  //! The crystal multiplicity of the event.
  Short_t fmult;
  vector<GermaniumHit*> fhits;
  ClassDef(Germanium, 1);
};



#endif

