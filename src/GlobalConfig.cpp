#include <cstring>
#include "GlobalConfig.h"

void GlobalConfig::init(void) {
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.identifier), "WFH Monitor");
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.date), __DATE__);
    std::strcpy(reinterpret_cast<char*>(this->configVolatile.time), __TIME__);
}

const GlobalConfigDef& GlobalConfig::getRo(void) {
    return configVolatile;
}

GlobalConfigDef& GlobalConfig::getRw(void) {
    return configVolatile;
}

bool GlobalConfig::save(const char* filePath) {
    const char* path = (filePath == nullptr) ? this->baseFilePath : filePath;
    // baseFilePath == nullptrの対策
    if (path == nullptr) {
        return false;
    }

    // take mutex & crititcal sectionで書き込みは行う
    bool result = false;
    this->sharedSd.operateCritial([&](SDFS& sd) {
        File f = sd.open(path, FILE_WRITE);
        // Fileが開けなければ失敗
        if (!f) return;
        
        // TODO: configVolatileの内容を不揮発化する
        f.println("TODO: generate configure json file!");

        // 成功していればconfigNonVolatileの内容を上書き
        this->configNonVolatile = this->configVolatile;
        // 成功
        result = true;
    });

    return result;
}
