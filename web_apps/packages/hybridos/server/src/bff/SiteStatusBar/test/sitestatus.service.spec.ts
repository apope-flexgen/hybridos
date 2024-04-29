import { Test, TestingModule } from '@nestjs/testing';
import { doesNotMatch } from 'assert';
import { Observable, of } from 'rxjs';
import {
  ISiteStatusService,
  SITE_STATUS_SERVICE,
} from 'src/bff/SiteStatusBar/sitestatus.interface';
import { SiteStatusService } from 'src/bff/SiteStatusBar/sitestatus.service';
import {
  mockAlertStateFimsMsg,
  mockAlertStateResponse,
  mockBooleanResponse,
  mockClothedNumberResponse,
  mockClothedValueBoolean,
  mockClothedValueNumber,
  mockClothedValueString,
  mockConfig,
  mockDBIConfig,
  mockDataAlertCountFimsMsg,
  mockDataSource,
  mockDataSourceBoolean,
  mockDataSourceBooleanFimsMsg,
  mockDataSourceInvalidField,
  mockDataSourceNumber,
  mockDataSourceNumberFimsMsg,
  mockDataSourceString,
  mockDataSourceStringFimsMsg,
  mockFieldData,
  mockNakedValueBoolean,
  mockNakedValueNumber,
  mockNakedValueString,
  mockNumberResponse,
  mockSiteConfiguration,
  mockSiteStateFimsMsg,
  mockSiteStateResponse,
  mockStringResponse,
  mockUnit,
  mockValueBoolean,
  mockValueNumber,
  mockValueString,
} from 'src/bff/SiteStatusBar/test/mocks';
import { DBI_SERVICE, IDBIService } from 'src/dbi/dbi.interface';
import { APP_ENV_SERVICE } from 'src/environment/appEnv.interface';
import { FIMS_SERVICE, FimsMsg, IFimsService } from 'src/fims/interfaces/fims.interface';

