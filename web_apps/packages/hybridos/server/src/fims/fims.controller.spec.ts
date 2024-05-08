import { Test, TestingModule } from '@nestjs/testing';
import { FIMS_SERVICE, IFimsService } from './interfaces/fims.interface';
import { ISocketAuthService, SOCKET_AUTH_SERVICE } from './interfaces/socketAuth.service.interface';
import { FimsController } from './fims.controller';
import { ThrottlerModule } from '@nestjs/throttler';
import { PERMISSIONS_SERVICE } from '../permissions/interfaces/permission.interface';
import { AUTH_REQ, FIMS_RESPONSE, FIMS_GET, FIMS_SEND } from './fims.mocks';
import { LoggingService } from '../logging/logging.service';

describe('FimsController', () => {
  let fimsService: IFimsService;
  let socketAuthService: ISocketAuthService;
  let fimsController: FimsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [ThrottlerModule.forRoot()],
      controllers: [FimsController],
      providers: [
        {
          provide: FIMS_SERVICE,
          useValue: {
            send: jest.fn(),
          },
        },
        {
          provide: SOCKET_AUTH_SERVICE,
          useValue: {
            generateToken: jest.fn(),
          },
        },
        {
          provide: PERMISSIONS_SERVICE,
          useValue: {
            ConfirmRoleAccess: jest.fn(),
          },
        },
        LoggingService,
      ],
    }).compile();

    fimsController = module.get<FimsController>(FimsController);
    fimsService = module.get<IFimsService>(FIMS_SERVICE);
    socketAuthService = module.get<ISocketAuthService>(SOCKET_AUTH_SERVICE);
  });

  it('fimsService should be defined', () => {
    expect(fimsService).toBeDefined();
  });

  it('socketAuthService should be defined', () => {
    expect(socketAuthService).toBeDefined();
  });

  describe('test oneTimeAuth', () => {
    it('should use argument', async () => {
      const generateTokenService = jest.spyOn(socketAuthService, 'generateToken');
      await fimsController.oneTimeAuth(AUTH_REQ);
      expect(generateTokenService).toHaveBeenCalledWith('username');
    });

    it('should return token', async () => {
      jest.spyOn(socketAuthService, 'generateToken').mockReturnValue('exampleToken');
      expect(await fimsController.oneTimeAuth(AUTH_REQ)).toMatchObject({
        token: 'exampleToken',
      });
    });
  });

  describe('test get', () => {
    it('should use the argument', async () => {
      const fimsSendService = jest.spyOn(fimsService, 'send');
      await fimsController.get(FIMS_GET);
      expect(fimsSendService).toHaveBeenCalledWith({
        method: 'get',
        uri: FIMS_GET.uri,
        replyto: FIMS_GET.replyto,
        body: '',
        username: '',
      });
    });

    it('should return fimsmsg', async () => {
      jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      expect(await fimsController.get(FIMS_GET)).toMatchObject(FIMS_RESPONSE);
    });
  });

  describe('test set', () => {
    it('should use the argument', async () => {
      const fimsSendService = jest.spyOn(fimsService, 'send');
      await fimsController.set(FIMS_SEND);
      expect(fimsSendService).toHaveBeenCalledWith({
        method: 'set',
        uri: FIMS_SEND.uri,
        replyto: FIMS_SEND.replyto,
        body: FIMS_SEND.body,
        username: '',
      });
    });

    it('should return fimsmsg', async () => {
      jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      expect(await fimsController.set(FIMS_SEND)).toMatchObject(FIMS_RESPONSE);
    });
  });

  describe('test post', () => {
    it('should use the argument', async () => {
      const fimsSendService = jest.spyOn(fimsService, 'send');
      await fimsController.post(FIMS_SEND);
      expect(fimsSendService).toHaveBeenCalledWith({
        method: 'post',
        uri: FIMS_SEND.uri,
        replyto: FIMS_SEND.replyto,
        body: FIMS_SEND.body,
        username: '',
      });
    });

    it('should return fimsmsg', async () => {
      jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      expect(await fimsController.post(FIMS_SEND)).toMatchObject(FIMS_RESPONSE);
    });
  });

  describe('test delete', () => {
    it('should use the argument', async () => {
      const fimsSendService = jest.spyOn(fimsService, 'send');
      await fimsController.delete(FIMS_SEND);
      expect(fimsSendService).toHaveBeenCalledWith({
        method: 'del',
        uri: FIMS_SEND.uri,
        replyto: FIMS_SEND.replyto,
        body: FIMS_SEND.body,
        username: '',
      });
    });

    it('should return fimsmsg', async () => {
      jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      expect(await fimsController.delete(FIMS_SEND)).toMatchObject(FIMS_RESPONSE);
    });
  });
});
