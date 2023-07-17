import { EventEmitter2 } from '@nestjs/event-emitter';
import { Worker } from 'worker_threads';
import * as path from 'path';

export default class FimsListener {
  eventEmitter: EventEmitter2;
  worker: Worker;
  workerReplyto: Worker;
  constructor(eventEmitter: EventEmitter2) {
    this.eventEmitter = eventEmitter;
  }
  open(): boolean {
    try {
      const workerPath = path.join(__dirname, 'fims/worker/fimsWorker.js');
      this.worker = new Worker(workerPath);

      // listen for messages from the worker
      this.worker.on('message', (message) => {
        this.eventEmitter.emit('fims.message', message);
      });

      // listen for errors from the worker
      this.worker.on('error', (error) => {
        console.error(error);
      });

      const workerPathReplyto = path.join(__dirname, 'fims/worker/fimsWorker_replyto.js');
      this.workerReplyto = new Worker(workerPathReplyto);

      this.workerReplyto.on('message', (message) => {
        this.eventEmitter.emit('fims.reply', message);
      });

      this.workerReplyto.on('error', (error) => {
        console.error(error);
      });

      return true;
    } catch (e) {
      console.error(e);
      return false;
    }
  }
}
