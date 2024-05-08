import { Injectable } from '@nestjs/common';

@Injectable()
export class LoggerFilter {
  // this class will read from a config file or a UI config later
  requestMayBeLogged(method: string, statusCode: number, originalUrl: string): boolean {
    const verbs = {
      POST: true,
      GET: true,
      PUT: true,
      DELETE: true,
      OPTIONS: false,
      CONNECT: false,
      HEAD: false,
      TRACE: false,
      PATCH: false,
    };
    const statusRegex = /[0-9][0-9][0-9]/g;
    // can use a look ahead to filter for what goes to log
    // const endpointRegex = /^(?!\/fims\/).*/g; -- will not log /fims/ endpoint traffic
    const endpointRegex = /.*/g;
    const configObject = { verbs: verbs, status: statusRegex };
    const statusString = statusCode.toString();
    return configObject.verbs[method] == true &&
      statusString.match(statusRegex) &&
      originalUrl.match(endpointRegex)
      ? true
      : false;
  }
  commonLogToFile(logType: string): boolean {
    const commonLogConfig = {
      error: true,
      log: true,
      warn: true,
      debug: true,
      verbose: true,
    };
    return commonLogConfig[logType];
  }
}
