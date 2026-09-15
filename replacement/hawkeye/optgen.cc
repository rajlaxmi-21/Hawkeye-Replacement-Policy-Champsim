#include "optgen.h"


OPTgen::OPTgen(std::size_t num_sets, std::size_t associativity, std::size_t history_multiplier) 
               : associativity_(associativity), max_history_(history_multiplier * associativity), sets_(num_sets)
{ }

bool OPTgen::access(std::size_t set_idx, uint64_t address)
{
    SetState& s = sets_[set_idx];
    
    std::size_t cur_ind = s.base_ind + s.occupancy_vector.size();
    s.occupancy_vector.push_back(0);

    if( s.occupancy_vector.size() > max_history_ )
    {
        s.occupancy_vector.erase(s.occupancy_vector.begin());
        s.base_ind++;
    }

    auto it = s.last_pos.find(address);
    bool is_first_time = (it == s.last_pos.end()) || (it->second < s.base_ind);
    
    bool hit = false;
    if(!is_first_time)
    {
        std::size_t start = it->second - s.base_ind;
        std::size_t end = cur_ind - s.base_ind;

        bool can_fit = true;
        for(std::size_t i = start; i < end; i++)
        {
            if(s.occupancy_vector[i] >= associativity_)
            {
                can_fit = false;
                break;
            }
        }

        if(can_fit)
        {
            hit = true;
            for(std::size_t i = start; i < end; i++)
            {
                s.occupancy_vector[i]++;
            }
        }
    }
    s.last_pos[address] = cur_ind;
    return hit;    
}

bool OPTgen::has_previous(std::size_t set_idx,
                           uint64_t address) const
{
    const SetState& s = sets_[set_idx];

    auto it = s.last_pos.find(address);

    return it != s.last_pos.end() &&
           it->second >= s.base_ind;
}