import { Test, TestingModule } from '@nestjs/testing';
import { HttpThrottlerGuard } from './httpthrottler.guard';
import { ThrottlerGuard, ThrottlerModule, ThrottlerStorage } from '@nestjs/throttler';
import { LIMIT, TTL } from './fims.mocks';

describe('HttpThrottlerGuard', () => {
    let guard: HttpThrottlerGuard;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            imports: [
                ThrottlerModule.forRoot()
            ],
            providers: [
                HttpThrottlerGuard,
            ],
        }).compile();

        guard = module.get<HttpThrottlerGuard>(HttpThrottlerGuard);
    });

    it('should be defined', () => {
        expect(guard).toBeDefined();
    });

    describe('test handleRequest', () => {
        afterEach(() => {
            jest.clearAllMocks();
        });

        it('should use context arg', async () => {
            const mockExecutionContext = {
                switchToHttp: () => ({
                    getRequest: () => ({ ip: '192.168.0.0' }),
                    getResponse: () => jest.fn(),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            const keySpy = jest.spyOn(ThrottlerGuard.prototype as any, 'generateKey');
            await guard.handleRequest(mockExecutionContext as any, LIMIT, TTL);
            expect(keySpy).toHaveBeenCalledWith(mockExecutionContext, mockExecutionContext.switchToHttp().getRequest().ip);
        });

        // cannot mock this.storageService, so cannot test remainer of guard
    });

});