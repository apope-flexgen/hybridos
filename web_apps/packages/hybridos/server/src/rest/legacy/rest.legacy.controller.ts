import {
  Controller,
  Get,
  HttpCode,
  Param,
  Put,
  Req,
  UseFilters,
  UseGuards,
  HttpStatus,
  Inject,
} from '@nestjs/common';
import { ApprovedRoles } from '../../decorators/roles.decorator';
import { Roles } from '../../../../shared/types/api/Users/Users.types';

import { BasicAccessGuard } from 'src/auth/guards/basicAccessAuth.guard';
import { UserNotFoundFilter } from 'src/users/filters/userNotFound.filter';
import { BasicAuth } from 'src/decorators/basicAuth.decorator';
import { RolesGuard } from 'src/auth/guards/roles.guard';
import { Throttle } from '@nestjs/throttler';
import { REST_API_LIMIT, REST_API_TTL } from 'src/environment/appEnv.constants';
import {
  ApiOkResponse,
  ApiResponse,
  ApiTags,
  ApiSecurity,
  ApiDefaultResponse,
} from '@nestjs/swagger';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { AggregatedEndpoints } from 'src/decorators/aggregatedEndpoints.decorator';
import { LegacyRestPutParams } from './params/put.params';
import { LegacyRestPermissionsGuard } from './guards/rest.legacy.permission.guard';
import {
  ILegacyRestService,
  LEGACY_REST_API_PREFIX,
  LEGACY_REST_API_PREFIXES,
  LEGACY_REST_SERVICE,
} from './interfaces/rest.legacy.interface';
import {
  LegacyRestPutResponse,
  LEGACY_REST_PUT_RESPONSE_DESCRIPTION,
} from './responses/restput.response';
import {
  LegacyRestGetResponse,
  LEGACY_REST_GET_RESPONSE_DESCRIPTION,
} from './responses/restget.response';
import { LegacyRestGetParams } from './params/restget.params';

@UseGuards(BasicAccessGuard, RolesGuard, LegacyRestPermissionsGuard)
@BasicAuth()
@ApiSecurity('basicAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller(LEGACY_REST_API_PREFIXES)
@ApiTags(LEGACY_REST_API_PREFIX)
@Throttle(REST_API_LIMIT, REST_API_TTL)
export class LegacyRestController {
  constructor(
    @Inject(LEGACY_REST_SERVICE) private readonly legacyRestService: ILegacyRestService,
  ) {}

  @Get(':endpoint(*)')
  @AggregatedEndpoints()
  @ApprovedRoles(Roles.Rest)
  @ApiOkResponse({
    type: LegacyRestGetResponse,
    description: LEGACY_REST_GET_RESPONSE_DESCRIPTION,
  })
  @UseFilters(UserNotFoundFilter)
  async get(@Param() params: LegacyRestGetParams, @Req() request): Promise<LegacyRestGetResponse> {
    return await this.legacyRestService.get(`/${params.endpoint}`, request.user);
  }

  @Put(':endpoint(*)/:value')
  @ApiResponse({
    type: LegacyRestPutResponse,
    description: LEGACY_REST_PUT_RESPONSE_DESCRIPTION,
    status: HttpStatus.ACCEPTED,
  })
  @ApprovedRoles(Roles.Rest)
  @HttpCode(HttpStatus.ACCEPTED)
  @UseFilters(UserNotFoundFilter)
  async put(@Param() params: LegacyRestPutParams, @Req() request): Promise<LegacyRestPutResponse> {
    return await this.legacyRestService.set(`/${params.endpoint}`, params.value, request.user);
  }
}
