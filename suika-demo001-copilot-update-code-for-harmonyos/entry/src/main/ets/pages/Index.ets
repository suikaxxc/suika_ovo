import { AIComponent } from '../view/AIComponent';
import { StatusComponent } from '../view/StatusComponent';
import { ControlComponent } from '../view/ControlComponent';
import { DataDebugComponent } from '../view/DataDebugComponent';
import { SettingsComponent, SettingsChangeData } from '../view/SettingsComponent';
import AquaticTankViewModel from '../viewmodel/AquaticTankViewModel';
import { MqttMessage } from '@ohos/mqtt';
import MQTTUtil from '../common/utils/MQTTUtil';
import { PlantParamsConfig } from '../common/constants/Constant';
import { notificationManager } from '@kit.NotificationKit';
import { promptAction } from '@kit.ArkUI';

interface MqttData {
  waterLevel?: number;
  waterTemp?: number;
  lightIntensity?: number;
  tdsValue?: number;
  turbidityValue?: number;
  pumpStatus?: number;
  waterPumpStatus?: number;
  heaterStatus?: number;
  fanSpeed?: number;
  ledStatus?: number;
  alarmStatus?: number;
  alarmMessage?: string;
  controlMode?: number;
}

interface ControlData {
  type: string;
  value: number;
  timestamp: number;
}

interface SettingsData {
  type: string;
  plantTypeIndex?: number;
  waterTempMin?: number;
  waterTempMax?: number;
  waterLevelMin?: number;
  waterLevelMax?: number;
  lightThreshold?: number;
  lightDuration?: number;
  timestamp: number;
}

@Entry
@Component
struct Index {
  private static readonly TAB_STATUS: number = 0;
  private static readonly TAB_CONTROL: number = 1;
  private static readonly TAB_SETTINGS: number = 2;
  private static readonly TAB_AI: number = 3;
  private static readonly TAB_DEBUG: number = 4;
  @State item: AquaticTankViewModel = new AquaticTankViewModel();
  @State currentTab: number = Index.TAB_STATUS;
  @State showHeader: boolean = true;
  @State headerBlur: number = 0;
  private mqttUtil: MQTTUtil = new MQTTUtil();
  private readonly alarmNotificationId: number = 1001;
  private static readonly DEFAULT_ALARM_MESSAGE: string = '水生植物养殖缸检测到异常，请及时处理';
  private lastNotifiedAlarmStatus: number = 0;
  private lastNotifiedAlarmMessage: string = '';
  private notificationEnabled: boolean = true;
  private lastTabScrollOffsets: number[] = [0, 0, 0, 0, 0];
  @State manualDataMode: boolean = false;
  // 用户最后一次切换模式的时间戳，用于防止MQTT数据覆盖用户刚刚的操作
  private lastModeChangeTime: number = 0;
  // 用户最后一次发送任何控制命令的时间戳，用于防止在手动控制期间被自动模式覆盖
  private lastControlCommandTime: number = 0;
  // 模式同步防抖时间（毫秒）- 在用户切换模式后的这段时间内，不从硬件同步模式
  // 增加到30秒以确保手动模式不会被意外覆盖
  private static readonly MODE_SYNC_DEBOUNCE_MS: number = 30000;
  private static readonly HEADER_TOGGLE_SCROLL_DELTA: number = 10;

