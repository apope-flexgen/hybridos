import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus, Inject } from '@nestjs/common';
import { Response } from 'express';

import { MfaRequiredException } from '../exceptions/mfaRequired.exception';
import { IMfaService, MFA_SERVICE } from '../interfaces/mfa.service.interface';

@Catch(MfaRequiredException)
export class MfaRequiredFilter implements ExceptionFilter {
  constructor(
    @Inject(MFA_SERVICE)
    private readonly mfaService: IMfaService,
  ) {}
  async catch(exception: MfaRequiredException, host: ArgumentsHost) {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();
    const status = HttpStatus.OK;

    const { oneTimeAccessToken, username, qrCode } = await this.mfaService.mfaResponse(
      exception.user,
    );

    response.status(status).json({
      accessToken: `Bearer ${oneTimeAccessToken}`,
      username: username,
      mfaRequired: true,
      ...(qrCode && { qrCode: qrCode }), // only add qrCode field if it is not null
    });
  }
}
