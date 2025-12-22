import { MqttAsync, MqttClient, MqttMessage } from '@ohos/mqtt';
import { MQTT_URL, MQTT_TOPIC_DATA, MQTT_TOPIC_CONTROL } from '../constants/Constant';

export default class MQTTUtil {
  private mqttAsyncClient: MqttClient | null = null;

  async connect(callback: (err: Error, data: MqttMessage) => void) {

    // 1、实例化MqttClient
    this.mqttAsyncClient = MqttAsync.createMqtt({
      url: MQTT_URL,
      clientId: 'harmonyos_aquatic_tank_' + Date.now(),
      persistenceType: 1,
    });

    // 2、连接
    let isConnect = await this.mqttAsyncClient.connect(
      {
        userName: "userName",
        password: "password",
        connectTimeout: 300
      }
    );
    if (!isConnect) {
      console.error('MQTT连接失败');
      return;
    }

    console.info('MQTT连接成功');

    // 3、设置回调
    this.mqttAsyncClient.messageArrived(callback);

    // 4、订阅水生植物养殖缸数据主题
    this.mqttAsyncClient.subscribe({
      topic: MQTT_TOPIC_DATA,
      qos: 2
    });
  }

  // 发布控制指令
  async publish(message: string): Promise<boolean> {
    if (!this.mqttAsyncClient) {
      console.error('MQTT客户端未初始化');
      return false;
    }

    try {
      await this.mqttAsyncClient.publish({
        topic: MQTT_TOPIC_CONTROL,
        qos: 1,
        payload: message
      });
      console.info('控制指令发送成功:', message);
      return true;
    } catch (error) {
      console.error('控制指令发送失败:', error);
      return false;
    }
  }

  // 断开连接
  async disconnect(): Promise<void> {
    if (this.mqttAsyncClient) {
      try {
        await this.mqttAsyncClient.disconnect();
        console.info('MQTT断开连接');
      } catch (error) {
        console.error('MQTT断开连接失败:', error);
      }
    }
  }
}