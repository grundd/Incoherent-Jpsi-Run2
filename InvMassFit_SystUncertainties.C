// InvMassFit_SystUncertainties.C
// David Grund, Apr 04, 2022

// cpp headers
#include <iostream>
#include <algorithm> // find max element
// root headers
#include "TMath.h"
// my headers
#include "AnalysisManager.h"
#include "AnalysisConfig.h"
#include "SetPtBinning.h"
#include "InvMassFit_Utilities.h"

using namespace RooFit;

// main function
void LoadValues_TailParamAndYields();
void PrintOutput(TString str, 
                 Double_t signal_val[][6], 
                 Double_t signal_err[][6], 
                 Double_t signal_devAbs[][6], 
                 Double_t signal_devRel[][6], 
                 TString labels[],
                 Double_t (*par_values)[6] = NULL);
// support functions
// see InvMassFit_Utilities.h

// arrays of 6 elements:
// first index is for the "allbins" range, others are for the 5 bins
Double_t fAlpha_L_val[6] = { 0 };
Double_t fAlpha_L_err[6] = { 0 };
Double_t fAlpha_R_val[6] = { 0 };
Double_t fAlpha_R_err[6] = { 0 };
Double_t fN_L_val[6] = { 0 };
Double_t fN_L_err[6] = { 0 };
Double_t fN_R_val[6] = { 0 };
Double_t fN_R_err[6] = { 0 };

Double_t fYield_val[6] = { 0 };
Double_t fYield_err[6] = { 0 };

Bool_t debug = kTRUE;
Bool_t do_fits = kTRUE;
// which values/ranges to vary
Bool_t do_low = kTRUE;
Bool_t do_upp = kTRUE;
Bool_t do_a_L = kTRUE;
Bool_t do_a_R = kTRUE;
Bool_t do_n_L = kTRUE;
Bool_t do_n_R = kTRUE;

