import { INestApplication } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'
import { EventsService } from '../../src/bff/Events/events.service'
import { EventsController } from '../../src/bff/Events/events.controller'

import request from '../testReqAgent'

import { createTestApiApplication } from '../testUtils'
import { EventListResponse, EventsRequestParams } from '../../../shared/types/dtos/events.dto'

export const buildURI = (filters: any): string => {
    const filtersArray = [];
    if (filters.startTime != null) {
        filtersArray.push(['startTime', filters.startTime]);
    }
    if (filters.endTime != null) {
        filtersArray.push(['endTime', filters.endTime]);
    }
    if (filters.severity != null) {
        filters.severity.forEach((singleSeverity: string) => filtersArray.push(['severity', singleSeverity]));
    }
    if (filters.source != null) {
        filtersArray.push(['source', filters.source]);
    }
    if (filters.search?.length) {
        filtersArray.push(['search', filters.search]);
    }
    if (filters.limit != null) {
        filtersArray.push(['limit', filters.limit.toString()]);
    }
    if (filters.page != null) {
        const page = (filters.page === 0) ? 1 : (filters.page + 1);
        filtersArray.push(['page', page.toString()]);
    }

    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const URI = `/events?${filtersWithAmpersand}`;

    return URI;
};

describe('Events (e2e)', () => {
    let app: INestApplication
    let eventsService: EventsService
    const tenMinutesAgoMs = (1000 * 60 * 10)

    const successResponse = {
        count: 1,
        data: [
            {
                id: '123',
                message: 'some event',
                severity: 1,
            }
        ]
    } as EventListResponse

    beforeEach(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            controllers: [EventsController],
            providers: [
                {
                    provide: EventsService,
                    useValue: {
                        eventsFiltered: jest.fn(),
                    },
                },
            ],
        }).compile()

        app = createTestApiApplication(moduleFixture)
        eventsService = app.get(EventsService)
        await app.init()
    })

    it('/GET events - success', () => {
        const uri = buildURI({
            startTime: new Date(Date.now() - (tenMinutesAgoMs)).toISOString(),
            endTime: new Date(Date.now()).toISOString(),
            severity: ['Fault', 'Alarm'],
            source: undefined,
            search: '',
            limit: 10,
            page: 0,
        })
        jest.spyOn(eventsService, 'eventsFiltered').mockImplementation(() =>
            Promise.resolve(successResponse)
        )

        return request(app.getHttpServer())
            .get(uri)
            .expect(200)
            .expect(successResponse)
    })

    it('/GET events - 400 Bad Reqeust - page', () => {
        const uri = buildURI({
            startTime: new Date(Date.now() - (tenMinutesAgoMs)).toISOString(),
            endTime: new Date(Date.now()).toISOString(),
            severity: ['Fault', 'Alarm'],
            source: undefined,
            search: '',
            limit: 10,
            page: 'page',
        })

        return request(app.getHttpServer())
            .get(uri)
            .expect(400)
            .expect({
                statusCode: 400,
                message: [ 'page must be a number string' ],
                error: 'Bad Request'
            })
    })

    it('/GET events - 400 Bad Reqeust - limit', () => {
        const uri = buildURI({
            startTime: new Date(Date.now() - (tenMinutesAgoMs)).toISOString(),
            endTime: new Date(Date.now()).toISOString(),
            severity: ['Fault', 'Alarm'],
            source: undefined,
            search: '',
            limit: 'limit',
            page: 1,
        })

        return request(app.getHttpServer())
            .get(uri)
            .expect(400)
            .expect({
                statusCode: 400,
                message: [ 'limit must be a number string' ],
                error: 'Bad Request'
            })
    })

    afterAll(async () => {
        await app.close()
    })
})
