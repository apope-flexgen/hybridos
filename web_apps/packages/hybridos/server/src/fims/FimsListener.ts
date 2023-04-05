import { EventEmitter2 } from '@nestjs/event-emitter'
import { Worker } from 'worker_threads'
import FIMS from '@flexgen/fims'

export default class FimsListener {
    eventEmitter: EventEmitter2
    constructor(eventEmitter: EventEmitter2) {
        this.eventEmitter = eventEmitter
    }
    open(): boolean {
        try {
            if (WORKER_PATH.includes('fimsWorker')) {
                // open a listener worker thread for FIMS messages
                // worker MUST be created with URL obj and it MUST use import.meta.url
                // or require('url').pathToFileURL(__filename).toString() as the base,
                // otherwise webpack will not be able to import the worker file
                const workerURL: URL = new URL(
                    WORKER_PATH,
                    require('url').pathToFileURL(__filename).toString()
                )

                // create the worker
                const worker = new Worker(workerURL)

                // listen for messages from the worker
                worker.on('message', (message) => {
                    this.eventEmitter.emit('fims.message', message)
                })

                // listen for errors from the worker
                worker.on('error', (error) => {
                    console.error(error)
                })
            } else {
                FIMS.connect('nest_web_server')
                FIMS.subscribeTo('')
                FIMS.receiveWithTimeout(null, (data: string) => {
                    const json = JSON.parse(data)
                    this.eventEmitter.emit('fims.message', json)
                })
            }
            return true
        } catch (e) {
            console.error(e)
            return false
        }
    }
}
