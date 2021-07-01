#ifndef HASHMAP_INCLUDED
#define HASHMAP_INCLUDED

#include "provided.h"
#include <iostream>
#include <vector>
#include <functional>

// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

const int START_BUCKET_AMOUNT = 8;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;
    
    void print() const;

private:
    int m_bucketCount;
    int m_associations;
    double m_maximumLoadFactor;

    struct KeyValuePair {
        KeyValuePair(KeyType key, ValueType value) : key(key), value(value) {};
        KeyValuePair(const KeyValuePair& pair)
        {
            key = pair.key;
            value = pair.value;
        }
        KeyValuePair operator=(const KeyValuePair& pair)
        {
            key = pair.key;
            value = pair.value;
        }
        
        KeyType key;
        ValueType value;
    };
    
    struct BUCKET {
        std::list<KeyValuePair> m_content;
    };
    
    std::vector<BUCKET*> m_buckets;
    
    void cleanUp();
    
    double currentLoadFactor() const;
    void expandMap();
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) : m_bucketCount(START_BUCKET_AMOUNT), m_associations(0), m_maximumLoadFactor(maximumLoadFactor)
{
    for (int i = 0; i < START_BUCKET_AMOUNT; i++)
        m_buckets.push_back(new BUCKET);
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    cleanUp(); // cleanUp is all we need!
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    cleanUp();
    m_bucketCount = START_BUCKET_AMOUNT;
    m_associations = 0;
    for (int i = 0; i < START_BUCKET_AMOUNT; i++)
        m_buckets.push_back(new BUCKET);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_associations;  // Return # of associations. Easy!
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    unsigned int hasher(const KeyType& k);
    unsigned int hashedValue = hasher(key);
    int bucket = hashedValue % m_bucketCount;
    ValueType* pointerVal = find(key);
    if (pointerVal != nullptr)
    {
        *pointerVal = value;
    }
    else
    {
        KeyValuePair pair(key, value);
        m_buckets[bucket]->m_content.push_back(pair);
        m_associations++;
        if (currentLoadFactor() > m_maximumLoadFactor)
            expandMap();
    }
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int hasher(const KeyType& k);
    unsigned int hashedValue = hasher(key);
    int bucket = hashedValue % m_bucketCount;
    typename std::list<KeyValuePair>::iterator it = m_buckets[bucket]->m_content.begin();
    while (it != m_buckets[bucket]->m_content.end())
    {
        if ((*it).key == key)
            return &((*it).value);
        it++;
    }
    return nullptr;
}

// PRIVATE MEMBER FUNCTIONS

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::cleanUp()
{
    typename std::vector<BUCKET*>::iterator it = m_buckets.begin();
    while (it != m_buckets.end())
    {
        BUCKET* tempBucket = (*it);
        it = m_buckets.erase(it);
        delete tempBucket;
    }
}

template<typename KeyType, typename ValueType>
double ExpandableHashMap<KeyType, ValueType>::currentLoadFactor() const
{
    return m_associations*1.0 / m_bucketCount;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::expandMap()
{
    unsigned int hasher(const KeyType& k);
    int newSize = m_bucketCount * 2; // double the bucket count
    std::vector<BUCKET*> newBucket;
    for (int i = 0; i < newSize; i++)
        newBucket.push_back(new BUCKET);
    for (int j = 0; j < m_bucketCount; j++)
    {
        if (m_buckets[j]->m_content.size() == 0) continue;
        typename std::list<KeyValuePair>::iterator it = m_buckets[j]->m_content.begin();
        int bucket; // bucket number to be determined in while loop
        while (it != m_buckets[j]->m_content.end())
        {
            bucket = hasher((*it).key) % newSize;
            KeyValuePair pair((*it).key, (*it).value);
            newBucket[bucket]->m_content.push_back(pair);
            it++;
        }
    }
    
    cleanUp(); // delete old m_buckets;
    m_bucketCount = newSize;
    m_buckets.swap(newBucket);
}

// DELETE THIS FUNCTION LATER

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::print() const
{
    for (int i = 0; i < m_bucketCount; i++)
    {
        if (m_buckets[i]->m_content.size() == 0) continue;
        auto it = m_buckets[i]->m_content.begin();
        while (it != m_buckets[i]->m_content.end())
        {
            std::cout << "KEY: (" << (*it).key.latitudeText << ", " << (*it).key.longitudeText << ")" << std::endl;
            auto it2 = (*it).value.begin();
            std::cout << "VALUE..." << std::endl;
            while (it2 != (*it).value.end())
            {
               std::cout << "START: (" << (*it2).start.latitudeText << ", " << (*it2).start.longitudeText << ")" << std::endl;
               std::cout << "END: (" << (*it2).end.latitudeText << ", " << (*it2).end.longitudeText << ")" << std::endl;
                std::cout << "NAME: " << (*it2).name << std::endl << "..." << std::endl;
                it2++;
            }
            std::cout << std::endl;
            it++;
        }
    }
}

#endif // HASHMAP_INCLUDED
