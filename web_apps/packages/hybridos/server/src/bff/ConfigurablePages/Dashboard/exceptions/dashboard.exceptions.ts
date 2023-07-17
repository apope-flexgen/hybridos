export class DashboardConfigNotFoundException extends Error {
  constructor(msg: string) {
    super(`Dashboard Config file not found - ${msg}`);
  }
}
