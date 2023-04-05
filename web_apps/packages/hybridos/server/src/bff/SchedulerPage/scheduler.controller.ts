import { Controller, Param, Body, Get, Query, Post, Delete, Req } from '@nestjs/common'
import { ApiTags } from '@nestjs/swagger'
import { SchedulerService } from './scheduler.service'
import { EventFilters } from './dto/eventfilters.dto'
import { DeleteEvent } from './dto/deleteevent.dto'
import { Event } from './dto/event.dto'
import { SchedulerConfiguration } from './dto/config.dto'
import { EventParams } from './params/events.params'
import { Modes } from './dto/mode.dto'
import { ApiMode } from '../../../../shared/types/dtos/scheduler.dto'
import { DeleteMode } from './dto/deletemode.dto'
import { FimsMsg } from 'src/fims/interfaces/fims.interface'
import { Request } from 'express';

@ApiTags('scheduler')
@Controller('scheduler')
export class SchedulerController {
    constructor(private readonly schedulerService: SchedulerService) {}
    @Get('events')
    async allEvents(): Promise<any> {
        const getAllEventsResponse = await this.schedulerService.getAllEvents()
        return getAllEventsResponse
    }
    @Delete('events/:siteId/:eventId')
    async delete(@Req() request: Request, @Param() siteId: {siteId: string}, @Param() eventId: {eventId: string}): Promise<FimsMsg> {
        const deleteEventResponse = await this.schedulerService.deleteSchedulerEvent(request, siteId, eventId)
        return deleteEventResponse
    }
    @Post('events/:siteId')
    async create(@Req() request: Request, @Param() siteId: any, @Body() event: any): Promise<FimsMsg> {
        const addEventResponse = await this.schedulerService.addSchedulerEvent(request, siteId, event)
        return addEventResponse
    }
    @Post('events/:siteId/:eventId')
    async updateEvent(@Req() request: Request, @Param() siteId: any, @Param() eventId: any, @Body() updatedEvent: any): Promise<FimsMsg> {
        const updateEventResponse = await this.schedulerService.updateEvent(request, siteId, eventId, updatedEvent)
        return updateEventResponse
    }
    @Post('events/:siteId/:eventId/exceptions')
    async addException(@Req() request: Request, @Param() siteId: any, @Param() eventId: any, @Body() exception: {data: string}): Promise<any> {
        const addExceptionResponse = await this.schedulerService.addException(request, siteId, eventId, exception)
        return addExceptionResponse
    }
    @Get('modes')
    async modes(): Promise<any> {
        const getModesResponse = await this.schedulerService.getSchedulerModes()
        return getModesResponse
    }
    @Post('modes')
    async createModes(@Req() request: Request, @Body() modeDto: any): Promise<any> {
        const addModeResponse = await this.schedulerService.updateModes(request, modeDto)
        return addModeResponse
    }
    @Get('configuration')
    async configuration(): Promise<any> {
        const getConfigurationResponse = await this.schedulerService.getSchedulerConfiguration()
        return getConfigurationResponse
    }
    @Post('configuration')
    async createConfig(@Req() request: Request, @Body() configDto: any): Promise<{ data: string }> {
        const addConfigResponse = await this.schedulerService.addSchedulerConfiguration(request, configDto)
        return addConfigResponse
    }
    @Get('timezones')
    async timezones(): Promise<any> {
        const getTimezonesResponse = await this.schedulerService.getSchedulerTimezones()
        return getTimezonesResponse
    }
    @Get('connected')
    async connected(): Promise<any> {
        const getTimezonesResponse = await this.schedulerService.getConnected()
        return getTimezonesResponse
    }
}
