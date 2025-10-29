# IoT Device Hub

[![codecov](https://codecov.io/github/futail2a/IotDeviceHub/graph/badge.svg?token=9AQAJHNJBO)](https://codecov.io/github/futail2a/IotDeviceHub)


これはIoT機器のハブとして様々なビジネスロジックを追加できるアプリケーションです。

IoT Decice Hubは周囲のIoT機器にBLE経由でセンサデータを取得したりコマンドを送信することができます。

また、MQTT経由で外部のアプリケーションからコマンドを送ることも可能です。


## セットアップ

1. BLE機器の設定
* config/ディレクトリにあるtemplate_config.jsonをコピーしてconfig.jsonを作成します。
* config.jsonに以下のフォーマットで接続したいBLE機器のMACアドレスを追加します。
* 現在対応している機器はSwitchBotのスマート電球、ボット、人感センサーの三種類です。使用しないものについては空にします。("mac": "")


```
  "devices":
  {
    "woBulb":
    {
      "mac": "xx:xx:xx:xx:xx:xx"
    },
    "woHand":
    {
      "mac": "xx:xx:xx:xx:xx:xx"
    },
    "woMotionSensor":
    {
      "mac": "xx:xx:xx:xx:xx:xx"
    }
  },
```

2. MQTTブローカーの設定
* config/config.jsonに以下のフォーマットでMQTTブローカーの設定を追加します。
* ブローカーのIPアドレスとポート番号は必須です
* TLS接続をする場合はCA証明書のパスを追加します。また、クライアント証明もする場合はクライアント証明書、クライアント鍵のパスを追加します。必要がない場合は空にします。

```
  "mqtt":
  {
    "brokerIpv4": "127.0.0.1",
    "brokerPort": 1883,
    "caCert": "",
    "clientCert": "",
    "clientKey": ""
  }
```

3. ビルドとテスト
* 一番上のディレクトリで以下のcmakeコマンドを実行し、ビルドします。
```
$mkdir build
$cd build
$cmake ..

$make all
```
* ビルドが成功したら以下のコマンドでテストを実行します。
```
$make test
```
* Generate coverage report
```
$make coverage
```

(参考)
https://qiita.com/iydmsk/items/0021d1ef14660184f396


4. 実行とサンプルコマンド
* ビルドしてできたバイナリIotDeviceHubを実行するか、インストール後に以下を実行してサービスを起動するとアプリケーションが立ち上がります。
```
$ ./IotDeviceHub
#OR
$ systemctl daemon-reload
$ systemctl start iotdevicehub
```

その後設定したMQTTブローカーに対して"exec_bot"という名前のトピックをPUBLISHすると、SwitchBotボットに接続されている場合はスイッチ動作をします。

## 対象機器
* SwitchBot
    * スマート電球
    * ボット
    * 人感センサー

もしBLE機器を追加したい場合はdevices/ディレクトリにあるBleDeviceHandler.cppにあるBleDeviceHandlerクラスを継承して独自の機器を追加することができます。

## 既知の問題
* アドバタイズパケットの読み込みとBLE接続後で利用できる機能の併用ができない
    * BleSockScanManagerとBleDbusConnectionManagerの併用ができず、またBleSockScanManageのアドバタイズパケットのパースが未完了のため

## 参考
https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/tree/latest
