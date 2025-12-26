// ==================== 智能水生植物养殖缸系统常量 ====================

// 环境安全级别
export const StatusArray: string[] = ["安全", "警告", "危险", "未知"];

// 报警状态
export const AlarmStatusArray: string[] = ["正常", "警告", "危险"];

// 控制模式
export const ControlModeArray: string[] = ["自动模式", "手动模式"];

// 设备状态
export const DeviceStatusArray: string[] = ["关闭", "开启"];

// Wi-Fi连接状态
export const WifiStatusArray: string[] = ["未连接", "已连接"];

// 生长阶段
export const GrowthStageArray: string[] = ["幼苗期", "生长期", "成熟期"];

// 水生植物品种列表
export const PlantTypeArray: string[] = [
  "绿萝",
  "睡莲",
  "铜钱草",
  "水葫芦",
  "富贵竹",
  "碗莲",
  "水仙",
  "风信子"
];

// 植物参数配置接口
export interface PlantParams {
  name: string;
  waterTempMin: number;
  waterTempMax: number;
  lightThreshold: number;
  lightDuration: number;
  waterLevelMin: number;  // 单位: mm (YW01传感器范围0-90mm)
  waterLevelMax: number;  // 单位: mm (YW01传感器范围0-90mm)
  description: string;
}

// 各种植物的最优生长参数 (水位单位: mm, YW01传感器范围0-90mm)
export const PlantParamsConfig: PlantParams[] = [
  {
    name: "绿萝",
    waterTempMin: 18,
    waterTempMax: 28,
    lightThreshold: 300,
    lightDuration: 8,
    waterLevelMin: 18,   // 20% -> 18mm
    waterLevelMax: 54,   // 60% -> 54mm
    description: "绿萝喜温暖湿润，耐阴性强，适合水培养殖"
  },
  {
    name: "睡莲",
    waterTempMin: 22,
    waterTempMax: 30,
    lightThreshold: 800,
    lightDuration: 10,
    waterLevelMin: 45,   // 50% -> 45mm
    waterLevelMax: 81,   // 90% -> 81mm
    description: "睡莲需充足阳光，水位要求较高，开花美观"
  },
  {
    name: "铜钱草",
    waterTempMin: 16,
    waterTempMax: 26,
    lightThreshold: 400,
    lightDuration: 6,
    waterLevelMin: 27,   // 30% -> 27mm
    waterLevelMax: 63,   // 70% -> 63mm
    description: "铜钱草生命力强，寓意招财进宝，易于养护"
  },
  {
    name: "水葫芦",
    waterTempMin: 20,
    waterTempMax: 32,
    lightThreshold: 600,
    lightDuration: 8,
    waterLevelMin: 36,   // 40% -> 36mm
    waterLevelMax: 72,   // 80% -> 72mm
    description: "水葫芦繁殖快速，可净化水质，开紫色小花"
  },
  {
    name: "富贵竹",
    waterTempMin: 18,
    waterTempMax: 25,
    lightThreshold: 200,
    lightDuration: 6,
    waterLevelMin: 23,   // 25% -> ~23mm
    waterLevelMax: 45,   // 50% -> 45mm
    description: "富贵竹寓意吉祥，耐阴耐寒，四季常青"
  },
  {
    name: "碗莲",
    waterTempMin: 20,
    waterTempMax: 28,
    lightThreshold: 700,
    lightDuration: 10,
    waterLevelMin: 41,   // 45% -> ~41mm
    waterLevelMax: 77,   // 85% -> ~77mm
    description: "碗莲小巧精致，开花清雅，适合室内观赏"
  },
  {
    name: "水仙",
    waterTempMin: 10,
    waterTempMax: 18,
    lightThreshold: 500,
    lightDuration: 8,
    waterLevelMin: 18,   // 20% -> 18mm
    waterLevelMax: 36,   // 40% -> 36mm
    description: "水仙花香清雅，冬春开花，传统名花"
  },
  {
    name: "风信子",
    waterTempMin: 12,
    waterTempMax: 20,
    lightThreshold: 400,
    lightDuration: 6,
    waterLevelMin: 14,   // 15% -> ~14mm
    waterLevelMax: 32,   // 35% -> ~32mm
    description: "风信子花色丰富，香气浓郁，水培易活"
  }
];

// 服务器IP
const HOST_IP = '10.0.2.2'

// MQTT服务端地址（需要tcp://协议前缀）
export const MQTT_URL = `tcp://${HOST_IP}:1883`

// 大模型服务API
export const WEB_BASE_URL = `http://${HOST_IP}:8080/`

// MQTT主题
export const MQTT_TOPIC_DATA = 'tank/data';
export const MQTT_TOPIC_CONTROL = 'tank/control';