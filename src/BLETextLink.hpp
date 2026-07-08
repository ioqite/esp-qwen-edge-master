/*
 * BLETextLink.hpp - ESP32-S3 BLE 5.0 持久连接双向文本链路 (单文件库)
 *
 * 适用于: Arduino core for ESP32 v3.3.4 (基于 ESP-IDF 5.x, Bluedroid 栈)
 *
 * ============================================================================
 *  特性
 * ============================================================================
 *   1. 持久连接   - 开机自动连接配置的对端 MAC, 连接保持不主动断开
 *   2. 自动重连   - 断开后由后台任务自动重连 (默认 3 秒间隔)
 *   3. 双向文本   - 单条 GATT 连接即支持双向:
 *                   Master 写入 Slave 的 CHAR_RX  ->  Slave 收
 *                   Slave 通过 CHAR_TX 通知        ->  Master 收
 *   4. 三类回调   - onReceive / onConnect / onDisconnect, 在用户 loop() 上下文派发
 *                   (BLE 回调任务仅做缓冲, 用户可在回调中执行耗时操作)
 *   5. 角色自动   - AUTO 模式按 MAC 字典序决定主从, 两端代码可完全一致
 *   6. BLE 5.0    - 默认 MTU=512, 单包有效载荷 509 字节; 2M PHY 自动协商
 *
 * ============================================================================
 *  使用步骤
 * ============================================================================
 *   1. 两块 ESP32-S3 烧录同一份示例 (本机 MAC 互填为对端)
 *   2. link.begin(peerMac);  link.onReceive(...); link.onConnect(...);
 *   3. loop() 中调用 link.loop()  -- 必须!
 *   4. link.send("hello") 发送;  onReceive 回调接收
 *
 * ============================================================================
 *  协议 (双端 UUID 必须一致)
 * ============================================================================
 *   Service        6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 *   CHAR_RX (写)   6E400002-B5A3-F393-E0A9-E50E24DCCA9E  远端写入 -> 本机接收
 *   CHAR_TX (通知) 6E400003-B5A3-F393-E0A9-E50E24DCCA9E  本机通知 -> 远端接收
 */

#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#if defined(CONFIG_BLUEDROID_ENABLED)
#include <esp_gatts_api.h>
#endif
#include <functional>
#include <string>

class BLETextLink {
public:
    using MsgCallback   = std::function<void(const String&)>;
    using EventCallback = std::function<void()>;

    enum Role : uint8_t { SLAVE = 0, MASTER = 1, AUTO = 2 };

    // ---------------- 公共 UUID (可修改, 双端须一致) ----------------
    static constexpr const char* SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr const char* CHAR_RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr const char* CHAR_TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

    BLETextLink() = default;
    ~BLETextLink() { end(); }

    // ====================================================================
    //  begin - 初始化 BLE, 启动 Peripheral, 若为 MASTER 则启动连接任务
    void begin(const String& peerAddr,
               Role role = AUTO,
               const String& name = "ESP32S3-BLE",
               uint16_t mtu = 512)
    {
        _peerAddr = peerAddr;
        _name     = name;
        _mtu      = mtu;
        _role     = role;
        _stopTask = false;

        BLEDevice::init(_name.c_str());
        BLEDevice::setMTU(_mtu);
        _decideRole();

        // 预分配回调对象 (重连时复用, 避免 new 造成的内存碎片)
        _serverCb = new ServerCb(this);
        _charRxCb = new CharRxCb(this);
        _clientCb = new ClientCb(this);

        _startServer();

        if (_effectiveRole == MASTER) {
            xTaskCreatePinnedToCore(_connTaskStub, "BLELink", 8192,
                                    this, 1, &_taskHandle, 0);
        }

        Serial.printf("[BLELink] role=%s peer=%s local=%s\n",
            _effectiveRole == MASTER ? "MASTER" : "SLAVE",
            _peerAddr.c_str(), localAddress().c_str());
    }

