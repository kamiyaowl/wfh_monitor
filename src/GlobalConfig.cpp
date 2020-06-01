#include <cstring>
#include "GlobalConfig.h"

void GlobalConfig::init(void) {
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.identifier), "WFH Monitor");
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.date), __DATE__);
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.time), __TIME__);
    this->configVolatile.checksum = 0; /**< init時点ではchecksumは反映しない */
}

const GlobalConfigDef& GlobalConfig::getRo(void) {
    return configVolatile;
}

GlobalConfigDef& GlobalConfig::getRw(void) {
    return configVolatile;
}
