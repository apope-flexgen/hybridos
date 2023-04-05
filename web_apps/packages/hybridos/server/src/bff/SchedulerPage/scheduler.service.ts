import { Inject, Injectable } from '@nestjs/common'
import { EventFilters } from './dto/eventfilters.dto'
import { v4 as uuid } from 'uuid'
import { Request } from 'express';
import { SchedulerConfiguration } from './dto/config.dto'
import {
    SchedulerEvent,
    EventsObject
} from '../../../../shared/types/dtos/scheduler.dto'
import { map, merge, Observable } from 'rxjs'
import { MergedSchedulerData } from './scheduler.interface'
import { FimsMsg, FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface'
import { Modes } from './dto/mode.dto'
import { IValidJWTService, VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'

@Injectable()
export class SchedulerService {
    constructor(
        @Inject(FIMS_SERVICE)
        private readonly fimsService: IFimsService,
        @Inject(VALID_JWT_SERVICE)
        private readonly validJwtService: IValidJWTService,
    ) {}
    async getAllEvents(): Promise<EventsObject> {
        const events =  await this.fimsService.get('/scheduler/events')
        return events.body 
    }
    async getSchedulerModes(): Promise<string | Record<string, unknown>>  {
        const modes =  await this.fimsService.get('/scheduler/modes')
        return modes.body
    }
    async getSchedulerConfiguration(): Promise<string | Record<string, unknown>> {
        const configuration =  await this.fimsService.get('/scheduler/configuration')
        return configuration.body
    }
    async getSchedulerTimezones(): Promise<string | Record<string, unknown>> {
        const timezones =  await this.fimsService.get('/scheduler/timezones')
        return timezones.body
    }
    async getConnected(): Promise<string | Record<string, unknown>> {
        const connected =  await this.fimsService.get('/scheduler/connected')
        return connected.body
    }
    async deleteSchedulerEvent(request: Request, siteId: {siteId: string}, eventId: {eventId: string}): Promise<FimsMsg> {
        const site = siteId.siteId
        const event = eventId.eventId
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);

        const schedulerDeleteEventResponse = await this.fimsService.send({
            method: 'del',
            uri: `/scheduler/events/${site}/${event}`,
            replyto: '/web_server/scheduler/delete_event',
            body: '',
            username: user.sub
        });

        return schedulerDeleteEventResponse
    }
    async addSchedulerEvent(request: Request, siteId: {siteId: string}, event: SchedulerEvent): Promise<FimsMsg> {
        const site = siteId.siteId
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);

        const schedulerAddEventResponse = await this.fimsService.send({
            method: 'post',
            uri: `/scheduler/events/${site}`,
            replyto: '/web_server/scheduler/add_event',
            body: JSON.stringify(event),
            username: user.sub
        });

        return schedulerAddEventResponse
    }
    async updateEvent(request: Request, siteId: {siteId: string}, eventId: {eventId: string}, eventBody: any): Promise<FimsMsg> {
        const site = siteId.siteId
        const event = eventId.eventId
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);

        const schedulerUpdateEventResponse = await this.fimsService.send({
            method: 'set',
            uri: `/scheduler/events/${site}/${event}`,
            replyto: '/web_server/scheduler/update_event',
            body: JSON.stringify(eventBody),
            username: user.sub
        });
        
        return schedulerUpdateEventResponse
    }
    async addException(request: Request, siteId: {siteId: string}, eventId: {eventId: string}, exception: {data: string}): Promise<any> {
        const site = siteId.siteId
        const event = eventId.eventId
        const exceptionString = exception.data
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);

        const schedulerAddExceptionResponse = await this.fimsService.send({
            method: 'post',
            uri: `/scheduler/events/${site}/${event}/repeat/exceptions`,
            replyto: '/web_server/scheduler/add_exception',
            body: JSON.stringify(exceptionString),
            username: user.sub
        });

        return schedulerAddExceptionResponse.body
    }
    async addSchedulerConfiguration(request: Request, configDto: SchedulerConfiguration): Promise<{ data: string }> {
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);
        let id = uuid()
        const type = configDto.scheduler_type
        if (configDto.local_schedule?.name || configDto.local_schedule?.name.length === 0) {
            id = configDto.local_schedule.name
            .toLowerCase()
            .replaceAll('/', '')
            .replaceAll(' ', '')
            configDto.local_schedule.id = id
        }
        if(type === 'FM') {
            const path = configDto.web_sockets.clients
            if(path?.length) {
                path.forEach((obj: any) => {
                    obj.id = obj.name.toLowerCase().replaceAll('/', '').replaceAll(' ', '')
                })
            }
            
        }

        // TODO: scheduler/configuration should return something, but it gives a 408 request timeout.
        // this solution is a bandaide.
        this.fimsService.send({
            method: 'set',
            uri: '/scheduler/configuration',
            replyto: '/web_server/scheduler/add_config',
            body: JSON.stringify(configDto),
            username: user.sub
        });

        this.fimsService.send({
            method: 'pub',
            uri: '/scheduler/configuration',
            replyto: '/web_server/scheduler/add_config_pub',
            body: JSON.stringify(configDto),
            username: user.sub
        });

        return { data: `SET request sent to /scheduler/configuration`}
    }
    async updateModes(request: Request, modesDto: Modes): Promise<FimsMsg> {
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);
        
        const modesResponse = await this.fimsService.send({
            method: 'set',
            uri: '/scheduler/modes',
            replyto: '/web_server/scheduler/update_mode',
            body: modesDto,
            username: user.sub
        });
    
        return modesResponse
    }
    private getUriSpecificObservable = (
        uri: string,
    ): Observable<MergedSchedulerData> => {
        const fimsSubscribe = this.fimsService.subscribe(uri)


        const newObservable: Observable<MergedSchedulerData> = fimsSubscribe.pipe(
            map((event) => {
                return { [uri]: event.body } as MergedSchedulerData
            })
        )

        return newObservable
    }
    getMergedStream = (
        URIs: string[],
    ): Observable<MergedSchedulerData> => {
        const uriSpecificObservables = URIs.map((uri) => {
            return this.getUriSpecificObservable(uri)
        })
        const merged = merge(...uriSpecificObservables)
        return merged
    }
}
