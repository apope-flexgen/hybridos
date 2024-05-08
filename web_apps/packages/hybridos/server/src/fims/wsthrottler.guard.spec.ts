import { Test, TestingModule } from '@nestjs/testing';
import { WsThrottlerGuard } from './wsthrottler.guard';
import { ThrottlerGuard, ThrottlerModule } from '@nestjs/throttler';
import { LIMIT, TTL } from './fims.mocks';

describe('WsThrottlerGuard', () => {
  let guard: WsThrottlerGuard;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [ThrottlerModule.forRoot()],
      providers: [WsThrottlerGuard],
    }).compile();

    guard = module.get<WsThrottlerGuard>(WsThrottlerGuard);
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
        switchToWs: () => ({
          getClient: () => ({
            _socket: {
              remoteAddress: '192.168.0.0',
            },
          }),
        }),
        getClass: () => jest.fn(),
        getHandler: () => jest.fn(),
      };
      const keySpy = jest.spyOn(ThrottlerGuard.prototype as any, 'generateKey');
      await guard.handleRequest(mockExecutionContext as any, LIMIT, TTL);
      expect(keySpy).toHaveBeenCalledWith(
        mockExecutionContext,
        mockExecutionContext.switchToWs().getClient()._socket.remoteAddress,
      );
    });

    // cannot mock this.storageService, so cannot test remainder of guard
  });
});
