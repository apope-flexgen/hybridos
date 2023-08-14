import {
  Body,
  Controller,
  Delete,
  Get,
  HttpException,
  Inject,
  Param,
  Post,
  Put,
  Query,
  UseFilters,
  UseGuards,
} from '@nestjs/common';
import {
  ApiCreatedResponse,
  ApiDefaultResponse,
  ApiOkResponse,
  ApiSecurity,
  ApiTags,
} from '@nestjs/swagger';
import { RolesGuard } from 'src/auth/guards/roles.guard';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { Roles } from '../../../shared/types/api/Users/Users.types';
import { ApprovedRoles } from '../decorators/roles.decorator';
import {
  AUDIT_LOGGING_SERVICE,
  IAuditLoggingService,
} from '../logging/auditLogging/interfaces/auditLogging.service.interface';
import { AllUsers } from './dtos/allusers.dto';
import { User } from './dtos/user.dto';
import { NegativeOldPasswordCapacityFilter } from './filters/negativeOldPasswordCapacity.filter';
import { OldPasswordMatchFilter } from './filters/oldPasswordMatch.filter';
import { UserNotFoundFilter } from './filters/userNotFound.filter';
import { IUsersService, USERS_SERVICE } from './interfaces/users.service.interface';
import { DeleteUserParams } from './params/deleteuser.params';
import { ReadUserParams } from './params/readuser.params';
import { UpdateUserParams } from './params/updateuser.params';
import { AllUsersResponse } from './responses/allusers.response';
import { DeleteUserResponse } from './responses/deleteuser.response';
import { UserResponse } from './responses/user.response';

@ApiTags('users')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('users')
@UseGuards(RolesGuard)
@UseFilters(UserNotFoundFilter)
export class UsersController {
  constructor(
    @Inject(USERS_SERVICE)
    private usersService: IUsersService,
    @Inject(AUDIT_LOGGING_SERVICE)
    private readonly auditLoggingService: IAuditLoggingService,
  ) {}

  @ApiOkResponse({ type: AllUsersResponse })
  @Get()
  async all(
    @Query() query: AllUsers,
    @UserFromAccessToken() user: User,
  ): Promise<AllUsersResponse> {
    return await this.usersService.all(query.role, user?.role);
  }

  @ApiOkResponse({ type: UserResponse })
  @Put(':id')
  @ApprovedRoles(Roles.Admin, Roles.Developer)
  @UseFilters(OldPasswordMatchFilter, NegativeOldPasswordCapacityFilter)
  async update(
    @Param() params: UpdateUserParams,
    @Body() user: User,
    @UserFromAccessToken() loggedInUser: User,
  ): Promise<UserResponse> {
    const response = await this.usersService.update(params.id, user);
    const data = {
      modified_field: 'user updated',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, loggedInUser);
    return response;
  }

  @ApiOkResponse({ type: DeleteUserResponse })
  @Delete(':id')
  @ApprovedRoles(Roles.Admin, Roles.Developer)
  async delete(
    @Param() params: DeleteUserParams,
    @UserFromAccessToken() user: User,
  ): Promise<DeleteUserResponse> {
    const response = await this.usersService.delete(params.id);
    const data = {
      modified_field: 'user deleted',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);
    return response;
  }

  @ApiCreatedResponse({ type: UserResponse })
  @Post()
  @ApprovedRoles(Roles.Admin, Roles.Developer)
  async create(
    @Body() user: User,
    @UserFromAccessToken() loggedInUser: User,
  ): Promise<UserResponse> {
    const response = await this.usersService.create(user);
    const data = {
      modified_field: 'user created',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, loggedInUser);
    return response;
  }

  @ApiOkResponse({ type: UserResponse })
  @Get(':id')
  async read(
    @Param() params: ReadUserParams,
    @UserFromAccessToken() user: User,
  ): Promise<UserResponse> {
    const response = await this.usersService.readById(params.id);
    const data = {
      modified_field: 'user read',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);
    return response;
  }
}
