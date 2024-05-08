import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus, Inject } from '@nestjs/common';
import { Response } from 'express';

import { PasswordExpiredException } from '../exceptions/passwordExpired.exception';
import { IPassExpService, PASS_EXP_SERVICE } from '../interfaces/passExp.service.interface';

@Catch(PasswordExpiredException)
export class PassExpFilter implements ExceptionFilter {
  constructor(
    @Inject(PASS_EXP_SERVICE)
    private readonly passExpService: IPassExpService,
  ) {}
  async catch(exception: PasswordExpiredException, host: ArgumentsHost) {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();
    const status = HttpStatus.OK;

    const { oneTimeAccessToken, ...body } = await this.passExpService.passExpResponse(
      exception.user,
    );

    response.status(status).json({ accessToken: `Bearer ${oneTimeAccessToken}`, ...body });
  }
}
