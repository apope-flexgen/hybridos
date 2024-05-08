import { Test, TestingModule } from '@nestjs/testing';
import { HttpThrottleExceptionFilter } from './httpthrottler.filter';
import { HttpStatus, Response } from '@nestjs/common';
import { ThrottlerException } from '@nestjs/throttler';
import { LoggingService } from '../logging/logging.service';
import { CoreHttpExceptionsFilter } from '../filters/core.http.exception.filter';

describe('HttpThrottleExceptionFilter', () => {
  let filter: HttpThrottleExceptionFilter;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [HttpThrottleExceptionFilter, LoggingService],
    }).compile();

    filter = module.get<HttpThrottleExceptionFilter>(HttpThrottleExceptionFilter);
  });

  it('should be defined', () => {
    expect(filter).toBeDefined();
  });

  describe('test catch', () => {
    it('should use arguments', () => {
      // only mock data requred by class
      const mockExecutionContext = {
        switchToHttp: () => ({
          getRequest: () => ({ url: '/mocked/request/url' }),
          getResponse: () => ({
            status: () => ({ json: () => {} }),
          }),
        }),
        getType: () => jest.fn(),
      };
      const mockException = new ThrottlerException();
      const catchSpy = jest.spyOn(filter, 'catch');
      filter.catch(mockException, mockExecutionContext as any);
      expect(catchSpy).toHaveBeenCalledWith(mockException, mockExecutionContext);
    });

    it('should format json properly', () => {
      const jsonFunc = jest.fn();
      const mockExecutionContext = {
        switchToHttp: () => ({
          getRequest: () => ({ url: '/mocked/request/url' }),
          getResponse: () => ({
            status: () => ({ json: jsonFunc }),
          }),
        }),
        getType: () => jest.fn(),
      };
      const mockException = new ThrottlerException();
      jest.spyOn(Date.prototype as any, 'toISOString').mockReturnValue(42);
      filter.catch(mockException, mockExecutionContext as any);
      expect(jsonFunc).toHaveBeenCalledWith({
        statusCode: 429,
        timestamp: new Date().toISOString(),
        path: '/mocked/request/url',
        error: 'Too many messages',
      });
    });
  });
});
