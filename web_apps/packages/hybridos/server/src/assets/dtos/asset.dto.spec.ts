import { ArgumentMetadata, ValidationPipe } from '@nestjs/common';
import { Test, TestingModule } from '@nestjs/testing';
import { Status, Control, Alarm, Asset, Assets } from './asset.dto';

describe('AssetsDTO', () => {
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

    it('validate name absent', async () => {
      const testObj = {
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

    it('validate scalar absent', async () => {
      const testObj = {
        name: 'example name',
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

    it('validate units absent', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
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

    it('validate uri absent', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });
  });

  describe('validate Control DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Control,
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
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
        invalidKey: 'invalid value',
      };
      const resultObj = {
        inputType: 'example inputType',
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
          'inputType must be a string',
          'name must be a string',
          'scalar must be a string',
          'units must be a string',
          'uri must be a string',
        ]);
      });
    });

    it('validate inputType type', async () => {
      const testObj = {
        inputType: 0,
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['inputType must be a string']);
      });
    });

    it('validate inputType absent', async () => {
      const testObj = {
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['inputType must be a string']);
      });
    });

    it('validate name type', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 0,
        scalar: 'example scalar',
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['name must be a string']);
      });
    });

    it('validate name absent', async () => {
      const testObj = {
        inputType: 'example inputType',
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
        inputType: 'example inputType',
        name: 'example name',
        scalar: 0,
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['scalar must be a string']);
      });
    });

    it('validate scalar absent', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        units: 'example units',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['scalar must be a string']);
      });
    });

    it('validate units type', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        units: 0,
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['units must be a string']);
      });
    });

    it('validate units absent', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        uri: 'example uri',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['units must be a string']);
      });
    });

    it('validate uri type', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
        uri: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });

    it('validate uri absent', async () => {
      const testObj = {
        inputType: 'example inputType',
        name: 'example name',
        scalar: 'example scalar',
        units: 'example units',
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['uri must be a string']);
      });
    });
  });

  describe('validate Alarm DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Alarm,
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
        alarmFields: [],
        faultFields: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        alarmFields: [],
        faultFields: [],
        invalidKey: 'invalid value',
      };
      const resultObj = {
        alarmFields: [],
        faultFields: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'alarmFields must be an array',
          'faultFields must be an array',
        ]);
      });
    });

    it('validate alarmFields type', async () => {
      const testObj = {
        alarmFields: 0,
        faultFields: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['alarmFields must be an array']);
      });
    });

    it('validate alarmFields empty', async () => {
      const testObj = {
        faultFields: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['alarmFields must be an array']);
      });
    });

    it('validate faultFields type', async () => {
      const testObj = {
        alarmFields: [],
        faultFields: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['faultFields must be an array']);
      });
    });

    it('validate faultFields empty', async () => {
      const testObj = {
        alarmFields: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['faultFields must be an array']);
      });
    });
  });

  describe('validate Asset DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Asset,
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
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
        invalidKey: 'invalid value',
      };
      const resultObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual([
          'alarms must be an object',
          'batchControls must be an array',
          'controls must be an array',
          'info must be an object',
          'statuses must be an array',
          'summary must be an array',
          'summaryControls must be an array',
        ]);
      });
    });

    it('validate alarms type', async () => {
      const testObj = {
        alarms: 0,
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['alarms must be an object']);
      });
    });

    it('validate alarms empty', async () => {
      const testObj = {
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['alarms must be an object']);
      });
    });

    it('validate batchControls type', async () => {
      const testObj = {
        alarms: {},
        batchControls: 0,
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['batchControls must be an array']);
      });
    });

    it('validate batchControls empty', async () => {
      const testObj = {
        alarms: {},
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['batchControls must be an array']);
      });
    });

    it('validate controls type', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: 0,
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['controls must be an array']);
      });
    });

    it('validate controls empty', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['controls must be an array']);
      });
    });

    it('validate info type', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: 0,
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['info must be an object']);
      });
    });

    it('validate info empty', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        statuses: [],
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['info must be an object']);
      });
    });

    it('validate statuses type', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: 0,
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['statuses must be an array']);
      });
    });

    it('validate statuses empty', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        summary: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['statuses must be an array']);
      });
    });

    it('validate summary type', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: 0,
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['summary must be an array']);
      });
    });

    it('validate summary empty', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summaryControls: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['summary must be an array']);
      });
    });

    it('validate summaryControls type', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
        summaryControls: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['summaryControls must be an array']);
      });
    });

    it('validate summaryControls empty', async () => {
      const testObj = {
        alarms: {},
        batchControls: [],
        controls: [],
        info: {},
        statuses: [],
        summary: [],
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['summaryControls must be an array']);
      });
    });
  });

  describe('validate Assets DTO', () => {
    let target: ValidationPipe;
    const metadata: ArgumentMetadata = {
      type: 'body',
      metatype: Assets,
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
        data: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(testObj);
    });

    it('validate invalid key', async () => {
      const testObj = {
        data: [],
        invalidKey: 'invalid value',
      };
      const resultObj = {
        data: [],
      };
      expect(await target.transform(testObj, metadata)).toEqual(resultObj);
    });

    it('validate empty', async () => {
      const testObj = {};
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['data must be an array']);
      });
    });

    it('validate data type', async () => {
      const testObj = {
        data: 0,
      };
      await target.transform(testObj, metadata).catch((err) => {
        expect(err.getResponse().message).toEqual(['data must be an array']);
      });
    });
  });
});