void InvMassFit_SystUncertainties(Int_t iAnalysis)
{
    InitAnalysis(iAnalysis);

    // if the n parameters of the double-sided CB are fixed throughout the analysis, we do not vary them
    // and no systematic uncertainty is assigned
    if(isNParInDSCBFixed)
    {
        do_n_L = kFALSE;
        do_n_R = kFALSE;
    }

    gSystem->Exec("mkdir -p Results/" + str_subfolder + "InvMassFit_SystUncertainties/");

    InvMassFit_PrepareData(2);

    SetPtBinning();

    LoadValues_TailParamAndYields();

    //#####################################################################################################
    // 1) vary the lower boundary
    TString str1 = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/boundary_low/";
    // first index -> allbins + 5 pT bins
    // second index -> considered variation of the varied parameter
    Double_t signal_1_val[6][6] = { 0 };
    Double_t signal_1_err[6][6] = { 0 };
    Double_t signal_1_devAbs[6][6] = { 0 };
    Double_t signal_1_devRel[6][6] = { 0 };
    if(do_low)
    {
        Double_t low_bound[6] = {2.0, 2.1, 2.2, 2.3, 2.4, 2.5};
        TString labels[7] = {""}; // label of the variation in the "output.txt" file
        labels[0] = "2.2GeV"; // zero position -> original value
        TString str1_all[6];
        for(Int_t iVar = 0; iVar < 6; iVar++)
        {
            labels[iVar+1] = Form("%.1fGeV", low_bound[iVar]);
            // prepare the path
            str1_all[iVar] = str1 + Form("mass_%.1f/", low_bound[iVar]);
            gSystem->Exec("mkdir -p " + str1_all[iVar]);
            // do the fits
            if(do_fits)
            {
                for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str1_all[iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,low_bound[iVar],4.5,fAlpha_L_val[iBin],fAlpha_R_val[iBin],fN_L_val[iBin],fN_R_val[iBin],str_out,kTRUE);
                    signal_1_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_1_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_1_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_1_val[iBin][iVar]);
                    signal_1_devRel[iBin][iVar] = signal_1_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str1,signal_1_val,signal_1_err,signal_1_devAbs,signal_1_devRel,labels);
    }

    //#####################################################################################################
    // 2) vary the upper boundary
    TString str2 = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/boundary_upp/";
    Double_t signal_2_val[6][6] = { 0 };
    Double_t signal_2_err[6][6] = { 0 };
    Double_t signal_2_devAbs[6][6] = { 0 };
    Double_t signal_2_devRel[6][6] = { 0 };
    if(do_upp)
    {
        Double_t upp_bound[6] = {4.0, 4.2, 4.4, 4.6, 4.8, 5.0};
        TString labels[7] = {""};
        labels[0] = "4.5GeV";
        TString str2_all[6];
        for(Int_t iVar = 0; iVar < 6; iVar++)
        {
            labels[iVar+1] = Form("%.1fGeV", upp_bound[iVar]);
            // prepare the path
            str2_all[iVar] = str2 + Form("mass_%.1f/", upp_bound[iVar]);
            gSystem->Exec("mkdir -p " + str2_all[iVar]);
            // do the fits
            if(do_fits)
            {
                for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str2_all[iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,2.2,upp_bound[iVar],fAlpha_L_val[iBin],fAlpha_R_val[iBin],fN_L_val[iBin],fN_R_val[iBin],str_out,kTRUE);
                    signal_2_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_2_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_2_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_2_val[iBin][iVar]);
                    signal_2_devRel[iBin][iVar] = signal_2_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str2,signal_2_val,signal_2_err,signal_2_devAbs,signal_2_devRel,labels);
    }

    //#####################################################################################################
    // 3a) vary the alpha_L parameter
    TString str3a = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/alpha_L/";
    Double_t signal_3a_val[6][6] = { 0 };
    Double_t signal_3a_err[6][6] = { 0 };
    Double_t signal_3a_devAbs[6][6] = { 0 };
    Double_t signal_3a_devRel[6][6] = { 0 };
    if(do_a_L)
    {
        Double_t alpha_L[6][6] = { 0 };
        TString labels[7] = {""};
        labels[0] = "no var";
        TString str3a_all[6][6];
        for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
        {
            for(Int_t iVar = 0; iVar < 6; iVar++)
            {
                labels[iVar+1] = Form("var %i", iVar);
                // prepare the path
                str3a_all[iBin][iVar] = str3a + Form("var_%i/", iVar+1);
                gSystem->Exec("mkdir -p " + str3a_all[iBin][iVar]);
                // calculate the variations of the parameter
                if(iVar == 0) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] - fAlpha_L_err[iBin];
                if(iVar == 1) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] - 2./3. * fAlpha_L_err[iBin];
                if(iVar == 2) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] - 1./3. * fAlpha_L_err[iBin];
                if(iVar == 3) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] + 1./3. * fAlpha_L_err[iBin];
                if(iVar == 4) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] + 2./3. * fAlpha_L_err[iBin];
                if(iVar == 5) alpha_L[iBin][iVar] = fAlpha_L_val[iBin] + fAlpha_L_err[iBin];
            }
        }
        // print the values
        if(debug){
            std::cout << std::fixed << std::setprecision(2);
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
                std::cout << iBin << "\t";
                for(Int_t iVar = 0; iVar < 6; iVar++){
                   std::cout << alpha_L[iBin][iVar] << "\t";
                }
                std::cout << "\n";
            }
        }
        // do invariant mass fits
        if(do_fits)
        {
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
            {
                for(Int_t iVar = 0; iVar < 6; iVar++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str3a_all[iBin][iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,2.2,4.5,alpha_L[iBin][iVar],fAlpha_R_val[iBin],fN_L_val[iBin],fN_R_val[iBin],str_out,kTRUE);    
                    signal_3a_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_3a_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_3a_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_3a_val[iBin][iVar]);
                    signal_3a_devRel[iBin][iVar] = signal_3a_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str3a,signal_3a_val,signal_3a_err,signal_3a_devAbs,signal_3a_devRel,labels,alpha_L);
    }

    //#####################################################################################################
    // 3b) vary the alpha_R parameter
    TString str3b = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/alpha_R/";
    Double_t signal_3b_val[6][6] = { 0 };
    Double_t signal_3b_err[6][6] = { 0 };
    Double_t signal_3b_devAbs[6][6] = { 0 };
    Double_t signal_3b_devRel[6][6] = { 0 };
    if(do_a_R)
    {
        Double_t alpha_R[6][6] = { 0 };
        TString labels[7] = {""};
        labels[0] = "no var";
        TString str3b_all[6][6];
        for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
        {
            for(Int_t iVar = 0; iVar < 6; iVar++)
            {
                labels[iVar+1] = Form("var %i", iVar);
                // prepare the path
                str3b_all[iBin][iVar] = str3b + Form("var_%i/", iVar+1);
                gSystem->Exec("mkdir -p " + str3b_all[iBin][iVar]);
                // calculate the variations of the parameter
                if(iVar == 0) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] - fAlpha_R_err[iBin];
                if(iVar == 1) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] - 2./3. * fAlpha_R_err[iBin];
                if(iVar == 2) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] - 1./3. * fAlpha_R_err[iBin];
                if(iVar == 3) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] + 1./3. * fAlpha_R_err[iBin];
                if(iVar == 4) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] + 2./3. * fAlpha_R_err[iBin];
                if(iVar == 5) alpha_R[iBin][iVar] = fAlpha_R_val[iBin] + fAlpha_R_err[iBin];
            }
        }
        // print the values
        if(debug){
            std::cout << std::fixed << std::setprecision(3);
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
                std::cout << iBin << "\t";
                for(Int_t iVar = 0; iVar < 6; iVar++){
                   std::cout << alpha_R[iBin][iVar] << "\t";
                }
                std::cout << "\n";
            }
        }
        // do invariant mass fits
        if(do_fits)
        {
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
            {
                for(Int_t iVar = 0; iVar < 6; iVar++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str3b_all[iBin][iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,2.2,4.5,fAlpha_L_val[iBin],alpha_R[iBin][iVar],fN_L_val[iBin],fN_R_val[iBin],str_out,kTRUE);    
                    signal_3b_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_3b_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_3b_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_3b_val[iBin][iVar]);
                    signal_3b_devRel[iBin][iVar] = signal_3b_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str3b,signal_3b_val,signal_3b_err,signal_3b_devAbs,signal_3b_devRel,labels,alpha_R);
    }

    //#####################################################################################################
    // 3c) vary the n_L parameter
    TString str3c = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/n_L/";
    Double_t signal_3c_val[6][6] = { 0 };
    Double_t signal_3c_err[6][6] = { 0 };
    Double_t signal_3c_devAbs[6][6] = { 0 };
    Double_t signal_3c_devRel[6][6] = { 0 };
    if(do_n_L)
    {
        Double_t n_L[6][6] = { 0 };
        TString labels[7] = {""};
        labels[0] = "no var";
        TString str3c_all[6][6];
        for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
        {
            for(Int_t iVar = 0; iVar < 6; iVar++)
            {
                labels[iVar+1] = Form("var %i", iVar);
                // prepare the path
                str3c_all[iBin][iVar] = str3c + Form("var_%i/", iVar+1);
                gSystem->Exec("mkdir -p " + str3c_all[iBin][iVar]);
                // calculate the variations of the parameter
                if(iVar == 0) n_L[iBin][iVar] = fN_L_val[iBin] - fN_L_err[iBin];
                if(iVar == 1) n_L[iBin][iVar] = fN_L_val[iBin] - 2./3. * fN_L_err[iBin];
                if(iVar == 2) n_L[iBin][iVar] = fN_L_val[iBin] - 1./3. * fN_L_err[iBin];
                if(iVar == 3) n_L[iBin][iVar] = fN_L_val[iBin] + 1./3. * fN_L_err[iBin];
                if(iVar == 4) n_L[iBin][iVar] = fN_L_val[iBin] + 2./3. * fN_L_err[iBin];
                if(iVar == 5) n_L[iBin][iVar] = fN_L_val[iBin] + fN_L_err[iBin];
            }
        }
        // print the values
        if(debug){
            std::cout << std::fixed << std::setprecision(3);
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
                std::cout << iBin << "\t";
                for(Int_t iVar = 0; iVar < 6; iVar++){
                   std::cout << n_L[iBin][iVar] << "\t";
                }
                std::cout << "\n";
            }
        }
        // do invariant mass fits
        if(do_fits)
        {
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
            {
                for(Int_t iVar = 0; iVar < 6; iVar++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str3c_all[iBin][iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,2.2,4.5,fAlpha_L_val[iBin],fAlpha_R_val[iBin],n_L[iBin][iVar],fN_R_val[iBin],str_out,kTRUE);    
                    signal_3c_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_3c_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_3c_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_3c_val[iBin][iVar]);
                    signal_3c_devRel[iBin][iVar] = signal_3c_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str3c,signal_3c_val,signal_3c_err,signal_3c_devAbs,signal_3c_devRel,labels,n_L);
    }

    //#####################################################################################################
    // 3d) vary the n_R parameter
    TString str3d = "Results/" + str_subfolder + "InvMassFit_SystUncertainties/n_R/";
    Double_t signal_3d_val[6][6] = { 0 };
    Double_t signal_3d_err[6][6] = { 0 };
    Double_t signal_3d_devAbs[6][6] = { 0 };
    Double_t signal_3d_devRel[6][6] = { 0 };
    if(do_n_R)
    {
        Double_t n_R[6][6] = { 0 };
        TString labels[7] = {""};
        labels[0] = "no var";
        TString str3d_all[6][6];
        for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
        {
            for(Int_t iVar = 0; iVar < 6; iVar++)
            {
                labels[iVar+1] = Form("var %i", iVar);
                // Prepare the path
                str3d_all[iBin][iVar] = str3d + Form("var_%i/", iVar+1);
                gSystem->Exec("mkdir -p " + str3d_all[iBin][iVar]);
                // Calculate the variations of the parameter
                if(iVar == 0) n_R[iBin][iVar] = fN_R_val[iBin] - fN_R_err[iBin];
                if(iVar == 1) n_R[iBin][iVar] = fN_R_val[iBin] - 2./3. * fN_R_err[iBin];
                if(iVar == 2) n_R[iBin][iVar] = fN_R_val[iBin] - 1./3. * fN_R_err[iBin];
                if(iVar == 3) n_R[iBin][iVar] = fN_R_val[iBin] + 1./3. * fN_R_err[iBin];
                if(iVar == 4) n_R[iBin][iVar] = fN_R_val[iBin] + 2./3. * fN_R_err[iBin];
                if(iVar == 5) n_R[iBin][iVar] = fN_R_val[iBin] + fN_R_err[iBin];
            }
        }
        // Print the values
        if(debug){
            std::cout << std::fixed << std::setprecision(3);
            for(Int_t iBin = 0; iBin < nPtBins; iBin++){
                std::cout << iBin << "\t";
                for(Int_t iVar = 0; iVar < 6; iVar++){
                   std::cout << n_R[iBin][iVar] << "\t";
                }
                std::cout << "\n";
            }
        }
        // Do invariant mass fits
        if(do_fits)
        {
            for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
            {
                for(Int_t iVar = 0; iVar < 6; iVar++)
                {
                    // "allbins" -> opt == 3
                    // first pT bin -> opt == 4
                    TString str_out = str3d_all[iBin][iVar];
                    if(iBin == 0) str_out += "allbins";
                    else          str_out += Form("bin%i",iBin);
                    InvMassFit_DoFit(iBin+3,2.2,4.5,fAlpha_L_val[iBin],fAlpha_R_val[iBin],fN_L_val[iBin],n_R[iBin][iVar],str_out,kTRUE);    
                    signal_3d_val[iBin][iVar] = N_Jpsi_all[0];
                    signal_3d_err[iBin][iVar] = N_Jpsi_all[1];
                    signal_3d_devAbs[iBin][iVar] = TMath::Abs(fYield_val[iBin] - signal_3d_val[iBin][iVar]);
                    signal_3d_devRel[iBin][iVar] = signal_3d_devAbs[iBin][iVar] / fYield_val[iBin] * 100.;
                }
            }
        }
        // Print the output to a text file
        PrintOutput(str3d,signal_3d_val,signal_3d_err,signal_3d_devAbs,signal_3d_devRel,labels,n_R);
    }

    //#####################################################################################################
    // calculate the systematic uncertainty for each bin and each source as a maximum relative deviation over bins
    Double_t SystUncr_low[nPtBins+1];
    Double_t SystUncr_upp[nPtBins+1];
    Double_t SystUncr_a_L[nPtBins+1];
    Double_t SystUncr_a_R[nPtBins+1];
    Double_t SystUncr_n_L[nPtBins+1];
    Double_t SystUncr_n_R[nPtBins+1];
    Double_t SystUncr_tot[nPtBins+1];
    // find the maximum values
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++)
    {
        Double_t arr1_row[6];
        for(Int_t i = 0; i < 6; i++) arr1_row[i] = signal_1_devRel[iBin][i];
        SystUncr_low[iBin] = *max_element(arr1_row, arr1_row + 6);
        Double_t arr2_row[6];
        for(Int_t i = 0; i < 6; i++) arr2_row[i] = signal_2_devRel[iBin][i];
        SystUncr_upp[iBin] = *max_element(arr2_row, arr2_row + 6);
        Double_t arr3a_row[6];
        for(Int_t i = 0; i < 6; i++) arr3a_row[i] = signal_3a_devRel[iBin][i];
        SystUncr_a_L[iBin] = *max_element(arr3a_row, arr3a_row + 6);
        Double_t arr3b_row[6];
        for(Int_t i = 0; i < 6; i++) arr3b_row[i] = signal_3b_devRel[iBin][i];
        SystUncr_a_R[iBin] = *max_element(arr3b_row, arr3b_row + 6);
        Double_t arr3c_row[6];
        for(Int_t i = 0; i < 6; i++) arr3c_row[i] = signal_3c_devRel[iBin][i];
        SystUncr_n_L[iBin] = *max_element(arr3c_row, arr3c_row + 6);
        Double_t arr3d_row[6];
        for(Int_t i = 0; i < 6; i++) arr3d_row[i] = signal_3d_devRel[iBin][i];
        SystUncr_n_R[iBin] = *max_element(arr3d_row, arr3d_row + 6);
    }
    ofstream outfile(Form("Results/%s/InvMassFit_SystUncertainties/syst_uncertainties_%ibins.txt", str_subfolder.Data(), nPtBins));
    outfile << std::fixed << std::setprecision(2)
            << "systematic uncertainties per bin [%]\n"
            << "bin index 0 corresponds to the 'allbins' range\n"
            << "bin \tlow \tupp \taL \taR \tnL \tnR \ttotal \n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++){
        SystUncr_tot[iBin] = TMath::Sqrt(TMath::Power(SystUncr_low[iBin], 2) 
                                       + TMath::Power(SystUncr_upp[iBin], 2) 
                                       + TMath::Power(SystUncr_a_L[iBin], 2) 
                                       + TMath::Power(SystUncr_a_R[iBin], 2) 
                                       + TMath::Power(SystUncr_n_L[iBin], 2) 
                                       + TMath::Power(SystUncr_n_R[iBin], 2));
        outfile << iBin << "\t" 
                << SystUncr_low[iBin] << "\t" 
                << SystUncr_upp[iBin] << "\t" 
                << SystUncr_a_L[iBin] << "\t" 
                << SystUncr_a_R[iBin] << "\t" 
                << SystUncr_n_L[iBin] << "\t" 
                << SystUncr_n_R[iBin] << "\t"
                << SystUncr_tot[iBin] << "\n"; 
    }
    outfile.close();
    Printf("Results printed to syst_uncertainties_%ibins.txt.", nPtBins); 

    // print the total syst errors from the signal extraction to the output file for PhotoCrossSec_Calculate.C
    outfile.open(Form("Results/%s/InvMassFit_SystUncertainties/ErrSystSignalExtraction_%ibins.txt", str_subfolder.Data(), nPtBins));
    outfile << std::fixed << std::setprecision(1);
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++){
        outfile << iBin << "\t" << SystUncr_tot[iBin] << "\n";   
    }
    outfile.close();
    Printf("Results printed to ErrSystSignalExtraction_%ibins.txt.", nPtBins);
    Printf("Done.");
    return;
}

