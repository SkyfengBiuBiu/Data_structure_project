// Datastructures.cc

#include "datastructures.hh"

#include <random>

#include <cmath>


std::minstd_rand rand_engine; // Reasonably quick pseudo-random generator

template<typename Type>
Type random_in_range(Type start, Type end) {
    auto range = end - start;
    ++range;

    auto num = std::uniform_int_distribution<unsigned long int>(0, range - 1)(rand_engine);

    return static_cast<Type>(start + num);
}

// Modify the code below to implement the functionality of the class.
// Also remove comments from the parameter names when you implement
// an operation (Commenting out parameter name prevents compiler from
// warning about unused parameters on operations you haven't yet implemented.)

Datastructures::Datastructures() {

    std::cout << "Datastructures object is being created" << std::endl;
    beaid_beacons = IDToBeacons();
    lightbeams = Lightbeams();
    alphalist = Alphalist();
    brightness_list = Brightnesslist();
    alphaids = IDcollection();
    brightnessids = IDcollection();
    alphalist_valid = false;
    brightness_valid = false;
    out_lightbeams = Outlightbeams();

}

Datastructures::~Datastructures() {

    std::cout << "Datastructures object is being deleted" << std::endl;
}

int Datastructures::beacon_count() {
    return int(beaid_beacons.size());
}

void Datastructures::clear_beacons() {
    if (!beaid_beacons.empty()) {
        //All the containers need to be cleared
        beaid_beacons.clear();
        alphalist.clear();
        brightness_list.clear();
        alphaids.clear();
        brightnessids.clear();
        lightbeams.clear();
        out_lightbeams.clear();
    }


}

std::vector<BeaconID> Datastructures::all_beacons() {
    IDcollection result;
    for (auto &data : beaid_beacons) {
        //Use a vector to collected all the beacon ids
        result.push_back(data.first);
    }
    return result;
}

bool Datastructures::add_beacon(BeaconID id, std::string const &name, Coord xy, Color color) {
    if (beaid_beacons.find(id) == beaid_beacons.end()) {
        auto p = std::make_shared<Beacon>(Beacon{id, name, xy, color});
        auto l = p.get();
        //Once added one new element, these three containers needs to perform inserting function.
        beaid_beacons.insert({id, p});
        alphalist.insert(l);
        brightness_list.insert(l);
        alphalist_valid = brightness_valid = false;
        return true;
    } else {
        return false;
    }
}

std::string Datastructures::get_name(BeaconID id) {
    BeaconPointer get_beacon = beaid_beacons.at(id);
    return (*get_beacon).name;
}

Coord Datastructures::get_coordinates(BeaconID id) {
    BeaconPointer get_beacon = beaid_beacons.at(id);
    return (*get_beacon).coord;
}

Color Datastructures::get_color(BeaconID id) {
    std::shared_ptr<Beacon> get_beacon = beaid_beacons.at(id);
    return (*get_beacon).color;
}

std::vector<BeaconID> Datastructures::beacons_alphabetically() {
    //Sorting process had been finished in add_beacon, the left part is to put sorted elements into a vector.
    if (!alphalist_valid) {
        alphaids.clear();
        std::set<Beacon *>::iterator it;
        for (it = alphalist.begin(); it != alphalist.end(); ++it) {
            alphaids.push_back((*it)->id);
        }
        alphalist_valid = true;
    }
    return alphaids;
}

std::vector<BeaconID> Datastructures::beacons_brightness_increasing() {
    //Sorting process had been finished in add_beacon, the left part is to put sorted elements into a vector.
    if (!brightness_valid) {
        brightnessids.clear();
        brightness_valid = true;
        std::set<Beacon *>::iterator it;
        for (it = brightness_list.begin(); it != brightness_list.end(); ++it) {
            brightnessids.push_back((*it)->id);
        }
    }
    return brightnessids;
}

BeaconID Datastructures::min_brightness() {
    if (beacon_count() == 0) return NO_ID;
    else {
        //Return the first element in brightness_list
        return (*brightness_list.begin())->id;
    }

}

BeaconID Datastructures::max_brightness() {
    if (beacon_count() == 0) return NO_ID;
    else {
        //Return the last element in brightness_list
        return (*(--brightness_list.end()))->id;
    }
}

