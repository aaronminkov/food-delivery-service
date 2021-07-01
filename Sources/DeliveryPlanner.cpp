#include "provided.h"
#include "ExpandableHashMap.h"
#include <vector>
#include <list>
#include <cmath>
using namespace std;

string getDirection(double angle)
{
    string direction;
    if (1 <= angle && angle < 22.5)
        direction = "east";
    else if (22.5 <= angle && angle < 67.5)
        direction = "northeast";
    else if (67.5 <= angle && angle < 112.5)
        direction = "north";
    else if (112.5 <= angle && angle < 157.5)
        direction = "northwest";
    else if (157.5 <= angle && angle < 202.5)
        direction = "west";
    else if (202.5 <= angle && angle < 247.5)
        direction = "southwest";
    else if (247.5 <= angle && angle < 292.5)
        direction = "south";
    else if (292.5 <= angle && angle < 337.5)
        direction = "southeast";
    else
        direction = "east";
    return direction;
}

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_map;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm) : m_map(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    PointToPointRouter ppr(m_map); // create a new pointtopointrouter class
    DeliveryOptimizer dopt(m_map); // create a new delivery optimizer class
    vector<DeliveryRequest> optimizedDeliveries;
    optimizedDeliveries = deliveries;
    double oldCrowDistance, newCrowDistance;
    dopt.optimizeDeliveryOrder(depot, optimizedDeliveries, oldCrowDistance, newCrowDistance);

    list<StreetSegment> segRoute; // input for ppr
    double distance; // input for ppr
    
    GeoCoord prev = depot; // holds previous destination coordinate (starts at depot)
    auto it = optimizedDeliveries.begin();
    while(it != optimizedDeliveries.end())
    {
        DeliveryResult destRoute = ppr.generatePointToPointRoute(prev, (*it).location, segRoute, distance);
        if (destRoute != DELIVERY_SUCCESS) return destRoute; // if point router doesn't get route, return!
        totalDistanceTravelled += distance;
        StreetSegment previousSegment;
        auto segIt = segRoute.begin(); // iterate the destination route
        while (segIt != segRoute.end())
        {
            DeliveryCommand newCommand;
            string streetName = (*segIt).name; // maintain streetname
            double commandDistance = 0;
            // FIX THIS BC WRONG DIRECTION
            double angle = angleOfLine(*segIt);
            if (segIt != segRoute.begin())
            {
                double diffAngle = angleBetween2Lines(previousSegment, *segIt);
                if (diffAngle >= 1 && diffAngle < 180)
                {
                    DeliveryCommand turnCommand;
                    turnCommand.initAsTurnCommand("left", segIt->name);
                    commands.push_back(turnCommand);
                }
                else if (diffAngle >= 180 && diffAngle <= 359)
                {
                    DeliveryCommand turnCommand;
                    turnCommand.initAsTurnCommand("right", segIt->name);
                    commands.push_back(turnCommand);
                }
            }
            previousSegment = *segIt;
            while ((*segIt).name == streetName)
            {
                commandDistance += distanceEarthMiles(segIt->start, segIt->end);
                segIt++;
            }
            string direction = getDirection(angle);
            newCommand.initAsProceedCommand(direction, streetName, commandDistance);
            commands.push_back(newCommand);
        }
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(it->item);
        commands.push_back(deliver);
        prev = it->location;
        it++;
    }
    
    // time to go back to the depot!
    DeliveryResult returnHome = ppr.generatePointToPointRoute(prev, depot, segRoute, distance);
    if (returnHome != DELIVERY_SUCCESS) return returnHome; // if point router doesn't get route, return!
    totalDistanceTravelled += distance;
    StreetSegment previousSegment;
    auto segIt = segRoute.begin(); // iterate the destination route
    while (segIt != segRoute.end())
    {
        DeliveryCommand newCommand;
        string streetName = (*segIt).name; // maintain streetname
        double commandDistance = 0;
        double angle = angleOfLine(*segIt);
        if (segIt != segRoute.begin())
        {
            double diffAngle = angleBetween2Lines(*segIt, previousSegment);
            if (diffAngle >= 1 && diffAngle < 180)
            {
                DeliveryCommand turnCommand;
                turnCommand.initAsTurnCommand("left", segIt->name);
                commands.push_back(turnCommand);
            }
            else if (diffAngle >= 180 && diffAngle <= 359)
            {
                DeliveryCommand turnCommand;
                turnCommand.initAsTurnCommand("right", segIt->name);
                commands.push_back(turnCommand);
            }
        }
        previousSegment = *segIt;
        while ((*segIt).name == streetName)
        {
            commandDistance += distanceEarthMiles(segIt->start, segIt->end);
            segIt++;
        }
        string direction = getDirection(angle);
        newCommand.initAsProceedCommand(direction, streetName, commandDistance);
        commands.push_back(newCommand);
    }
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
