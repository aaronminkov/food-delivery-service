#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* m_map;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm) : m_map(sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    // calculate the old crow distance
    GeoCoord prev = depot;
    for (int i = 0; i < deliveries.size(); i++)
    {
        oldCrowDistance += distanceEarthMiles(prev, deliveries[i].location);
        prev = deliveries[i].location;
    }
    oldCrowDistance += distanceEarthMiles(prev, depot);
    prev = depot; // reset prev to depot because we will loop again!
    
    vector<DeliveryRequest> reorderedAndVisited;
    DeliveryRequest previouslyPushed("depot", depot);
    while (!deliveries.empty())
    {
        DeliveryRequest nextToPush = deliveries[0];
        auto deleteMe = deliveries.begin(); // the one we'll delete (not necesarrily begin())
        
        auto findNew = deliveries.begin();
        while (findNew != deliveries.end())
        {
            if (distanceEarthMiles(findNew->location, previouslyPushed.location) < distanceEarthMiles(nextToPush.location, previouslyPushed.location))
            {
                nextToPush = (*findNew); // find the next closest neighbor
                deleteMe = findNew; // set that closest neighbor's location in the vector to delete
            }
            findNew++;
        }
        reorderedAndVisited.push_back(nextToPush);
        previouslyPushed = nextToPush;
        deliveries.erase(deleteMe);
    }
    deliveries = reorderedAndVisited; // set the return vector to the reordered vector!
    
    // calculate the new crow distance
    prev = depot;
    for (int i = 0; i < deliveries.size(); i++)
    {
        newCrowDistance += distanceEarthMiles(prev, deliveries[i].location);
        prev = deliveries[i].location;
    }
    newCrowDistance += distanceEarthMiles(prev, depot);
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