    // ---------------- 回调注册 ----------------
    void onReceive(MsgCallback cb)      { _onRx = cb; }
    void onConnect(EventCallback cb)    { _onCn = cb; }
    void onDisconnect(EventCallback cb) { _onDc = cb; }

    // ====================================================================
    //  send - 发送文本到对端
    //   - 持久连接, 无需每次重连
    //   - 超过 MTU-3 字节自动截断 (509 字节 @MTU=512)
    //   - 返回 true 表示已成功写入底层 (实际送达依赖 GATT ACK)
    bool send(const String& text)
    {
        if (text.length() == 0) return false;
        size_t maxN = (size_t)_mtu - 3;
        size_t n = (text.length() < maxN) ? text.length() : maxN;

        if (_effectiveRole == MASTER) {
            if (!_client || !_client->isConnected() || !_remoteRx) return false;
            _remoteRx->writeValue((uint8_t*)text.c_str(), n, true);
            return true;
        } else {
            if (!_connected || !_charTx) return false;
            _charTx->setValue((const uint8_t*)text.c_str(), n);
            _charTx->notify();
            return true;
        }
    }

    // ---------------- 状态查询 ----------------
    bool   isConnected() const { return _connected; }
    String localAddress() const {
        return String(BLEDevice::getAddress().toString().c_str());
    }
    String peerAddress() const { return _peerAddr; }
    Role   role()        const { return _effectiveRole; }

    // ====================================================================
    //  loop - 必须在 Arduino loop() 中调用
    //   1. 派发接收消息到用户 onReceive
    //   2. 派发连接 / 断开事件到用户 onConnect / onDisconnect
    //   3. MASTER 角色的自动重连由后台任务执行, 不在此处阻塞
    void loop()
    {
        // 1. 派发接收消息
        String rx;
        for (;;) {
            portENTER_CRITICAL(&_rxMux);
            bool has = (_rxHead != _rxTail);
            if (has) { rx = _rxBuf[_rxTail]; _rxTail = (_rxTail + 1) % RX_BUF_LEN; }
            portEXIT_CRITICAL(&_rxMux);
            if (!has) break;
            if (_onRx) _onRx(rx);
        }
        // 2. 派发事件
        if (_evtConnected)    { _evtConnected = false;    if (_onCn) _onCn(); }
        if (_evtDisconnected) { _evtDisconnected = false; if (_onDc) _onDc(); }
    }

    void end()
    {
        _stopTask = true;
        if (_taskHandle) {
            // 等待任务退出 (最多约 1 秒)
            for (int i = 0; i < 10 && _taskHandle; ++i) vTaskDelay(pdMS_TO_TICKS(100));
            _taskHandle = nullptr;
        }
        _stopClient();
        if (_server) BLEDevice::getAdvertising()->stop();
    }

private:
    // ---------------- 接收环形缓冲 (BLE task 写, loop task 读) ----------------
    static const int RX_BUF_LEN = 16;
    String          _rxBuf[RX_BUF_LEN];
    volatile size_t _rxHead = 0, _rxTail = 0;
    portMUX_TYPE    _rxMux = portMUX_INITIALIZER_UNLOCKED;

    // ---------------- 事件标志 (BLE task 置位, loop task 清除并回调) ----------------
    volatile bool _evtConnected    = false;
    volatile bool _evtDisconnected = false;

    // ---------------- 配置 ----------------
    String   _peerAddr;
    String   _name;
    Role     _role = AUTO;
    Role     _effectiveRole = SLAVE;
    uint16_t _mtu = 512;

    // ---------------- 状态 ----------------
    volatile bool _connected  = false;
    volatile bool _connecting = false;
    volatile bool _stopTask   = false;
    TaskHandle_t  _taskHandle = nullptr;

