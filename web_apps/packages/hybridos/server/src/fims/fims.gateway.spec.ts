import { Test, TestingModule } from '@nestjs/testing';
import { FimsGateway } from './fims.gateway';
import { FIMS_SERVICE, FimsMsg, IFimsService } from './interfaces/fims.interface';
import { ThrottlerModule } from '@nestjs/throttler';
import { PERMISSIONS_SERVICE } from '../permissions/interfaces/permission.interface';
import { LoggingService } from '../logging/logging.service';
import { Observable } from 'rxjs';
import { FIMS_RESPONSE, FIMS_MSG, SUB_URI, SUB_REQ } from './fims.mocks';

describe('FimsGateway', () => {
    let fimsGateway: FimsGateway;
    let fimsService: IFimsService;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            imports: [
                ThrottlerModule.forRoot()
            ],
            providers: [
                FimsGateway,
                {
                    provide: FIMS_SERVICE,
                    useValue: {
                        send: jest.fn(),
                        subscribe: jest.fn(),
                    },
                },
                {
                    provide: PERMISSIONS_SERVICE,
                    useValue: {
                        ConfirmRoleAccess: jest.fn(),
                    },
                },
                {
                    provide: LoggingService,
                    useValue: {}
                },
            ],
        }).compile();

        fimsGateway = module.get<FimsGateway>(FimsGateway);
        fimsService = module.get<IFimsService>(FIMS_SERVICE);
    });

    it('fimsService should be defined', () => {
        expect(fimsService).toBeDefined();
    });

    describe('test request', () => {
        it('should use the data', async () => {
            const fimsServiceSend = jest.spyOn(fimsService, 'send');
            await fimsGateway.request(FIMS_MSG);
            expect(fimsServiceSend).toHaveBeenCalledWith({
                method: FIMS_MSG.method,
                uri: FIMS_MSG.uri,
                replyto: FIMS_MSG.replyto,
                body: FIMS_MSG.body,
                username: FIMS_MSG.username,
            } as FimsMsg);
        });

        it('should should return fims response', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue(FIMS_RESPONSE);
            expect(await fimsGateway.request(FIMS_MSG)).toEqual(FIMS_RESPONSE);
        });

        it('should log an exception', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockImplementation(() => {throw new Error('Example Fims Send Error')});
            const consoleLog = jest.spyOn(console, 'log');
            await fimsGateway.request(FIMS_MSG);
            expect(consoleLog).toHaveBeenCalled();
        });
    });

    describe('test fimsNoReply', () => {
        it('should use the data', async () => {
            const fimsServiceSend = jest.spyOn(fimsService, 'send');
            await fimsGateway.fimsNoReply(FIMS_MSG);
            expect(fimsServiceSend).toHaveBeenCalledWith({
                method: FIMS_MSG.method,
                uri: FIMS_MSG.uri,
                replyto: FIMS_MSG.replyto,
                body: FIMS_MSG.body,
                username: FIMS_MSG.username,
            } as FimsMsg);
        });

        it('should log an exception', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockImplementation(() => {throw new Error('Example Fims Send Error')});
            const consoleLog = jest.spyOn(console, 'log');
            await fimsGateway.fimsNoReply(FIMS_MSG);
            expect(consoleLog).toHaveBeenCalled();
        });
    });

    describe('test subscribe', () => {
        it('should use the arguments', () => {
            const fimsServiceSub = jest.spyOn(fimsService, 'subscribe');
            fimsGateway.subscribe(SUB_URI, SUB_REQ)
            expect(fimsServiceSub).toHaveBeenCalledWith(SUB_URI, SUB_REQ);
        });

        it('should return observable', () => {
            jest
                .spyOn(fimsService, 'subscribe')
                .mockReturnValue(new Observable());
            expect(fimsGateway.subscribe(SUB_URI, SUB_REQ)).toEqual(new Observable())
        });
    });
});