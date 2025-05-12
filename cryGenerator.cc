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

//This calculates CRs at a specified location on a specified date.
//TODO: 
// - Some kind of averaging if we have a long exposure?
int main(int argc, char* argv[]) {
    
    //How many particles to throw
    int nps=1000000;

    //Parse command line
    std::string outputName = "cry_output.root";  // default
    if (argc > 1) {
        outputName = argv[1];
    }

    //Check CRY paths set
    const char* crypath = std::getenv("CRYPATH");
    if (!crypath) {
        std::cerr << "Error: CRYPATH environment variable not set." << std::endl;
        return 1;
    }
    std::string cryDataDir = std::string(crypath) + "/../data";

    //Configure CRY
    //Blacksburg = 37.229572
    std::string inputConfig = R"(returnMuons 1
        returnNeutrons 1
        returnProtons 1
        returnGammas 1
        returnElectrons 1
        returnPions 1
        subboxLength 300
        altitude 0
        date 3-6-2015
        latitude 37.229572
        )";

    //Load setup file, create generator
    CRYSetup* setup = new CRYSetup(inputConfig, cryDataDir);
    CRYGenerator *gen = new CRYGenerator(setup);

    //Set-up output file
    TFile* outfile = new TFile(outputName.c_str(), "RECREATE");
    TTree* tree = new TTree("cryTree", "CRY particles");

    std::vector<int> pdg;
    std::vector<float> energy, u, v, w, x, y, z;
    tree->Branch("pdgcode", &pdg);
    tree->Branch("energy_MEV",  &energy);
    tree->Branch("u",       &u);
    tree->Branch("v",       &v);
    tree->Branch("w",       &w);
    tree->Branch("x_cm",       &x);
    tree->Branch("y_cm",       &y);

    // Generate N events
    int i=0;
    while (i < nps) {
        std::vector<CRYParticle *> particles;
        gen->genEvent(&particles);
        
        //Calculate core of event
        double coreX = 0;
        double coreY = 0;
        for (auto* p : particles) coreX += p->x();
        for (auto* p : particles) coreY += p->y();
        coreX /= particles.size();
        coreY /= particles.size();

        //Only keep central (fully contained) events
        if ((coreX > 100) || (coreY > 100)) continue; 

        //Clear vectors
        pdg.clear();
        energy.clear();
        u.clear();
        v.clear();
        w.clear();
        x.clear();
        y.clear();
        z.clear();

        //Push back particles
        for (CRYParticle* p : particles) {
            pdg.push_back( cryToPDG(p->id(),p->charge()) );  // fallback to 0 if unknown
            energy.push_back(p->ke());
            u.push_back(p->u());
            v.push_back(p->v());
            w.push_back(p->w());
            x.push_back(100*(p->x()-coreX));
            y.push_back(100*(p->y()-coreY));
        }
        //Fill
        tree->Fill();
        i++;
    }

    //Write
    outfile->cd();
    tree->Write("cryTree",TObject::kOverwrite);
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
