#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <cstdint>

#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#include "SharedResource.h"

/**
 * @brief GlobalConfigで管理するデータ型を示します
 */
struct GlobalConfigDef {
    uint8_t identifier[12]; /**< 識別子を埋め込みます "WFH Monitor\0" */
    uint8_t date[12]; /**< __DATE__を埋め込みます "Mmm dd yyyy\0"*/
    uint8_t time[12]; /**< __TIME__を埋め込みます "hh:mm:ss\0"*/
    uint32_t checksum; /**< 検査用のchecksum, 不揮発化する際のみ使用する */
};

/**
 * @brief WFH Terminalの設定データのInit/Read/Modify/Save/Loadを行うクラスです
 * @note TaskBaseを継承したクラスで操作する場合はSharedResouceクラスでラップして処理すること、また配置にはCPU DataCacheを考慮すること
 */
class GlobalConfig {
    public:
        GlobalConfig(SharedResource<SDFS>& sharedSd): sharedSd(sharedSd) {}
        virtual ~GlobalConfig(void) {}

        /**
         * @brief すべての値を初期値で上書きします
         */
        void init(void);

        /**
         * @brief configの参照を取得します
         * @note この参照を不用意に引き回してはいけません。SharedResouceを使い排他制御している範囲でLifetimeを持つように制御してください
         * 
         * @return const GlobalConfigDef&  configVolatileの参照
         */
        const GlobalConfigDef& getRo(void);

        /**
         * @brief configの書き換え可能な参照を取得します
         * @note この参照を不用意に引き回してはいけません。SharedResouceを使い排他制御している範囲でLifetimeを持つように制御してください
         * 
         * @return GlobalConfigDef& configVolatileの参照
         */
        GlobalConfigDef& getRw(void);
        // bool load(void); // TODO: FatFsクラスを渡せるようにする?
        // bool save(void); // TODO: FatFsクラスを渡せるようにする?

    protected:
        SharedResource<SDFS>& sharedSd; /**< Semaphore, CriticalSectionの制定可能なSD Peripheral */
        GlobalConfigDef configVolatile; /**< 動作中に書き換わる領域 */
        GlobalConfigDef configNonVolatile; /**< Load時、またSave後に不揮発化されているオリジナルデータを格納する */
};
#endif /* GLOBAL_CONFIG_H */