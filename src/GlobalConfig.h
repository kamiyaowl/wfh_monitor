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
};

/**
 * @brief WFH Terminalの設定データのInit/Read/Modify/Save/Loadを行うクラスです
 * @note TaskBaseを継承したクラスで操作する場合はSharedResouceクラスでラップして処理すること、また配置にはCPU DataCacheを考慮すること
 */
class GlobalConfig {
    public:
    /**
     * @brief Construct a new Global Config object
     * 
     * @param sharedSd SDカードのペリフェラル
     * @param configPath Globalな設定の保存先として使うFilePath
     */
        GlobalConfig(SharedResource<SDFS>& sharedSd, const char* configPath): sharedSd(sharedSd), baseFilePath(configPath) {}

        /**
         * @brief Destroy the Global Config object
         */
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

        /**
         * @brief init後の値、もしくはload/saveした後の値に巻き戻します
         */
        void clear(void);
        /**
         * @brief configの内容をSD Cardから読み出します
         * 
         * @param filePath 読み込み先、省略した場合はconstructorで指定したパスに書き込みます
         * @return true 読み出し成功
         * @return false 読み出し失敗
         */
        bool load(const char* filePath);

        /**
         * @brief 現在のconfigの内容をSD Cardに不揮発化します
         * 
         * @param filePath 書き込み先、省略した場合はconstructorで指定したパスに書き込みます
         * @return true 保存成功
         * @return false 保存失敗
         */
        bool save(const char* filePath);

    protected:
        const char* baseFilePath;
        SharedResource<SDFS>& sharedSd; /**< Semaphore, CriticalSectionの制定可能なSD Peripheral */
        GlobalConfigDef configVolatile; /**< 動作中に書き換わる領域 */
        GlobalConfigDef configNonVolatile; /**< Load時、またSave後に不揮発化されているオリジナルデータを格納する */
};
#endif /* GLOBAL_CONFIG_H */