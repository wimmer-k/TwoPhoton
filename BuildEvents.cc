#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>

#include "TFile.h"
#include "TEnv.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2S.h"
#include "TH1S.h"
#include "TCutG.h"
#include "TStopwatch.h"
#include "TTreeIndex.h"

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
  int mult = 0;
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "inputfiles", &InputFile);
  interface->Add("-o", "outputfile", &OutputFile);
  interface->Add("-s", "settingsfile", &SettingFile);
  interface->Add("-v", "verbose", &vl);
  interface->Add("-n", "nmax", &nmax);
  interface->Add("-m", "minimum germanium mult to write event", &mult);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least the input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file: " << InputFile<<endl;
  
  cout<<"output file: "<<OutputFile<< endl;

  TFile* infile = new TFile(InputFile);

  TTree* tr = (TTree*)infile->Get("Data_R");
  if(tr == NULL){
    cout << "could not find tree build in file " << InputFile << endl;
    return 3;
  }

  TEnv* sett = new TEnv(SettingFile);
  evtwdw = sett->GetValue("Event.Window",500000);

  ifstream gecalfile(sett->GetValue("Ge.Calfile","calfile.dat"));
  double gain[nge];
  double offset[nge];
  int ch;
  for(int i=0;i<nge;i++){
    gecalfile >> ch >> offset[ch] >> gain[ch]; 
  }

  
  
  ULong64_t timestamp = -1;
  tr->SetBranchAddress("Timestamp",&timestamp);
  UShort_t board = -1;
  tr->SetBranchAddress("Board",&board);
  UShort_t channel = -1;
  tr->SetBranchAddress("Channel",&channel);
  UShort_t raw = -1;
  tr->SetBranchAddress("Energy",&raw);
  TArrayS *trace;
  tr->SetBranchAddress("Samples",&trace);
  
  Double_t nentries = tr->GetEntries();
  if(nmax>0)
    nentries = nmax;
  cout << nentries << " entries in tree" << endl;
  cout << "building tree index" << endl;
  tr->LoadBaskets(2e9);
  tr->BuildIndex("0","Timestamp");
  TTreeIndex *index = (TTreeIndex*)tr->GetTreeIndex();
  
  TFile* ofile = new TFile(OutputFile,"recreate");
  ofile->cd();

  TTree* stree = new TTree("sorted","timesorted events");
  Germanium* ge = new Germanium();
  stree->Branch("germanium",&ge,320000);
  ULong64_t beamTS = 0;
  stree->Branch("beamTS",&beamTS,320000);

  
  Int_t nbytes = 0;
  Int_t status;

  ULong64_t last_TS = 0;
  ULong64_t last_gammaTS = 0;
  ULong64_t last_beamTS = 0;

  TH1F* htdiff = new TH1F("htdiff","htdiff",1000,-2000000,2000000);
  TH1F* htdiff_geb = new TH1F("htdiff_geb","htdiff_geb",1000,0,20000000);
    
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    if(vl>2)
      cout << "getting entry " << i << endl;
    Long64_t sorted = tr->LoadTree( index->GetIndex()[i] );
    if(vl>2)
      cout << "getting sorted entry " << sorted << endl;
    
    status = tr->GetEvent(sorted);
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
    //cout << timestamp << endl;

    
    Long64_t diff = timestamp - last_TS;
    htdiff_geb->Fill(last_beamTS-last_gammaTS);
    htdiff->Fill(diff);
    if(abs(diff)>evtwdw && beamTS>0){
      if(ge->GetMult()>mult)
	stree->Fill();
      ge->Clear();
      beamTS=0;
    }
      

    
    if(board==0){
      //cout << "ge channel " << channel << endl;     
      GermaniumHit *hit = new GermaniumHit();
      hit->SetTimeStamp(timestamp);
      hit->SetChannel(channel);
      hit->SetRaw(raw);
      hit->SetTrace(trace);
      Float_t energy = raw*gain[channel]+offset[channel];
      hit->SetEnergy(energy);
      
      
      if(abs(diff)<evtwdw){
	ge->AddHit(hit);
      }
      
      last_TS = timestamp;
      last_gammaTS = timestamp;
    }
    if(board==3){
      //cout << "beam channel " << channel << endl;
      beamTS = timestamp;
      last_TS = timestamp;
      last_beamTS = timestamp;
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
  ofile->cd();
  stree->Write("",TObject::kOverwrite);
  htdiff->Write();
  htdiff_geb->Write();
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
