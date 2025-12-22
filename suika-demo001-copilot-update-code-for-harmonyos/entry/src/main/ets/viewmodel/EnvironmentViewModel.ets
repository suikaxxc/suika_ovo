export default class EnvironmentViewModel {
  //0正常，1，警告;2，危险
  stat: number = 0;

  //空气温度
  temp: number = 0;

  //单位为：ppm，ppm浓度（parts per million）是用溶质质量占全部溶液质量的百万分比来表示的浓度
  // MQ-2测试范围为：300-10000ppm
  gas: number = 0;

  //空气湿度
  hum: number = 0;
}