export class DBIDocumentNotFoundException extends Error {
  constructor(msg: string) {
    super(msg);
  }
}
