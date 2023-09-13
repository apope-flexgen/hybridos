import { Test, TestingModule } from '@nestjs/testing';
import { WsThrottleExceptionFilter } from './wsthrottler.filter';
import { WsException } from '@nestjs/websockets';
import { EXCEPTION_MSG } from './fims.mocks';

describe('WsThrottleExceptionFilter', () => {
    let filter: WsThrottleExceptionFilter;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            providers: [
                WsThrottleExceptionFilter,
            ],
        }).compile();

        filter = module.get<WsThrottleExceptionFilter>(WsThrottleExceptionFilter);
    });

    it('should be defined', () => {
        expect(filter).toBeDefined();
    });

    describe('test catch', () => {
        it('should use arguments', () => {
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        send: () => jest.fn(),
                    }),
                }),
            };
            const mockException = new WsException(EXCEPTION_MSG);
            const consoleLogSpy = jest.spyOn(console, 'log');
            filter.catch(mockException, mockExecutionContext as any);
            expect(consoleLogSpy).toHaveBeenCalledWith('ws exception: ', EXCEPTION_MSG);
        });

        it('should format output', () => {
            const sendSpy = jest.fn();
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({ send: sendSpy })
                }),
            };
            const mockException = new WsException(EXCEPTION_MSG);
            filter.catch(mockException, mockExecutionContext as any);
            expect(sendSpy).toHaveBeenCalledWith(JSON.stringify({
                error: EXCEPTION_MSG
            }));
        });
    });
});