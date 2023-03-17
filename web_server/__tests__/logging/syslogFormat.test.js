const { syslog } = require("winston/lib/winston/config");
const syslogFormat = require("../../src/logging/syslogFormat");

const reset = {
    level:"",
    message:""
}

const mockEntryCommon = {
    level:"warn",
    message:"Hello Test"
}

const mockEntryMiddleware = {
    level:"warn",
    message:"Hello Test",
    meta: {
        req:{
            headers:{
                originalUrl:"/test",
                "user-agent":"googlebot",
                host:"localhost"
            }
        }
    }
}

describe('syslogFormat', () => {
    beforeEach(()=>{
        syslogFormat.template(reset)
    })
    test('output JSON from Common Log with log entry', () => {
        expect(syslogFormat.template(mockEntryCommon)).not.toContain(
            `"host":"localhost"`
        );
    });
    test('output JSON from HTTP Middleware with log entry', () => {
        expect(syslogFormat.template(mockEntryMiddleware)).toContain(
            `"message":"Hello Test","url":"/test","useragent":"googlebot","host":"localhost"}`
        );
    });
    test('output JSON from HTTP Middleware NO log entry', () => {
        expect(syslogFormat.template()).toContain(`"level":""`)
      });
});