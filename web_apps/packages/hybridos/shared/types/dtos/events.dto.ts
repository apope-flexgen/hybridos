// FIXME
// import { SeverityType } from '@flexgen/storybook/dist/types/commonTypes'

export class EventsRequestParams {
    startTime?: string
    endTime?: string
    source?: string
    severity?: any
    search?: string
    limit?: number
    page?: number
}

export class Event {
    id: string
    severity: any
    source: string
    message: string
    timestamp: string
}

// Event data returned from standalone_storage query via fims.
export class FimsEventBody {
    _id: string
    severity: any
    source: string
    message: string
    timestamp: string
}

 export class EventListResponse {
    count: number
    data: Event[]
 }
