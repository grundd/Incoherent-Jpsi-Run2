// AxE_PtBins_Utilities.h
// David Grund, Jun 07, 2022

// cpp headers
#include <fstream>
#include <iomanip> // std::setprecision()
// root headers
#include "TSystem.h"
#include "TFile.h"
#include "TH1.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TMath.h"

TH1F* AxE_PtBins_hNRec = NULL; 
TH1F* AxE_PtBins_hNGen = NULL; 
TH1F* AxE_PtBins_hAxE = NULL;
Double_t NRec_total;
Double_t NGen_total;

void AxE_PtBins_SaveToFile(Double_t N_total, TH1F* hist, TString name) {
    ofstream outfile (name.Data());
    outfile << "0\t" << N_total << "\n";
    for(Int_t iBin = 1; iBin <= hist->GetNbinsX(); iBin++) {
        outfile << iBin << "\t" << hist->GetBinContent(iBin) << "\n";
    }
    outfile.close();
    Printf("*** File saved in %s.***", name.Data());
}

Double_t CalculateErrorBayes(Double_t k, Double_t n){ // k = NRec, n = NGen

    Double_t var = (k + 1) * (k + 2) / (n + 2) / (n + 3) - (k + 1) * (k + 1) / (n + 2) / (n + 2);
    Double_t err = TMath::Sqrt(var);

    return err;
}

void AxE_PtBins_FillHistNRec(Double_t fCutZ)
{
    // check if the corresponding text file already exists
    TString file;
    if(fCutZ == cut_fVertexZ) file = "Results/" + str_subfolder + "AxE_PtBins/";
    else                      file = "Results/" + str_subfolder + Form("VertexZ_SystUncertainties/Zcut%.1f_AxE_PtBins/", fCutZ);
    file.Append(Form("NRec_%ibins.txt", nPtBins));

    ifstream ifs;
    ifs.open(file);
    if(!ifs.fail())
    {
        // this configuration has already been calculated
        Printf("*** The file %s already exists. ***", file.Data());
        // fill AxE_PtBins_hNRec with data from the file
        Int_t i_bin; Double_t NRec;
        // fiducial
        ifs >> i_bin >> NRec_total;
        // pT bins
        for(Int_t iBin = 1; iBin <= nPtBins; iBin++) {
            ifs >> i_bin >> NRec;
            AxE_PtBins_hNRec->SetBinContent(iBin, NRec);
        }
        ifs.close(); 
        return;
    } 
    else 
    {
        // this configuration is yet to be calculated
        Printf("*** Calculating N rec per bin for %s... ***", file.Data());
        TFile *fRec = TFile::Open((str_in_MC_fldr_rec + "AnalysisResults_MC_kIncohJpsiToMu.root").Data(), "read");
        if(fRec) Printf("MC rec file loaded.");
        TTree *tRec = dynamic_cast<TTree*> (fRec->Get(str_in_MC_tree_rec.Data()));
        if(tRec) Printf("MC rec tree loaded.");
        ConnectTreeVariablesMCRec(tRec);

        // |> *********** for VertexZ_SystUncertainties.C ***********
        // save the original value of cut_fVertexZ
        Printf("Original cut on vertex Z: %.1f", cut_fVertexZ);
        Double_t fCutZ_orig = cut_fVertexZ;
        if(fCutZ != cut_fVertexZ) {
            // set the new value of cut_fVertexZ
            cut_fVertexZ = fCutZ;
            Printf("New cut on vertex Z: %.1f", cut_fVertexZ);
        }
        // <| *****************************************************

        // go over tree entries and calculate NRec in the total range and in bins
        Int_t NRec[6] = { 0 };
        for(Int_t iEntry = 0; iEntry < tRec->GetEntries(); iEntry++) {
            tRec->GetEntry(iEntry);
            // m between 2.2 and 4.5 GeV/c^2
            // & pT from 0.2 to 1.0 GeV/c
            if(EventPassedMCRec(0, 3)) NRec[0]++;
            // & pT in range of a given bin 
            for(Int_t iBin = 1; iBin <= nPtBins; iBin++) if(EventPassedMCRec(0, 4, iBin)) NRec[iBin]++;
        }
        for(Int_t iBin = 1; iBin <= nPtBins; iBin++) AxE_PtBins_hNRec->SetBinContent(iBin, NRec[iBin]);
        Printf("*** Finished! ***");

        // |> *********** for VertexZ_SystUncertainties.C ***********
        if(cut_fVertexZ != fCutZ_orig)
        {
            // set back the original value of cut_fVertexZ
            cut_fVertexZ = fCutZ_orig;
            Printf("Restoring the original cut on vertex Z: %.1f", cut_fVertexZ);  
        }
        // <| *****************************************************
        
        AxE_PtBins_SaveToFile(NRec[0], AxE_PtBins_hNRec, file);
        return;
    }
}