    // ---------------- Central 资源 ----------------
    BLEClient*                _client   = nullptr;
    BLERemoteCharacteristic*  _remoteRx = nullptr;
    BLERemoteCharacteristic*  _remoteTx = nullptr;

    // ---------------- Peripheral 资源 ----------------
    BLEServer*        _server = nullptr;
    BLECharacteristic* _charRx = nullptr;
    BLECharacteristic* _charTx = nullptr;

    // ---------------- 嵌套回调类 (前向声明 + 友元) ----------------
    class ServerCb;
    class CharRxCb;
    class ClientCb;
    class NotifyCb;  // No longer used, subscribe now uses lambdas
    friend class ServerCb; friend class CharRxCb;
    friend class ClientCb;

    ServerCb* _serverCb = nullptr;
    CharRxCb* _charRxCb = nullptr;
    ClientCb* _clientCb = nullptr;

    // ---------------- 用户回调 ----------------
    MsgCallback   _onRx = nullptr;
    EventCallback _onCn = nullptr;
    EventCallback _onDc = nullptr;

    // ============================================================
    //  接收缓冲写入 (BLE 回调上下文调用)
    void _pushRx(const String& s)
    {
        portENTER_CRITICAL(&_rxMux);
        _rxBuf[_rxHead] = s;
        _rxHead = (_rxHead + 1) % RX_BUF_LEN;
        if (_rxHead == _rxTail) _rxTail = (_rxTail + 1) % RX_BUF_LEN; // 满则丢最旧
        portEXIT_CRITICAL(&_rxMux);
    }

    // ============================================================
    //  角色决策 (AUTO 模式: MAC 字典序小者为 MASTER)
    void _decideRole()
    {
        if (_role == MASTER)      { _effectiveRole = MASTER; return; }
        if (_role == SLAVE)       { _effectiveRole = SLAVE;  return; }
        String me = localAddress();
        _effectiveRole = _macLessThan(me, _peerAddr) ? MASTER : SLAVE;
    }
    static bool _macLessThan(const String& a, const String& b)
    {
        String A = a; A.toUpperCase(); A.replace(":", "");
        String B = b; B.toUpperCase(); B.replace(":", "");
        return A < B;
    }

