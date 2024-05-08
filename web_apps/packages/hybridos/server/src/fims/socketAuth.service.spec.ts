import { Test, TestingModule } from '@nestjs/testing';
import { SocketAuthService } from './socketAuth.service';
import { JwtService } from '@nestjs/jwt';
import { TOKEN, USER, USER_NO_UNAME } from './fims.mocks';

describe('SocketAuth', () => {
  let socketAuthService: SocketAuthService;
  let jwtService: JwtService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        SocketAuthService,
        {
          provide: JwtService,
          useValue: {
            sign: jest.fn(),
            verify: jest.fn(),
          },
        },
      ],
    }).compile();

    socketAuthService = module.get<SocketAuthService>(SocketAuthService);
    jwtService = module.get<JwtService>(JwtService);
  });

  it('socketAuthService should be defined', () => {
    expect(socketAuthService).toBeDefined();
  });

  it('jwtService should be defined', () => {
    expect(jwtService).toBeDefined();
  });

  describe('test addToken', () => {
    it('should use the token', () => {
      const addTokenMock = jest.spyOn(socketAuthService, 'addToken');
      socketAuthService.addToken(TOKEN);
      expect(addTokenMock).toHaveBeenCalledWith(TOKEN);
    });

    it('should return void', () => {
      expect(socketAuthService.addToken(TOKEN)).toBeUndefined();
    });
  });

  describe('test invalidateToken', () => {
    it('should use the token', () => {
      const invalidateTokenMock = jest.spyOn(socketAuthService, 'invalidateToken');
      socketAuthService.invalidateToken(TOKEN);
      expect(invalidateTokenMock).toHaveBeenCalledWith(TOKEN);
    });

    it('should return', () => {
      const invalidateTokenMock = jest.spyOn(socketAuthService, 'invalidateToken');
      socketAuthService.invalidateToken(TOKEN);
      expect(invalidateTokenMock).toHaveReturned();
    });
  });

  describe('test containsToken', () => {
    it('should use the token', () => {
      const containsTokenMock = jest.spyOn(socketAuthService, 'containsToken');
      socketAuthService.containsToken(TOKEN);
      expect(containsTokenMock).toHaveBeenCalledWith(TOKEN);
    });

    it('should return', () => {
      const containsTokenMock = jest.spyOn(socketAuthService, 'containsToken');
      socketAuthService.containsToken(TOKEN);
      expect(containsTokenMock).toHaveReturned();
    });
  });

  describe('test generateToken', () => {
    it('should use the user', () => {
      // Redefine read only property of performance.now()
      Object.defineProperty(performance, 'now', {
        value: jest.fn(),
        configurable: true,
        writable: true,
      });
      jest.spyOn(performance, 'now').mockReturnValue(123);
      const jwtServiceSign = jest.spyOn(jwtService, 'sign');
      socketAuthService.generateToken(USER);
      expect(jwtServiceSign).toHaveBeenCalledWith({
        role: USER.role,
        oneTime: 'fims-socket',
        sub: USER.username,
        nonce: performance.now(),
      });
    });

    it('should return a token', () => {
      jest.spyOn(jwtService, 'sign').mockReturnValue(TOKEN);
      expect(socketAuthService.generateToken(USER)).toEqual(TOKEN);
    });

    it('should handle optional username', () => {
      // Redefine read only property of performance.now()
      Object.defineProperty(performance, 'now', {
        value: jest.fn(),
        configurable: true,
        writable: true,
      });
      jest.spyOn(performance, 'now').mockReturnValue(123);
      const jwtServiceSign = jest.spyOn(jwtService, 'sign');
      socketAuthService.generateToken(USER_NO_UNAME);
      expect(jwtServiceSign).toHaveBeenCalledWith({
        role: USER_NO_UNAME.role,
        oneTime: 'fims-socket',
        sub: USER_NO_UNAME.username,
        nonce: performance.now(),
      });
    });
  });

  describe('test validateToken', () => {
    it('should use the token', () => {
      const jwtServiceVerify = jest.spyOn(jwtService, 'verify');
      socketAuthService.validateToken(TOKEN);
      expect(jwtServiceVerify).toHaveBeenCalledWith(TOKEN);
    });

    it('should return after error', async () => {
      jest.spyOn(jwtService, 'verify').mockImplementation(() => new Error('Verify error'));
      expect(socketAuthService.validateToken(TOKEN)).toBeFalsy();
    });

    it('should return invalidateToken if containsToken', () => {
      jest.spyOn(socketAuthService, 'containsToken').mockReturnValue(true);
      jest.spyOn(socketAuthService, 'invalidateToken').mockReturnValue(true);
      expect(socketAuthService.validateToken(TOKEN)).toBeTruthy();
    });

    it('should call return false if not containsToken', () => {
      jest.spyOn(socketAuthService, 'containsToken').mockReturnValue(false);
      expect(socketAuthService.validateToken(TOKEN)).toBeFalsy();
    });
  });
});