void AxE_PtBins_FillHistNGen()
{
    // check if the corresponding text file already exists
    TString file = "Results/" + str_subfolder + "AxE_PtBins/";
    file.Append(Form("NGen_%ibins.txt", nPtBins));

    ifstream ifs;
    ifs.open(file);
    if(!ifs.fail())
    {
        // this configuration has already been calculated
        Printf("*** The file %s already exists. ***", file.Data());
        // fill AxE_PtBins_hNGen with data from the text file
        Int_t i_bin; Double_t NGen;
        // fiducial
        ifs >> i_bin >> NGen_total;
        // pT bins
        for(Int_t iBin = 1; iBin <= nPtBins; iBin++) {
            ifs >> i_bin >> NGen;
            AxE_PtBins_hNGen->SetBinContent(iBin, NGen);
        }
        ifs.close(); 
        return;
    } 
    else 
    {
        // this configuration is yet to be calculated
        Printf("*** Calculating N gen per bin for %s... ***", file.Data());
        TFile *fGen = TFile::Open((str_in_MC_fldr_gen + "AnalysisResults_MC_kIncohJpsiToMu.root").Data(), "read");
        if(fGen) Printf("MC gen file loaded.");
        TTree *tGen = dynamic_cast<TTree*> (fGen->Get(str_in_MC_tree_gen.Data()));
        if(tGen) Printf("MC gen tree loaded.");
        ConnectTreeVariablesMCGen(tGen);

        // go over tree entries and calculate NRec in the total range and in bins
        Int_t NGen[6] = { 0 };
        for(Int_t iEntry = 0; iEntry < tGen->GetEntries(); iEntry++) {
            tGen->GetEntry(iEntry);
            // m between 2.2 and 4.5 GeV/c^2
            // & pT from 0.2 to 1.0 GeV/c
            if(EventPassedMCGen(3)) NGen[0]++;
            // & pT in range of a given bin 
            for(Int_t iBin = 1; iBin <= nPtBins; iBin++) if(EventPassedMCGen(4, iBin)) NGen[iBin]++;
        }
        for(Int_t iBin = 1; iBin <= nPtBins; iBin++) AxE_PtBins_hNGen->SetBinContent(iBin, NGen[iBin]);
        Printf("*** Finished! ***");
        
        AxE_PtBins_SaveToFile(NGen[0], AxE_PtBins_hNGen, file);
        return;
    }
    return;
}

