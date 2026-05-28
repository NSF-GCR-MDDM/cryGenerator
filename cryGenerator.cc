#include "CRYGenerator.h"
#include "CRYSetup.h"
#include "CRYParticle.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <cmath>

#include "TTree.h"
#include "TFile.h"

int cryToPDG(int idCode,int charge);

//This calculates CRs at a specified location on a specified date.
//TODO: 
// - Some kind of time-averaging option for a long exposure?
int main(int argc, char* argv[]) {
    auto startTime = std::chrono::steady_clock::now();
    bool SAVE_ANGLES = false; //If you want to store the angles in the output tree, for debugging.

    //How many particles to throw
    int nps=2e7;
    float altitude = 0; //m, limited options in cry:  0, 2100, and 11300.        
    float latitude = 37.229572;  //Blacksburg = 37.229572, Leibstadt = 47.60095
    int length_m = 70; //m, limited options in cry: 1, 3, 10, 30, 100, and 300 m
                        // For box sizes in between these discrete values, the next 
                        // largest table is utilized and particles outside of the specified window are dropped.
                        // We leave this set to the largest value and apply a fiducial volume cut. Then we center
                        // shower particles around the geometric center of the shower. 

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
    std::ostringstream configStream;
    configStream << R"(returnMuons 1
    returnNeutrons 1
    returnProtons 1
    returnGammas 0
    returnElectrons 0
    returnPions 1
    subboxLength )" << length_m << R"(
    altitude )" << altitude << R"(
    date 9-6-2025
    latitude )" << latitude << R"(
    )";
    
    std::string inputConfig = configStream.str();

    //Load setup file, create generator
    CRYSetup* setup = new CRYSetup(inputConfig, cryDataDir);
    CRYGenerator *gen = new CRYGenerator(setup);

    //Set-up output file
    TFile* outfile = new TFile(outputName.c_str(), "RECREATE");
    TTree* tree = new TTree("cryTree", "CRY particles");

    std::vector<int> pdgCode;
    std::vector<float> energy;
    std::vector<float> x, y;
    std::vector<float> u,v,w;
    
    tree->Branch("pdgCode", &pdgCode);
    tree->Branch("energy_MeV",  &energy);
    tree->Branch("u",       &u);
    tree->Branch("v",       &v);
    tree->Branch("w",       &w);
    tree->Branch("x_mm",       &x);
    tree->Branch("y_mm",       &y);

    std::vector<float> theta,phi;
    if (SAVE_ANGLES==true) {
        tree->Branch("theta",       &theta);
        tree->Branch("phi",       &phi);
    }

    // Generate N events
    int i=0;
    while (i < nps) {
        std::vector<CRYParticle *> particles;
        particles.clear();
        gen->genEvent(&particles);

        //Clear vectors
        pdgCode.clear();
        energy.clear();
        u.clear();
        v.clear();
        w.clear();
        x.clear();
        y.clear();
        theta.clear();
        phi.clear();

        //Push back particles
        for (CRYParticle* p : particles) {
            pdgCode.push_back( cryToPDG(p->id(),p->charge()) ); 
            energy.push_back(p->ke()); //Default unit is MeV
            u.push_back(p->u());
            v.push_back(p->v());
            w.push_back(p->w());
            x.push_back(1000*(p->x())); //Default units are m, we convert to mm for our branch
            y.push_back(1000*(p->y())); 
            if (SAVE_ANGLES==true) {
                theta.push_back(std::acos(-p->w()));
                phi.push_back(std::atan2(p->v(), p->u()));
            }
        }
        //Fill
        tree->Fill();
        i++;
    }

    //Make header tree
    // Create a new tree for metadata
    TTree* headerTree = new TTree("headerTree", "CRY metadata");

    //Normalization
    float timeSimulated_s = gen->timeSimulated();
    float areaSimulated_cm2 = (length_m*100)*(length_m*100);
    float norm = static_cast<float>(nps) / (timeSimulated_s*areaSimulated_cm2);

    // Link branches
    headerTree->Branch("altitude", &altitude);
    headerTree->Branch("latitude", &latitude);
    headerTree->Branch("showers_per_cm2_per_s", &norm);
    headerTree->Branch("nEvents", &nps);

    // Fill once
    headerTree->Fill();
    headerTree->Write("headerTree", TObject::kOverwrite);

    //Write
    outfile->cd();
    tree->Write("cryTree",TObject::kOverwrite);
    outfile->Close();

    auto endTime = std::chrono::steady_clock::now();
    double runTime_s = std::chrono::duration<double>(endTime - startTime).count();

    std::cout << "Finished CRY generation." << std::endl;
    std::cout << "Run time: " << runTime_s << " s" << std::endl;
    std::cout << "CRY simulation corresponds to: " << timeSimulated_s << " s of real time" << std::endl;
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
