#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

unsigned int hasher(const string& s)
{
    return std::hash<string>()(s);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_coordMap;
};

StreetMapImpl::StreetMapImpl() // nothing to do here
{
}

StreetMapImpl::~StreetMapImpl() // nothing to do here
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream data(mapFile);
    if (!data) return false;
    
    string s;
    for (;;) // this loops for the amount of street names
    {
        string name, amount, startLat, startLong, endLat, endLong;
        if (!getline(data, name) || !getline(data, amount)) break;
        for (int i = 0; i < stoi(amount); i++) // this loops an for the amount of segments contained by a street name
        {
            // associate the actual coord
            if (!getline(data, startLat, ' ') || !getline(data, startLong, ' ') || !getline(data, endLat, ' ') || !getline(data, endLong)) return false;
            GeoCoord startCoord(startLat, startLong);
            GeoCoord endCoord(endLat, endLong);
            StreetSegment seg(startCoord, endCoord, name);
            vector<StreetSegment>* findCoord = m_coordMap.find(startCoord);
            if (findCoord != nullptr)
                findCoord->push_back(seg);
            else
            {
                vector<StreetSegment> segVec;
                segVec.push_back(seg);
                m_coordMap.associate(startCoord, segVec);
            }
            
            // associate the reverse
            StreetSegment reverseSeg(endCoord, startCoord, name);
            vector<StreetSegment>* findCoord2 = m_coordMap.find(endCoord);
            if (findCoord2 != nullptr)
                findCoord2->push_back(reverseSeg);
            else
            {
                vector<StreetSegment> revSegVec;
                revSegVec.push_back(reverseSeg);
                m_coordMap.associate(endCoord, revSegVec);
            }
        }
    }
    // m_coordMap.print();
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    if (!m_coordMap.find(gc)) return false;
    segs.clear();
    const vector<StreetSegment> temp = (*m_coordMap.find(gc));
    segs = temp;
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
