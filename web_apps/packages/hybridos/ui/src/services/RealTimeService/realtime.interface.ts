export default interface RealTimeService {
  // sends {data} to {destination}. if sent successfully, returns true, else false
  send: (destination: string, data: any, namepsace?: string) => Promise<string>;
  // listens to data on the real time service and calls back to callbackFn with
  // data of type T. if given, only listens for messages on {namespace}
  listen: <T>(callbackFn: (data: T) => void, namespace: string) => void;
  // sets accessToken
  setAccessToken: (accessToken: string) => void;
}
