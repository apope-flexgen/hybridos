// TODO: fix this rule
/* eslint-disable max-nested-callbacks */
import { Test } from '@nestjs/testing'
import { LoggingService } from './logging.service'
import { LoggerFilter } from './logging_filter/logger-filter.service'

describe('LoggingService', () => {
    let loggingService: LoggingService

    const mockLoggerFilter = {
        requestMayBeLogged: jest.fn(() => {
            return true
        }),
        commonLogToFile: jest.fn(() => {
            return true
        }),
    }
    beforeEach(async () => {
        const moduleRef = await Test.createTestingModule({
            providers: [LoggingService, LoggerFilter],
        }).compile()
        loggingService = await moduleRef.resolve<LoggingService>(LoggingService)
    })
    describe('log', () => {
        it('log should contain string argument', () => {
            const mockMakeLog = jest.spyOn(loggingService, 'makeLog')
            mockMakeLog.mockImplementation(() => null)
            loggingService.log({ message: 'hello' }, '[HTTP]')
            expect(mockMakeLog).toHaveBeenCalled()
            mockMakeLog.mockRestore()
        })
    })

    describe('error', () => {
        it('log should contain string argument', () => {
            const mockMakeLog = jest.spyOn(loggingService, 'makeLog')
            mockMakeLog.mockImplementation(() => null)
            loggingService.error({ message: 'hello' }, '[HTTP]')
            expect(mockMakeLog).toHaveBeenCalled()
            mockMakeLog.mockRestore()
        })
    })
    describe('warn', () => {
        it('log should contain string argument', () => {
            const mockMakeLog = jest.spyOn(loggingService, 'makeLog')
            mockMakeLog.mockImplementation(() => null)
            loggingService.warn({ message: 'hello' }, '[HTTP]')
            expect(mockMakeLog).toHaveBeenCalled()
            mockMakeLog.mockRestore()
        })
    })
    describe('debug', () => {
        it('log should contain string argument', () => {
            const mockMakeLog = jest.spyOn(loggingService, 'makeLog')
            mockMakeLog.mockImplementation(() => null)
            loggingService.debug({ message: 'hello' }, '[HTTP]')
            expect(mockMakeLog).toHaveBeenCalled()
            mockMakeLog.mockRestore()
        })
    })
    describe('verbose', () => {
        it('log should contain string argument', () => {
            const mockMakeLog = jest.spyOn(loggingService, 'makeLog')
            mockMakeLog.mockImplementation(() => null)
            loggingService.verbose({ message: 'hello' }, '[HTTP]')
            expect(mockMakeLog).toHaveBeenCalled()
            mockMakeLog.mockRestore()
        })
    })
    /*
    describe('makeLog', ()=>{
        it('should return true', async ()=>{
            
            expect(loggingService.makeLog("hello","[HTTP]","log")).toContain("hello");
        })
    })
    */
})
