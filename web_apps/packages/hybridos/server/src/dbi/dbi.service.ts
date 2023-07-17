import { Inject } from '@nestjs/common';
import { FIMS_SERVICE, IFimsService } from '../fims/interfaces/fims.interface';
import { DBI_URIs, IDBIService } from './dbi.interface';
import { DBIDocumentNotFoundException } from './exceptions/dbi.exceptions';

export class DBIService implements IDBIService {
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: IFimsService,
  ) {}

  getFromDBI = async (URI: DBI_URIs): Promise<any> => {
    const response = await this.fimsService.get(`/dbi${URI}`);

    if (typeof response.body === 'string' && response.body.includes('not found in collection')) {
      throw new DBIDocumentNotFoundException(response.body);
    }

    return response.body;
  };

  postToDBI = async (URI: DBI_URIs, data: any): Promise<any> => {
    const response = await this.fimsService.send({
      method: 'post',
      uri: `/dbi${URI}`,
      replyto: '',
      body: data,
      username: '',
    });
    return response.body;
  };
}
