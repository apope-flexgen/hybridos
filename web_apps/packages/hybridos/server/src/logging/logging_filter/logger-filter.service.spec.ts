import { Test } from '@nestjs/testing';
import { LoggerFilter } from './logger-filter.service';

describe('LoggingFilter', () => {
  let loggerFilter: LoggerFilter;
  beforeEach(async () => {
    const moduleRef = await Test.createTestingModule({
      providers: [LoggerFilter],
    }).compile();
    loggerFilter = moduleRef.get<LoggerFilter>(LoggerFilter);
  });
  describe('requestMayBeLogged', () => {
    it('should return true', async () => {
      expect(loggerFilter.requestMayBeLogged('POST', 200, '/test')).toBe(true);
    });
  });
  describe('commonLogToFile', () => {
    it('should return true', async () => {
      const result = 'hello';
      expect(loggerFilter.commonLogToFile('error')).toBe(true);
    });
  });
});
