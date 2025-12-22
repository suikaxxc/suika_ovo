import { http } from '@kit.NetworkKit';
import { WEB_BASE_URL } from '../constants/Constant';

export default class HttpUtil {
  /**
   * T:是数据返回类型
   * K：为POST 参数对象类型
   *
   * @param url
   * @param params
   * @returns
   */

  static post<T, K>(url: string, params: K): Promise<T> {

    const httpRequest = http.createHttp()
    return new Promise((resolve: (value: T) => void, reject: (value?: string) => void) => {
      httpRequest.request(WEB_BASE_URL + url, {
        method: http.RequestMethod.POST,
        header: {
          "Content-type": "application/json; charset=utf-8"
        },
        extraData: JSON.stringify(params) || "",
        connectTimeout: 600000,
        readTimeout: 600000
      }, (error: Error, data: http.HttpResponse) => {
        if (!error) {
          resolve(JSON.parse(data.result as string))
        } else {
          reject(error.message || JSON.stringify(error));
        }
        httpRequest.off('headersReceive');
        httpRequest.destroy();
      }
      )
    })
  }
}