void LoadValues_TailParamAndYields()
{
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) 
    {
        ifstream ifs;
        // load the MC values of tail parameters in pT bins
        TString path_MC = "Results/" + str_subfolder + "InvMassFit_MC/";
        if(iBin == 0) path_MC += "allbins/allbins.txt";
        else          path_MC += Form("%ibins/bin%i.txt",nPtBins,iBin);
        ifs.open(path_MC.Data());
        char name[8];
        for(Int_t i = 0; i < 4; i++){
            if(i == 0) ifs >> name >> fAlpha_L_val[iBin] >> fAlpha_L_err[iBin];
            if(i == 1) ifs >> name >> fAlpha_R_val[iBin] >> fAlpha_R_err[iBin];
            if(i == 2) ifs >> name >> fN_L_val[iBin] >> fN_L_err[iBin];
            if(i == 3) ifs >> name >> fN_R_val[iBin] >> fN_R_err[iBin];
        }
        ifs.close();
        // load the values of yields in pT bins
        TString path_data = "Results/" + str_subfolder + "InvMassFit/";
        if(iBin == 0) path_data += "allbins/allbins_signal.txt";
        else          path_data += Form("%ibins/bin%i_signal.txt",nPtBins,iBin);
        ifs.open(path_data.Data());
        ifs >> fYield_val[iBin] >> fYield_err[iBin];
        ifs.close();
        Printf("Values for bin %i loaded.", iBin);
    }

    // print the results
    TString path_output = "Results/" + str_subfolder + Form("InvMassFit_SystUncertainties/input_values_%ibins.txt", nPtBins);
    ofstream outfile(path_output.Data());
    outfile << "bin \ta_L\terr\ta_R\terr\tn_L\terr\tn_R\terr\tN\terr\n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) 
    {
        if(iBin == 0) outfile << "all";
        else          outfile << iBin;
        outfile << "\t" << std::fixed << std::setprecision(2)
                << fAlpha_L_val[iBin] << "\t" 
                << fAlpha_L_err[iBin] << "\t" 
                << fAlpha_R_val[iBin] << "\t" 
                << fAlpha_R_err[iBin] << "\t" 
                << fN_L_val[iBin] << "\t" 
                << fN_L_err[iBin] << "\t" 
                << fN_R_val[iBin] << "\t" 
                << fN_R_err[iBin] << "\t"
                << fYield_val[iBin] << "\t"
                << fYield_err[iBin] << "\n";
    }
    outfile.close();
    Printf("Results printed to %s.", path_output.Data()); 

    return;
}

