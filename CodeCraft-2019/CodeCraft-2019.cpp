#include "iostream"
#include "fstream"
#include <ctime>
#include "lib/solution.h"
#include "lib/read_files.h"

int main(int argc, char *argv[])
{
    std::cout << "Begin" << std::endl;
	clock_t start = clock();
	if(argc < 6){
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}
	
	std::string carPath(argv[1]);
	std::string roadPath(argv[2]);
	std::string crossPath(argv[3]);
	std::string presetAnswerPath(argv[4]);
	std::string answerPath(argv[5]);
	
	std::cout << "carPath is " << carPath << std::endl;
	std::cout << "roadPath is " << roadPath << std::endl;
	std::cout << "crossPath is " << crossPath << std::endl;
	std::cout << "presetAnswerPath is " << presetAnswerPath << std::endl;
	std::cout << "answerPath is " << answerPath << std::endl;
	
	// TODO:read input filebuf
	map<int, Car*> carsMap;
	map<int, Road*> roadsMap;
	map<int, Cross*> crossesMap;
	map<int, Car*> presetMap;
	read_car(carPath.c_str(), carsMap);
	read_road(roadPath.c_str(), roadsMap);
	read_cross(crossPath.c_str(), crossesMap);
	read_preset_answer(presetAnswerPath.c_str(), carsMap, presetMap);


	// TODO:process
	Solver solver(carsMap, roadsMap, crossesMap, presetMap);
	solver.manage_unlock();

	// TODO:write output file
	int carID;
	Car *car;
    ofstream outFile(answerPath);
    if (outFile.is_open()) {
        for (auto &item: solver.manCarsMap) {
			carID = item.first;
			car = item.second;
            vector<int> historyRoadsID = car->histroyRoadsID;
            int startTime = car->startTime;
            int len = historyRoadsID.size();
            string historyRoads = "";
            for(int i=0; i<len; i++)
                historyRoads += (i==(len-1))?to_string(historyRoadsID[i]):to_string(historyRoadsID[i])+",";
            string result = "";
            result += "("+ to_string(carID) + ","+ to_string(startTime) + "," + historyRoads + ")";
            outFile << result;
            outFile << "\n";
        }
    }
    outFile.close();
	
	cout << "total time used: " << (double)(clock()-start)/CLOCKS_PER_SEC << " s" << endl;
	return 0;
}