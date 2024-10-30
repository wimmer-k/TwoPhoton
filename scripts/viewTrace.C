#include "Germanium.hh"
TTree* tr;
bool loaded = false;
Germanium* ge = new Germanium();
vector<Int_t> times;


void LoadTree(char*);

void ViewTrace(int event){
  LoadTree("test2.root");
  int status = tr->GetEvent(event);
  cout << "status " << status << endl;
  if(status == -1){
    cerr<<"Error occured, couldn't read entry "<<event<<" from tree "<<tr->GetName()<<endl;
    return;
  }
  else if(status == 0){
    cerr<<"Error occured, entry "<<event<<" in tree "<<tr->GetName()<<endl;
    return;
  }
  TMultiGraph* gr = new TMultiGraph();
  for(int h=0;h<ge->GetMult();h++){
    GermaniumHit* hit = ge->GetHit(h);
    //vector <Int_t> trace(std::begin(hit->GetTrace()), std::end(hit->GetTrace()));
    vector <Short_t> tr_short= hit->GetTrace();
    vector <Int_t> trace;
    trace.resize(tr_short.size()); copy(tr_short.begin(), tr_short.end(), trace.begin());
    hit->PrintHit();
    //cout << hit->GetEnergy() << "\t" << hit->GetTrace().size() << "\t" << trace.size() << endl;
    TGraph* gg = new TGraph(trace.size(),&times[0],&trace[0]);
    gr->Add(gg,"L");
  }
  gr->Draw("A");
}

void LoadTree(char* filen){
  if(loaded)
    return;
  TFile* f = new TFile(filen);
  tr = (TTree*)f->Get("sorted");
  tr->SetBranchAddress("germanium",&ge);
  for(int i=0;i<1400;i++){
    times.push_back(i);
  }
}
