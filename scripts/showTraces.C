TCanvas* c;
bool opened = false;
void openCanvas(){
  c = new TCanvas();
  c->Divide(2,2);
  opened = true;
}
void showTraces(int ch){
  if(!opened)
    openCanvas();
  TFile *f = new TFile("htest.root");
  TH2F *h[4];

  h[0] = (TH2F*)f->Get(Form("htrace_photo_g%02d",ch));
  h[1] = (TH2F*)f->Get(Form("htrace_compton_g%02d",ch));
  h[2] = (TH2F*)f->Get(Form("htrace_300lowT_g%02d",ch));
  h[3] = (TH2F*)f->Get(Form("htrace_300higT_g%02d",ch));
  
  for(int i=0;i<4;i++){
    c->cd(i+1);
    h[i]->GetYaxis()->SetRangeUser(2000,6000);
    h[i]->Draw("colz");
  }
}
