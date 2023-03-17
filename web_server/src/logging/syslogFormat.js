const winston = require('winston');
const {
    printf,
} = winston.format;

function parseLogMapping(headers, field){
    let out = "";
    try{
        if(field in headers && headers[field] != null){
            out = headers[field];
        }
    } catch (e) {
        //console.log(`${e}`)
    } finally {
      return out
    }
}

syslogFormat = printf((entry)=> {
    const logBody = new Object;
    logBody.timestamp = `${new Date().toISOString()}`
    if (entry){
        logBody.level = entry.level;
        logBody.application = "web_server";
        logBody.message = entry.message;
        const fields = ["originalUrl", "user-agent", "host"]
        try {
            fields.forEach(i=>{
                if (entry.meta !== undefined){
                    const val1 = parseLogMapping(entry.meta.req.headers,i)
                    if (val1){
                    logBody[i] = val1
                    }
                }
            })
        } catch(e) {
            //console.log(`${e}`);
        }    
        
    } 
    return JSON.stringify(logBody);
});

module.exports = syslogFormat;