  aboutToAppear(): void {
    // 初始化默认植物参数（绿萝）
    this.applyDefaultPlantParams();

    // 模拟初始数据（实际应用中从MQTT获取）
    this.initMockData();

    // 连接MQTT接收实时数据
    this.refreshNotificationPermission();
    this.mqttUtil.connect(
      (arg1: Error | MqttMessage, arg2?: MqttMessage) => {
        // 兼容两种回调签名：
        // 1) (data)
        // 2) (err, data)
        let data: MqttMessage | undefined = undefined;
        let err: Error | undefined = undefined;
        if (arg2 !== undefined) {
          err = arg1 as Error;
          data = arg2;
        } else if ((arg1 as MqttMessage).payload !== undefined) {
          data = arg1 as MqttMessage;
        } else if (arg1 instanceof Error) {
          err = arg1;
        }

        if (err) {
          console.error('MQTT消息回调异常:', err);
        }
        if (!data?.payload) {
          return;
        }

        try {
          const receivedData: MqttData = JSON.parse(String(data.payload));
          const now = Date.now();
          // 计算用户最后一次操作的时间
          const lastUserAction = Math.max(this.lastModeChangeTime, this.lastControlCommandTime);
          const isDebouncing = (now - lastUserAction) < Index.MODE_SYNC_DEBOUNCE_MS;

          // 更新传感器数据（手动演示模式下不覆盖）
          if (!this.manualDataMode) {
            this.item.waterLevel = receivedData.waterLevel ?? this.item.waterLevel;
            this.item.waterTemp = receivedData.waterTemp ?? this.item.waterTemp;
            this.item.lightIntensity = receivedData.lightIntensity ?? this.item.lightIntensity;
            this.item.tdsValue = receivedData.tdsValue ?? this.item.tdsValue;
            this.item.turbidityValue = receivedData.turbidityValue ?? this.item.turbidityValue;
          }
          this.item.alarmStatus = receivedData.alarmStatus ?? this.item.alarmStatus;
          this.item.alarmMessage = receivedData.alarmMessage ?? this.item.alarmMessage;
          this.syncAlarmNotification(this.item.alarmStatus, this.item.alarmMessage);

          // 执行器状态：只在非防抖期间从硬件同步，防止用户手动控制被覆盖
          if (!isDebouncing) {
            this.item.pumpStatus = receivedData.pumpStatus ?? this.item.pumpStatus;
            this.item.waterPumpStatus = receivedData.waterPumpStatus ?? this.item.waterPumpStatus;
            this.item.heaterStatus = receivedData.heaterStatus ?? this.item.heaterStatus;
            this.item.fanSpeed = receivedData.fanSpeed ?? this.item.fanSpeed;
            this.item.ledStatus = receivedData.ledStatus ?? this.item.ledStatus;
            // 控制模式也在非防抖期间同步
            if (receivedData.controlMode !== undefined) {
              this.item.controlMode = receivedData.controlMode;
            }
          } else {
            // 防抖期间，记录日志但不同步执行器状态和控制模式
            console.info('防抖中: 忽略硬件状态同步，剩余时间=' +
              (Index.MODE_SYNC_DEBOUNCE_MS - (now - lastUserAction)) + 'ms');
          }
          this.item.wifiStatus = 1; // 收到数据说明已连接
          this.item.lastUpdateTime = new Date().toLocaleTimeString();
          console.info('MQTT实时数据同步成功:', JSON.stringify(receivedData));
        } catch (e) {
          console.error('解析MQTT数据失败:', e, 'payload=', String(data.payload));
        }
      }
    );
  }

  applyDefaultPlantParams(): void {
    const defaultParams = PlantParamsConfig[0]; // 绿萝
    if (defaultParams) {
      this.item.waterTempMin = defaultParams.waterTempMin;
      this.item.waterTempMax = defaultParams.waterTempMax;
      this.item.waterLevelMin = defaultParams.waterLevelMin;
      this.item.waterLevelMax = defaultParams.waterLevelMax;
      this.item.lightThreshold = defaultParams.lightThreshold;
      this.item.lightDuration = defaultParams.lightDuration;
    }
  }

  initMockData(): void {
    // 模拟传感器初始数据 (水位单位: mm, YW01传感器范围0-90mm)
    this.item.waterLevel = 50;  // 50mm
    this.item.waterTemp = 24;
    this.item.lightIntensity = 450;
    this.item.tdsValue = 280;
    this.item.turbidityValue = 120;
    this.item.fanSpeed = 0;
    this.item.waterPumpStatus = 0;
    this.item.wifiStatus = 0;
    this.item.alarmStatus = 0;
  }

