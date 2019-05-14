#include "iostream"
#include "fstream"
#include <ctime>
#include "lib/solution.h"
#include "lib/read_files.h"
#include "lib/judge.h"

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
	read_answer(answerPath.c_str(), carsMap, presetMap);
	Judgement judge(carsMap, roadsMap, crossesMap);
	judge.judge();
	cout << "total time used: " << (double)(clock() - start) / CLOCKS_PER_SEC << " s" << endl;
	return 0;
}