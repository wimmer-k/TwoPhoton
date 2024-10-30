{
  const int nge = 14;
  TFile *f = new TFile("htest.root");
  TH2F *h[nge];
  TH1F *t[nge];

  TCanvas* c = new TCanvas();
  c->cd();
  double max = 0;
  double shift[nge];
  for(int i=0;i<nge;i++){
    h[i] = (TH2F*)f->Get(Form("hE_dTraw_g%02d",i));
    t[i] = (TH1F*)h[i]->ProjectionX(Form("hdTraw_g%02d",i),415,421);
    t[i]->SetLineColor(1+i%5);
    t[i]->Rebin(2);
    if(i==0)
      t[i]->Draw();
    t[i]->Draw("same");

    if(t[i]->GetMaximum()>max)
      max = t[i]->GetMaximum();
    cout << i <<"\t" << t[i]->GetXaxis()->GetBinCenter(t[i]->GetMaximumBin()) << endl;
  }
  t[0]->GetYaxis()->SetRangeUser(0,max*1.2);
}
