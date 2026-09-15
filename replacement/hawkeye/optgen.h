#ifndef OPTGEN_H
#define OPTGEN_H

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
class OPTgen 
{ 
    private:
       struct SetState
       {
            std::vector<int> occupancy_vector; 
            std::unordered_map<uint64_t, std::size_t> last_pos; // maps addresses to their last access index
            std::size_t base_ind=0; 
       };
       
       std::size_t associativity_;
       std::size_t max_history_;
       std::vector<SetState> sets_;

    public:

// num_sets: number of cache sets tracked independently 
// associativity: W, the cache associativity (occupancy vector cap) 
// history_multiplier: length of tracked history, in units of the set's // capacity (paper uses 8x; see Figure 2). Default 8.

    OPTgen(std::size_t num_sets, std::size_t associativity, std::size_t history_multiplier = 8);

// Processes one access to `address`, mapped to set `set_idx`, per Section 3.1.

    bool access(std::size_t set_idx, uint64_t address);    
    
    bool has_previous(std::size_t set_idx, uint64_t address) const;
};

#endif