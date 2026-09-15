#include "predictor.h"

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries, int counter_bits)
    : num_entries_(num_entries),
      counter_bits_(counter_bits),
      max_counter_value_((1 << counter_bits) - 1),
      table_(num_entries, 1 << (counter_bits - 1))   
{
}

std::size_t HawkeyePredictor::hash(uint64_t pc) const
{
    uint64_t hashed = (pc ^ (pc >> 12));
    return static_cast<std::size_t>(hashed) & (num_entries_ - 1);
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit)
{
    std::size_t index = hash(pc);
    int& counter = table_[index];

    if (opt_hit) 
    {
        if (counter < max_counter_value_) 
        {
            counter++;
        }
    } 
    else 
    {
        if(counter>0) counter--;
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const
{
    std::size_t index = hash(pc);
    int thresh = (max_counter_value_ + 1) / 2;
    return table_[index] >= thresh;
}

int HawkeyePredictor::get_counter(uint64_t pc) const
{
    std::size_t index = hash(pc);
    return table_[index];
}