void PrintOutput(TString str, Double_t signal_val[][6], Double_t signal_err[][6], Double_t signal_devAbs[][6], Double_t signal_devRel[][6], TString labels[], Double_t (*par_values)[6])
{
    // if 3a, 3b, 3c, 3d: print the values of the varied tail parameters
    ofstream outfile((str + "output.txt").Data());
    outfile << std::fixed << std::setprecision(2)
            << "bin index 0 corresponds to the 'allbins' range\n";
    if(par_values != NULL)
    {
        outfile << "\nparameters val \n"
                << "bin \t";
        for(Int_t iVar = 0; iVar < 6; iVar++) outfile << Form("%s\t", labels[iVar+1].Data());
        outfile << "\n";
        for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
            outfile << iBin << "\t";
            for(Int_t iVar = 0; iVar < 6; iVar++) {
                outfile << par_values[iBin][iVar] << "\t";
            }
            outfile << "\n";
        }
    }
    outfile << "\noriginal signal \n"
            << "bin \t" << Form("%s\n", labels[0].Data());
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
        outfile << iBin << "\t" << fYield_val[iBin] << "\n";
    }
    outfile << "\nnew signal val \n"
            << "bin \t";
    for(Int_t iVar = 0; iVar < 6; iVar++) outfile << Form("%s\t", labels[iVar+1].Data());
    outfile << "\n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
        outfile << iBin << "\t";
        for(Int_t iVar = 0; iVar < 6; iVar++) {
            outfile << signal_val[iBin][iVar] << "\t";
        }
        outfile << "\n";
    }
    outfile << "\nnew signal err \n"
            << "bin \t";
    for(Int_t iVar = 0; iVar < 6; iVar++) outfile << Form("%s\t", labels[iVar+1].Data());
    outfile << "\n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
        outfile << iBin << "\t";
        for(Int_t iVar = 0; iVar < 6; iVar++) {
            outfile << signal_err[iBin][iVar] << "\t";
        }
        outfile << "\n";
    }
    outfile << "\ndeviation absolute\n"
            << "bin \t";
    for(Int_t iVar = 0; iVar < 6; iVar++) outfile << Form("%s\t", labels[iVar+1].Data());
    outfile << "\n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
        outfile << iBin << "\t";
        for(Int_t iVar = 0; iVar < 6; iVar++) {
            outfile << signal_devAbs[iBin][iVar] << "\t";
        }
        outfile << "\n";
    }
    outfile << Form("\ndeviation relative [%%]\n")
            << "bin \t";
    for(Int_t iVar = 0; iVar < 6; iVar++) outfile << Form("%s\t", labels[iVar+1].Data());
    outfile << "\n";
    for(Int_t iBin = 0; iBin < nPtBins+1; iBin++) {
        outfile << iBin << "\t";
        for(Int_t iVar = 0; iVar < 6; iVar++) {
            outfile << signal_devRel[iBin][iVar] << "\t";
        }
        outfile << "\n";
    }
    outfile.close();
    Printf("Results printed to %s.", (str + "output.txt").Data()); 

    return;
}