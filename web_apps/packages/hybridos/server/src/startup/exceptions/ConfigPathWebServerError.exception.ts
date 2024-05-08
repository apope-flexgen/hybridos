export class ConfigPathWebServerError extends Error {
  constructor(message: string) {
    super();
    this.message = message;
  }
}
