import { ArgumentMetadata, ValidationPipe } from '@nestjs/common';
import { OneTimeAuthResponse } from './oneTimeAuth.response';

describe('OneTimeAuth', () => {
    describe('validate OneTimeAuthResponse', () => {
        let target: ValidationPipe;
        const metadata: ArgumentMetadata = {
            type: 'body',
            metatype: OneTimeAuthResponse,
            data: ''
        };

        beforeEach(() => {
            target = new ValidationPipe({
                transform: true, whitelist: true,
            });
        });

        it('validate normal', async () => {
            const testObj = {
                token: 'token'
            };
            expect(await target.transform(testObj, metadata)).toEqual(testObj);
        });

        it('validate invalid key', async () => {
            const testObj = {
                token: 'token',
                invalidKey: 'invalid value',
            };
            const resultObj = {
                token: 'token',
            };
            expect(await target.transform(testObj, metadata)).toEqual(resultObj);
        });

        it('validate token type', async () => {
            const testObj = {
                token: 5,
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "token must be a string",
                        ])
                });
        });

        it('validate token absent', async () => {
            const testObj = {
            };
            await target.transform(testObj, metadata)
                .catch(err => {
                    expect(err.getResponse().message)
                        .toEqual([
                            "token must be a string",
                        ])
                });
        });
    });
});