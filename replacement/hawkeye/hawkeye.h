// hawkeye.h
#ifndef HAWKEYE_H 
#define HAWKEYE_H 
#include <vector> 
#include "modules.h" 
#include "optgen.h" 
#include "predictor.h"
#include "rrip.h"
#include <unordered_map>
#include "cache.h"

struct hawkeye : public champsim::modules::replacement 
{ 
    
    // TODO: instantiate different modules
    OPTgen optgen_;
    HawkeyePredictor predictor_;
    // TODO: Add any new data structures or functions to connect each of the modules
    long NUM_SET;
    long NUM_WAY;

    std::vector<std::vector<int>> rrpv_;                
    std::vector<uint64_t> line_pc_;         
    std::vector<Classification> line_cls_;
    std::unordered_map<uint64_t, uint64_t> last_pc_;
    // TODO: Complete the definitions for the following functions that are required across all replacement policies. You can use the other replacement policies as a reference. Each should be implemented primarily by calling optgen.access(...), predictor.train(...)/predict(...), and update_rrpv(...)/find_victim(...) from rrip.h, do not re-implement OPTgen/predictor/RRIP logic here.
    explicit hawkeye(CACHE* cache);
    std::size_t idx(long set, long way) const;
    // find_victim (args);
    long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                      const champsim::cache_block* current_set, champsim::address ip,
                      champsim::address full_addr, access_type type);
    // replacement_cache_fill (args);
    void replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 champsim::address full_addr, champsim::address ip,
                                 champsim::address victim_addr, access_type type);
    // update_replacement_state (args); 
    void update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                   champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type,
                                   uint8_t hit);

}; 

#endif