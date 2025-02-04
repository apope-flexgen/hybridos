import { INestApplication } from '@nestjs/common';
import { JwtModule, JwtService } from '@nestjs/jwt';
import { Test, TestingModule } from '@nestjs/testing';
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser';
import { authenticator } from 'otplib';

import request from './../testReqAgent';

import { SITE_ADMINS_SERVICE } from '../../src/siteAdmins/interfaces/siteAdmin.service.interface';
import { AuthController } from '../../src/auth/auth.controller';
import { AuthService } from '../../src/auth/auth.service';
import { AUTH_SERVICE } from '../../src/auth/interfaces/auth.service.interface';
import { MFA_SERVICE } from '../../src/auth/interfaces/mfa.service.interface';
import { PASS_EXP_SERVICE } from '../../src/auth/interfaces/passExp.service.interface';
import { RADIUS_SERVICE } from '../../src/auth/interfaces/radius.service.interface';
import { MfaService } from '../../src/auth/mfa.service';
import { PassExpService } from '../../src/auth/passExp.service';
import { AccessTokenMfaStrategy } from '../../src/auth/strategies/accessToken.mfa.strategy';
import { TotpStrategy } from '../../src/auth/strategies/totp.strategy';
import { USERS_SERVICE } from '../../src/users/interfaces/users.service.interface';
import { RefreshTokenService } from '../../src/auth/refreshToken.service';
import { RefreshTokenStrategy } from '../../src/auth/strategies/refreshToken.strategy';
import { AppEnvService } from '../../src/environment/appEnv.service';
import * as testUtils from '../testUtils';
import { AUDIT_LOGGING_SERVICE } from 'src/logging/auditLogging/interfaces/auditLogging.service.interface';
import { useContainer } from 'class-validator';
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword';
import path from 'path';
import { VALID_ACCESS_TOKEN_SERVICE } from 'src/auth/interfaces/validAccessToken.service.interface';
import { ValidAccessTokenService } from 'src/auth/validAccessToken.service';

describe('AuthController Login (e2e)', () => {
  let app: INestApplication;
  let jwtService: JwtService;

  const VALID_ID = 'user-id';

  const USER_MFA_NOTENABLED = testUtils.adminUser(false, false);
  const USER_MFA_ENABLED = testUtils.adminUser(false, true);

  beforeAll(async () => {
    const moduleFixture: TestingModule = await Test.createTestingModule({
      imports: [
        JwtModule.register({
          secret: 'supersecretkey',
        }),
      ],
      controllers: [AuthController],
      providers: [
        {
          provide: USERS_SERVICE,
          useValue: {
            readByUsername: jest.fn().mockResolvedValue(USER_MFA_NOTENABLED),
            enableMfa: jest.fn().mockResolvedValue(USER_MFA_ENABLED),
          },
        },
        ValidPasswordConstraint,
        {
          provide: SITE_ADMINS_SERVICE,
          useValue: {
            find: jest.fn().mockReturnValue(testUtils.site(false, true, false)),
          },
        },
        {
          provide: RADIUS_SERVICE,
          useValue: {},
        },
        {
          provide: AUTH_SERVICE,
          useClass: AuthService,
        },
        {
          provide: MFA_SERVICE,
          useClass: MfaService,
        },
        {
          provide: PASS_EXP_SERVICE,
          useClass: PassExpService,
        },
        {
          provide: VALID_ACCESS_TOKEN_SERVICE,
          useClass: ValidAccessTokenService,
        },
        RefreshTokenService,
        AppEnvService,
        {
          provide: 'WEB_UI_CONFIG_PATH',
          useValue: path.resolve(__dirname, '../../test/configs/test-web_ui.json'),
        },
        {
          provide: 'WEB_SERVER_CONFIG_PATH',
          useValue: path.resolve(__dirname, '../../test/configs/test-config.json'),
        },
        AccessTokenMfaStrategy,
        TotpStrategy,
        RefreshTokenStrategy,
        {
          provide: AUDIT_LOGGING_SERVICE,
          useValue: {
            postAuditLog: jest.fn(),
          },
        },
      ],
    }).compile();

    app = testUtils.createTestApiApplication(moduleFixture);
    useContainer(moduleFixture, { fallbackOnErrors: true });
    jwtService = moduleFixture.get<JwtService>(JwtService);
    app.use(cookieParser());
    await app.init();
  });

  describe('/login/mfa', () => {
    it('successfully logs in user', () => {
      const URL = `/login/mfa`;

      const payload = {
        username: testUtils.VALID_USERNAME,
        oneTime: 'mfa',
        sub: VALID_ID,
      };
      const MFA_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
        expiresIn: '12d',
        secret: process.env.JWT_SECRET_KEY_MFA,
      })}`;

      const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY);
      const mfaRequestBody = {
        username: testUtils.VALID_USERNAME,
        totp: CURRENT_TOTP,
      };

      return request(app.getHttpServer())
        .post(URL)
        .set('Authorization', MFA_ACCESS_TOKEN)
        .send(mfaRequestBody)
        .then((res) => {
          expect(res.status).toBe(200);
          expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE));
          expect(res.headers['set-cookie']).toBeTruthy();
        });
    });

    it('fails (incorrect code)', () => {
      const URL = `/login/mfa`;

      const payload = {
        username: testUtils.VALID_USERNAME,
        oneTime: 'mfa',
        sub: VALID_ID,
      };
      const MFA_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
        expiresIn: '12d',
        secret: process.env.JWT_SECRET_KEY_MFA,
      })}`;

      const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY);
      const INVALID_TOTP = CURRENT_TOTP.split('').reverse().join('');
      const mfaRequestBody = {
        username: testUtils.VALID_USERNAME,
        totp: INVALID_TOTP,
      };

      return request(app.getHttpServer())
        .post(URL)
        .set('Authorization', MFA_ACCESS_TOKEN)
        .send(mfaRequestBody)
        .then((res) => {
          expect(res.status).toBe(401);
          expect(res.body).toStrictEqual({
            statusCode: 401,
            message: 'Incorrect TOTP Code',
          });
          expect(res.headers['set-cookie']).toBeFalsy();
        });
    });

    it('fails (invalid accessToken)', () => {
      const URL = `/login/mfa`;

      const payload = {
        username: testUtils.VALID_USERNAME,
        oneTime: 'mfa',
        sub: VALID_ID,
      };
      const MFA_ACCESS_TOKEN = jwtService.sign(payload, {
        expiresIn: '12d',
        secret: 'incorrectSecretKey',
      });

      const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY);
      const mfaRequestBody = {
        username: testUtils.VALID_USERNAME,
        totp: CURRENT_TOTP,
      };

      return request(app.getHttpServer())
        .post(URL)
        .set('Authorization', MFA_ACCESS_TOKEN)
        .send(mfaRequestBody)
        .then((res) => {
          expect(res.status).toBe(401);
          expect(res.body).toStrictEqual({
            statusCode: 401,
            message: 'Unauthorized',
          });
          expect(res.headers['set-cookie']).toBeFalsy();
        });
    });
  });
});
