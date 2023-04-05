import { Configuration, ModeDTO, SchedulerEvent, Timezone } from "../../../../shared/types/dtos/scheduler.dto";

export type MergedSchedulerData = {
    '/scheduler/modes'?: ModeDTO[],
    '/scheduler/events'?: SchedulerEvent[],
    '/scheduler/configuration'?: Configuration,
    '/scheduler/connected'?: {[siteId: string]: boolean},
    '/scheduler/timezones'?: Timezone,
}
