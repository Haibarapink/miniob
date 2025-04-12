#pragma once

#include "common/mm/mem_pool.h"

namespace oceanbase {

class Block {
    friend class BlockHandler;
public:
    char buffer[0];
};

class BlockManager {
public:

private:
    
};
}