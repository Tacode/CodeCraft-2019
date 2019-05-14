#ifndef READ_FILES_H
#define READ_FILES_H
#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include "car.h"
#include "road.h"
#include "cross.h"
using namespace std;


void read_car(const char*, map<int, Car*> &);
void read_road(const char*, map<int, Road*> &);
void read_cross(const char*, map<int, Cross*> &);
void read_preset_answer(const char*, map<int, Car*>&, map<int, Car*>&);
void read_answer(const char*, map<int,Car*>&, map<int, Car*>&);
#endif // READ_FILES_H
