import { Inject, Injectable } from '@nestjs/common'
import dayjs from 'dayjs'
import { FimsService } from '../../fims/fims.service'
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface'
import { getformattedTimestamp } from '../../utils/timestamp'
import { EventsRequest } from './dtos/events.dto'
import { Event, EventListResponse, FimsEventBody } from '../../../../shared/types/dtos/events.dto'
import { SeverityType } from '@flexgen/storybook/dist/types/commonTypes'

const eventsDataTemplate = [
    {
        severity: 'Alarm',
        source: 'Assets',
        message: 'Process Scheduler is hung or dead',
    },
    {
        severity: 'Fault',
        source: 'Modbus Client',
        message: 'Site_Manager primary_controller status changed to: true or dead',
    },
    {
        severity: 'Info',
        source: 'Modbus Server',
        message: 'Site_Manager primary_controller status changed to: true or dead',
    },
    {
        severity: 'Status',
        source: 'Site',
        message: 'Process Scheduler is hung or dead',
    },
]

const severityLookup = ['', 'Info', 'Status', 'Alarm', 'Fault']

@Injectable()
export class EventsService {
    constructor(@Inject(FIMS_SERVICE) private readonly fimsService: FimsService) {}
    async eventsFiltered(query: EventsRequest): Promise<EventListResponse> {
        const startTime = query.startTime
        const startTimeTimestamp = getformattedTimestamp(startTime)
        const endTime = query.endTime
        const endTimeTimestamp = getformattedTimestamp(endTime)
        const source = query.source
        const severities = query.severity ? query.severity : []
        const search = query.search
        const limit = query.limit
        const page = query.page
        const orderBy = query.orderBy
        const order = query.order

        const data = await this.getEventsData(
            startTimeTimestamp,
            endTimeTimestamp,
            source,
            this.transformSeverities(severities),
            search,
            limit,
            page,
            order,
            orderBy
        )

        const count = await this.getEventsCount(
            startTimeTimestamp,
            endTimeTimestamp,
            source,
            this.transformSeverities(severities),
            search,
        )

        return {
            data: data,
            count: count
        }
    }
    getMockEventsData() {
        const eventsData: Event[] = []

        for (let i = 0; i < 40000; i += 1) {
            const now = dayjs()
            const newTimeStamp = now.subtract(i * 10, 'minute').format('YYYY/MM/DD HH:mm:ss')
            const randomIndexForSeverity = Math.floor(Math.random() * 4)
            const randomIndexForSource = Math.floor(Math.random() * 4)
            const randomIndexForMessage = Math.floor(Math.random() * 4)
            const newDataObject = {
                id: i.toString(),
                severity: eventsDataTemplate[randomIndexForSeverity].severity as SeverityType,
                source: eventsDataTemplate[randomIndexForSource].source,
                message: eventsDataTemplate[randomIndexForMessage].message,
                timestamp: newTimeStamp,
            }
            eventsData.push(newDataObject)
        }
        return eventsData
    }
    async getEventsData(startTime, endTime, source, severities, search, limit, page, order, orderBy) {
        const sort = {
            [orderBy]: parseInt(order)
        }
        if (orderBy !== 'timestamp') {
            sort['timestamp'] = -1
        }
        let filter: any = {
            severity: severities,
            limit: parseInt(limit),
            after: startTime !== null ? getformattedTimestamp(startTime) : null,
            before: endTime !== null ? getformattedTimestamp(endTime) : null,
            source,
            message: search,
            page: page,
            sort: sort,
        }

        const fimsData = await this.fimsService.get('/events', filter)

        const eventsFromFims = fimsData.body as unknown as FimsEventBody[]

        const eventsData: Event[] = eventsFromFims.map((data) => this.transformData(data))
        return eventsData
    }
    async getEventsCount(startTime, endTime, source, severities, search) {
        let filter: any = {
            severity: severities,
            after: startTime !== null ? getformattedTimestamp(startTime) : null,
            before: endTime !== null ? getformattedTimestamp(endTime) : null,
            source,
            message: search,
            count: true,
        }

        const eventsCount = await this.fimsService.get('/events', filter)

        const eventsCountFromFims = eventsCount.body as unknown as number
        return eventsCountFromFims
    }
    transformSeverities(severities: any) {
        if ((typeof severities) === 'string') {
            return [severityLookup.indexOf(severities)]
        }
        return severities.map((severity) => severityLookup.indexOf(severity))
    }
    transformData(data: FimsEventBody): Event {
        const transformed = {
            id: data._id,
            severity: severityLookup[data.severity],
            source: data.source,
            message: data.message,
            timestamp: dayjs(data.timestamp).format('YYYY-MM-DD HH:mm:ss'),
        }
        return transformed
    }
}
