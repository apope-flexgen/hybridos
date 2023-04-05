import { LifecycleStatusEnum } from '../models/lifecycle.model'

export interface ConfigFileMetadata {
    id?: number
    name?: string
    version?: number
    dateRetired?: string
    dateCreated?: string
    dateActive?: string
    lifeCycleStatus?: LifecycleStatusEnum
    creatorName?: string
    moduleName?: string
    archiveVersions?: ConfigFileMetadata[]
}
