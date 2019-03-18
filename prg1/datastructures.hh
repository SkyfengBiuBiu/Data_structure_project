// Datastructures.hh

#ifndef DATASTRUCTURES_HH
#define DATASTRUCTURES_HH

#include <string>
#include <vector>
#include <utility>
#include <limits>
#include <map>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <list>
#include <set>
// Type for beacon IDs
using BeaconID = std::string;

// Return value for cases where required beacon was not found
BeaconID const NO_ID = "----------";

// Return value for cases where integer values were not found
int const NO_VALUE = std::numeric_limits<int>::min();

// Return value for cases where name values were not found
std::string const NO_NAME = "-- unknown --";

// Type for a coordinate (x, y)
struct Coord
{
    int x = NO_VALUE;
    int y = NO_VALUE;
};

// Example: Defining == and hash function for Coord so that it can be used
// as key for std::unordered_map/set, if needed
inline bool operator==(Coord c1, Coord c2) { return c1.x == c2.x && c1.y == c2.y; }
inline bool operator!=(Coord c1, Coord c2) { return !(c1==c2); } // Not strictly necessary

struct CoordHash
{
    std::size_t operator()(Coord xy) const
    {
        auto hasher = std::hash<int>();
        auto xhash = hasher(xy.x);
        auto yhash = hasher(xy.y);
        // Combine hash values (magic!)
        return xhash ^ (yhash + 0x9e3779b9 + (xhash << 6) + (xhash >> 2));
    }
};

// Example: Defining < for Coord so that it can be used
// as key for std::map/set
inline bool operator<(Coord c1, Coord c2)
{
    if (c1.y < c2.y) { return true; }
    else if (c2.y < c1.y) { return false; }
    else { return c1.x < c2.x; }
}

// Return value for cases where coordinates were not found
Coord const NO_COORD = {NO_VALUE, NO_VALUE};

// Type for color (RGB)
struct Color
{
    int r = NO_VALUE;
    int g = NO_VALUE;
    int b = NO_VALUE;
};

// Equality and non-equality comparisons for Colors
inline bool operator==(Color c1, Color c2) { return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b; }
inline bool operator!=(Color c1, Color c2) { return !(c1==c2); }

// Return value for cases where color was not found
Color const NO_COLOR = {NO_VALUE, NO_VALUE, NO_VALUE};

// Type for light transmission cost (used only in the second assignment)
using Cost = int;

// Return value for cases where cost is unknown
Cost const NO_COST = NO_VALUE;




//This is the beacon struct
struct Beacon {
    BeaconID id{NO_ID};
    std::string name{NO_NAME};
    Coord coord{NO_COORD};
    Color color{NO_COLOR};
};

//
struct SortedByName
{
    bool operator()(Beacon* p1, Beacon* p2)
    {
        return  p1->name < p2->name;
    }
};

struct SortedByBrightness
{

    bool operator()(Beacon* p1, Beacon* p2)
    {
        int brightness1=3*(p1->color.r)+6*(p1->color.g)+(p1->color.b);
        int brightness2=3*(p2->color.r)+6*(p2->color.g)+(p2->color.b);
        return brightness1 < brightness2;
    }
};

typedef std::vector<BeaconID>    SourceIds;
typedef std::unordered_map<BeaconID, SourceIds> Outlightbeams;
typedef std::shared_ptr<Beacon> BeaconPointer;
typedef std::unordered_map<BeaconID,BeaconPointer> IDToBeacons;
typedef std::unordered_map<BeaconID,BeaconID> Lightbeams;
typedef std::set<Beacon*,SortedByName> Alphalist;
typedef std::set<Beacon*,SortedByBrightness> Brightnesslist;
typedef std::vector<BeaconID> IDcollection;
typedef std::vector<Color> Colorcollection;
// This is the class you are supposed to implement

class Datastructures
{
private:
    // Add stuff needed for your class implementation here
    IDToBeacons beaid_beacons;
    Lightbeams lightbeams;
    Alphalist alphalist;
    Brightnesslist brightness_list;
    IDcollection alphaids;
    IDcollection brightnessids;
    bool alphalist_valid;
    bool brightness_valid;
    Outlightbeams out_lightbeams;

public:

    Datastructures();
    ~Datastructures();

    // Estimate of performance:O(n)
    // Short rationale for estimate: The simple counting operation is used to loop the elements in  unordered_map structure.
    int beacon_count();

    // Estimate of performance:O(n)
    // Short rationale for estimate:The clear function is based on erase all element. So the complexity depends on the size of map.
    void clear_beacons();

    // Estimate of performance:O(n)
    // Short rationale for estimate: It is based on the simple loop of this map.
    std::vector<BeaconID> all_beacons();

