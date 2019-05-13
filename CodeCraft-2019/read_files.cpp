#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "lib/read_files.h"
using namespace  std;


void read_car(const char* carPath, map<int, Car*> &carsMap) {
    ifstream fin;
    fin.open(carPath);
    string line;
    vector<int> v;
    Car *carPtr = NULL;
    while(!fin.eof()) {
        getline(fin, line);
        if (line[0] == '#')
            continue;
        line = line.substr(1, line.length()-2); // 去掉首尾的（）
        char *cstr = new char[line.length()+1];
        strcpy(cstr, line.c_str());
        char *p = strtok(cstr, ",");
        v.clear();
        while(p != NULL) {
            v.push_back(atoi(p));
            p = strtok(NULL, ",");
        }
        carPtr = new Car(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
        carsMap[v[0]] = carPtr;
        delete[] cstr;
    }

    fin.close();
}

void read_road(const char* roadPath, map<int,Road*> &roadsMap) {
    ifstream fin;
    fin.open(roadPath);
    string line;
    Road * roadPtr = NULL;
    vector<int> v;
    while (!fin.eof()) {
        getline(fin, line);
        if (line[0] == '#')
            continue;
        line = line.substr(1, line.length()-2);
        char *cstr = new char[line.length()+1];
        strcpy(cstr, line.c_str());
        char *p = strtok(cstr, ",");
        v.clear();
        while(p != NULL) {
            v.push_back(atoi(p));
            p = strtok(NULL, ",");
        }
        roadPtr = new Road(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
        roadsMap[v[0]] = roadPtr;
        delete[] cstr;
    }
    fin.close();
}

void read_cross(const char* crossPath, map<int, Cross*> &crossesMap) {
    ifstream fin;
    fin.open(crossPath);
    string line;
    Cross *crossPtr = NULL;
    vector<int> v;
    while(!fin.eof()) {
        getline(fin, line);
        if (line[0] == '#')
            continue;
        line = line.substr(1, line.length()-2);
        char * cstr = new char[line.length()+1];
        strcpy(cstr, line.c_str());
        char *p = strtok(cstr, ",");
        vector<int> v;
        while(p != NULL) {
            v.push_back(atoi(p));
            p = strtok(NULL, ",");
        }
        crossPtr = new Cross(v[0], v[1], v[2], v[3], v[4]);
        crossesMap[v[0]] = crossPtr;
        delete[] cstr;
        v.clear();
    }
    fin.close();
}

void read_preset_answer(const char* preset_answer_path, map<int, Car*> &carsMap, map<int, Car*> &presetMap) {
    ifstream fin;
    fin.open(preset_answer_path, ios::in);
    string line;
    Car* carPtr = NULL;
    vector<int> v;
    while(!fin.eof()) {
        getline(fin, line);
        if (line[0] == '#')
            continue;
        line = line.substr(1, line.length()-1);
        char * cstr = new char[line.length() + 1];
        strcpy(cstr, line.c_str());
        char *p = strtok(cstr, ",");
        v.clear();
        while(p != NULL) {
            v.push_back(atoi(p));
            p = strtok(NULL, ",");
        }
        carPtr = carsMap[v[0]];
        carPtr->startTime = v[1];
        delete[] cstr;
        for (int i = 2; i < (int) v.size(); i++) {
            carPtr->histroyRoadsID.push_back(v[i]);
            carPtr->roadsIDQueue.push(v[i]);
        }
        presetMap[v[0]] = carPtr;
    }
    fin.close();
}

void read_answer(const char* answer_path, map<int, Car*> &carsMap, map<int, Car*> &presetMap)
{
    ifstream fin;
    fin.open(answer_path, ios::in);
    string line;
    Car* carPtr = NULL;
    vector<int> v;
    int carNum = 0;
    
    while (!fin.eof())
    {
        getline(fin, line);
        if(line.length() < 1)
            break;
        line = line.substr(1, line.length() - 2);
        char *cstr = new char[line.length()+1];
        strcpy(cstr, line.c_str());
        char *p = strtok(cstr, ",");
        v.clear();
        while(p != NULL) {
            v.push_back(atoi(p));
            p = strtok(NULL, ",");
        }

        carPtr = carsMap[v[0]];
        carPtr->startTime = v[1];
        delete[] cstr;
        for (int i = 2; i < (int) v.size(); i++) {
            carPtr->histroyRoadsID.push_back(v[i]);
            carPtr->roadsIDQueue.push(v[i]);
        }
        carNum++;
        assert(carPtr->planTime <= carPtr->startTime);
    }
    fin.close();
    cout << "carNum :" << carsMap.size() << " carNum in preset: "<< presetMap.size()<< " carNum in answer: " << carNum << endl;
    if (carsMap.size() != carNum)
        carNum += presetMap.size();

    assert(carNum == (int)carsMap.size());
}