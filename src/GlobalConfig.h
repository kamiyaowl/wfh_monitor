#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <cstdint>
#include <cstring>

#include <ArduinoJson.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#include "SharedResource.h"

/**
 * @brief GlobalConfigで使用するJson Key一覧です
 */
namespace GlobalConfigKeys {
    static constexpr char* Identifier             = "identifier";
    static constexpr char* Date                   = "date";
    static constexpr char* Time                   = "time";
    static constexpr char* UseWiFi                = "useWiFi";
    static constexpr char* ApSsid                 = "apSsid";
    static constexpr char* ApPassWord             = "apPassword";
    static constexpr char* ApTimeoutMs            = "apTimeoutMs";
    static constexpr char* UseAmbient             = "useAmbient";
    static constexpr char* AmbientIntervalMs      = "AmbientIntervalMs";
    static constexpr char* AmbientChannelId       = "ambientChanelId";
    static constexpr char* AmbientWriteKey        = "ambientWriteKey";
    static constexpr char* GroveTaskFps           = "groveTaskFps";
    static constexpr char* ButtonTaskFps          = "buttonTaskFps";
    static constexpr char* UiTaskFps              = "uiTaskFps";
    static constexpr char* GroveTaskPrintSerial   = "groveTaskPrintSerial";
    static constexpr char* GroveTaskPrintFile     = "groveTaskPrintFile";
    static constexpr char* BrightnessHoldMs       = "brightnessHoldMs";
    static constexpr char* BrightnessTransitionMs = "brightnessTransitionMs";
}

/**
 * @brief GlobalConfigで使用する初期値一覧です
 */
