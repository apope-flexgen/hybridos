import { ArgumentMetadata, ValidationPipe } from '@nestjs/common';
import { FimsMsg } from './fimsMsg.response';

describe('FimsMsg', () => {
  describe('validate FimsMsg', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: FimsMsg,
      data: '',
    };

    beforeEach(() => {
      target = new ValidationPipe({
        transform: true,
        whitelist: true,
      });
    });

    it('validate normal', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
        invalidKey: 'invalid value',
      };
      const resultObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'method must be a string',
          'uri must be a string',
          'replyto must be a string',
          'body should not be empty',
          'username must be a string',
        ]);
      });
    });

    it('validate method type', async () => {
      const testObj = {
        method: 5,
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['method must be a string']);
      });
    });

    it('validate method absent', async () => {
      const testObj = {
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['method must be a string']);
      });
    });

    it('validate uri type', async () => {
      const testObj = {
        method: 'method',
        uri: 5,
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });

    it('validate uri absent', async () => {
      const testObj = {
        method: 'method',
        replyto: '/example/replyto',
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });

    it('validate replyto type', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: 5,
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['replyto must be a string']);
      });
    });

    it('validate replyto absent', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        body: {},
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['replyto must be a string']);
      });
    });

    it('validate body empty', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: '',
        username: 'username',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['body should not be empty']);
      });
    });

    it('validate username type', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
        username: 5,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['username must be a string']);
      });
    });

    it('validate username absent', async () => {
      const testObj = {
        method: 'method',
        uri: '/example/uri',
        replyto: '/example/replyto',
        body: {},
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['username must be a string']);
      });
    });
  });
});
