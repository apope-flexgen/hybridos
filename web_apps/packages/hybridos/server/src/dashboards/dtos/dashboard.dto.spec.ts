import { ArgumentMetadata, ValidationPipe } from '@nestjs/common';
import { Test, TestingModule } from '@nestjs/testing';
import { Dashboard, Info, Item, Status } from './dashboard.dto';

describe('DashboardsDTO', () => {
  describe('validate Item DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Item,
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
        name: 'example name',
        uri: '/example/uri',
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        name: 'example name',
        uri: '/example/uri',
        invalidKey: 'invalid value',
      };
      const resultObj = {
        name: 'example name',
        uri: '/example/uri',
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'name must be a string',
          'uri must be a string',
        ]);
      });
    });

    it('validate name type', async () => {
      const testObj = {
        name: 0,
        uri: '/example/uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['name must be a string']);
      });
    });

    it('validate name absent', async () => {
      const testObj = {
        uri: '/example/uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['name must be a string']);
      });
    });

    it('validate uri error', async () => {
      const testObj = {
        name: 'example name',
        uri: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });

    it('validate uri absent', async () => {
      const testObj = {
        name: 'example name',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });

    it('validate name and uri error', async () => {
      const testObj = {
        name: 0,
        uri: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'name must be a string',
          'uri must be a string',
        ]);
      });
    });
  });

  describe('validate Info DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Info,
      data: '',
    };

    beforeEach(() => {
      target = new ValidationPipe({
        transform: true,
        whitelist: true,
      });
    });

    it('validate completely populated', async () => {
      const testObj = {
        baseURI: '/base/uri',
        batteryView: true,
        batteryViewSourceURI: '/battery_source',
        batteryViewURI: '/battery_view',
        isTemplate: true,
        items: [{ name: 'example item', uri: '/item/uri' }],
        name: 'example name',
        sourceURIs: ['/source/uri'],
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        baseURI: '/base/uri',
        batteryView: true,
        batteryViewSourceURI: '/battery_source',
        batteryViewURI: '/battery_view',
        isTemplate: true,
        items: [{ name: 'example item', uri: '/item/uri' }],
        name: 'example name',
        sourceURIs: ['/source/uri'],
        invalidKey: 'invalid value',
      };
      const resultObj = {
        baseURI: '/base/uri',
        batteryView: true,
        batteryViewSourceURI: '/battery_source',
        batteryViewURI: '/battery_view',
        isTemplate: true,
        items: [{ name: 'example item', uri: '/item/uri' }],
        name: 'example name',
        sourceURIs: ['/source/uri'],
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate baseURI type', async () => {
      const testObj = {
        baseURI: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['baseURI must be a string']);
      });
    });

    it('validate batteryView type', async () => {
      const testObj = {
        batteryView: 'invalid',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['batteryView must be a boolean value']);
      });
    });

    it('validate batteryViewSourceURI type', async () => {
      const testObj = {
        batteryViewSourceURI: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['batteryViewSourceURI must be a string']);
      });
    });

    it('validate batteryViewSourceURI null', async () => {
      const testObj = {
        batteryViewSourceURI: null,
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate batteryViewURI type', async () => {
      const testObj = {
        batteryViewURI: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['batteryViewURI must be a string']);
      });
    });

    it('validate isTemplate type', async () => {
      const testObj = {
        isTemplate: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['isTemplate must be a boolean value']);
      });
    });

    it('validate items type', async () => {
      const testObj = {
        items: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['items must be an array']);
      });
    });

    it('validate items array type', async () => {
      const testObj = {
        items: [0],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['items must be an array']);
      });
    });

    it('validate name type', async () => {
      const testObj = {
        name: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['name must be a string']);
      });
    });

    it('validate sourceURIs type', async () => {
      const testObj = {
        sourceURIs: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['sourceURIs must be an array']);
      });
    });

    it('validate sourceURIs array type', async () => {
      const testObj = {
        sourceURIs: [0],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['sourceURIs must be an array']);
      });
    });
  });

  describe('validate Status DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Status,
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
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
        invalidKey: 'invalid value',
      };
      const resultObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'name must be a string',
          'scalar must be a string',
          'units must be a string',
          'uri must be a string',
        ]);
      });
    });

    it('validate name type', async () => {
      const testObj = {
        name: 0,
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['name must be a string']);
      });
    });

    it('validate scalar type', async () => {
      const testObj = {
        name: 'example name',
        scalar: 0,
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['scalar must be a string']);
      });
    });

    it('validate units type', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 0,
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['units must be a string']);
      });
    });

    it('validate uri type', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });
  });

  describe('validate Dashboard DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Dashboard,
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
        info: {
          baseURI: '/base/uri',
          batteryView: true,
          batteryViewSourceURI: '/battery_source',
          batteryViewURI: '/battery_view',
          isTemplate: true,
          items: [{ name: 'example item', uri: '/item/uri' }],
          name: 'example name',
          sourceURIs: ['/source/uri'],
        },
        status: [
          {
            name: 'example name',
            scalar: 'example scalar',
            units: 'example units',
            uri: 'example uri',
          },
        ],
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        info: {
          baseURI: '/base/uri',
          batteryView: true,
          batteryViewSourceURI: '/battery_source',
          batteryViewURI: '/battery_view',
          isTemplate: true,
          items: [{ name: 'example item', uri: '/item/uri' }],
          name: 'example name',
          sourceURIs: ['/source/uri'],
        },
        status: [
          {
            name: 'example name',
            scalar: 'example scalar',
            units: 'example units',
            uri: 'example uri',
          },
        ],
        invalidKey: 'invalid value',
      };
      const resultObj = {
        info: {
          baseURI: '/base/uri',
          batteryView: true,
          batteryViewSourceURI: '/battery_source',
          batteryViewURI: '/battery_view',
          isTemplate: true,
          items: [{ name: 'example item', uri: '/item/uri' }],
          name: 'example name',
          sourceURIs: ['/source/uri'],
        },
        status: [
          {
            name: 'example name',
            scalar: 'example scalar',
            units: 'example units',
            uri: 'example uri',
          },
        ],
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'info must be an object',
          'status must be an array',
        ]);
      });
    });

    it('validate info', async () => {
      const testObj = {
        info: 0,
        status: [
          {
            name: 'example name',
            scalar: 'example scalar',
            units: 'example units',
            uri: 'example uri',
          },
        ],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['info must be an object']);
      });
    });

    it('validate status', async () => {
      const testObj = {
        info: {
          baseURI: '/base/uri',
          batteryView: true,
          batteryViewSourceURI: '/battery_source',
          batteryViewURI: '/battery_view',
          isTemplate: true,
          items: [{ name: 'example item', uri: '/item/uri' }],
          name: 'example name',
          sourceURIs: ['/source/uri'],
        },
        status: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['status must be an array']);
      });
    });

    it('validate status type', async () => {
      const testObj = {
        info: {
          baseURI: '/base/uri',
          batteryView: true,
          batteryViewSourceURI: '/battery_source',
          batteryViewURI: '/battery_view',
          isTemplate: true,
          items: [{ name: 'example item', uri: '/item/uri' }],
          name: 'example name',
          sourceURIs: ['/source/uri'],
        },
        status: [0],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['status must be an array']);
      });
    });
  });
});
