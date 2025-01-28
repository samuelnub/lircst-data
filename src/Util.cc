#include "Util.hh"

#include <vector>
#include <ctime>

#include "npy.hh"
#include "BMPMini.hh"

using namespace std;
using namespace image;

namespace lircst {
    G4int Util::GenMapKey(G4int x, G4int y, G4int bin) {
        return x + fNumPixelsY * (y + fNumPixelsY * bin);
    }

    G4double Util::BinToEnergy(G4int bin) {
        return fEnergyMin + (fEnergyMax - fEnergyMin) / fNumBins * bin;
    }

    G4bool Util::ExportData(AccumulableMap<G4int> data, G4String filename = "") {
        G4String folder = "output/";
        if (filename == "") {
            filename = to_string(time(0)); // Not a guarantee for unix timestamps, but good enough for our purposes
        }
        G4String imgExtention = ".bmp";
        
        std::map<G4int,G4int> map = data.GetMap();

        // Export .npy file of shape i x j x bin
        npy::npy_data<G4int> npyData;
        npyData.shape = {fNumPixelsX, fNumPixelsY, fNumBins};
        npyData.data.reserve(fNumPixelsX * fNumPixelsY * fNumBins);
        npyData.fortran_order = false;
        // Iterate over i, j, bin, and if count exists in map, add it to npyData
        for (G4int i = 0; i < fNumPixelsX; i++) {
            for (G4int j = 0; j < fNumPixelsY; j++) {
                for (G4int bin = 0; bin < fNumBins; bin++) {
                    G4int key = GenMapKey(i, j, bin);
                    if (map.find(key) != map.end()) {
                        npyData.data.push_back(map[key]);
                    } else {
                        npyData.data.push_back(0);
                    }
                }
            }
        }
        npy::write_npy(folder + filename + ".npy", npyData);
        

        // Deconstruct key into i, j, bin
        auto unpackedData = vector<vector<vector<G4int>>>(fNumPixelsX, vector<vector<G4int>>(fNumPixelsY, vector<G4int>(fNumBins, 0)));

        for (const auto& entry : map) {
            G4int key = entry.first;
            G4int value = entry.second;

            int bin = key / (fNumPixelsY * fNumPixelsY);
            int remainder1 = key % (fNumPixelsY * fNumPixelsY);
            int j = remainder1 / fNumPixelsY;
            int i = remainder1 % fNumPixelsY;

            unpackedData[i][j][bin] = value;
        }

        // For image export, integrate over energy bins
        // This is mainly for visualisation (our real data that we will learn over, is the 3D tensors eventually)
        BMPMini bmp;

        vector<uint8_t> pixelData(fNumPixelsX * fNumPixelsY); // Grayscale, 1 channel

        for (G4int i = 0; i < fNumPixelsX; i++) {
            for (G4int j = 0; j < fNumPixelsY; j++) {
                G4double totalEnergy = 0;
                for (G4int k = 0; k < fNumBins; k++) {
                    totalEnergy += BinToEnergy(k) * unpackedData[i][j][k];
                }
                G4double one = 1.0;
                G4double maxEnergy = fEnergyMax * 2; // Arbitrary scaling factor to allow accumulated photons to shine through
                uint8_t pixelValue = static_cast<uint8_t>(min(totalEnergy / maxEnergy, one) * 255);
                pixelData[i * fNumPixelsX + j] = static_cast<uint8_t>(pixelValue);
            }
        }

        bmp.write(ImageView(fNumPixelsX, fNumPixelsY, 1, pixelData.data()), folder + filename + imgExtention);

        return true;
    }
}