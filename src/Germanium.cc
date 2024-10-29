#include "Germanium.hh"

////////////////// Germanium Hit/////////////////////////////

/////////////////////////////////////////////////////////
GermaniumHit::GermaniumHit(){
  Clear();
}
void GermaniumHit::Clear(){
  ftimestamp = -1;
  fchannel = -1;
  fraw = -1;
  fenergy = -1;
  ftrace.clear();
}
void GermaniumHit::PrintHit(){
  cout << __PRETTY_FUNCTION__ << endl;
  cout << "channel " << fchannel << ", ts " << ftimestamp<< endl;
  cout << " raw " << fraw << ", energy " << fenergy << endl;
}
void GermaniumHit::SetTrace(TArrayS *trace){
  ftrace.clear();
  // Check if trace is not null
  if(trace){
    // Reserve space for the vector (optional, for efficiency)
    ftrace.reserve(trace->GetSize());     
    // Copy the data from TArrayS to the vector
    for(int i=0; i<trace->GetSize(); i++) {
      ftrace.push_back(trace->At(i)); // Use At() to access elements
    }
  }  
}
////////////////// Germanium ////////////////////////////////

/////////////////////////////////////////////////////////
Germanium::Germanium(){
  Clear();
}
void Germanium::Clear(){
  fhitpattern = 0;
  fmult = 0;
  for(vector<GermaniumHit*>::iterator det=fhits.begin(); det!=fhits.end(); det++){
    delete *det;
  }
  fhits.clear();
}
void Germanium::AddHit(GermaniumHit* det){
  fhits.push_back(det);
  fmult++;
  fhitpattern |= 1 << (int)det->GetChannel();
}
void Germanium::PrintEvent(){
  cout << __PRETTY_FUNCTION__ << endl;
  cout << "mult " << fmult << endl;
  cout << "hitpattern " << fhitpattern << endl;
  for(UShort_t i=0;i<fhits.size();i++)
    fhits[i]->PrintHit();
}
ClassImp(Germanium);
