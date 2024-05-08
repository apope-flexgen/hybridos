import { Test, TestingModule } from '@nestjs/testing';
import { FIMS_SERVICE, IFimsService } from '../fims/interfaces/fims.interface';
import { IDBIService } from './dbi.interface';
import { DBIService } from './dbi.service';
import { DBIDocumentNotFoundException } from './exceptions/dbi.exceptions';
import { DBI_DEST_URI, FIMS_RESPONSE, FIMS_RESPONSE_ERR } from './dbi.mocks';

describe('AssetsService', () => {
  let dbiService: IDBIService;
  let fimsService: IFimsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        DBIService,
        {
          provide: FIMS_SERVICE,
          useValue: {
            get: jest.fn(),
            send: jest.fn(),
          },
        },
      ],
    }).compile();

    dbiService = module.get<DBIService>(DBIService);
    fimsService = module.get<IFimsService>(FIMS_SERVICE);
  });

  it('fimsService should be defined', () => {
    expect(fimsService).toBeDefined();
  });

  describe('test getFromDBI', () => {
    it('should use the URI', async () => {
      const getFromDBIService = jest.spyOn(fimsService, 'get').mockResolvedValue(FIMS_RESPONSE);
      await dbiService.getFromDBI(DBI_DEST_URI);
      expect(getFromDBIService).toHaveBeenCalledWith(`/dbi${DBI_DEST_URI}`);
    });

    it('should return DBI body', async () => {
      jest.spyOn(fimsService, 'get').mockResolvedValue(FIMS_RESPONSE);
      expect(await dbiService.getFromDBI(DBI_DEST_URI)).toMatchObject(FIMS_RESPONSE.body);
    });

    it('should throw DBI exception', async () => {
      jest.spyOn(fimsService, 'get').mockResolvedValue(FIMS_RESPONSE_ERR);
      await expect(dbiService.getFromDBI(DBI_DEST_URI)).rejects.toThrowError(
        new DBIDocumentNotFoundException(FIMS_RESPONSE_ERR.body),
      );
    });
  });

  describe('test postToDBI', () => {
    it('should use the arguments', async () => {
      const postToDBIService = jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      await dbiService.postToDBI(DBI_DEST_URI, 'example data');
      expect(postToDBIService).toHaveBeenCalledWith({
        method: 'post',
        uri: `/dbi${DBI_DEST_URI}`,
        replyto: '',
        body: 'example data',
        username: '',
      });
    });

    it('should return DBI body', async () => {
      jest.spyOn(fimsService, 'send').mockResolvedValue(FIMS_RESPONSE);
      expect(await dbiService.postToDBI(DBI_DEST_URI, 'example data')).toEqual(FIMS_RESPONSE.body);
    });
  });
});