  handleControlChange(controlType: string, value: number): void {
    // 记录用户操作时间戳，用于防抖机制
    const now = Date.now();
    if (controlType === 'mode') {
      // 模式切换记录专用时间戳
      this.lastModeChangeTime = now;
    } else if (controlType !== 'oledPage') {
      // 非翻页的控制命令也记录时间戳，防止在手动操作期间被自动模式覆盖
      this.lastControlCommandTime = now;
    }
    // 发送控制指令到Hi3861设备
    const controlData: ControlData = {
      type: controlType,
      value: value,
      timestamp: now
    };
    console.info('发送控制指令:', JSON.stringify(controlData));
    // 通过MQTT发送控制指令到硬件端
    this.mqttUtil.publish(JSON.stringify(controlData));
  }

  handleSettingsChange(settings: SettingsChangeData): void {
    console.info('设置更新:', JSON.stringify(settings));
    if (settings.plantTypeIndex !== undefined) {
      const plantData: ControlData = {
        type: 'plant',
        value: settings.plantTypeIndex,
        timestamp: Date.now()
      };
      this.mqttUtil.publish(JSON.stringify(plantData));
    }

    const shouldSendSettings: boolean =
      settings.waterTempMin !== undefined ||
      settings.waterTempMax !== undefined ||
      settings.waterLevelMin !== undefined ||
      settings.waterLevelMax !== undefined ||
      settings.lightThreshold !== undefined ||
      settings.lightDuration !== undefined;

    if (!shouldSendSettings) {
      return;
    }

    // 通过MQTT发送养护参数到硬件端
    const settingsData: SettingsData = {
      type: 'settings',
      plantTypeIndex: settings.plantTypeIndex,
      waterTempMin: settings.waterTempMin,
      waterTempMax: settings.waterTempMax,
      waterLevelMin: settings.waterLevelMin,
      waterLevelMax: settings.waterLevelMax,
      lightThreshold: settings.lightThreshold,
      lightDuration: settings.lightDuration,
      timestamp: Date.now()
    };
    this.mqttUtil.publish(JSON.stringify(settingsData));
  }

  private syncAlarmNotification(alarmStatus: number, alarmMessage: string): void {
    const message = alarmMessage && alarmMessage.length > 0 ? alarmMessage : Index.DEFAULT_ALARM_MESSAGE;
    if (alarmStatus > 0) {
      if (this.lastNotifiedAlarmStatus === alarmStatus && this.lastNotifiedAlarmMessage === message) {
        return;
      }
      const request: notificationManager.NotificationRequest = {
        id: this.alarmNotificationId,
        content: {
          notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
          normal: {
            title: '智能养殖缸报警',
            text: message,
            additionalText: `报警等级：${alarmStatus}`
          }
        }
      };
      if (!this.notificationEnabled) {
        promptAction.showToast({
          message: '检测到报警，请在系统设置中开启通知以接收APP信息通报',
          duration: 2200
        });
      }
      notificationManager.publish(request).catch((err: Error) => {
        console.error('发布报警通知失败:', JSON.stringify(err));
      });
    } else if (this.lastNotifiedAlarmStatus > 0) {
      notificationManager.cancel(this.alarmNotificationId).catch((err: Error) => {
        console.error('清理报警通知失败:', JSON.stringify(err));
      });
    }
    this.lastNotifiedAlarmStatus = alarmStatus;
    this.lastNotifiedAlarmMessage = alarmStatus > 0 ? message : '';
  }

  private refreshNotificationPermission(): void {
    notificationManager.isNotificationEnabled().then((enabled: boolean) => {
      this.notificationEnabled = enabled;
      console.info(`系统通知开关状态: ${enabled}`);
    }).catch((err: Error) => {
      this.notificationEnabled = true;
      console.error('获取通知权限状态失败:', JSON.stringify(err));
    });
  }

  private handleTabScroll(tabIndex: number, offsetY: number): void {
    const safeOffset = Math.max(0, offsetY);
    const lastOffset = this.lastTabScrollOffsets[tabIndex] ?? 0;
    const delta = safeOffset - lastOffset;
    this.lastTabScrollOffsets[tabIndex] = safeOffset;
    if (safeOffset <= 2 && !this.showHeader) {
      this.showHeader = true;
      this.headerBlur = 0;
      return;
    }
    if (delta > Index.HEADER_TOGGLE_SCROLL_DELTA && this.showHeader) {
      this.showHeader = false;
      this.headerBlur = 10;
    } else if (delta < -Index.HEADER_TOGGLE_SCROLL_DELTA && !this.showHeader) {
      this.showHeader = true;
      this.headerBlur = 0;
    }
  }