    // ============================================================
    //  启动 Peripheral (Server + 广播)
    void _startServer()
    {
        _server = BLEDevice::createServer();
        _server->setCallbacks(_serverCb);

        BLEService* svc = _server->createService(SERVICE_UUID);

        _charRx = svc->createCharacteristic(
            CHAR_RX_UUID,
            BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
        _charRx->setCallbacks(_charRxCb);

        _charTx = svc->createCharacteristic(
            CHAR_TX_UUID,
            BLECharacteristic::PROPERTY_NOTIFY);

        svc->start();

        BLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->addServiceUUID(SERVICE_UUID);
        adv->setScanResponse(true);
        adv->setMinPreferred(0x06);
        adv->setMinPreferred(0x12);
        BLEDevice::startAdvertising();
    }

    // ============================================================
    //  Central 连接管理
    void _stopClient()
    {
        if (_client) {
            if (_client->isConnected()) _client->disconnect();
            delete _client;
            _client = nullptr;
            _remoteRx = nullptr;
            _remoteTx = nullptr;
        }
    }

    // 在后台任务中执行: 阻塞式 connect, 5s 超时
    void _doConnect()
    {
        _stopClient();
        _client = BLEDevice::createClient();
        _client->setClientCallbacks(_clientCb);

        if (!_client->connect(BLEAddress(_peerAddr.c_str()))) {
            Serial.printf("[BLELink] connect %s failed\n", _peerAddr.c_str());
            _stopClient();
            return;
        }
        _client->setMTU(_mtu);

        BLERemoteService* svc = _client->getService(BLEUUID(SERVICE_UUID));
        if (!svc) { Serial.println("[BLELink] service not found"); _stopClient(); return; }

        _remoteRx = svc->getCharacteristic(BLEUUID(CHAR_RX_UUID));
        _remoteTx = svc->getCharacteristic(BLEUUID(CHAR_TX_UUID));
        if (!_remoteRx) { Serial.println("[BLELink] RX char not found"); _stopClient(); return; }

        if (_remoteTx && _remoteTx->canNotify()) {
            _remoteTx->subscribe(true, [this](BLERemoteCharacteristic* c, uint8_t* data, size_t length, bool isNotify) {
                if (length) _pushRx(String((const char*)data, length));
            });
        }

        _connected = true;
        _evtConnected = true;
        Serial.printf("[BLELink] connected to %s (MTU=%u)\n",
                      _peerAddr.c_str(), _client->getMTU());
    }

    static void _connTaskStub(void* arg)
    {
        static_cast<BLETextLink*>(arg)->_connTask();
        vTaskDelete(nullptr);
    }

    void _connTask()
    {
        const uint32_t RETRY_MS = 3000;
        uint32_t last = 0;
        for (;;) {
            if (_stopTask) return;
            if (!_connected && !_connecting) {
                if (millis() - last > RETRY_MS) {
                    _connecting = true;
                    _doConnect();
                    _connecting = false;
                    last = millis();
                }
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    // ============================================================
    //  回调类实现 (嵌套)
    // ============================================================

    // ---- Peripheral Server 端 ----
    class ServerCb : public BLEServerCallbacks {
    public:
        ServerCb(BLETextLink* p) : _p(p) {}

        void onConnect(BLEServer* s) override
        {
            // SLAVE 角色: 任意远端连入即视为与配置 peer 建链
            if (_p->_effectiveRole == SLAVE) {
                _p->_connected = true;
                _p->_evtConnected = true;
            }
        }

        void onDisconnect(BLEServer* s) override
        {
            if (_p->_effectiveRole == SLAVE) {
                _p->_connected = false;
                _p->_evtDisconnected = true;
                s->getAdvertising()->start();  // 重新广播, 等待 master 重连
            }
        }

    private:
        BLETextLink* _p;
    };

    // ---- 本机 CHAR_RX 被写入 (Peripheral 端接收) ----
    class CharRxCb : public BLECharacteristicCallbacks {
    public:
        CharRxCb(BLETextLink* p) : _p(p) {}
        void onWrite(BLECharacteristic* c) override
        {
            uint8_t* data = c->getData();
            size_t len = c->getLength();
            if (len > 0) _p->_pushRx(String((const char*)data, len));
        }
    private:
        BLETextLink* _p;
    };

    // ---- Central 端连接事件 ----
    class ClientCb : public BLEClientCallbacks {
    public:
        ClientCb(BLETextLink* p) : _p(p) {}
        void onConnect(BLEClient*) override {}
        void onDisconnect(BLEClient*) override
        {
            _p->_connected = false;
            _p->_evtDisconnected = true;
            _p->_remoteRx = nullptr;
            _p->_remoteTx = nullptr;
            // 注意: 不在此 delete _client, 由后台任务在下一次 _doConnect() 中清理
        }
    private:
        BLETextLink* _p;
    };

    // ---- NotifyCb retained for API compatibility but no longer used internally ----
#if defined(CONFIG_BLUEDROID_ENABLED) && !defined(CONFIG_NIMBLE_ENABLED)
    class NotifyCb : public BLERemoteCharacteristicCallbacks {
#else
    class NotifyCb {
#endif
    public:
        NotifyCb(BLETextLink* p) : _p(p) {}
#if defined(CONFIG_BLUEDROID_ENABLED) && !defined(CONFIG_NIMBLE_ENABLED)
        void onNotify(BLERemoteCharacteristic*, uint8_t* d, size_t n, bool) override
        {
            if (n) _p->_pushRx(String((const char*)d, n));
        }
#endif
    private:
        BLETextLink* _p;
    };
};
