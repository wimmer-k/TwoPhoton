#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>

#include "TFile.h"
#include "TEnv.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCutG.h"
#include "TStopwatch.h"

#include "CommandLineInterface.hh"
#include "Germanium.hh"

using namespace TMath;
using namespace std;

bool signal_received = false;
void signalhandler(int sig);
double get_time();
Long64_t evtwdw = 500000;
const int nge = 14;

int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  char* InputFile = NULL;
  char* OutputFile = NULL;
  char* SettingFile = NULL;
  int vl = 0;
  int nmax = 0;
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "inputfiles", &InputFile);
  interface->Add("-o", "outputfile", &OutputFile);
  interface->Add("-s", "settingsfile", &SettingFile);
  interface->Add("-v", "verbose", &vl);
  interface->Add("-n", "nmax", &nmax);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least the input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file: " << InputFile<<endl;
  
  cout<<"output file: "<<OutputFile<< endl;

  TFile* infile = new TFile(InputFile);

  TTree* tr = (TTree*)infile->Get("sorted");
  if(tr == NULL){
    cout << "could not find tree build in file " << InputFile << endl;
    return 3;
  }

  Germanium* ge = new Germanium();
  tr->SetBranchAddress("germanium",&ge);
  ULong64_t beamTS = 0;
  tr->SetBranchAddress("beamTS",&beamTS);

  Double_t nentries = tr->GetEntries();
  if(nmax>0)
    nentries = nmax;
  cout << nentries << " entries in tree" << endl;

  
  Int_t nbytes = 0;
  Int_t status;
  TFile* ofile = new TFile(OutputFile,"recreate");
  ofile->cd();
  
  TList *hlist = new TList();
  TH1F* hdT = new TH1F("hdT","hdT",2100,-1e5,2e6);hlist->Add(hdT);
  TH1F* hdTraw = new TH1F("hdTraw","hdTraw",2100,-1e5,2e6);hlist->Add(hdTraw);
  TH1F* hE = new TH1F("hE","hE",8000,0,4000);hlist->Add(hE);
  TH2F* hE_dT = new TH2F("hE_dT","hE_dT",2100,-1e5,2e6,2000,0,4000);hlist->Add(hE_dT);

  TH2F* hE_summary = new TH2F("hE_summary","hE_summary",15,0,15,4000,0,4000);hlist->Add(hE_summary);
  TH2F* hdT_summary = new TH2F("hdT_summary","hdT_summary",15,0,15,2100,-1e5,2e6);hlist->Add(hdT_summary);

  TH2F* hE_dT_g[nge];
  TH2F* hE_dTraw_g[nge];
  for(int g=0;g<nge;g++){
    hE_dT_g[g] = new TH2F(Form("hE_dT_g%02d",g),Form("hE_dT_g%02d",g),2100,-1e5,2e6,2000,0,4000);hlist->Add(hE_dT_g[g]);
    hE_dTraw_g[g] = new TH2F(Form("hE_dTraw_g%02d",g),Form("hE_dTraw_g%02d",g),2100,-1e5,2e6,2000,0,4000);hlist->Add(hE_dTraw_g[g]);
  }

  
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    if(vl>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(vl>2)
      cout << "status " << status << endl;
    if(status == -1){
      cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<tr->GetName()<<endl;
      return 5;
    }
    else if(status == 0){
      cerr<<"Error occured, entry "<<i<<" in tree "<<tr->GetName()<<endl;
      return 6;
    }
    nbytes += status;

    if(vl>1)
      cout << ge->GetMult() << endl;
    for(int h=0; h<ge->GetMult(); h++){
      GermaniumHit* hit = ge->GetHit(h);
      hdTraw->Fill(hit->GetTimeStamp()*1.0-beamTS*1.0);
      hdT->Fill(hit->GetTimeShifted()*1.0-beamTS*1.0);
      hE->Fill(hit->GetEnergy());
      hE_dT->Fill(hit->GetTimeShifted()*1.0-beamTS*1.0,hit->GetEnergy());
      hE_dT_g[hit->GetChannel()]->Fill(hit->GetTimeShifted()*1.0-beamTS*1.0,hit->GetEnergy());
      hE_dTraw_g[hit->GetChannel()]->Fill(hit->GetTimeStamp()*1.0-beamTS*1.0,hit->GetEnergy());
      hE_summary->Fill(hit->GetChannel(),hit->GetEnergy());
      hdT_summary->Fill(hit->GetChannel(),hit->GetTimeShifted()*1.0-beamTS*1.0);
      
    }

    
    if(i%10000 == 0){
      double time_end = get_time();
      cout<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<(100.*i)/nentries<<" % done\t"<<(Float_t)i/(time_end - time_start)<<" events/s " << (nentries-i)*(time_end - time_start)/(Float_t)i<<"s to go \r"<<flush;
    }
  }
  cout << endl;


  cout << "writing to file" << endl;
  cout << endl;
  ofile->cd();
  TH1F* h1;
  TH2F* h2;
  TIter next(hlist);
  while( (h1 = (TH1F*)next()) ){
    if(h1->GetEntries()>0)
      h1->Write("",TObject::kOverwrite);
  }
  while( (h2 = (TH2F*)next()) ){
    if(h2->GetEntries()>0)
      h2->Write("",TObject::kOverwrite);
  }

  ofile->Close();
  delete tr;
  
  double time_end = get_time();
  cout << "Run time " << time_end - time_start << " s." << endl;

  return 0;
}
void signalhandler(int sig){
  if (sig == SIGINT){
    signal_received = true;
  }
}

double get_time(){
    struct timeval t;
    gettimeofday(&t, NULL);
    double d = t.tv_sec + (double) t.tv_usec/1000000;
    return d;
}
