import {
  Body,
  Controller,
  Delete,
  Get,
  Inject,
  Post,
  Put,
  Query,
  Req,
  UseFilters,
  UseGuards,
} from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiTags } from '@nestjs/swagger';
import { ApiSecurity } from '@nestjs/swagger';
import { Throttle } from '@nestjs/throttler';
import { FIMS_API_LIMIT, FIMS_API_TTL } from 'src/environment/appEnv.constants';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';

import { BodyDTO, GetDTO } from './dto/fims.dto';
import { FimsHttpPermissionsGuard } from './guards/fims.http.guard';
import { HttpThrottleExceptionFilter } from './httpthrottler.filter';
import { HttpThrottlerGuard } from './httpthrottler.guard';
import { FIMS_SERVICE, IFimsService } from './interfaces/fims.interface';
import { ISocketAuthService, SOCKET_AUTH_SERVICE } from './interfaces/socketAuth.service.interface';
import { FimsMsg } from './responses/fimsMsg.response';
import { OneTimeAuthResponse } from './responses/oneTimeAuth.response';

@ApiSecurity('bearerAuth')
@Controller('fims')
@ApiDefaultResponse({ type: DefaultApiError })
@UseGuards(HttpThrottlerGuard)
@Throttle(FIMS_API_LIMIT, FIMS_API_TTL)
@ApiTags('fims')
@UseFilters(new HttpThrottleExceptionFilter())
export class FimsController {
  constructor(
    @Inject(FIMS_SERVICE) private readonly fimsService: IFimsService,
    @Inject(SOCKET_AUTH_SERVICE)
    private readonly socketAuthService: ISocketAuthService,
  ) {}

  @Get('one-time-auth')
  @ApiOkResponse({ type: OneTimeAuthResponse })
  async oneTimeAuth(@Req() req): Promise<OneTimeAuthResponse> {
    const token = this.socketAuthService.generateToken(req.user);
    return {
      token,
    };
  }

  @Get('get')
  @UseGuards(FimsHttpPermissionsGuard)
  async get(@Query() query: GetDTO): Promise<FimsMsg> {
    const msg: FimsMsg = {
      method: 'get',
      uri: query.uri,
      replyto: query.replyto,
      body: '',
      username: '',
    };
    return await this.fimsService.send(msg);
  }

  @Put('set')
  @UseGuards(FimsHttpPermissionsGuard)
  async set(@Body() body: BodyDTO): Promise<FimsMsg> {
    const msg: FimsMsg = {
      method: 'set',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.fimsService.send(msg);
  }

  @Post('post')
  @UseGuards(FimsHttpPermissionsGuard)
  async post(@Body() body: BodyDTO): Promise<FimsMsg> {
    const msg: FimsMsg = {
      method: 'post',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.fimsService.send(msg);
  }

  @Delete('delete')
  @UseGuards(FimsHttpPermissionsGuard)
  async delete(@Body() body: BodyDTO): Promise<FimsMsg> {
    const msg: FimsMsg = {
      method: 'del',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.fimsService.send(msg);
  }
}