void AxE_PtBins_Calculate(Double_t fCutZ)
{
    AxE_PtBins_hNRec = new TH1F("AxE_PtBins_hNRec","N_rec per bin",nPtBins,ptBoundaries);
    AxE_PtBins_hNGen = new TH1F("AxE_PtBins_hNGen","N_gen per bin",nPtBins,ptBoundaries);

    AxE_PtBins_FillHistNRec(fCutZ);
    AxE_PtBins_FillHistNGen();

    AxE_PtBins_hAxE = (TH1F*)AxE_PtBins_hNRec->Clone("AxE_PtBins_hAxE");
    AxE_PtBins_hAxE->SetTitle("AxE per bin");
    AxE_PtBins_hAxE->Sumw2();
    AxE_PtBins_hAxE->Divide(AxE_PtBins_hNGen);

    // Draw the histogram:
    TCanvas *c = new TCanvas("c", "c", 900, 600);
    c->SetTopMargin(0.02);
    c->SetBottomMargin(0.14);
    c->SetRightMargin(0.03);
    c->SetLeftMargin(0.145);
    // gStyle
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(1);
    gStyle->SetPaintTextFormat("4.2f");
    // Marker and line
    //AxE_PtBins_hAxE->SetMarkerStyle(21);
    //AxE_PtBins_hAxE->SetMarkerColor(kBlue);
    //AxE_PtBins_hAxE->SetMarkerSize(1.0);
    AxE_PtBins_hAxE->SetLineColor(kBlue);
    AxE_PtBins_hAxE->SetLineWidth(1.0);
    // Vertical axis
    AxE_PtBins_hAxE->GetYaxis()->SetTitle("#it{N}_{rec}^{MC}/#it{N}_{gen}^{MC}");
    AxE_PtBins_hAxE->GetYaxis()->SetTitleSize(0.056);
    AxE_PtBins_hAxE->GetYaxis()->SetTitleOffset(1.3);
    AxE_PtBins_hAxE->GetYaxis()->SetLabelSize(0.056);
    AxE_PtBins_hAxE->GetYaxis()->SetDecimals(3);
    AxE_PtBins_hAxE->GetYaxis()->SetRangeUser(0.0,AxE_PtBins_hAxE->GetBinContent(1)*1.1);
    // Horizontal axis
    AxE_PtBins_hAxE->GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    AxE_PtBins_hAxE->GetXaxis()->SetTitleSize(0.056);
    AxE_PtBins_hAxE->GetXaxis()->SetTitleOffset(1.2);
    AxE_PtBins_hAxE->GetXaxis()->SetLabelSize(0.056);
    AxE_PtBins_hAxE->GetXaxis()->SetLabelOffset(0.015);
    AxE_PtBins_hAxE->GetXaxis()->SetDecimals(1);
    // Eventually draw it
    AxE_PtBins_hAxE->Draw("P E1");
    // legend
    TLegend *l = new TLegend(0.52,0.77,0.85,0.97);
    l->AddEntry((TObject*)0,Form("ALICE Simulation"),"");
    l->AddEntry((TObject*)0,Form("Pb#minusPb #sqrt{#it{s}_{NN}} = 5.02 TeV"),"");
    l->AddEntry((TObject*)0,Form("inc J/#psi #rightarrow #mu^{+}#mu^{-}"),"");
    l->SetTextSize(0.056);
    l->SetBorderSize(0);
    l->SetFillStyle(0);
    l->Draw();
    // legend 2
    TLegend *l2 = new TLegend(0.15,0.17,0.35,0.32);
    l2->AddEntry((TObject*)0,Form("|#it{y}| < 0.8"),"");
    l2->AddEntry((TObject*)0,Form("2.2 < #it{m} < 4.5 GeV/#it{c}^{2}"),"");
    l2->SetTextSize(0.056);
    l2->SetBorderSize(0);
    l2->SetFillStyle(0);
    l2->Draw();

    // save the figures and print the results to txt file
    TString str;
    if(fCutZ == cut_fVertexZ) str = "Results/" + str_subfolder + Form("AxE_PtBins/AxE_%ibins", nPtBins);
    else                      str = "Results/" + str_subfolder + Form("VertexZ_SystUncertainties/Zcut%.1f_AxE_PtBins/AxE_%ibins", fCutZ, nPtBins);
    c->Print((str + ".pdf").Data());
    c->Print((str + ".png").Data());

    // compare errors that Root gives with CalculateErrorBayes
    Bool_t DebugErrors = kFALSE;
    if(DebugErrors){
        Double_t ErrRoot = 0;
        Double_t ErrBayes = 0;    
        for(Int_t i = 1; i <= nPtBins; i++) {
            ErrRoot = AxE_PtBins_hAxE->GetBinError(i);
            ErrBayes = CalculateErrorBayes(AxE_PtBins_hNRec->GetBinContent(i),AxE_PtBins_hNGen->GetBinContent(i));
            Printf("Root: %.5f, Bayes: %.5f", ErrRoot, ErrBayes);
        }
    }

    // calculate the total value of AxE
    Double_t AxE_tot_val = NRec_total / NGen_total;
    Double_t AxE_tot_err = CalculateErrorBayes(NRec_total, NGen_total);
    Printf("Total AxE = (%.4f pm %.4f)%%", AxE_tot_val*100., AxE_tot_err*100.);

    // print the results to a text file
    // index zero -> fiducial cross section
    ofstream outfile((str + ".txt").Data());
    outfile << std::fixed << std::setprecision(3);
    outfile //<< "Bin \tAxE [%%] \tAxE_err [%%] \n";
            << "0\t" << AxE_tot_val*100. << "\t" << AxE_tot_err*100. << "\n";
    for(Int_t i = 1; i <= nPtBins; i++){
        outfile << i << "\t" << AxE_PtBins_hAxE->GetBinContent(i)*100. << "\t" << AxE_PtBins_hAxE->GetBinError(i)*100. << "\n";
    }
    outfile.close();
    Printf("*** Results printed to %s.***", (str + ".txt").Data());

    return;
}