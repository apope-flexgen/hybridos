import { ConsoleLogger, Injectable, Scope } from '@nestjs/common'
import { Logger } from '@nestjs/common'
import { LoggerFilter } from './logging_filter/logger-filter.service'
import { LogText } from './log_text/log-text'
import * as fs from 'fs'
import * as crypto from 'crypto'

@Injectable({ scope: Scope.TRANSIENT })
export class LoggingService extends ConsoleLogger {
    private readonly logger = new Logger()
    private loggerFilter = new LoggerFilter()
    writeToFileSystem(message: string): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            const filename = '/var/log/web_server/nest_web_server.log'
            fs.open(filename, 'a', (err, fd) => {
                if (err) {
                    reject()
                } else {
                    fs.write(fd, `${message}\n`, (err) => {
                        if (err) {
                            reject()
                        } else {
                            resolve()
                        }
                    })
                }
            })
        })
    }
    makeUuid(): Promise<string> {
        return new Promise<string>((resolve, reject) => {
            try {
                resolve(crypto.randomUUID())
            } catch (e) {
                reject()
            }
        })
    }
    log(log: LogText, context: string, stack?: string): void {
        this.makeLog(log, context, 'log')
    }
    error(log: LogText, context: string, stack?: string): void {
        this.makeLog(log, context, 'error')
    }
    warn(log: LogText, context: string, stack?: string): void {
        this.makeLog(log, context, 'warn')
    }
    debug(log: LogText, context: string, stack?: string): void {
        this.makeLog(log, context, 'debug')
    }
    verbose(log: LogText, context: string, stack?: string): void {
        this.makeLog(log, context, 'verbose')
    }
    async makeLog(log: LogText, context: string, logType: string): Promise<void> {
        this.makeUuid()
            .then((uuid) => {
                log.uuid = uuid
                log.level = logType
                super[logType](JSON.stringify(log))
                if (this.loggerFilter.commonLogToFile(logType)) {
                    log.context = context
                    log.timestamp = new Date().toISOString()
                    this.writeToFileSystem(JSON.stringify(log))
                        .then(() => {
                            return
                        })
                        .catch(() => {
                            return
                        })
                }
            })
            .catch(() => {
                return
            })
    }
}