describe('SiteStatusService', () => {
  let siteStatusService: ISiteStatusService;
  let fimsService: IFimsService;
  let dbiService: IDBIService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        { provide: SITE_STATUS_SERVICE, useClass: SiteStatusService },
        {
          provide: DBI_SERVICE,
          useValue: {
            getFromDBI: jest.fn(),
            postToDBI: jest.fn(),
          },
        },
        {
          provide: FIMS_SERVICE,
          useValue: {
            subscribe: jest.fn(),
            get: jest.fn(),
          },
        },
        {
          provide: APP_ENV_SERVICE,
          useValue: { getSiteConfiguration: jest.fn().mockReturnValue(mockSiteConfiguration) },
        },
      ],
    }).compile();

    siteStatusService = module.get<ISiteStatusService>(SITE_STATUS_SERVICE);
    dbiService = module.get<IDBIService>(DBI_SERVICE);
    fimsService = module.get<IFimsService>(FIMS_SERVICE);
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  it('siteStatusService should be defined', () => {
    expect(siteStatusService).toBeDefined();
  });

  describe('test subscribeToSiteStatus', () => {
    it('should emit correct objects', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(mockDBIConfig);

      const expectedValues = [
        mockStringResponse,
        mockNumberResponse,
        mockBooleanResponse,
        mockSiteStateResponse,
        mockAlertStateResponse
      ];
      const emittedValues = [];

      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceStringFimsMsg as FimsMsg));
      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceNumberFimsMsg as FimsMsg));
      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceBooleanFimsMsg as FimsMsg));
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockSiteStateFimsMsg as FimsMsg));
      jest.spyOn(fimsService, 'get').mockResolvedValue(mockDataAlertCountFimsMsg as FimsMsg);
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockAlertStateFimsMsg as FimsMsg));
      jest.spyOn(siteStatusService, 'buildAlertStateObservable').mockResolvedValueOnce(of(mockAlertStateResponse));

      const obs = await siteStatusService.subscribeToSiteStatus();

      await new Promise((resolve, reject) => {
        obs.subscribe({
          next: (value) => emittedValues.push(value),
          error: (err) => reject(err),
          complete: () => {
            resolve({});
          },
        });
      });

      expect(emittedValues.length).toEqual(expectedValues.length);
      expect(emittedValues).toEqual(expect.arrayContaining(expectedValues));
    });
  });

  describe('test getActiveAlertsData', () => {
    it('should emit correct value, a number', async () => {
      jest.spyOn(fimsService, 'get').mockResolvedValue(mockDataAlertCountFimsMsg as FimsMsg);

      const numActiveAlerts = await siteStatusService.getActiveAlertsData();

      expect(numActiveAlerts).toBeDefined();
    });
  });

  describe('test getConfig', () => {
    it('should add an index property to each entry', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(mockDBIConfig);

      const result = await siteStatusService.getConfig();

      expect(result).toStrictEqual(mockConfig);
    });
  });

  describe('test getObservable', () => {
    it('should return site status bar observable', async () => {
      const expectedValues = [
        mockStringResponse,
        mockNumberResponse,
        mockBooleanResponse,
        mockSiteStateResponse,
        mockAlertStateResponse
      ];
      const emittedValues = [];

      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceStringFimsMsg as FimsMsg));
      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceNumberFimsMsg as FimsMsg));
      jest
        .spyOn(fimsService, 'subscribe')
        .mockReturnValueOnce(of(mockDataSourceBooleanFimsMsg as FimsMsg));
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockSiteStateFimsMsg as FimsMsg));
       jest.spyOn(fimsService, 'get').mockResolvedValue(mockDataAlertCountFimsMsg as FimsMsg);
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockAlertStateFimsMsg as FimsMsg));
      jest.spyOn(siteStatusService, 'buildAlertStateObservable').mockResolvedValueOnce(of(mockAlertStateResponse));

      const obs = await siteStatusService.getObservable(mockConfig);

      await new Promise((resolve, reject) => {
        obs.subscribe({
          next: (value) => emittedValues.push(value),
          error: (err) => reject(err),
          complete: () => {
            resolve({});
          },
        });
      });

      expect(emittedValues.length).toEqual(expectedValues.length);
      expect(emittedValues).toEqual(expect.arrayContaining(expectedValues));
    });
  });

  describe('test buildSiteStateObservable', () => {
    it('should return site state', (done) => {
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockSiteStateFimsMsg as FimsMsg));

      const obs = siteStatusService.buildSiteStateObservable();

      obs.subscribe({
        next: (value) => expect(value).toStrictEqual(mockSiteStateResponse),
        error: (err) => done(err),
        complete: () => done(),
      });
    });
  });

  describe('test buildDataSourceObservable', () => {
    describe('should return observable', () => {
      it('string', (done) => {
        jest
          .spyOn(fimsService, 'subscribe')
          .mockReturnValueOnce(of(mockDataSourceStringFimsMsg as FimsMsg));

        const obs = siteStatusService.buildDataSourceObservable(mockDataSourceString);

        obs.subscribe({
          next: (value) => expect(value).toStrictEqual(mockStringResponse),
          error: (err) => done(err),
          complete: () => done(),
        });
      });

      it('number', (done) => {
        jest
          .spyOn(fimsService, 'subscribe')
          .mockReturnValueOnce(of(mockDataSourceNumberFimsMsg as FimsMsg));

        const obs = siteStatusService.buildDataSourceObservable(mockDataSourceNumber);

        obs.subscribe({
          next: (value) => expect(value).toStrictEqual(mockNumberResponse),
          error: (err) => done(err),
          complete: () => done(),
        });
      });

      it('boolean', (done) => {
        jest
          .spyOn(fimsService, 'subscribe')
          .mockReturnValueOnce(of(mockDataSourceBooleanFimsMsg as FimsMsg));

        const obs = siteStatusService.buildDataSourceObservable(mockDataSourceBoolean);

        obs.subscribe({
          next: (value) => expect(value).toStrictEqual(mockBooleanResponse),
          error: (err) => done(err),
          complete: () => done(),
        });
      });
    });
  });

  describe('test getFieldData', () => {
    it('should return fieldData', () => {
      const result = siteStatusService.getFieldData(mockFieldData, mockDataSource);

      expect(result).toBeDefined();
    });

    it('should return undefined', () => {
      const result = siteStatusService.getFieldData(mockFieldData, mockDataSourceInvalidField);

      expect(result).toBeUndefined();
    });
  });

  describe('test fieldDataExists', () => {
    it('should return true when field exists', () => {
      const result = siteStatusService.fieldDataExists(mockFieldData, mockDataSource);

      expect(result).toBe(true);
    });

    it('should return false when field does not exist', () => {
      const result = siteStatusService.fieldDataExists(mockFieldData, mockDataSourceInvalidField);

      expect(result).toBe(false);
    });
  });

  describe('test processData', () => {
    describe('naked', () => {
      describe('string', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockNakedValueString,
            mockDataSourceString,
          );

          expect(response).toStrictEqual(mockStringResponse);
        });
      });

      describe('number', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockNakedValueNumber,
            mockDataSourceNumber,
          );

          expect(response).toStrictEqual(mockNumberResponse);
        });
      });

      describe('boolean', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockNakedValueBoolean,
            mockDataSourceBoolean,
          );

          expect(response).toStrictEqual(mockBooleanResponse);
        });
      });
    });

    describe('clothed', () => {
      describe('string', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockClothedValueString,
            mockDataSourceString,
          );

          expect(response).toStrictEqual(mockStringResponse);
        });
      });

      describe('number', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockClothedValueNumber,
            mockDataSourceNumber,
          );

          expect(response).toStrictEqual(mockClothedNumberResponse);
        });
      });

      describe('boolean', () => {
        it('should return object with value as string', () => {
          const response = siteStatusService.processData(
            mockClothedValueBoolean,
            mockDataSourceBoolean,
          );

          expect(response).toStrictEqual(mockBooleanResponse);
        });
      });
    });
  });

  describe('test processNakedData', () => {
    describe('string', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processNakedData(
          mockNakedValueString,
          mockDataSourceString,
        );

        expect(response).toStrictEqual(mockStringResponse);
      });
    });

    describe('number', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processNakedData(
          mockNakedValueNumber,
          mockDataSourceNumber,
        );

        expect(response).toStrictEqual(mockNumberResponse);
      });
    });

    describe('boolean', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processNakedData(
          mockNakedValueBoolean,
          mockDataSourceBoolean,
        );

        expect(response).toStrictEqual(mockBooleanResponse);
      });
    });
  });

  describe('test processClothedData', () => {
    describe('string', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processClothedData(
          mockClothedValueString,
          mockDataSourceString,
        );

        expect(response).toStrictEqual(mockStringResponse);
      });
    });

    describe('number', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processClothedData(
          mockClothedValueNumber,
          mockDataSourceNumber,
        );

        expect(response).toStrictEqual(mockClothedNumberResponse);
      });
    });

    describe('boolean', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.processClothedData(
          mockClothedValueBoolean,
          mockDataSourceBoolean,
        );

        expect(response).toStrictEqual(mockBooleanResponse);
      });
    });
  });

  describe('test buildResponse', () => {
    describe('string', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.buildResponse(mockDataSourceString, mockValueString);

        expect(response).toStrictEqual(mockStringResponse);
      });
    });

    describe('number', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.buildResponse(
          mockDataSourceNumber,
          mockValueNumber,
          mockUnit,
        );

        expect(response).toStrictEqual(mockNumberResponse);
      });
    });

    describe('boolean', () => {
      it('should return object with value as string', () => {
        const response = siteStatusService.buildResponse(mockDataSourceBoolean, mockValueBoolean);

        expect(response).toStrictEqual(mockBooleanResponse);
      });
    });
  });
});
