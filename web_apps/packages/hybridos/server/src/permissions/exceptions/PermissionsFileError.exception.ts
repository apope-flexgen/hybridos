export class PermissionsFileError extends Error {
  constructor(message: string) {
    super();
    this.message = message;
  }
}
