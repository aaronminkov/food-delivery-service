#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <set>
#include <iostream>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    struct OrderedCoord {
        OrderedCoord(GeoCoord coo) : coord(coo) {
            f = 0;
            g = 0;
            h = 0;
        };
        double f, g, h;
        GeoCoord coord;
        
        // compare OrderedCoord with f
        bool operator<(const OrderedCoord& newCoord) const
        {
            return f < newCoord.f;
        }
    };
    const StreetMap* m_map;
    ExpandableHashMap<GeoCoord, GeoCoord> locationOflastPointious;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) : m_map(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    if (start == end)
    {
        route.clear();
        totalDistanceTravelled = 0.0;
        return DELIVERY_SUCCESS;
    }
    route.clear();
    totalDistanceTravelled = 0.0;
    
    set<OrderedCoord> open; // I care about order for open, so used set.
    list<OrderedCoord> closed; // slower if ordered for closed, so used list.
    
    ExpandableHashMap<GeoCoord, GeoCoord> visited; // keep track of lastPointiously visited segments
           
    OrderedCoord startingPoint(start);
    open.insert(startingPoint);
    vector<StreetSegment> segments;
    
    while (!open.empty())
    {
        OrderedCoord currPoint = *(open.begin());
        auto popOff = open.begin();
        open.erase(popOff); // remove first element from open list
        
        closed.push_back(currPoint); // mark currPoint as visited and expanded.
        
        if (currPoint.coord.latitudeText == end.latitudeText && currPoint.coord.longitudeText == end.longitudeText) // check if we're at the end coordinate
        {
            for (;;)
            {
                if (currPoint.coord == start) break;
                GeoCoord* lastPoint = visited.find(currPoint.coord);
                if (lastPoint == nullptr) return BAD_COORD; // if the point has not been visited, bad coord!
                m_map->getSegmentsThatStartWith(*lastPoint, segments);
                
                auto it = segments.begin();
                while (it != segments.end())
                {
                    if ((*it).end != currPoint.coord)
                    {
                        it++;
                        continue;
                    }
                        
                    //create a street segment to push to our route
                    string streetName = (*it).name;
                    StreetSegment currSeg(*lastPoint, currPoint.coord, streetName);
                    
                    // push front to get proper order. push_back makes reverse order.
                    route.push_front(currSeg);
                    
                    // add this distance to the total
                    totalDistanceTravelled += distanceEarthMiles(*lastPoint, currPoint.coord);
                    it++;
                }
                currPoint = *(lastPoint);
            }
            return DELIVERY_SUCCESS;
        }
        
        // get any associated segments
        m_map->getSegmentsThatStartWith(currPoint.coord, segments);
        if (segments.empty()) return BAD_COORD; // if there's no segments found, bad coord!
            
        auto it = segments.begin();
        while (it != segments.end())
        {
            bool inList = false;
            auto closedIterator = closed.begin();
            while (closedIterator != closed.end())
            {
                if ((*it).end == (*closedIterator).coord)
                {
                    inList = true;
                    break;
                }
                closedIterator++;
            }
            if (inList)
            {
                it++;
                continue;
            }
            OrderedCoord childOfCurr((*it).end); //create a child based on current iterator's end
            
            visited.associate(childOfCurr.coord, currPoint.coord);
            
            childOfCurr.g = currPoint.g + distanceEarthMiles(currPoint.coord, childOfCurr.coord);
            // g of child is the current point's g as well as distance from currpoint to child coord
            childOfCurr.h = distanceEarthMiles(childOfCurr.coord, end); // h is distance from child to end
            childOfCurr.f = childOfCurr.g + childOfCurr.h;

            auto openIterator = open.begin();
            while (openIterator != open.end())
            {
                if (childOfCurr.g > (*openIterator).g  && childOfCurr.coord == (*openIterator).coord)
                {
                    inList = true;
                    break;
                }
                openIterator++;
            }

            if (!inList) open.insert(childOfCurr); //insert the generated child into the open list to expand
            it++;
        }
    }
    return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
