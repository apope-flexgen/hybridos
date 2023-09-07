import { Test, TestingModule } from '@nestjs/testing';
import { ApiService } from './api.service';

function napi_fims () {
  return ({
    connect: (pname) => true
  })
}

jest.mock(
  '/fims-node-dir/fims.node',
  () => ({
    napi_fims: napi_fims
  }),
  {
    virtual: true,
  }  
);

describe('ApiService', () => {
  let service: ApiService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [ApiService],
    }).compile();

    service = module.get<ApiService>(ApiService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
