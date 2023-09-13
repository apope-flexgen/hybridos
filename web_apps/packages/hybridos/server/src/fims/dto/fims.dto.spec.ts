import { ArgumentMetadata, ValidationPipe } from '@nestjs/common';
import { BodyDTO, FimsMsgDTO, GetDTO } from './fims.dto';

describe('FimsDTO', () => {
    describe('validate GetDTO', () => {
        let target: ValidationPipe;
        const metadata: ArgumentMetadata = {
            type: 'body',
            metatype: GetDTO,
            data: ''
        };

        beforeEach(() => {
            target = new ValidationPipe({
                transform: true, whitelist: true,
            });
        });

        it('validate normal', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            expect(await target.transform(testObj, metadata)).toEqual(testObj);
        });

        it('validate invalid key', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
                invalidKey: 'invalid value',
            };
            const resultObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            expect(await target.transform(testObj, metadata)).toEqual(resultObj);
        });

        it('validate empty', async () => {
            const testObj = {};
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                            "uri should not be empty",
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate uri type', async () => {
            const testObj = {
                uri: 5,
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                        ])
                });
        });

        it('validate uri empty', async () => {
            const testObj = {
                uri: '',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri should not be empty",
                        ])
                });
        });

        it('validate uri absent', async () => {
            const testObj = {
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                            "uri should not be empty",
                        ])
                });
        });

        it('validate replyto type', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: 5,
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                        ])
                });
        });

        it('validate replyto empty', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate replyto absent', async () => {
            const testObj = {
                uri: '/example/uri',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });
    });

    describe('validate BodyDTO', () => {
        let target: ValidationPipe;
        const metadata: ArgumentMetadata = {
            type: 'body',
            metatype: BodyDTO,
            data: ''
        };

        beforeEach(() => {
            target = new ValidationPipe({
                transform: true, whitelist: true,
            });
        });

        it('validate normal', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
                body: 'body',
            };
            expect(await target.transform(testObj, metadata)).toEqual(testObj);
        });

        it('validate invalid key', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
                body: 'body',
                invalidKey: 'invalid value',
            };
            const resultObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
                body: 'body',
            };
            expect(await target.transform(testObj, metadata)).toEqual(resultObj);
        });

        it('validate empty', async () => {
            const testObj = {};
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                            "uri should not be empty",
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate uri type', async () => {
            const testObj = {
                uri: 5,
                replyto: '/example/replyto',
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                        ])
                });
        });

        it('validate uri empty', async () => {
            const testObj = {
                uri: '',
                replyto: '/example/replyto',
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri should not be empty",
                        ])
                });
        });

        it('validate uri absent', async () => {
            const testObj = {
                replyto: '/example/replyto',
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                            "uri should not be empty",
                        ])
                });
        });

        it('validate replyto type', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: 5,
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                        ])
                });
        });

        it('validate replyto empty', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '',
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate replyto absent', async () => {
            const testObj = {
                uri: '/example/uri',
                body: 'body',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate body type', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
                body: 5,
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "body must be a string",
                        ])
                });
        });
    });

    describe('validate FimsMsgDTO', () => {
        let target: ValidationPipe;
        const metadata: ArgumentMetadata = {
            type: 'body',
            metatype: FimsMsgDTO,
            data: ''
        };

        beforeEach(() => {
            target = new ValidationPipe({
                transform: true, whitelist: true,
            });
        });

        it('validate normal', async () => {
            const testObj = {
                method: 'method',
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            expect(await target.transform(testObj, metadata)).toEqual(testObj);
        });

        it('validate invalid key', async () => {
            const testObj = {
                method: 'method',
                uri: '/example/uri',
                replyto: '/example/replyto',
                invalidKey: 'invalid value',
            };
            const resultObj = {
                method: 'method',
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            expect(await target.transform(testObj, metadata)).toEqual(resultObj);
        });

        it('validate empty', async () => {
            const testObj = {};
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "method must be a string",
                            "method should not be empty",
                            "uri must be a string",
                            "uri should not be empty",
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate method type', async () => {
            const testObj = {
                method: 5,
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "method must be a string",
                        ])
                });
        });

        it('validate method empty', async () => {
            const testObj = {
                method: '',
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "method should not be empty",
                        ])
                });
        });

        it('validate method absent', async () => {
            const testObj = {
                uri: '/example/uri',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "method must be a string",
                            "method should not be empty",
                        ])
                });
        });

        it('validate uri type', async () => {
            const testObj = {
                method: 'method',
                uri: 5,
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                        ])
                });
        });

        it('validate uri empty', async () => {
            const testObj = {
                method: 'method',
                uri: '',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri should not be empty",
                        ])
                });
        });

        it('validate uri absent', async () => {
            const testObj = {
                method: 'method',
                replyto: '/example/replyto',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "uri must be a string",
                            "uri should not be empty",
                        ])
                });
        });

        it('validate replyto type', async () => {
            const testObj = {
                method: 'method',
                uri: '/example/uri',
                replyto: 5,
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                        ])
                });
        });

        it('validate replyto empty', async () => {
            const testObj = {
                method: 'method',
                uri: '/example/uri',
                replyto: '',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto should not be empty",
                        ])
                });
        });

        it('validate replyto absent', async () => {
            const testObj = {
                method: 'method',
                uri: '/example/uri',
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "replyto must be a string",
                            "replyto should not be empty",
                        ])
                });
        });
    });
});