  build() {
    Column() {
      // 顶部标题栏
      Column() {
        Row() {
          Image($r('app.media.aquarium'))
            .width(32)
            .height(32)
          Text("智能水生植物养殖缸")
            .fontSize(22)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.White)
            .margin({ left: 10 })
        }
        .width('100%')
        .height(64)
        .padding({ left: 16, right: 16, top: 4, bottom: 4 })
        .backgroundColor('#42A5F5')
        .justifyContent(FlexAlign.Start)
      }
      .width('100%')
      .height(this.showHeader ? 64 : 0)
      .opacity(this.showHeader ? 1 : 0)
      .blur(this.headerBlur)
      .clip(true)
      .animation({ duration: 260, curve: Curve.EaseInOut })

      // 内容区域（导航栏控制四个页面）
      Column() {
        // Tab功能区：占据剩余空间，确保四个页面单屏完整可见
        Tabs({ barPosition: BarPosition.End, index: this.currentTab }) {
          TabContent() {
            Scroll() {
              StatusComponent({ item: this.item })
                .margin({ top: 10, bottom: 10 })
            }
            .scrollBar(BarState.Auto)
            .scrollable(ScrollDirection.Vertical)
            .onDidScroll((xOffset: number, yOffset: number) => {
              void xOffset;
              this.handleTabScroll(Index.TAB_STATUS, yOffset);
            })
            .width('100%')
            .height('100%')
          }
          .tabBar(this.TabBuilder(Index.TAB_STATUS, "状态总览", $r('app.media.aquarium')))

          TabContent() {
            ControlComponent({
              item: $item,
              onControlChange: (type: string, value: number) => this.handleControlChange(type, value),
              onScrollOffsetChange: (offsetY: number) => this.handleTabScroll(Index.TAB_CONTROL, offsetY)
            })
          }
          .tabBar(this.TabBuilder(Index.TAB_CONTROL, "设备管理", $r('app.media.control')))

          TabContent() {
            SettingsComponent({
              item: $item,
              onSettingsChange: (settings: object) => this.handleSettingsChange(settings),
              onScrollOffsetChange: (offsetY: number) => this.handleTabScroll(Index.TAB_SETTINGS, offsetY)
            })
          }
          .tabBar(this.TabBuilder(Index.TAB_SETTINGS, "养护设置", $r('app.media.settings')))

          TabContent() {
            AIComponent({
              item: this.item,
              onScrollOffsetChange: (offsetY: number) => this.handleTabScroll(Index.TAB_AI, offsetY)
            })
          }
          .tabBar(this.TabBuilder(Index.TAB_AI, "AI建议", $r('app.media.ai')))

          TabContent() {
            DataDebugComponent({
              item: $item,
              manualDataMode: $manualDataMode,
              onScrollOffsetChange: (offsetY: number) => this.handleTabScroll(Index.TAB_DEBUG, offsetY)
            })
          }
          .tabBar(this.TabBuilder(Index.TAB_DEBUG, "数据调试", $r('app.media.settings')))
        }
        .layoutWeight(1)
        .width('100%')
        .barMode(BarMode.Fixed)
        .barWidth('100%')
        .barHeight(56)
        .backgroundColor('#F0F4F8')
        .clip(true)
        .onChange((index: number) => {
          this.currentTab = index;
        })
      }
        .layoutWeight(1)
        .padding({ top: 0, bottom: 10 })
        .backgroundColor('#F3F6FA')
        .clip(true)
      }
    .width('100%')
    .height('100%')
  }

  @Builder
  TabBuilder(index: number, name: string, icon: Resource) {
    Column() {
      Image(icon)
        .width(22)
        .height(22)
        .fillColor(this.currentTab === index ? '#42A5F5' : '#999999')
      Text(name)
        .fontSize(12)
        .fontColor(this.currentTab === index ? '#42A5F5' : '#999999')
        .margin({ top: 4 })
    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .backgroundColor('#FAFCFF')
  }
}
