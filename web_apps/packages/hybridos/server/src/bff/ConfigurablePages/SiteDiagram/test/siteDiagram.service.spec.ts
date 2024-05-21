import { Test, TestingModule } from '@nestjs/testing';

import { DBI_SERVICE, IDBIService } from 'src/dbi/dbi.interface';
import { FIMS_SERVICE, FimsMsg, IFimsService } from 'src/fims/interfaces/fims.interface';
import { ISiteDiagramService, SITE_DIAGRAM_SERVICE } from '../siteDiagram.interface';
import { SiteDiagramService } from '../siteDiagram.service';
import {
  fullMockURI,
  mockDBIConfig,
  mockFimsMsg,
  mockNonStaticDataObject,
  mockStaticDataObject,
} from './mocks';
import { of } from 'rxjs';

describe('SiteDiagramService', () => {
  let siteDiagramService: ISiteDiagramService;
  let fimsService: IFimsService;
  let dbiService: IDBIService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        { provide: SITE_DIAGRAM_SERVICE, useClass: SiteDiagramService },
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
      ],
    }).compile();

    siteDiagramService = module.get<ISiteDiagramService>(SITE_DIAGRAM_SERVICE);
    dbiService = module.get<IDBIService>(DBI_SERVICE);
    fimsService = module.get<IFimsService>(FIMS_SERVICE);
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  it('siteDiagramService should be defined', () => {
    expect(siteDiagramService).toBeDefined();
  });

  describe('test getSiteDiagramConfig', () => {
    it('should return a config file of SiteDiagramConfigBody type from dbi', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(mockDBIConfig);

      const result = await siteDiagramService.getSiteDiagramConfig();

      expect(result).toStrictEqual(mockDBIConfig);
    });
  });

  describe('test getStaticData', () => {
    it('should return an object that contains the static, non-changing data for each node on the diagram and its statuses', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(mockDBIConfig);

      const result = await siteDiagramService.getStaticData();

      expect(result).toStrictEqual(mockStaticDataObject);
    });
  });

  describe('test subscribeToUri', () => {
    it('should return an observable resulting from subscribing to the specified URI', (done) => {
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockFimsMsg as FimsMsg));

      const obs = siteDiagramService.subscribeToUri(
        fullMockURI,
        mockStaticDataObject.displayGroups[fullMockURI],
      );

      obs.subscribe({
        next: (value) => expect(value).toEqual(mockNonStaticDataObject),
        error: (err) => done(err),
        complete: () => done(),
      });
    });
  });

  describe('test getMergedStream', () => {
    it('should return the merged observable for the static and live data', async () => {
      const expectedValues = [mockStaticDataObject, mockNonStaticDataObject];
      const emittedValues = [];
      jest
        .spyOn(siteDiagramService, 'getStaticObservable')
        .mockReturnValueOnce(of(mockStaticDataObject));
      jest.spyOn(fimsService, 'subscribe').mockReturnValueOnce(of(mockFimsMsg as FimsMsg));

      const obs = siteDiagramService.getMergedStream(mockStaticDataObject);

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
});
