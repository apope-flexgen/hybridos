import { Controller, Param, Body, Get, Post, Delete, Req } from '@nestjs/common';
import {
  ApiCreatedResponse,
  ApiDefaultResponse,
  ApiOkResponse,
  ApiSecurity,
  ApiTags,
} from '@nestjs/swagger';
import { SchedulerService } from './scheduler.service';
import { Event } from './dto/event.dto';
import { ModeBody } from './dto/mode.dto';
import { FimsMsg } from 'src/fims/interfaces/fims.interface';
import { Request } from 'express';
import { SchedulerConfiguration } from './dto/config.dto';
import { EventId, SiteId } from './params/events.params';
import { ModeId } from './params/mode.params';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { SchedulerDescriptions } from './scheduler.constants';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';

@ApiTags('scheduler')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('scheduler')
export class SchedulerController {
  constructor(private readonly schedulerService: SchedulerService) {}
  @ApiOkResponse({ description: SchedulerDescriptions.eventsResponse })
  @Get('events')
  async allEvents(): Promise<{ [siteId: string]: Event[] }> {
    const getAllEventsResponse = await this.schedulerService.getAllEvents();
    return getAllEventsResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.eventsResponse })
  @Delete('events/:siteId/:eventId')
  async delete(
    @Req() request: Request,
    @Param() siteId: SiteId,
    @Param() eventId: EventId,
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const deleteEventResponse = await this.schedulerService.deleteSchedulerEvent(
      request,
      siteId,
      eventId,
      user.username,
    );
    return deleteEventResponse;
  }
  @ApiCreatedResponse({ description: SchedulerDescriptions.eventsResponse })
  @Post('events/:siteId')
  async create(
    @Req() request: Request,
    @Param() siteId: SiteId,
    @Body() event: Event,
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const addEventResponse = await this.schedulerService.addSchedulerEvent(
      request,
      siteId,
      event,
      user.username,
    );
    return addEventResponse;
  }
  @ApiCreatedResponse({ description: SchedulerDescriptions.eventsResponse })
  @Post('events/:siteId/:eventId')
  async updateEvent(
    @Req() request: Request,
    @Param() siteId: SiteId,
    @Param() eventId: EventId,
    @Body() updatedEvent: Event,
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const updateEventResponse = await this.schedulerService.updateEvent(
      request,
      siteId,
      eventId,
      updatedEvent,
      user.username,
    );
    return updateEventResponse;
  }
  @ApiCreatedResponse({ description: SchedulerDescriptions.eventsResponse })
  @Post('events/:siteId/:eventId/exceptions')
  async addException(
    @Req() request: Request,
    @Param() siteId: SiteId,
    @Param() eventId: EventId,
    @Body() exception: { data: string },
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const addExceptionResponse = await this.schedulerService.addException(
      request,
      siteId,
      eventId,
      exception,
      user.username,
    );
    return addExceptionResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.modesResponse })
  @Get('modes')
  async modes(): Promise<string | Record<string, unknown>> {
    const getModesResponse = await this.schedulerService.getSchedulerModes();
    return getModesResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.modesResponse })
  @Delete('modes/:modeId')
  async deleteMode(
    @Req() request: Request,
    @Param() modeId: ModeId,
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const deleteModeResponse = await this.schedulerService.deleteMode(
      request,
      modeId,
      user.username,
    );
    return deleteModeResponse;
  }
  @ApiCreatedResponse({ description: SchedulerDescriptions.modesResponse })
  @Post('modes')
  async createModes(
    @Req() request: Request,
    @Body() modeDto: { [id: string]: ModeBody },
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    const addModeResponse = await this.schedulerService.addNewMode(request, modeDto, user.username);
    return addModeResponse;
  }
  @ApiCreatedResponse({ description: SchedulerDescriptions.modesResponse })
  @Post('modes/:modeId')
  async updateMode(
    @Req() request: Request,
    @Body() modeDto: ModeBody,
    @Param() modeId: ModeId,
    @UserFromAccessToken() user: User,
  ): Promise<FimsMsg> {
    console.log(modeDto);
    const addModeResponse = await this.schedulerService.updateMode(
      request,
      modeDto,
      modeId,
      user.username,
    );
    return addModeResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.configurationResponse })
  @Get('configuration')
  async configuration(): Promise<string | Record<string, unknown>> {
    const getConfigurationResponse = await this.schedulerService.getSchedulerConfiguration();
    return getConfigurationResponse;
  }
  @ApiCreatedResponse({
    description: SchedulerDescriptions.configurationResponse,
  })
  @Post('configuration')
  async createConfig(
    @Req() request: Request,
    @Body() configDto: SchedulerConfiguration,
    @UserFromAccessToken() user: User,
  ): Promise<{ data: string }> {
    const addConfigResponse = await this.schedulerService.addSchedulerConfiguration(
      request,
      configDto,
      user.username,
    );
    return addConfigResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.timezonesResponse })
  @Get('timezones')
  async timezones(): Promise<string | Record<string, unknown>> {
    const getTimezonesResponse = await this.schedulerService.getSchedulerTimezones();
    return getTimezonesResponse;
  }
  @ApiOkResponse({ description: SchedulerDescriptions.connectedResponse })
  @Get('connected')
  async connected(): Promise<string | Record<string, unknown>> {
    const getTimezonesResponse = await this.schedulerService.getConnected();
    return getTimezonesResponse;
  }
}
