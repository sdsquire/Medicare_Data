// NOTE: THIS FUNCTION is meant to be run from WITHIN THE JUPYTER NOTEBOOK (FIPS_analysis.ipynb). It will not work otherwise. Just run the function

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include <chrono>

//Define struct to better hold the fips distances
struct FipsDist {
   std::string tract1;
   std::string tract2;
   double distance;

   FipsDist() : distance(-99999){}
   FipsDist(char tract1 [], char tract2 [], double distance) : tract1(tract1), tract2(tract2), distance(distance) {}
   explicit FipsDist(std::string line) {
      tract1 = line.substr(1, 5);
      line = line.substr(line.find(',')+1);
      tract1 += line.substr(1,6);
      line = line.substr(line.find(',')+1);
      distance = std::stod(line.substr(0,6));
      line = line.substr(line.find(',')+1);
      tract2 = line.substr(1, 5);
      line = line.substr(line.find(',')+1);
      tract2 += line.substr(1,6);
      }

      std::string toString() const {
         std::stringstream ss;
         ss << '"' << tract1 << "\"," << distance << ",\"" << tract2 << "\"";
         return ss.str();
      }
   };

int main() {
   std::ifstream inFS;
   std::ofstream outFS;
   std::set<std::string> adiTracts;
   std::set<std::string> physTracts;
   std::set<std::string> noAdiTracts;
   std::set<std::string> noDistTracts;
   std::set<std::string> noPhysTracts;
   std::set<std::string> uniqueTracts;
   std::map<std::string, FipsDist> closestFips;
   std::string currLine;

   // Reading in physician tracts ...
   inFS.open("processed_data/Phys_Tracts.txt");
   while(std::getline(inFS, currLine, ','))
      physTracts.insert(currLine);
   inFS.close();
   std::cout << "There are " << physTracts.size() << " physician tracts.\n";

   // Reading in ADI tracts...
   inFS.open("raw_data/fips_adi.csv");
   std::getline(inFS, currLine);
   while(std::getline(inFS, currLine))
      adiTracts.insert(currLine.substr(currLine.find(',')+2, 11));
   inFS.close();
   noDistTracts.insert(adiTracts.begin(), adiTracts.end());
   noPhysTracts.insert(adiTracts.begin(), adiTracts.end());
   std::cout << "There are " << adiTracts.size() << " adi tracts.\n";

   // MAIN PORTION //
   // Variables t1, t2, and counter do not impact the program, and are only used to measure performance
   auto t1 = std::chrono::high_resolution_clock::now();
   inFS.open("raw_data/fips_dist.csv");
   std::getline(inFS, currLine); //Drops column headings
   int counter = 0;
   FipsDist currFipsDist;
   while (std::getline(inFS, currLine)) {
      counter++;
		if (counter % 1000000 == 0)
         std::cout << "Line " << counter / 1000000 << "M\n";
		currFipsDist = FipsDist(currLine);
        if (currFipsDist.tract1.substr(0,5) == "06073")
            std::cout << "San Diego\n";
		uniqueTracts.insert(currFipsDist.tract1);
		uniqueTracts.insert(currFipsDist.tract2);
		noDistTracts.erase(currFipsDist.tract1);

		// Ensure that row includes a physician tract, and putting the physician tract in the tract2 position
		if (physTracts.find(currFipsDist.tract2) == physTracts.end())
         std::swap(currFipsDist.tract1, currFipsDist.tract2);
		if (physTracts.find(currFipsDist.tract2) == physTracts.end())
         continue;


      noPhysTracts.erase(currFipsDist.tract1);

		// Ensure that tract is in adiTracts
		if (adiTracts.find(currFipsDist.tract1) == adiTracts.end()) {
         noAdiTracts.insert(currFipsDist.tract1);
         continue;
		}
		// Add distance pair to map if it doesn't exist or if it is the shortest distance
		if (closestFips.find(currFipsDist.tract1) == closestFips.end())
         closestFips.insert({currFipsDist.tract1, currFipsDist});
		else if (currFipsDist.distance < closestFips.find(currFipsDist.tract1)->second.distance)
         closestFips.at(currFipsDist.tract1)=currFipsDist;
       std::string data = closestFips.at(currFipsDist.tract1).tract2;
       int temp = 0;
   }
   inFS.close();
   auto t2 = std::chrono::high_resolution_clock::now();
   outFS.open("processed_data/fips_dist.csv");
   outFS << "tract1,mi_to_tract,tract2\n";
   for (const auto& fipsPair : closestFips)
      outFS << fipsPair.second.toString() <<'\n';
   outFS.close();

   outFS.open("processed_data/aux_data_fips_dist/missing_tracts.txt");
   for (const auto& tract : noDistTracts)
      outFS << tract << '\n';
   outFS.close();

   outFS.open("processed_data/aux_data/fips_dist/unique_tracts.txt");
   for (const auto& tract : uniqueTracts)
      outFS << tract << '\n';
   outFS.close();

    outFS.open("processed_data/aux_data/fips_dist/no_physician.txt");
    for (const auto& tract : noPhysTracts)
        outFS << tract << '\n';
    outFS.close();

   std::cout << noAdiTracts.size() << " tracts did not have associated adis\n";
   std::cout << noDistTracts.size() << " adis did not have an associated distance\n";
   std::cout << noPhysTracts.size() << " adis did not have a closest physician (>100 mi)\n";
   std::cout << "Filtering through fips took " << std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count() << " seconds\n";
}
