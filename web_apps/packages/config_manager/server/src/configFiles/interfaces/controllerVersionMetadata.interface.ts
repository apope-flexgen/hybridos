import { LifecycleStatusEnum } from '../models/lifecycle.model'

export interface ControllerVersionMetadata {
    id?: number
    version?: number
    dateRetired?: string
    dateCreated?: string
    dateActive?: string
    lifeCycleStatus?: LifecycleStatusEnum
    creatorName?: string
    archiveVersions?: ControllerVersionMetadata[]
}
