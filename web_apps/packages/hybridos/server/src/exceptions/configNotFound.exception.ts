export class ConfigNotFoundException extends Error {
  constructor(msg: string) {
    super(`Config file not found - ${msg}`);
  }
}
