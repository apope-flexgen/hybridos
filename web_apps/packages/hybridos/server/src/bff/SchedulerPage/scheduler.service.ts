import { Inject, Injectable } from '@nestjs/common';
import { v4 as uuid } from 'uuid';
import { Request } from 'express';
import { SchedulerConfiguration } from './dto/config.dto';
import { EventsObject, schedulerEventForAPI } from '../../../../shared/types/dtos/scheduler.dto';
import { map, merge, Observable } from 'rxjs';
import { MergedSchedulerData } from './scheduler.interface';
import { FimsMsg, FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface';
import { ModeBody } from './dto/mode.dto';

@Injectable()
export class SchedulerService {
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: IFimsService,
  ) {}
  async getAllEvents(): Promise<EventsObject> {
    const events = await this.fimsService.get('/scheduler/events');
    return events.body;
  }
  async getSchedulerModes(): Promise<string | Record<string, unknown>> {
    const modes = await this.fimsService.get('/scheduler/modes');
    return modes.body;
  }
  async getSchedulerConfiguration(): Promise<string | Record<string, unknown>> {
    const configuration = await this.fimsService.get('/scheduler/configuration');
    return configuration.body;
  }
  async getSchedulerTimezones(): Promise<string | Record<string, unknown>> {
    const timezones = await this.fimsService.get('/scheduler/timezones');
    return timezones.body;
  }
  async getConnected(): Promise<string | Record<string, unknown>> {
    const connected = await this.fimsService.get('/scheduler/connected');
    return connected.body;
  }
  async deleteSchedulerEvent(
    request: Request,
    siteId: { siteId: string },
    eventId: { eventId: string },
    username: string,
  ): Promise<FimsMsg> {
    const site = siteId.siteId;
    const event = eventId.eventId;

    const schedulerDeleteEventResponse = await this.fimsService.send({
      method: 'del',
      uri: `/scheduler/events/${site}/${event}`,
      replyto: '/web_server/scheduler/delete_event',
      body: '',
      username: username,
    });

    return schedulerDeleteEventResponse;
  }
  async addSchedulerEvent(
    request: Request,
    siteId: { siteId: string },
    event: schedulerEventForAPI,
    username: string,
  ): Promise<FimsMsg> {
    const site = siteId.siteId;

    const schedulerAddEventResponse = await this.fimsService.send({
      method: 'post',
      uri: `/scheduler/events/${site}`,
      replyto: '/web_server/scheduler/add_event',
      body: JSON.stringify(event),
      username: username,
    });

    return schedulerAddEventResponse;
  }
  async updateEvent(
    request: Request,
    siteId: { siteId: string },
    eventId: { eventId: string },
    eventBody: schedulerEventForAPI,
    username: string,
  ): Promise<FimsMsg> {
    const site = siteId.siteId;
    const event = eventId.eventId;

    const schedulerUpdateEventResponse = await this.fimsService.send({
      method: 'set',
      uri: `/scheduler/events/${site}/${event}`,
      replyto: '/web_server/scheduler/update_event',
      body: JSON.stringify(eventBody),
      username: username,
    });

    return schedulerUpdateEventResponse;
  }
  async addException(
    request: Request,
    siteId: { siteId: string },
    eventId: { eventId: string },
    exception: { data: string },
    username: string,
  ): Promise<FimsMsg> {
    const site = siteId.siteId;
    const event = eventId.eventId;
    const exceptionString = exception.data;

    const schedulerAddExceptionResponse = await this.fimsService.send({
      method: 'post',
      uri: `/scheduler/events/${site}/${event}/repeat/exceptions`,
      replyto: '/web_server/scheduler/add_exception',
      body: JSON.stringify(exceptionString),
      username: username,
    });

    return schedulerAddExceptionResponse;
  }
  async addSchedulerConfiguration(
    request: Request,
    configDto: SchedulerConfiguration,
    username: string,
  ): Promise<{ data: string }> {
    let id = uuid();
    const type = configDto.scheduler_type;
    if (configDto.local_schedule?.name || configDto.local_schedule?.name.length === 0) {
      id = configDto.local_schedule.name.toLowerCase().replaceAll('/', '').replaceAll(' ', '');
      configDto.local_schedule.id = id;
    }
    if (type === 'FM') {
      const path = configDto.web_sockets.clients;
      if (path?.length) {
        path.forEach((obj: { id: string; name: string; ip: string; port: string | number }) => {
          obj.id = obj.name.toLowerCase().replaceAll('/', '').replaceAll(' ', '');
        });
      }
    }

    // TODO: scheduler/configuration should return something, but it gives a 408 request timeout.
    // this solution is a bandaid.
    this.fimsService.send({
      method: 'set',
      uri: '/scheduler/configuration',
      replyto: '/web_server/scheduler/add_config',
      body: JSON.stringify(configDto),
      username: username,
    });

    return { data: `SET request sent to /scheduler/configuration` };
  }
  async updateMode(
    request: Request,
    modeDto: ModeBody,
    modeId: { modeId: string },
    username: string,
  ): Promise<FimsMsg> {
    const id = modeId.modeId;

    const modesResponse = await this.fimsService.send({
      method: 'set',
      uri: `/scheduler/modes/${id}`,
      replyto: '/web_server/scheduler/update_mode',
      body: modeDto,
      username: username,
    });

    return modesResponse;
  }
  async addNewMode(
    request: Request,
    modesDto: { [id: string]: ModeBody },
    username: string,
  ): Promise<FimsMsg> {
    const modesResponse = await this.fimsService.send({
      method: 'set',
      uri: `/scheduler/modes`,
      replyto: '/web_server/scheduler/update_mode',
      body: modesDto,
      username: username,
    });

    return modesResponse;
  }
  async deleteMode(
    request: Request,
    modeId: { modeId: string },
    username: string,
  ): Promise<FimsMsg> {
    const mode = modeId.modeId;

    const deleteModeResponse = await this.fimsService.send({
      method: 'del',
      uri: `/scheduler/modes/${mode}`,
      replyto: '/web_server/scheduler/delete_mode',
      body: '',
      username: username,
    });

    return deleteModeResponse;
  }
  private getUriSpecificObservable = (uri: string): Observable<MergedSchedulerData> => {
    const fimsSubscribe = this.fimsService.subscribe(uri);

    const newObservable: Observable<MergedSchedulerData> = fimsSubscribe.pipe(
      map((event) => {
        return { [uri]: event.body } as MergedSchedulerData;
      }),
    );

    return newObservable;
  };
  getMergedStream = (URIs: string[]): Observable<MergedSchedulerData> => {
    const uriSpecificObservables = URIs.map((uri) => {
      return this.getUriSpecificObservable(uri);
    });
    const merged = merge(...uriSpecificObservables);
    return merged;
  };
}
