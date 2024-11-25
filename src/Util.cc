#include "Util.hh"

#include <vector>
#include <fstream>

using namespace std;

namespace lircst {
    G4int Util::GenMapKey(G4int x, G4int y, G4int bin) {
        return x + fNumPixelsY * (y + fNumPixelsY * bin);
    }

    G4bool Util::exportData(vector<vector<vector<G4double>>> data, G4String filename) {
        // TODO: this was auto gen lol

        ofstream file;
        file.open(filename);
        if (!file.is_open()) return false;

        for (G4int i = 0; i < fNumPixelsX; i++) {
            for (G4int j = 0; j < fNumPixelsY; j++) {
                for (G4int k = 0; k < fNumBins; k++) {
                    file << data[i][j][k] << " ";
                }
                file << endl;
            }
        }

        file.close();
        return true;
    }
}