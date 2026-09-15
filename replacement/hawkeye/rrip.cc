#include "rrip.h"

constexpr int maxRRPV = 7;

void update_rrpv(std::vector<int>& rrpv, std::size_t way, Classification cls, bool is_hit)
{
    if(cls == Classification::CACHE_AVERSE)
    {
        rrpv[way] = maxRRPV;
    }
    else
    {
        rrpv[way] = 0;
        if(!is_hit)
        {
            for(std::size_t i = 0; i < rrpv.size(); i++)
            {
                if(i != way && rrpv[i] < maxRRPV-1)
                {
                    rrpv[i]++;
                }
            }
        }
    }
}

std::size_t find_victim(std::vector<int>& rrpv) {
    while (true) {
        for (std::size_t i = 0; i < rrpv.size(); i++) 
        {
            if (rrpv[i] == maxRRPV) 
            {
                return i;
            }
        }
        
        for (std::size_t i = 0; i < rrpv.size(); i++) 
        {
            if (rrpv[i] < maxRRPV) 
            {
                rrpv[i] += 1;
            }
        }
    }
}