#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <cstdint>
#include <cstring>

#include <ArduinoJson.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#include "SharedResource.h"

/**
 * @brief WFH Terminalの設定データのInit/Read/Modify/Save/Loadを行うクラスです
 * @note TaskBaseを継承したクラスで操作する場合はSharedResouceクラスでラップして処理すること、また配置にはCPU DataCacheを考慮すること
 * 
 * @tparam N 管理ファイルに使用する領域、実際にはこの倍の領域が使用されます
 */
template<int N>
class GlobalConfig {
    public:
        /*
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
         * @param isMigrate 存在しない値のみを上書きする場合はtrue
         * 
         * @note 完全新規のconfigを生成する場合はisMigrate=falseで実行する
         */
        void init(bool isMigrate) {
            // 更新条件を満たすときだけconfigVolatileを書き換える
            auto f = [&](const char* key, auto value){
                if (!isMigrate || !this->configVolatile.containsKey(key)) { 
                    this->configVolatile[key] = value;
                }
            };
            // 一通り必要な初期値をセット
            f("identifier", "WFH Monitor");
            f("date"      , __DATE__);
            f("time"      , __TIME__);

            // migrationでなければNonVolatile側にも反映
            if (!isMigrate) {
                this->configNonVolatile = this->configVolatile;
            }
        }

        /**
         * @brief configの参照を取得します
         * @note この参照を不用意に引き回してはいけません。SharedResouceを使い排他制御している範囲でLifetimeを持つように制御してください
         * 
         * @return const GlobalConfigDef&  configVolatileの参照
         */
        const StaticJsonDocument<N>& getRo(void) {
            return configVolatile;
        }

        /**
         * @brief configの書き換え可能な参照を取得します
         * @note この参照を不用意に引き回してはいけません。SharedResouceを使い排他制御している範囲でLifetimeを持つように制御してください
         * 
         * @return GlobalConfigDef& configVolatileの参照
         */
        StaticJsonDocument<N>& getRw(void) {
            return configVolatile;
        }

        /**
         * @brief init後の値、もしくはload/saveした後の値に巻き戻します
         */
        void clear(void) {
            this->configVolatile = this->configNonVolatile;
        }

        /**
         * @brief configの内容をSD Cardから読み出します
         * 
         * @param filePath 読み込み先、省略した場合はconstructorで指定したパスに書き込みます
         * @return true 読み出し成功
         * @return false 読み出し失敗
         */
        bool load(const char* filePath, DeserializationError& deserializeError) {
            const char* path = (filePath == nullptr) ? this->baseFilePath : filePath;
            // baseFilePath == nullptrの対策
            if (path == nullptr) {
                deserializeError = DeserializationError::InvalidInput;
                return false;
            }

            // take mutex & crititcal sectionで読み込みは行う
            bool result = false;
            this->sharedSd.operateCritial([&](SDFS& sd) {
                File f = sd.open(path, FILE_READ);
                // Fileが開けなければ失敗
                if (!f) {
                    deserializeError = DeserializationError::InvalidInput;
                    return;
                }
                // configNonVolatileに読み出す
                deserializeError = deserializeJson(this->configNonVolatile, f);
                // file Handleはもう不要
                f.close();
                // DeserializeErrorが発生していれば終了
                if (deserializeError != DeserializationError::Ok) return;

                // 成功していればconfigVolatileの内容を上書き
                this->configVolatile = this->configNonVolatile;
                // versionが異なる場合のMigrationも実施しておく
                this->init(true);
                // 成功
                result = true;
            });
            return result;
        }

        /**
         * @brief 現在のconfigの内容をSD Cardに不揮発化します
         * 
         * @param filePath 書き込み先、省略した場合はconstructorで指定したパスに書き込みます
         * @return true 保存成功
         * @return false 保存失敗
         */
        bool save(const char* filePath) {
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
                // configVolatileの内容を不揮発化する
                const size_t byteWritten = serializeJson(this->configVolatile, f);
                // File Handleはもう不要
                f.close();
                // 1byteも書けていなければ失敗
                if (byteWritten == 0) return;

                // 成功していればconfigNonVolatileの内容を上書き
                this->configNonVolatile = this->configVolatile;
                // 成功
                result = true;
            });

            return result;
        }

    protected:
        const char* baseFilePath;
        SharedResource<SDFS>& sharedSd; /**< Semaphore, CriticalSectionの制定可能なSD Peripheral */
        StaticJsonDocument<N> configVolatile; /**< 動作中に書き換わる領域 */
        StaticJsonDocument<N> configNonVolatile; /**< Load時、またSave後に不揮発化されているオリジナルデータを格納する */
};
#endif /* GLOBAL_CONFIG_H */