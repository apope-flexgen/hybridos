const fims = require('./fimsInstance');
const { parentPort } = require('worker_threads');

function publish_on_receive() {
  console.log('***** listening');
  while (true) {
    fims.receiveWithTimeout(50, (data) => {
      if (data) {
        parentPort.postMessage(data);
      }
    });
  }
};

var subscribedToAll = fims.subscribeTo('/');
console.log("subscribedToAll:", subscribedToAll);

publish_on_receive();
