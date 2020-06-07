#ifndef SHAREDRESOURCEDEFS_H
#define SHAREDRESOURCEDEFS_H

#include <cstdint>

#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#include "GlobalConfig.h"
#include "FixedConfig.h"
#include "SharedResource.h"

/**
 * @brief Project上固有のリソースで、複数のTaskから操作されるものを定義します
 * @note インスタンスは必ずSharedResourceでラップしたものを定義してください
 */
struct SharedResourceDefs {
    SharedResource<Serial_>& serial;
    SharedResource<SDFS>& sd;
    SharedResource<GlobalConfig<FixedConfig::ConfigAllocateSize>>& config;
};

#endif /* SHAREDRESOURCEDEFS_H */