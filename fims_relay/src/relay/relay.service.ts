import { Injectable } from '@nestjs/common';
import * as zmq from 'zeromq';
import { pathToFileURL } from 'url';
import * as worker_threads from 'worker_threads';
import fims from '../fims/fimsInstance';
import { ApiService } from 'src/api/api.service';
import * as path from 'path'

@Injectable()
export class RelayService {
    private sock: zmq.Socket;
    private pull: zmq.Socket;
    private hasStarted = false;

    constructor(
        private readonly apiService: ApiService,
    ) {
        const FIMS_RELAY_PUB_ADDRESS = 'tcp://0.0.0.0:4000'
        const FIMS_RELAY_PULL_ADDRESS = 'tcp://0.0.0.0:4001'
        this.sock = zmq.socket('pub').bindSync(FIMS_RELAY_PUB_ADDRESS);
        this.pull = zmq.socket('pull').bindSync(FIMS_RELAY_PULL_ADDRESS);
    }
    
    public start(): void {
        const workerURL = path.join(__dirname, '../fims/fims-listen.js');
        const worker = new worker_threads.Worker(workerURL);

        worker.on('message', (msg: any) => {
            this.apiService.eventEmitter.emit('fims.message', msg);
            this.sock.send([msg.uri, JSON.stringify(msg)]);
        });

        this.pull.on('message', (msg: string) => {
            if (fims) {
                console.log('msg received: ', msg);
                fims.send(JSON.parse(msg));
            }
        });

        this.hasStarted = true;
    }

    public relayHasStarted(): boolean {
        return this.hasStarted;
    }

    public closeConnection(): boolean {
        return fims.closeConnection();
    }
}
