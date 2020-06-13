#include "../SysTimer.h"
#include "WifiTask.h"

void WifiTask::setup(void) {
    this->resource.config.operate([&](GlobalConfig<FixedConfig::ConfigAllocateSize>& config){
        auto fps = GlobalConfigDefaultValues::WifiTaskFps;
        config.read(GlobalConfigKeys::WifiTaskFps, fps);
        this->setFps(fps);
        // Wifi/Ambient使用有無
        this->isUseWifi = GlobalConfigDefaultValues::UseWiFi;
        this->isUseAmbient = GlobalConfigDefaultValues::UseAmbient;
        config.read(GlobalConfigKeys::UseWiFi, this->isUseWifi);
        config.read(GlobalConfigKeys::UseAmbient, this->isUseAmbient);
        // ambient送信に必要な情報も読み込んでおく
        if (this->isUseWifi && this->isUseAmbient) {
            auto channelId = GlobalConfigDefaultValues::AmbientChannelId;
            config.read(GlobalConfigKeys::AmbientChannelId, channelId);
            auto writeKey = config.getReadPtr<char>(GlobalConfigKeys::AmbientWriteKey);

            this->ambient.begin(channelId, writeKey, &this->client);
        }
    });
}

bool WifiTask::invokeNop(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // do nothing
    return true;
}

bool WifiTask::invokeGetWifiStatus(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // set timestamp
    resp.data.wifiStatus.timestamp = SysTimer::getTickCount();

    // WiFiを使っていない場合
    if (!this->isUseWifi) {
        // set status & invalid ip
        resp.data.wifiStatus.status = WL_DISCONNECTED;
        resp.data.wifiStatus.ipAddr[0] = 0;
        resp.data.wifiStatus.ipAddr[1] = 0;
        resp.data.wifiStatus.ipAddr[2] = 0;
        resp.data.wifiStatus.ipAddr[3] = 0;
        return false;
    }
    
    // set status
    resp.data.wifiStatus.status = wifi.status();
    // set ip addr
    const auto ip = this->wifi.localIP();
    resp.data.wifiStatus.ipAddr[0] = ip[0];
    resp.data.wifiStatus.ipAddr[1] = ip[1];
    resp.data.wifiStatus.ipAddr[2] = ip[2];
    resp.data.wifiStatus.ipAddr[3] = ip[3];
    return true;
}

bool WifiTask::invokeSend(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // Wifi及びAmbient無効の場合は何もしない
    if (!this->isUseWifi || !this->isUseAmbient) {
        return false;
    }
    // Wifiが接続ステータスになっていなければ失敗
    if (this->wifi.status() != WL_CONNECTED) {
        return false;
    }

    // データを準備(1~8)
    ambient.set(1, String(req.data.measureData.visibleLux).c_str());
    ambient.set(2, String(req.data.measureData.tempature).c_str());
    ambient.set(3, String(req.data.measureData.humidity).c_str());
    ambient.set(4, String(req.data.measureData.pressure).c_str());
    ambient.set(5, String(req.data.measureData.gas).c_str());
    // 送信
    const bool result = ambient.send(); // clear()も内部的にされている

    return result;
}

bool WifiTask::loop(void) {
    WifiTaskRequest req;
    WifiTaskResponse resp;
    // 応答Queueに空きができるまでは処理しても仕方ないので待つ
    if (this->sendQueue.emptyNum() == 0) {
        return false; // no abort
    }

    // 要求を受信(受信できるまでTask Suspendさせる)
    this->recvQueue.receive(&req, true); 
    // いい感じに処理
    resp.id = req.id;
    switch (req.id) {
        case WifiTaskRequestId::Nop:
            resp.isSuccess = this->invokeNop(req, resp);
            break;
        case WifiTaskRequestId::GetWifiStatus:
            resp.isSuccess = this->invokeGetWifiStatus(req, resp);
            break;
        case WifiTaskRequestId::SendSensorData:
            resp.isSuccess = this->invokeSend(req, resp);
            break;
        default:
            resp.isSuccess = false;
            break;
    }
    // 応答
    this->sendQueue.send(&resp);

    return false; // no abort
}
