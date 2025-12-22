/**
 * 水生植物养殖缸数据模型
 * Smart Aquatic Plant Tank ViewModel
 */
export default class AquaticTankViewModel {
  // 控制模式：0-自动模式，1-手动模式
  controlMode: number = 0;

  // 水位（百分比 0-100）
  waterLevel: number = 0;

  // 水位下限阈值
  waterLevelMin: number = 30;

  // 水位上限阈值
  waterLevelMax: number = 80;

  // 水温（摄氏度）
  waterTemp: number = 0;

  // 水温下限阈值
  waterTempMin: number = 20;

  // 水温上限阈值
  waterTempMax: number = 28;

  // 水质TDS值（ppm，由TDS传感器获取）
  tdsValue: number = 0;

  // 水质TDS最佳范围下限
  tdsMin: number = 100;

  // 水质TDS最佳范围上限
  tdsMax: number = 500;

  // 光照强度（lux）
  lightIntensity: number = 0;

  // 光照需求阈值（低于此值需补光）
  lightThreshold: number = 500;

  // 补光时长（小时/天）
  lightDuration: number = 8;

  // 补水泵状态：0-关闭，1-开启
  pumpStatus: number = 0;

  // 抽水泵状态：0-关闭，1-开启
  waterPumpStatus: number = 0;

  // 加热器状态：0-关闭，1-开启
  heaterStatus: number = 0;

  // 散热风扇速度（百分比 0-100，由PWM控制）
  fanSpeed: number = 0;

  // LED灯状态：0-关闭，1-开启
  ledStatus: number = 0;

  // 报警状态：0-正常，1-警告，2-危险
  alarmStatus: number = 0;

  // 报警信息
  alarmMessage: string = '';

  // Wi-Fi连接状态：0-未连接，1-已连接
  wifiStatus: number = 0;

  // 选中的植物品种索引
  plantTypeIndex: number = 0;

  // 生长阶段：0-幼苗期，1-生长期，2-成熟期
  growthStage: number = 0;

  // 系统运行时间（小时）
  runningHours: number = 0;

  // 最后更新时间
  lastUpdateTime: string = '';
}
