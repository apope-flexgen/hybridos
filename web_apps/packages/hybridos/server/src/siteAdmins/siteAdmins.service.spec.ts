import { getModelToken } from '@nestjs/mongoose';
import { Test } from '@nestjs/testing';
import { Model } from 'mongoose';

import { SiteAdminsService } from './siteAdmins.service';
import { SiteAdmin } from './interfaces/siteAdmin.interface';
import { SITE_ADMINS_SERVICE, ISiteAdminsService } from './interfaces/siteAdmin.service.interface';
import { SiteAdminDocument } from './schemas/siteAdmins.schema';

describe('SiteAdminsController', () => {
  let service: ISiteAdminsService;
  let model: Model<SiteAdminDocument>;
  const defaultSiteAdmins: SiteAdmin = {
    password: {
      multi_factor_authentication: false,
      password_expiration: false,
      minimum_password_length: 8,
      maximum_password_length: 128,
      password_expiration_interval: '8d',
      old_passwords: 0,
      password_regular_expression: JSON.stringify(
        /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/,
      ),

      lowercase: true,
      uppercase: true,
      digit: true,
      special: true,
    },
    radius: {
      is_enabled: false,
      ip_address: '127.0.0.1',
      port: '1812',
      secret_phrase: 'testing123',
      wait_time: 5000,
      is_local_auth_disabled: false,
    },
  };

  const mockSiteAdmin = (): SiteAdmin => defaultSiteAdmins;

  beforeEach(async () => {
    const module = await Test.createTestingModule({
      providers: [
        {
          provide: SITE_ADMINS_SERVICE,
          useClass: SiteAdminsService,
        },
        {
          provide: getModelToken('siteadmins'),
          useValue: {
            new: jest.fn().mockResolvedValue(mockSiteAdmin()),
            constructor: jest.fn().mockResolvedValue(mockSiteAdmin()),
            findOne: jest.fn(),
            create: jest.fn(),
            exec: jest.fn(),
            deleteMany: jest.fn(),
          },
        },
      ],
    }).compile();

    service = module.get<ISiteAdminsService>(SITE_ADMINS_SERVICE);
    model = module.get<Model<SiteAdminDocument>>(getModelToken('siteadmins'));
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  // find tests

  it('should return default siteAdmins (empty database)', async () => {
    jest.spyOn(model, 'findOne').mockReturnValue({
      exec: jest.fn().mockResolvedValueOnce(undefined),
    } as any);
    jest.spyOn(model, 'create').mockReturnValue(mockSiteAdmin() as any);
    const siteAdmin = await service.find();
    expect(siteAdmin).toEqual(defaultSiteAdmins);
  });

  it('should return current siteAdmins (non-empty database)', async () => {
    jest.spyOn(model, 'findOne').mockReturnValue({
      exec: jest.fn().mockResolvedValueOnce(mockSiteAdmin()),
    } as any);
    const siteAdmin = await service.find();
    expect(siteAdmin).toEqual(defaultSiteAdmins);
  });

  // create tests

  it('should insert a new siteAdmins', async () => {
    jest.spyOn(model, 'create').mockReturnValue(mockSiteAdmin() as any);
    const siteAdmin = await service.create(mockSiteAdmin());
    expect(siteAdmin).toEqual(defaultSiteAdmins);
  });
});
