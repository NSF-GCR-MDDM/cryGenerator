#include "CRYGenerator.h"
#include "CRYSetup.h"
#include "CRYParticle.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "TTree.h"
#include "TFile.h"

int cryToPDG(int idCode,int charge);

int main() {
    int nps=100000;

    // Configuration string — same format as CRY input
    std::string inputConfig = R"(returnMuons 1
        returnNeutrons 1
        returnProtons 1
        returnGammas 1
        returnElectrons 1
        returnPions 1
        subboxLength 300
        altitude 0
        date 3-6-2015
        latitude 45
        )";

    const char* crypath = std::getenv("CRYPATH");
    if (!crypath) {
        std::cerr << "Error: CRYPATH environment variable not set." << std::endl;
        return 1;
    }
    std::string cryDataDir = std::string(crypath) + "/../data";

    //Load setup file
    CRYSetup* setup = new CRYSetup(inputConfig, cryDataDir);
    
    //Create CRY generator
    CRYGenerator *gen = new CRYGenerator(setup);

    //Set-up output file
    TFile* outfile = new TFile("cry_output.root", "RECREATE");
    TTree* tree = new TTree("cry", "CRY particle output");

    std::vector<int> pdg;
    std::vector<float> energy, u, v, w, x, y, z;
    tree->Branch("pdgcode", &pdg);
    tree->Branch("energy",  &energy);
    tree->Branch("u",       &u);
    tree->Branch("v",       &v);
    tree->Branch("w",       &w);
    tree->Branch("x",       &x);
    tree->Branch("y",       &y);

    // Generate N events
    int i=0;
    while (i < nps) {
        std::vector<CRYParticle *> particles;
        gen->genEvent(&particles);
        
        double coreX = 0;
        double coreY = 0;
        for (auto* p : particles) coreX += p->x();
        for (auto* p : particles) coreY += p->y();
        coreX /= particles.size();
        coreY /= particles.size();

        if ((coreX > 100) || (coreY > 100)) continue; 

        pdg.clear();
        energy.clear();
        u.clear();
        v.clear();
        w.clear();
        x.clear();
        y.clear();
        z.clear();

        for (CRYParticle* p : particles) {
            pdg.push_back( cryToPDG(p->id(),p->charge()) );  // fallback to 0 if unknown
            energy.push_back(p->ke());
            u.push_back(p->u());
            v.push_back(p->v());
            w.push_back(p->w());
            x.push_back(p->x()-coreX);
            y.push_back(p->y()-coreY);
        }
        tree->Fill();
        i++;
    }

    outfile->cd();
    tree->Write();
    outfile->Close();
}

//neutron=0,proton=1,pion=2,kaon=3,muon=4,electron=5,gamma=6
int cryToPDG(int cryID, int charge) {
    switch (cryID) {
        case 0: return 2112; // neutron
        case 1: return 2212; // proton
        case 2: // pion
            if (charge == 1) return 211;   // π+
            else if (charge == -1) return -211; // π−
        case 3: // kaon
            if (charge == 1) return 321;   // K+
            else if (charge == -1) return -321; // K−
        case 4: // muon
            if (charge == 1) return -13;
            else if (charge ==-1) return 13;
        case 5: // electron
            if (charge == 1) return -11;
            else if (charge ==-1) return 11;
        case 6: return 22; // gamma
        default: 
            std::cout<<"Unknown particle found: "<<cryID<<","<<charge<<std::endl;       
            return 0; // unknown
    }
}