    // Estimate of performance:O(log(n))
    // Short rationale for estimate:Since beaid_beacons is unordered_map, but the types of alphalist and brightness_list are set. The average complexity of adding elemnent is log(n).
    bool add_beacon(BeaconID id, std::string const& name, Coord xy, Color color);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:This operation equals the search operation in unordered map.
    std::string get_name(BeaconID id);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:This operation equals the search operation in unordered map.
    Coord get_coordinates(BeaconID id);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:This operation equals the search operation in unordered map.
    Color get_color(BeaconID id);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance:O(n)
    // Short rationale for estimate:Because the sorting process had been finished in add_beacon function
    // and the alphalist is the ordered set according to alphabetical order of beacon names.
    // So, the left process in this function is to put the elements in alphalist into vector alphaids.
    // Since vector adds new element to the beck is O(1). So collecting all the elements is O(n).
    std::vector<BeaconID> beacons_alphabetically();

    // Estimate of performance:O(n)
    // Short rationale for estimate:Because the sorting process had been finished in add_beacon function
    // and the brightness_list is the ordered set according to increasing brightness of becons' own colors.
    // So, the left process in this function is to put the elements in brightness_list into vector brightnessids.
    // Since vector adds new element to the beck is O(1). So collecting all the elements is O(n).
    std::vector<BeaconID> beacons_brightness_increasing();

    // Estimate of performance:O(1)
    // Short rationale for estimate: If add_beacon function had been applied ,
    // the brightness_list had been sorted and organized.
    // The left operation is to take the first element: O (1).
    BeaconID min_brightness();

    // Estimate of performance:O(1)
    // Short rationale for estimate: If add_beacon function had been applied ,
    // the brightness_list had been sorted and organized.
    // The left operation is to take the last element: O (1).
    BeaconID max_brightness();

    // Estimate of performance:O(n)
    // Short rationale for estimate: The key point is to loop the elements in unordered_map and compare: O(n),
    //push the suitable elements into vector:O(1). So this operation is O(n).
    std::vector<BeaconID> find_beacons(std::string const& name);

    // Estimate of performance:O(log(n))
    // Short rationale for estimate:The basic idea is to search element with the specified id in beaid_beacons and to change the corresponding name.
    // This operation is (O(n)≈θ (1)). However, sorted sequence in set alphalist also need to be changed. Since the erase () and insert() in set is O(log(n)).
    // Thus, the total time complexity shall be O(log(n))
    bool change_beacon_name(BeaconID id, std::string const& newname);

    // Estimate of performance:O(log(n))
    // Short rationale for estimate: The basic idea is to search element with the specified id in beaid_beacons and to change the corresponding color.
    // This operation is (O(n)≈θ (1)). However, sorted sequence in set brightness_list also need to be changed. Since the erase () and insert() in set is O(log(n)).
    // Thus, the total time complexity shall be O(log(n))
    bool change_beacon_color(BeaconID id, Color newcolor);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate: For unordered_map, adding element is O(n)≈θ(1).
    bool add_lightbeam(BeaconID sourceid, BeaconID targetid);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:At first, the find function in unordered map out_lightbeams is O(n)≈θ(1).
    // And the next step is to sort the small vector.
    // Thus, the total time complexity is O(n)≈θ(1).
    std::vector<BeaconID> get_lightsources(BeaconID id);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:The worst case is to use "while" to loop the elements in unordered_map lightbeams :O(n)
    //The average time complexities for both vector to push element and unordered_map to find are  O(1).
    //Thus. the total average time complexity is O(n)≈θ(1).
    std::vector<BeaconID> path_outbeam(BeaconID id);

    // Non-compulsory operations

    // Estimate of performance:O(n)
    // Short rationale for estimate: Besides the erasing operation in unordered_map beaid_beacons: O(n)≈θ(1)
    // We need loop the elements in lightbeams,alphalist,brightness_list,alphaids,brightnessids and out_lightbeams to erase the suitable element: O(n)
    bool remove_beacon(BeaconID id);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate: In this place, I applied the DFS idea.
    // In this way, the algorithm would visit beacons before the given beacon id and get its lightsources.
    // Because get_lightsources is O(n)≈θ(1).
    // So the total average time complexity is O(n)≈θ (1).
    std::vector<BeaconID> path_inbeam_longest(BeaconID id);

    // Estimate of performance:O(n)≈θ(1)
    // Short rationale for estimate:Similar to the operation "path_inbeam_longest", only visit once for each beacon
    // and the a set of operations applied on this beacon is O(1).
    Color total_color(BeaconID id);


//The following functions are designed by myself for the better performance of the required functions.
    void dfs(BeaconID root, IDcollection list,
                             int len,int& maxLen,IDcollection& maxList);

    void averageColor(BeaconID targetId,BeaconID leaf,Color& color,IDcollection& leaf_vec);

    std::vector<BeaconID> get_lightsources_no_sort(BeaconID id);

    void findLeafNodes(BeaconID root,IDcollection& list);

    Color acculateColor(Color c,Colorcollection colorCollection);

};

#endif // DATASTRUCTURES_HH

