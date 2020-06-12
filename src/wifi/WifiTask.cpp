#include "WifiTask.h"

void WifiTask::setup(void) {
    this->resource.config.operate([&](GlobalConfig<FixedConfig::ConfigAllocateSize>& config){
        auto fps = GlobalConfigDefaultValues::WifiTaskFps;
        config.read(GlobalConfigKeys::WifiTaskFps, fps);
        this->setFps(fps);
        // TODO: ambient送信に必要な情報も読み込んでおく
    });
}

bool WifiTask::invokeNop(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // do nothing
    return true;
}

bool WifiTask::invokeGetWifiStatus(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // set status
    resp.data.wifiStatus.status = wifi.status();
    // set ip addr
    const auto ip = this->wifi.localIP();
    resp.data.wifiStatus.ipAddr[0] = ip[0];
    resp.data.wifiStatus.ipAddr[1] = ip[1];
    resp.data.wifiStatus.ipAddr[2] = ip[2];
    resp.data.wifiStatus.ipAddr[3] = ip[3];
    // always success
    return true;
}

bool WifiTask::invokeSend(const WifiTaskRequest& req, WifiTaskResponse& resp) {
    // TODO: #27 Ambient送信を行う
    return false;
}

bool WifiTask::loop(void) {
    WifiTaskRequest req;
    WifiTaskResponse resp;
    // 応答Queueに空きができるまでは処理しても仕方ないので待つ
    if (this->sendQueue.emptyNum() == 0) {
        return false; // no abort
    }
    // 受信できるまでは永久に待機
    const bool isSuccess = this->recvQueue.receive(&req, true); 
    if (!isSuccess) { 
        return false; // no abort
    }
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