std::vector<BeaconID> Datastructures::find_beacons(std::string const &name) {
    IDcollection ids = IDcollection();
    for (auto &data : beaid_beacons) {
        auto beacon_pointer = data.second;
        //Observe the element with the given name exits or not
        if (!name.compare(beacon_pointer.get()->name)) {
            //Collect the suitable elements
            ids.push_back(beacon_pointer.get()->id);
        }
    }
    return ids;
}

bool Datastructures::change_beacon_name(BeaconID id, const std::string &newname) {

    try {
        //alphalist needs to remove the old element
        alphalist.erase(beaid_beacons.at(id).get());
        beaid_beacons.at(id).get()->name=newname;
        //And insert the new one.
        alphalist.insert(beaid_beacons.at(id).get());
        alphalist_valid = brightness_valid = false;

        //The alphaids and brightnessids need to be recollected
        alphalist_valid = brightness_valid = false;
        return true;
    }
    catch (const std::out_of_range &e) {
        (void)e;
        return false;
    }

}

bool Datastructures::change_beacon_color(BeaconID id, Color newcolor) {

    try {
        //brightness_list needs to remove the old element
        brightness_list.erase(beaid_beacons.at(id).get());
        beaid_beacons.at(id).get()->color=newcolor;
        //And insert the new one.
        brightness_list.insert(beaid_beacons.at(id).get());
        //The alphaids and brightnessids need to be recollected
        alphalist_valid = brightness_valid = false;
        return true;
    }
    catch ( const std::out_of_range &e) {
        (void)e;
        return false;
    }

}

bool Datastructures::add_lightbeam(BeaconID sourceid, BeaconID targetid) {
    if (beaid_beacons.count(sourceid) <= 0 && beaid_beacons.count(targetid) <= 0) {
        // Check if the element inluded in the lightbeams or not
        return false;
    } else if (!lightbeams[sourceid].compare(NO_ID)) {
        // Check if the targeted element already exits or not
        return false;
    } else {
        lightbeams[sourceid] = targetid;
        out_lightbeams[targetid].push_back(sourceid);
        return true;
    }
}

std::vector<BeaconID> Datastructures::get_lightsources(BeaconID id) {
    SourceIds vec = SourceIds();
    auto it = out_lightbeams.find(id);
    if (it != out_lightbeams.end()) {
        //Find and collect the source ids
        vec = it->second;
        std::sort(vec.begin(), vec.end());
        return vec;
    } else {
        return {{NO_ID}};
    }
}
// This function designed  to help save time without sort
std::vector<BeaconID> Datastructures::get_lightsources_no_sort(BeaconID id) {
    SourceIds vec{};
    auto it = out_lightbeams.find(id);
    if (it != out_lightbeams.end()) {
        vec = it->second;
        return vec;
    } else {
        return {{NO_ID}};
    }
}

std::vector<BeaconID> Datastructures::path_outbeam(BeaconID id) {
    std::vector<BeaconID> vec{};
    if (beaid_beacons.count(id) > 0) {
        vec.push_back(id);
        auto it = lightbeams.find(id);
        //Apply the while loop find all the source light beams
        while (it != lightbeams.end()) {
            vec.push_back(it->second);
            it = lightbeams.find(it->second);
        }
        return vec;
    } else {
        return {{NO_ID}};
    }
}

bool Datastructures::remove_beacon(BeaconID id) {
    auto it = beaid_beacons.find(id);
    if (it != beaid_beacons.end()) {
        //Remove from the beacon map
        beaid_beacons.erase(it);
        //Remove from the lightbeams
        for (auto it = lightbeams.begin(); it != lightbeams.end();) {
            if (it->second == id || it->first == id) { lightbeams.erase(it++); }
            else { ++it; }
        }
        //Remove from the alphalist
        for (auto &p:alphalist) {
            if (p->id == id) {
                alphalist.erase(p);
                break;
            }
        }
        //Remove from the brightness_list
        for (auto &p:brightness_list) {
            if (p->id == id) {
                brightness_list.erase(p);
                break;
            }
        }
        //Remove from the alphaids
        //Remove from the brightnessids
        alphaids.erase(remove(alphaids.begin(), alphaids.end(), id), alphaids.end());
        brightnessids.erase(remove(brightnessids.begin(), brightnessids.end(), id), brightnessids.end());
        //Remove from the out_lightbeams
        for (auto it = out_lightbeams.begin(); it != out_lightbeams.end();) {
            std::vector vec = it->second;
            std::vector<BeaconID>::iterator vec_it = std::find(vec.begin(), vec.end(), id);
            if (it->first == id) {
                out_lightbeams.erase(it++);
            } else if (vec_it != vec.end()) {
                vec.erase(vec_it);
                if (vec.empty()) {
                    out_lightbeams.erase(it++);
                } else {
                    it->second = vec;
                    ++it;
                }
            } else {
                ++it;
            }
        }
        return true;
    } else {
        return false;
    }


}

