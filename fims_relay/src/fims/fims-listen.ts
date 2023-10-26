const fims = require('./fimsInstance');
const { parentPort } = require('worker_threads');

function publish_on_receive() {
  fims.receiveWithTimeout(50, (data) => {
    if (data) {
      parentPort.postMessage(data);
    }
  });
  setTimeout(() => {
    publish_on_receive();
  }, 2);
};

var subscribedToAll = fims.subscribeTo('/');
console.log("subscribedToAll:", subscribedToAll);

publish_on_receive();
