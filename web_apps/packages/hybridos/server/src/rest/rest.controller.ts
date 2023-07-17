import {
  Controller,
  Get,
  Post,
  HttpCode,
  Param,
  Put,
  Req,
  UseFilters,
  UseGuards,
  Delete,
  HttpStatus,
  Inject,
} from '@nestjs/common';
import { ApprovedRoles } from '../decorators/roles.decorator';
import { Roles } from '../../../shared/types/api/Users/Users.types';

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
import { REST_PUT_RESPONSE_DESCRIPTION, RestPutResponse } from './responses/restput.response';
import {
  RestGetResponse,
  REST_GET_RESPONSE_DESCRIPTION,
} from 'src/rest/responses/restget.response';
import { RestGetParams } from './params/restget.params';
import { RestPutParams } from './params/restput.params';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { RestPermissionsGuard } from './guards/rest.permission.guard';
import { AggregatedEndpoints } from 'src/decorators/aggregatedEndpoints.decorator';
import { REST_POST_RESPONSE_DESCRIPTION, RestPostResponse } from './responses/restpost.response';
import {
  REST_DELETE_RESPONSE_DESCRIPTION,
  RestDeleteResponse,
} from './responses/restdelete.response';
import { RestPostParams } from './params/restpost.params';
import { RestDeleteParams } from './params/restdelete.params';
import { RawBody, RawBodyRequirement } from '../decorators/rawBody.decorator';
import { IRestService, REST_API_PREFIX, REST_SERVICE } from './interfaces/rest.interface';

@UseGuards(BasicAccessGuard, RolesGuard, RestPermissionsGuard)
@BasicAuth()
@ApiSecurity('basicAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller(REST_API_PREFIX)
@ApiTags(REST_API_PREFIX)
@Throttle(REST_API_LIMIT, REST_API_TTL)
export class RestController {
  constructor(@Inject(REST_SERVICE) private readonly restService: IRestService) {}

  @Post(':endpoint(*)')
  @ApprovedRoles(Roles.Rest)
  @HttpCode(HttpStatus.ACCEPTED)
  @ApiResponse({
    type: RestPostResponse,
    description: REST_POST_RESPONSE_DESCRIPTION,
    status: HttpStatus.ACCEPTED,
  })
  @UseFilters(UserNotFoundFilter)
  async post(
    @Param() params: RestPostParams,
    @RawBody(RawBodyRequirement.REQUIRED) rawBody: string,
    @Req() request,
  ): Promise<RestPostResponse> {
    return await this.restService.post(`/${params.endpoint}`, rawBody, request.user);
  }

  @Get(':endpoint(*)')
  @AggregatedEndpoints()
  @ApprovedRoles(Roles.Rest)
  @ApiOkResponse({
    type: RestGetResponse,
    description: REST_GET_RESPONSE_DESCRIPTION,
  })
  @UseFilters(UserNotFoundFilter)
  async get(@Param() params: RestGetParams, @Req() request): Promise<RestGetResponse> {
    return await this.restService.get(`/${params.endpoint}`, request.user);
  }

  @Put(':endpoint(*)')
  @ApiResponse({
    type: RestPutResponse,
    description: REST_PUT_RESPONSE_DESCRIPTION,
    status: HttpStatus.ACCEPTED,
  })
  @ApprovedRoles(Roles.Rest)
  @HttpCode(HttpStatus.ACCEPTED)
  @UseFilters(UserNotFoundFilter)
  async put(
    @Param() params: RestPutParams,
    @RawBody(RawBodyRequirement.REQUIRED) rawBody: string,
    @Req() request,
  ): Promise<any> {
    return await this.restService.set(`/${params.endpoint}`, rawBody, request.user);
  }

  @Delete(':endpoint(*)')
  @ApprovedRoles(Roles.Rest)
  @HttpCode(HttpStatus.ACCEPTED)
  @ApiResponse({
    type: RestDeleteResponse,
    description: REST_DELETE_RESPONSE_DESCRIPTION,
    status: HttpStatus.ACCEPTED,
  })
  @UseFilters(UserNotFoundFilter)
  async delete(
    @Param() params: RestDeleteParams,
    @RawBody(RawBodyRequirement.OPTIONAL) rawBody: string,
    @Req() request,
  ): Promise<RestDeleteResponse> {
    return await this.restService.delete(`/${params.endpoint}`, rawBody ?? {}, request.user);
  }
}
