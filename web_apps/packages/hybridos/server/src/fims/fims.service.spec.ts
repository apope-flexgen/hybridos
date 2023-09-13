import { EventEmitter2 } from '@nestjs/event-emitter';
import { Test, TestingModule } from '@nestjs/testing';
import FimsListener from './FimsListener';
import { FimsService } from './fims.service';
import { FimsMsg } from './interfaces/fims.interface';
import { FimsErrorMessages } from './fims.constants';
import { FIMSSendFailedException, FIMSSendTimedOutException } from './exceptions/fims.exceptions';
import FIMS from '@flexgen/fims';
import {
    FIMS_RESPONSE,
    FIMS_URI,
    FIMS_BODY,
    FIMS_RESP_FAILED,
    FIMS_RESP_TIMED,
    SUB_URI,
    SUB_UID,
    SUB_OBS,
    FIMS_UID
} from './fims.mocks';

describe('FimsService', () => {
    let fimsService: FimsService;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            providers: [
                FimsService,
                {
                    provide: FIMS,
                    useValue: {
                        send: jest.fn(),
                    },
                },
            ],
        }).compile();

        fimsService = module.get<FimsService>(FimsService);

        jest.spyOn(FimsListener.prototype as any, 'open').mockImplementation(jest.fn());
    });

    describe('test get', () => {
        it('should use the arguments', async () => {
            const fimsSend = jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue({} as unknown as FimsMsg);
            await fimsService.get(FIMS_URI, FIMS_BODY);
            expect(fimsSend).toBeCalledWith({
                method: 'get',
                uri: FIMS_URI,
                replyto: '/web_server',
                body: FIMS_BODY,
                username: 'web_server',
            });
        });

        it('should return fims reply', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue(FIMS_RESPONSE);
            expect(await fimsService.get(FIMS_URI, FIMS_BODY)).toEqual(FIMS_RESPONSE);
        });

        it('should accept no body', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue(FIMS_RESPONSE);
            expect(await fimsService.get(FIMS_URI)).toEqual(FIMS_RESPONSE);
        });

        it('should throw failed exception', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue(FIMS_RESP_FAILED);
            expect(async () => {await fimsService.get(FIMS_URI, FIMS_BODY)})
                .rejects
                .toThrow(new FIMSSendFailedException({
                    method: 'get',
                    uri: FIMS_URI,
                    body: FIMS_BODY,
                }));
        });

        it('should throw timed out exception', async () => {
            jest
                .spyOn(fimsService, 'send')
                .mockResolvedValue(FIMS_RESP_TIMED);
            expect(async () => {await fimsService.get(FIMS_URI, FIMS_BODY)})
                .rejects
                .toThrow(new FIMSSendTimedOutException({
                    method: 'get',
                    uri: FIMS_URI,
                    body: FIMS_BODY,
                }));
        });
    });
    
    describe('test send', () => {
        let FIMS_MSG: FimsMsg;
        let eventAddListenerSpy: jest.SpyInstance;
        let getListeningFuncSpy: jest.SpyInstance;
        let fimsSendSpy: jest.SpyInstance;

        beforeEach(() => {
            FIMS_MSG = {
                method: 'get',
                uri: '/example/uri',
                replyto: '/example/reply',
                body: {},
                username: ''
            };
        });

        afterEach(() => {
            jest.clearAllMocks();
            if (eventAddListenerSpy) {
                eventAddListenerSpy.mockRestore();
            }
            if (getListeningFuncSpy) {
                getListeningFuncSpy.mockRestore();
            }
            if (fimsSendSpy) {
                fimsSendSpy.mockRestore();
            }
        });

        it('should use the argument', async () => {
            const fimsSend = jest.spyOn(FIMS, 'send');
            await fimsService.send(FIMS_MSG);
            expect(fimsSend).toBeCalledWith(FIMS_MSG);
        });

        it('should process argument', async () => {
            jest
                .spyOn(fimsService as any, 'newUID')
                .mockReturnValue('42')
            const fimsGetListeningFunc = jest.spyOn(fimsService as any, 'getListeningFunc')
            await fimsService.send(FIMS_MSG);
            expect(fimsGetListeningFunc).toBeCalledWith(
                '/ui_reply/example/reply/42',
                '42',
                expect.any(Function)
            )
        });

        it('should add listener', async () => {
            const respond = (e: any[]) => {};
            getListeningFuncSpy = jest
                .spyOn(fimsService as any, 'getListeningFunc')
                .mockReturnValue(respond);
            eventAddListenerSpy = jest.spyOn(EventEmitter2.prototype as any, 'addListener');
            await fimsService.send(FIMS_MSG);
            expect(eventAddListenerSpy).toBeCalledWith('fims.reply', respond);
        });

        it('should return promise', async () => {
            getListeningFuncSpy = jest
                .spyOn(fimsService as any, 'getListeningFunc')
                .mockImplementation((uri, uid, resolve: any) => {
                    resolve(FIMS_RESPONSE);
                });
            eventAddListenerSpy = jest
                .spyOn(EventEmitter2.prototype as any, 'addListener')
                .mockImplementation(jest.fn());
            fimsSendSpy = jest
                .spyOn(FIMS, 'send')
                .mockReturnValue(true);
            expect(await fimsService.send(FIMS_MSG)).toEqual(FIMS_RESPONSE);
        });

        it('should return error obj', async () => {
            fimsSendSpy = jest
                .spyOn(FIMS, 'send')
                .mockReturnValue(false);
            expect(await fimsService.send(FIMS_MSG)).toEqual({
                method: 'error',
                uri: FIMS_MSG.uri,
                replyto: FIMS_MSG.replyto,
                body: FimsErrorMessages.SEND_FAILED,
                username: FIMS_MSG.username,
            });
        });
    });
    
    describe('test subscribe', () => {
        let isUserAlreadySubscribedSpy: jest.SpyInstance;
        let createSubscribeObservableSpy: jest.SpyInstance;
        let registerSubscriptionSpy: jest.SpyInstance;

        afterEach(() => {
            if (isUserAlreadySubscribedSpy) {
                isUserAlreadySubscribedSpy.mockRestore();
            }
            if (createSubscribeObservableSpy) {
                createSubscribeObservableSpy.mockRestore();
            }
            if (registerSubscriptionSpy) {
                registerSubscriptionSpy.mockRestore();
            }
        });

        it('should return from subscribed', () => {
            const req = {
                _socket: {
                    remoteAddress: '',
                    remotePort: '',
                },
            };
            isUserAlreadySubscribedSpy = jest
                .spyOn(fimsService as any, 'isUserAlreadySubscribed')
                .mockReturnValue(true);
            expect(fimsService.subscribe(SUB_URI, req)).toBeUndefined();
        });

        it('should use arguments', () => {
            const req = {
                _socket: {
                    remoteAddress: 'myAddress',
                    remotePort: 'myPort',
                },
            };
            isUserAlreadySubscribedSpy = jest
                .spyOn(fimsService as any, 'isUserAlreadySubscribed')
                .mockReturnValue(true);
            fimsService.subscribe(SUB_URI, req);
            expect(isUserAlreadySubscribedSpy).toHaveBeenCalledWith(
                req._socket.remoteAddress + req._socket.remotePort,
                SUB_URI,
            );
        });

        it('should create observable', () => {
            const req = {
                _socket: {
                    remoteAddress: 'myAddress',
                    remotePort: 'myPort',
                },
            };
            isUserAlreadySubscribedSpy = jest
                .spyOn(fimsService as any, 'isUserAlreadySubscribed')
                .mockReturnValue(false);
            createSubscribeObservableSpy = jest
                .spyOn(fimsService as any, 'createSubscribeObservable')
                .mockReturnValue(SUB_OBS)
                .mockImplementation(jest.fn());
            fimsService.subscribe(SUB_URI, req);
            expect(createSubscribeObservableSpy).toHaveBeenCalledWith(SUB_URI, req);
        });

        it('should register', () => {
            const req = {
                _socket: {
                    remoteAddress: 'myAddress',
                    remotePort: 'myPort',
                },
            };
            isUserAlreadySubscribedSpy = jest
                .spyOn(fimsService as any, 'isUserAlreadySubscribed')
                .mockReturnValue(false);
            createSubscribeObservableSpy = jest
                .spyOn(fimsService as any, 'createSubscribeObservable')
                .mockReturnValue(SUB_OBS)
                .mockImplementation(jest.fn());
            registerSubscriptionSpy = jest
                .spyOn(fimsService as any, 'registerSubscription');
            fimsService.subscribe(SUB_URI, req);
            expect(registerSubscriptionSpy).toHaveBeenCalledWith(
                SUB_URI,
                req._socket.remoteAddress + req._socket.remotePort,
                undefined,
            );
        });
    });

    describe('test isUserAlreadySubscribed', () => {
        let mapHasSpy: jest.SpyInstance;
        let mapGetSpy: jest.SpyInstance;

        afterEach(() => {
            mapHasSpy.mockRestore();
            mapGetSpy.mockRestore();
        });

        it('should have uid and uri', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === SUB_UID) {
                        return true;
                    } else {
                        return true;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            expect(fimsService.isUserAlreadySubscribed(SUB_UID, SUB_URI)).toEqual(true);
        });

        it('should have uid and but not uri', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === SUB_UID) {
                        return true;
                    } else {
                        return false;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            expect(fimsService.isUserAlreadySubscribed(SUB_UID, SUB_URI)).toEqual(false);
        });

        it('should have not uid but yes uri', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === SUB_UID) {
                        return false;
                    } else {
                        return true;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            expect(fimsService.isUserAlreadySubscribed(SUB_UID, SUB_URI)).toEqual(false);
        });

        it('should have not uid or uri', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === SUB_UID) {
                        return false;
                    } else {
                        return false;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            expect(fimsService.isUserAlreadySubscribed(SUB_UID, SUB_URI)).toEqual(false);
        });
    });

    describe('test registerSubscription', () => {
        let mapHasSpy: jest.SpyInstance;
        let mapGetSpy: jest.SpyInstance;
        let mapSetSpy: jest.SpyInstance;

        afterEach(() => {
            mapHasSpy.mockRestore();
            mapSetSpy.mockRestore();
            if (mapGetSpy) {
                mapGetSpy.mockRestore();
            }
        });

        it('should use args with existing URI set', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockReturnValue(true);
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            mapSetSpy = jest.spyOn(Map.prototype as any, 'set');
            fimsService.registerSubscription(SUB_URI, SUB_UID, SUB_OBS);
            expect(mapGetSpy).toHaveBeenCalledWith(SUB_UID);
            expect(mapSetSpy).toHaveBeenCalledWith(SUB_URI, SUB_OBS);
        });

        it('should create URI set', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockReturnValue(false);
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            mapSetSpy = jest
                .spyOn(Map.prototype as any, 'set')
                .mockImplementation(jest.fn());
            fimsService.registerSubscription(SUB_URI, SUB_UID, SUB_OBS);
            expect(mapSetSpy.mock.calls[0]).toEqual([SUB_UID, new Map()]);
        });
    });

    describe('test openListener', () => {
        let fimsListenerOpenSpy: jest.SpyInstance;

        afterEach(() => {
            fimsListenerOpenSpy.mockRestore();
        });

        it('should call open', () => {
            fimsListenerOpenSpy = jest
                .spyOn(FimsListener.prototype as any, 'open');
            fimsService['openListener']();
            expect(fimsListenerOpenSpy).toBeCalled();
        });

        it('should return after opened', () => {
            fimsListenerOpenSpy = jest
                .spyOn(FimsListener.prototype as any, 'open')
                .mockReturnValue(true);
            expect(fimsService['openListener']()).toEqual(true);
        });

        it('should return after error', () => {
            fimsListenerOpenSpy = jest
                .spyOn(FimsListener.prototype as any, 'open')
                .mockReturnValue(false);
            expect(fimsService['openListener']()).toEqual(false);
        });
    });

    describe('test newUID', () => {
        const MY_UID = 'user123';

        let stringSubstrSpy: jest.SpyInstance;
        let uidHasSpy: jest.SpyInstance;
        let uidAddSpy: jest.SpyInstance;

        afterEach(() => {
            stringSubstrSpy.mockRestore();
            uidHasSpy.mockRestore();
            if (uidAddSpy) {
                uidAddSpy.mockRestore();
            }
        });

        it('should check for generated newUID', () => {
            stringSubstrSpy = jest
                .spyOn(String.prototype as any, 'substr')
                .mockReturnValue(MY_UID);
            uidHasSpy = jest.spyOn(Set.prototype as any, 'has');
            fimsService['newUID']();
            expect(uidHasSpy).toHaveBeenCalledWith(MY_UID);
        });

        it('should add newUID', () => {
            stringSubstrSpy = jest
                .spyOn(String.prototype as any, 'substr')
                .mockReturnValue(MY_UID);
            uidAddSpy = jest.spyOn(Set.prototype as any, 'add');
            fimsService['newUID']();
            expect(uidAddSpy).toHaveBeenCalledWith(MY_UID);
        });

        it('should recurse for existing URI', () => {
            // this mock setup prevents too much recursion from failing test
            let recurseUid = 1;
            stringSubstrSpy = jest
                .spyOn(String.prototype as any, 'substr')
                .mockImplementation((x, y) => {
                    return (recurseUid--).toString();
                });
            uidHasSpy = jest
                .spyOn(Set.prototype as any, 'has')
                .mockImplementation((uid: string) => {
                    if (parseInt(uid) <= 0) {
                        return false;
                    } else {
                        return true;
                    }
                });
            const newUidSpy = jest.spyOn(fimsService as any, 'newUID');
            uidAddSpy = jest.spyOn(Set.prototype as any, 'add');
            fimsService['newUID']();
            expect(newUidSpy).toHaveBeenCalledTimes(2);
            expect(uidAddSpy).toHaveBeenCalledTimes(1);
        });
    });

    describe('test getListeningFunc', () => {
        const resolveCallback = jest.fn();
        let eventRemoveListenerSpy: jest.SpyInstance;
        let uidDeleteSpy: jest.SpyInstance;

        beforeAll(() => {
            jest.useFakeTimers();
        });

        afterAll(() => {
            jest.useRealTimers();
        });

        afterEach(() => {
            if (eventRemoveListenerSpy) {
                eventRemoveListenerSpy.mockRestore();
            }
            if (uidDeleteSpy) {
                uidDeleteSpy.mockRestore();
            }
            jest.clearAllMocks();
        });

        it('should respond without issue', () => {
            eventRemoveListenerSpy = jest
                .spyOn(EventEmitter2.prototype as any, 'removeListener')
                .mockImplementation(jest.fn());
            uidDeleteSpy = jest
                .spyOn(Set.prototype as any, 'delete')
                .mockImplementation(jest.fn());
            const respond = fimsService['getListeningFunc'](FIMS_URI, FIMS_UID, resolveCallback);
            const e = { uri: FIMS_URI };
            respond(e);
            expect(eventRemoveListenerSpy).toHaveBeenCalledWith('fims.reply', respond)
            expect(uidDeleteSpy).toHaveBeenCalledWith(FIMS_UID);
            expect(resolveCallback).toHaveBeenCalled();
        });

        it('should not callback, uri not root', () => {
            const respond = fimsService['getListeningFunc'](FIMS_URI, FIMS_UID, resolveCallback);
            const e = { uri: '' };
            respond(e);
            expect(resolveCallback).not.toHaveBeenCalled();
        });

        it('should not callback, invalid e', () => {
            const respond = fimsService['getListeningFunc'](FIMS_URI, FIMS_UID, resolveCallback);
            const e = { invalid: 'e' };
            respond(e);
            expect(resolveCallback).not.toHaveBeenCalled();
        });

        it('should call timeout', () => {
            eventRemoveListenerSpy = jest
                .spyOn(EventEmitter2.prototype as any, 'removeListener')
                .mockImplementation(jest.fn());
            uidDeleteSpy = jest
                .spyOn(Set.prototype as any, 'delete')
                .mockImplementation(jest.fn());
            const respond = fimsService['getListeningFunc'](FIMS_URI, FIMS_UID, resolveCallback);
            expect(eventRemoveListenerSpy).not.toHaveBeenCalled();
            expect(uidDeleteSpy).not.toHaveBeenCalled();
            expect(resolveCallback).not.toHaveBeenCalled();
            jest.runAllTimers();
            expect(eventRemoveListenerSpy).toHaveBeenCalledWith('fims.message', respond)
            expect(uidDeleteSpy).toHaveBeenCalledWith(FIMS_UID);
            expect(resolveCallback).toHaveBeenCalledWith({
                method: 'error',
                uri: FIMS_URI,
                replyto: 'web_server',
                body: FimsErrorMessages.SEND_TIMED_OUT,
                username: 'error',
            });
        });
    });

    describe('test createSubscribeObservable', () => {
        let fromEventSpy: jest.SpyInstance;

        beforeAll(() => {
        });

        afterEach(() => {
            if (fromEventSpy) {
                fromEventSpy.mockRestore();
            }
        });

        it('should handle no events', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, falsy', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', 'invalid');
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, blank', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {});
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, no method', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                uri: FIMS_URI,
            });
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, wrong method', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'notPub',
                uri: FIMS_URI,
            });
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, no uri', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
            });
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle invalid event, wrong uri', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
                uri: '/notRoot'
            });
            expect(eventList).toEqual([]);
            sub.unsubscribe();
        });

        it('should handle valid event', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
                uri: FIMS_URI
            });
            expect(eventList).toEqual([{
                method: 'pub',
                uri: FIMS_URI,
            }]);
            sub.unsubscribe();
        });

        it('should handle 2 valid events', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
                uri: FIMS_URI
            });
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
                uri: FIMS_URI + '/extra/content'
            });
            expect(eventList).toEqual([
                {
                    method: 'pub',
                    uri: FIMS_URI,
                },
                {
                    method: 'pub',
                    uri: FIMS_URI + '/extra/content',
                },
            ]);
            sub.unsubscribe();
        });

        it('should handle 1 valid & 1 invalid event', () => {
            let eventList = [];
            let obs = fimsService['createSubscribeObservable'](FIMS_URI);
            let sub = obs.subscribe((e) => {
                eventList.push(e);
            })
            fimsService.eventEmitter.emit('fims.message', {
                method: 'pub',
                uri: FIMS_URI
            });
            fimsService.eventEmitter.emit('fims.message', {});
            expect(eventList).toEqual([
                {
                    method: 'pub',
                    uri: FIMS_URI,
                },
            ]);
            sub.unsubscribe();
        });

        it('should close request', () => {
            let onSpy = jest.fn();
            const request = {
                on: onSpy
            }
            fimsService['createSubscribeObservable'](FIMS_URI, request);
            expect(onSpy.mock.calls).toEqual([
                ['close', expect.any(Function)],
                ['error', expect.any(Function)],
            ]);
        });
    });

    describe('test doUnsubscribeCleanup', () => {
        const UID = 'abc123';
        const URI = '/example/uri';

        let mapHasSpy: jest.SpyInstance;
        let mapGetSpy: jest.SpyInstance;
        let mapDeleteSpy: jest.SpyInstance;

        afterEach(() => {
            mapHasSpy.mockRestore();
            mapGetSpy.mockRestore();
            if (mapDeleteSpy) {
                mapDeleteSpy.mockRestore();
            }
        });

        it('should do nothing', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockReturnValue(false);
            mapGetSpy = jest.spyOn(Map.prototype as any, 'get');
            fimsService['doUnsubscribeCleanup'](UID, URI);
            expect(mapGetSpy).not.toBeCalled();
        });

        it('should not delete anything', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === UID) {
                        return true;
                    } else {
                        return false;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map([['nonEmpty', 'map']]));
            mapDeleteSpy = jest.spyOn(Map.prototype as any, 'delete');
            fimsService['doUnsubscribeCleanup'](UID, URI);
            expect(mapDeleteSpy).not.toBeCalled();
        });

        it('should delete "uri from user" but not "user"', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockReturnValue(true);
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map([[URI, 'some observable'], ['something', 'else']]));
            mapDeleteSpy = jest.spyOn(Map.prototype as any, 'delete');
            fimsService['doUnsubscribeCleanup'](UID, URI);
            expect(mapDeleteSpy).toHaveBeenCalledWith(URI);
            expect(mapDeleteSpy).toHaveBeenCalledTimes(1);
        });

        it('should delete "uri from user" and "user"', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockReturnValue(true);
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map([[URI, 'some observable']]));
            mapDeleteSpy = jest.spyOn(Map.prototype as any, 'delete');
            fimsService['doUnsubscribeCleanup'](UID, URI);
            expect(mapDeleteSpy).toHaveBeenCalledTimes(2);
        });

        it('should delete "user" without URIs', () => {
            mapHasSpy = jest
                .spyOn(Map.prototype as any, 'has')
                .mockImplementation((input) => {
                    if (input === UID) {
                        return true;
                    } else {
                        return false;
                    }
                });
            mapGetSpy = jest
                .spyOn(Map.prototype as any, 'get')
                .mockReturnValue(new Map());
            mapDeleteSpy = jest.spyOn(Map.prototype as any, 'delete');
            fimsService['doUnsubscribeCleanup'](UID, URI);
            expect(mapDeleteSpy).toHaveBeenCalledWith(UID);
            expect(mapDeleteSpy).toHaveBeenCalledTimes(1);
        });
    });
});