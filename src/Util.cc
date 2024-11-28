#include "Util.hh"

#include <vector>
#include <fstream>

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

    G4bool Util::ExportData(AccumulableMap<G4int> data, G4String filename) {
        G4String folder = "output/";
        G4String imgExtention = ".bmp";
        
        std::map<G4int,G4int> map = data.GetMap();

        // Deconstruct key into i, j, bin
        auto unpackedData = vector<vector<vector<G4int>>>(fNumPixelsX, vector<vector<G4int>>(fNumPixelsY, vector<G4int>(fNumBins, 0)));

        for (const auto& entry : map) {
            G4int key = entry.first;
            G4int value = entry.second;

            int bin = key / (fNumPixelsY * fNumPixelsY);
            int remainder1 = key % (fNumPixelsY * fNumPixelsY);
            int j = remainder1 / fNumPixelsY;
            int i = remainder1 % fNumPixelsY;

            G4cout << "i, j, bin, value: " << i << ", " << j << ", " << bin << ", " << value << ", " << G4endl;
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
                uint8_t pixelValue = static_cast<uint8_t>(min(totalEnergy / fEnergyMax, one) * 255);
                if (pixelValue > 0) {
                    G4cout << "Pixel value at i, j, and totalEnergy: " << i << ", " << j << " is " << static_cast<G4int>(pixelValue) << " and " << totalEnergy << G4endl;
                }
                pixelData[i * fNumPixelsX + j] = static_cast<uint8_t>(pixelValue);
            }
        }

        bmp.write(ImageView(fNumPixelsX, fNumPixelsY, 1, pixelData.data()), folder + filename + imgExtention);

        return true;
    }
}