namespace GlobalConfigDefaultValues {
    static constexpr char*    Identifier             = "WFH Monitor";
    static constexpr char*    Date                   = __DATE__;
    static constexpr char*    Time                   = __TIME__;
    static constexpr bool     UseWiFi                = false;
    static constexpr char*    ApSsid                 = "your ap ssid";
    static constexpr char*    ApPassWord             = "your ap password";
    static constexpr uint32_t ApTimeoutMs            = 30000;
    static constexpr bool     UseAmbient             = true;
    static constexpr uint32_t AmbientIntervalMs      = 60000;
    static constexpr uint32_t AmbientChannelId       = 0;
    static constexpr char*    AmbientWriteKey        = "your writekey";
    static constexpr uint32_t GroveTaskFps           = 2;
    static constexpr uint32_t ButtonTaskFps          = 60;
    static constexpr uint32_t UiTaskFps              = 30;
    static constexpr bool     GroveTaskPrintSerial   = false;
    static constexpr bool     GroveTaskPrintFile     = false;
    static constexpr uint32_t BrightnessHoldMs       = 4000;
    static constexpr uint32_t BrightnessTransitionMs = 2000;
}

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
         * @brief 指定された値をconfigVolatileに書き込みます
         * 
         * @tparam T 書き込むデータの型
         * @param isOverwrite すでに値がセットされている場合でも上書きする場合はtrue
         * @param key セット対象のKey
         * @param value セットする値
         */
        template<typename T>
        void write(bool isOverwrite, const char* key, T& value) {
            if (isOverwrite || !this->configVolatile.containsKey(key)) { 
                this->configVolatile[key] = value;
            }
        }

        /**
         * @brief 指定されたKeyの値を読み出します
         * 
         * @tparam T 読み出す型
         * @param key 読み出し対象のKey
         * @param value 読みだしたデータの書き込み先、Keyが存在しない場合は操作しません
         * @return true 読み出し成功
         * @return false 読み出し失敗
         */
        template<typename T>
        bool read(const char* key, T& value) {
            // Keyが存在しない
            if (!this->configVolatile.containsKey(key)) {
                return false;
            }
            // 値を読み出す
            value = this->configVolatile[key];
            return true;
        }

        /**
         * @brief 指定されたKeyのポインタを取得します。文字列を読み出す場合などに利用してください
         * @note 読みだしたPointerのLifetimeは必要最低限にとどめてください。読みだした時点のスコープ以上に広げないことを推奨します
         * 
         * @tparam T 読み出す型
         * @param key 読み出し対象のKey
         * @return const T* 読み出し失敗
         */
        template<typename T>
        const T* getReadPtr(const char* key) {
            // Keyが存在しない
            if (!this->configVolatile.containsKey(key)) {
                return nullptr;
            }
            
            return this->configVolatile[key];
        }

        /**
         * @brief すべての値を初期値で上書きします
         * @param isMigrate 存在しない値のみを上書きする場合はtrue
         * 
         * @note 完全新規のconfigを生成する場合はisMigrate=falseで実行する
         */
        void init(bool isMigrate) {
            // 一通り必要な初期値をセット
            // TODO: もう少しいい感じに書けるかも。Keys/DefaultValuesを内包する型パラメータの指定は考察が必要
            this->write(!isMigrate, GlobalConfigKeys::Identifier              , GlobalConfigDefaultValues::Identifier);
            this->write(!isMigrate, GlobalConfigKeys::Date                    , GlobalConfigDefaultValues::Date);
            this->write(!isMigrate, GlobalConfigKeys::Time                    , GlobalConfigDefaultValues::Time);
            this->write(!isMigrate, GlobalConfigKeys::UseWiFi                 , GlobalConfigDefaultValues::UseWiFi);
            this->write(!isMigrate, GlobalConfigKeys::ApSsid                  , GlobalConfigDefaultValues::ApSsid);
            this->write(!isMigrate, GlobalConfigKeys::ApPassWord              , GlobalConfigDefaultValues::ApPassWord);
            this->write(!isMigrate, GlobalConfigKeys::ApTimeoutMs             , GlobalConfigDefaultValues::ApTimeoutMs);
            this->write(!isMigrate, GlobalConfigKeys::UseAmbient              , GlobalConfigDefaultValues::UseAmbient);
            this->write(!isMigrate, GlobalConfigKeys::AmbientIntervalMs       , GlobalConfigDefaultValues::AmbientIntervalMs);
            this->write(!isMigrate, GlobalConfigKeys::AmbientChannelId        , GlobalConfigDefaultValues::AmbientChannelId);
            this->write(!isMigrate, GlobalConfigKeys::AmbientWriteKey         , GlobalConfigDefaultValues::AmbientWriteKey);
            this->write(!isMigrate, GlobalConfigKeys::GroveTaskFps            , GlobalConfigDefaultValues::GroveTaskFps);
            this->write(!isMigrate, GlobalConfigKeys::ButtonTaskFps           , GlobalConfigDefaultValues::ButtonTaskFps);
            this->write(!isMigrate, GlobalConfigKeys::UiTaskFps               , GlobalConfigDefaultValues::UiTaskFps);
            this->write(!isMigrate, GlobalConfigKeys::GroveTaskPrintSerial    , GlobalConfigDefaultValues::GroveTaskPrintSerial);
            this->write(!isMigrate, GlobalConfigKeys::GroveTaskPrintFile      , GlobalConfigDefaultValues::GroveTaskPrintFile);
            this->write(!isMigrate, GlobalConfigKeys::BrightnessHoldMs        , GlobalConfigDefaultValues::BrightnessHoldMs);
            this->write(!isMigrate, GlobalConfigKeys::BrightnessTransitionMs  , GlobalConfigDefaultValues::BrightnessTransitionMs);

            // migrationでなければNonVolatile側にも反映(this->clear()対策)
            if (!isMigrate) {
                this->configNonVolatile = this->configVolatile;
            }
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
                // Date/Timeを最新ビルドのものに更新する
                this->write(true, GlobalConfigKeys::Date , GlobalConfigDefaultValues::Date);
                this->write(true, GlobalConfigKeys::Time , GlobalConfigDefaultValues::Time);
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