void Datastructures::dfs(BeaconID root, IDcollection list,
                         int len, int &maxLen, IDcollection &maxList) {
    std::vector<BeaconID> leaves{NO_ID};
    BeaconID leaf{};

    leaves = get_lightsources_no_sort(root);
    //Check if it is time to end
    if (root == NO_ID) {
        //Check the list  includes the most elements or not
        if (maxLen <= len) {
            maxLen = len;
            maxList = list;
        }
        return;
    } else {
        list.insert(list.begin(), root);
        for (auto data: leaves) {
            dfs(data, list, len + 1, maxLen, maxList);
        }
    }
}


std::vector<BeaconID> Datastructures::path_inbeam_longest(BeaconID id) {
    if (beaid_beacons.count(id) > 0) {
        std::vector<BeaconID> list{};
        int maxLen = 0;
        std::vector<BeaconID> maxList{};
        //Call DFS
        dfs(id, list, 0, maxLen, maxList);
        return maxList;
    } else {
        return {{NO_ID}};
    }

}


Color Datastructures::acculateColor(Color c, Colorcollection colorCollection) {
    int total_r{};
    int total_g{};
    int total_b{};
    for (Color each_color:colorCollection) {
        total_r += each_color.r;
        total_g += each_color.g;
        total_b += each_color.b;
    }
    total_r += c.r;
    total_g += c.g;
    total_b += c.b;

    int v_size = int(colorCollection.size()) + 1;
    //Sum up the colors and calculate  the total average color
    return Color{(total_r / v_size), (total_g / v_size), (total_b / v_size)};
}

void Datastructures::averageColor(BeaconID targetId, BeaconID leaf, Color &color, IDcollection &leaf_vec) {
    IDcollection vec{};
    BeaconID root{};

    try {
        root = lightbeams.at(leaf);
        vec=get_lightsources_no_sort(root);
        vec.erase(remove(vec.begin(), vec.end(), leaf), vec.end());
    }catch (const std::out_of_range& e) {
        (void)e;
        return;
    }

    if (leaf == targetId) {
        //Check if it is time to end
        return;
    } else {
        Colorcollection colorCollection{};
        if (!vec.empty()) {
            for (auto child:vec) {
                auto it = std::find(leaf_vec.begin(), leaf_vec.end(), child);
                //Collect the colors of the sub beacons
                if (it == leaf_vec.end()) {
                    BeaconID final_leaf = path_inbeam_longest(child).front();
                    Color color1 = get_color(final_leaf);
                    averageColor(child, final_leaf, color1, leaf_vec);
                    colorCollection.push_back(color1);
                } else {
                    colorCollection.push_back(get_color(child));
                }
            }

        }
        colorCollection.push_back(get_color(root));
        color = acculateColor(color, colorCollection);
        averageColor(targetId, root, color, leaf_vec);
    }

}

void Datastructures::findLeafNodes(BeaconID root, IDcollection &list) {
    // if node is null, return
    if (root == NO_ID)
        return;

    // if node is leaf node, collect its data
    IDcollection no_vec = {{NO_ID}};
    if (get_lightsources_no_sort(root) == no_vec) {
        list.push_back(root);
        return;
    }

    for (auto data:get_lightsources_no_sort(root))
        findLeafNodes(data, list);
}


Color Datastructures::total_color(BeaconID id) {
    if (beaid_beacons.count(id) > 0) {
        IDcollection leaf_vec{};
        findLeafNodes(id, leaf_vec);
        BeaconID final_leaf = path_inbeam_longest(id).front();
        Color color =get_color(final_leaf);
        averageColor(id, final_leaf, color, leaf_vec);
        return color;

    } else {
        return NO_COLOR;
    }
}
