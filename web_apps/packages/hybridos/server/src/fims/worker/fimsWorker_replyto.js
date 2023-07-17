// This needs to be a var, or TSC will complain about a collision
// with the other worker script
/* eslint-disable @typescript-eslint/no-var-requires */
const { parentPort } = require('worker_threads');
const fims = require('@flexgen/fims');

// FIXME: using const/let here causes mockWorker and fimsWorker
// to throw errors for redeclaring block-scoped variables. Using
// var works as a workaround, but we should figure out how to actually
// fix this.

// TODO: error handling? smarter loop? or just leave this
// for now and worry about doing it "right" when we have
// a better solution for this connection?
const listen = () => {
  fims.receiveWithTimeout(50, (data) => {
    if (data) {
      parentPort.postMessage(data);
    }
  });
  setTimeout(() => {
    listen();
  }, 2);
};

fims.connect('_web_server_replyto_listener');

const URIs = ['/ui_reply'];
fims.subscribeToList(URIs);
listen();
