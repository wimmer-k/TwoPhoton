{
  const int nge = 14;
  TFile *f = new TFile("htest.root");
  TFile *fc = new TFile("E_dT_cuts.root");
  TH2F *h[nge];
  TCutG *g[nge];

  TCanvas* c = new TCanvas();
  c->Divide(4,4);
  for(int i=0;i<nge;i++){
    h[i] = (TH2F*)f->Get(Form("hE_dT_g%02d",i));
    g[i] = (TCutG*)fc->Get(Form("compton_%02d",i));
    g[i]->SetLineColor(2);
    h[i]->GetXaxis()->SetRangeUser(-1e5,5e5);
    h[i]->GetYaxis()->SetRangeUser(0,1000);
    c->cd(i+1);
    h[i]->Draw("colz");
    g[i]->Draw("same");
  }
}
