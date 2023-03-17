class AuditLogger {
    constructor(fimsApi){
        this.fimsApi = fimsApi;
    }
    send(data){
        if (("username" in data) && ("userrole" in data) && ("modified_field" in data) && ("modified_value" in data)){
            data.created = Date.now()
            try{
                const msg = {
                    username: data.username,
                    method: 'set',
                    uri: `/dbi/audit/audit_log_${Date.now()}`,
                    replyto: null,
                    body: JSON.stringify(data),
                };
                this.fimsApi.send(msg);
            } catch (e){
                console.log(e);
            }
        }
    }
}

module.exports = AuditLogger;
