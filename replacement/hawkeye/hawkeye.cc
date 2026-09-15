#include "hawkeye.h" 
#include <algorithm>

hawkeye::hawkeye(CACHE* cache)
    : replacement(cache),
      optgen_(cache->NUM_SET, cache->NUM_WAY),   // num_sets, associativity (history_multiplier defaults to 8)
      predictor_(),                               // defaults: 8192 entries, 3-bit counters
      NUM_SET(cache->NUM_SET),
      NUM_WAY(cache->NUM_WAY),
      rrpv_(cache->NUM_SET, std::vector<int>(cache->NUM_WAY, 0)),  //default rrpv value will be overwritten by replacement_cache_fill() when a new line is filled
      line_pc_(cache->NUM_SET * cache->NUM_WAY, 0),
      line_cls_(cache->NUM_SET * cache->NUM_WAY, Classification::CACHE_AVERSE)
{
}
//fucntion below converts set and way to a single index for line_pc_ and line_cls_ vectors
std::size_t hawkeye::idx(long set, long way) const 
{
    return static_cast<std::size_t>(set) * static_cast<std::size_t>(NUM_WAY)+ static_cast<std::size_t>(way);
}

long hawkeye::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                           const champsim::cache_block* current_set, champsim::address ip,
                           champsim::address full_addr, access_type type)
{
    std::size_t victim_way = ::find_victim(rrpv_[set]);
    std::size_t victim_idx = idx(set, static_cast<long>(victim_way));
    if (line_cls_[victim_idx] == Classification::CACHE_FRIENDLY) 
    {
        predictor_.train(line_pc_[victim_idx], /*opt_hit=*/false);
    }

    return static_cast<long>(victim_way);
}

void hawkeye::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                      champsim::address full_addr, champsim::address ip,
                                      champsim::address victim_addr, access_type type)
{

    uint64_t pc = ip.to<uint64_t>();
    bool friendly = predictor_.predict(pc);
    Classification cls = friendly ? Classification::CACHE_FRIENDLY
                                   : Classification::CACHE_AVERSE;
    std::size_t i = idx(set, way);
    line_pc_[i]  = pc;
    line_cls_[i] = cls;
    update_rrpv(rrpv_[set], static_cast<std::size_t>(way), cls, /*is_hit=*/false);

}

void hawkeye::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                        champsim::address full_addr, champsim::address ip,
                                        champsim::address victim_addr, access_type type,
                                        uint8_t hit)
{
    uint64_t addr = full_addr.to<uint64_t>() >> 6;
    uint64_t pc   = ip.to<uint64_t>();
    
    //check existence of this addr in OPTgen's history
    bool has_previous = optgen_.has_previous(
        static_cast<std::size_t>(set), addr
    );
    
    // OPTgen processes every access.
    bool opt_hit = optgen_.access(
        static_cast<std::size_t>(set), addr
    );
    
    //we train the predicotr only if this address has been seen before in OPTGen's hisotry
    if (has_previous) {
        auto it = last_pc_.find(addr);
    
        if (it != last_pc_.end()) {
            predictor_.train(it->second, opt_hit);
        }
    }
    
    last_pc_[addr] = pc;

    if (hit)
    {
        bool friendly = predictor_.predict(pc);
        Classification cls = friendly ? Classification::CACHE_FRIENDLY
                                       : Classification::CACHE_AVERSE;
        std::size_t i = idx(set, way);
        line_pc_[i]  = pc;
        line_cls_[i] = cls;

        update_rrpv(rrpv_[set], static_cast<std::size_t>(way), cls, /*is_hit=*/true);
    }
}