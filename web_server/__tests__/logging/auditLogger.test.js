const { JsonWebTokenError } = require("jsonwebtoken");
const auditLogger = require("../../src/logging/auditLogger")

class mockFims {
    constructor(){}
    send = jest.fn()
}

const mockData = {
    username:"linus",
    userrole:"maker",
    modified_field:"reality",
    modified_value:"os",
}

const mockDataFail = {
    userrole:"maker",
    modified_field:"reality",
    modified_value:"os",
}

describe("audit logging tests", ()=>{
    test('audit logger makes a log with COMPLETE ARGS and calls fims send', () => {
        mock = new mockFims()
        logger = new auditLogger(mock);
        result = logger.send(mockData)
        expect(mock.send).toHaveBeenCalled();
    });
    test('audit logger has INCOMPLETE args then tries to call fims send', () => {
        mock = new mockFims()
        logger = new auditLogger(mock);
        result = logger.send(mockDataFail)
        expect(mock.send).not.toHaveBeenCalled();
    });
})
