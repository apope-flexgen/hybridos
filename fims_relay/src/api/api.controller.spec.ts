import { Test, TestingModule } from '@nestjs/testing';
import { ApiController } from './api.controller';
import { ApiService } from './api.service';
import { FimsMsg } from './interface/fimsmsg.interface';

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

describe('ApiController', () => {
  let controller: ApiController;
  let apiService: ApiService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [ApiController],
      providers: [ApiService],
    }).compile();

    controller = module.get<ApiController>(ApiController);
    apiService = module.get<ApiService>(ApiService)
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
  
  it('should return a FimsMsg', async () => {
    const result: FimsMsg = {
      method: 'get',
      uri: '/goodbye',
      replyto: '/hello',
      body: '',
      username: '',
    };
    jest.spyOn(apiService, 'send').mockImplementation(async () => result);

    expect(await controller.get({uri: '/hello', replyto: '/goodbye'})).toBe(result);
  });
});
