export const FimsDescriptions = {
  bodyURI:
    'The URI string associated with the message. All processes subscribed to this URI will receive the message.',
  bodyReplyTo:
    'Field containing another URI that receiver processes should send an acknowledgement message.',
  body: 'Body of the fims message; Typically either a single value or a stringified JSON object.',
  optionalBody:
    'Optional body of the fims message; Typically either a single value or a stringified JSON object.',
  getURI: 'URI of the GET message, indicates what data is requested',
  method: 'Method to use to send message - either “set”, “get”, “post”, “del”, or “pub”.',
  username: 'Typically username of user who sent the fims message',
};

export enum FimsErrorMessages {
  SEND_FAILED = 'fims.send failed',
  SEND_TIMED_OUT = 'fims request timed out',
}
