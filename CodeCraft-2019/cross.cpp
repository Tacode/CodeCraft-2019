#include "lib/cross.h"

Cross::Cross()
{

}

Cross::Cross(int id, vector<int> &roadsID)
{
    id_ = id;
    roadsID_ = roadsID;

    outArea_ = 0;
    inArea_ = 0;
    outCarNum_ = 0;
    inCarNum_ = 0;
    outDensity_ = 0;
    inDensity_ = 0;
}

Cross::Cross(int id, int north, int east, int south, int west) {
    if (roadsID_.size() > 0) {
        cout << " roadsID_ is not empty." << endl;
        exit(-1);
    }
    id_ = id;
    roadsID_.push_back(north);
    roadsID_.push_back(east);
    roadsID_.push_back(south);
    roadsID_.push_back(west);
}