export default interface ISocketConnectionManager {
  send: (event: string, data: any) => Promise<void>
  listen: (fn: (event: MessageEvent) => void) => Promise<void>
}
